#define _POSIX_SOURCE
#include "builtins.h"
#include "exec.h"
#include "shell_structs.h"
#include <string.h>
#include "memmove.h"
#include <errno.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <limits.h>

/*for print job desc (debug) */
#include "argparse.h"
#include <stdio.h>

extern int setenv(const char *name, const char *value, int overwrite);
extern int setpgrp(void);
extern pid_t getpgid(pid_t);
extern int gethostname(char *name, size_t len);
extern ssize_t readlink(const char *path, char *buf, size_t bufsiz);
extern char *ctermid(char *);
extern int kill(pid_t pid, int sig);

struct variable{
    char * name;
    char * value;
};
/*
static void signal_handler(int sn) {
    if (sn == SIGTSTP) {
        fputs("SIGTSTP", stderr);
    }
    if (sn == SIGINT) {
        _exit(2);
    }
}
*/
bool debug;
bool is_interactive;
int background_jobs_n;
struct job ** background_jobs;
pid_t * background;
struct job * foreground;
size_t bsz, brsz, bjsz, bjrsz;
pid_t for_c_pid;

size_t rvars_sz, rvars_rsz, rvars_n;
struct variable * rvars;
static int tty_fd;

int status;

static int findpid(pid_t p) {
    int i;
    for (i = 0; i < background_jobs_n; i++) {
        if (background[i] == p)
            return i;
    }
    return -1;
}

static void delete_pid(pid_t p) {
    int i, j;
    /*size_t a, b;*/
    for (i = 0; background[i] != p && i < background_jobs_n; i++);
    if (i == background_jobs_n) return;
    for (j = i; j < background_jobs_n; j++)
        background[j] = background[j + 1];
    background_jobs_n--;
    bsz -= sizeof(pid_t);
    bjsz -= sizeof(struct job *);
    if (bsz < brsz / 2) {
        /*
        a = brsz / 2;
        b = bjrsz / 2;
        truncate_mem((void**)&background_jobs, &a, &bjrsz);
        truncate_mem((void**)&background, &b, &bjrsz);
        */
        if (errno == ENOMEM) {
            printf("failed to allocate memory\n");
            exit_shell();
        }
    }
}

static char cwdbuf[PATH_MAX + 1];

static bool builtin_hook(struct job * x) {
    int st, i, l;
    int dd;
    char * ss;
    if (x->commandsc == 1) {
        if (strcmp(x->commands[0]->name, "exit") == 0) {
            exit_shell();
        }
        if (strcmp(x->commands[0]->name, "cd") == 0) {
            if (x->commands[0]->argc < 2) return true;
            chdir(x->commands[0]->args[1]);
            getcwd(cwdbuf, sizeof(cwdbuf));
            setenv("PWD", cwdbuf, 1);
            return true;
        }
        if (strcmp(x->commands[0]->name, "export") == 0) {
            if (x->commands[0]->argc < 2) return true;
            l = strlen(x->commands[0]->args[1]);
            for (i = 0; i < l && x->commands[0]->args[1][i] != '='; i++);
            ss = x->commands[0]->args[1];
            if (i == l) return true;
            ss[i] = '\0';
            setenv(ss, ss + i + 1, 1);
            ss[i] = '=';
            return true;
        }
        if (strcmp(x->commands[0]->name, "bg") == 0) {
            if (x->commands[0]->argc < 2) {
                dd = -background[background_jobs_n - 1];
            } else {
                if (1 != sscanf(x->commands[0]->args[1], "%d", &dd))
                    return false;
            }
            if (dd == 1 || findpid(-dd) == -1) {
                printf("no such job\n");
                fflush(stdout);
                return true;
            }
            kill(dd, SIGCONT);
            waitpid(-dd, &st, WUNTRACED | WNOHANG);
            if (WIFSTOPPED(st)) {
                return true;
            } else {
                printf("command exited with status %d\n", WEXITSTATUS(st));
                delete_pid(-dd);
                status = WEXITSTATUS(st);
                return true;
            }
        }
        if (strcmp(x->commands[0]->name, "fg") == 0) {
            if (x->commands[0]->argc < 2) {
                dd = -background[background_jobs_n - 1];
            } else {
                if (1 != sscanf(x->commands[0]->args[1], "%d", &dd))
                    return false;
            }
            if (dd == 1 || findpid(-dd) == -1) {
                printf("no such job\n");
                fflush(stdout);
                return true;
            }
            tcsetpgrp(tty_fd, getpgid(-dd));
            kill(dd, SIGCONT);
            waitpid(-dd, &st, WUNTRACED);
            tcsetpgrp(tty_fd, getpgid(getpid()));
            if (WIFSTOPPED(st)) {
                return true;
            } else {
                printf("command exited with status %d\n", WEXITSTATUS(st));
                delete_pid(-dd);
                status = WEXITSTATUS(st);
                return true;
            }
        }
    }
    return false;
}

static bool builtin_find(char * s) {
    int i;
    for (i = 0; i < builtins_n; i++) {
        if (strcmp(s, builtin_names[i]) == 0) {
            return true;
        }
    }
    return false;
}

static void builtin_exec(struct command * cs) {
    int i;
    for (i = 0; i < builtins_n; i++) {
        if (strcmp(cs->name, builtin_names[i]) == 0) {
            exit((*functions[i])(cs));
        }
    }
    exit(10); 
}

static void clr_signals();
static void exec_com(struct command * cs) {
    clr_signals();
    if (builtin_find(cs->name)) builtin_exec(cs);
    execvp(cs->name, cs->args);
    printf("command not found\n");
    exit(10);
}
/*
static void controller_signal_handler(int sn) {
    if (sn == SIGTSTP) {
        tcsetpgrp(tty_fd, getpgid(getppid()));
        pause();
    }
    if (sn == SIGINT) {
        tcsetpgrp(tty_fd, getpgid(getppid()));
        _exit(2);
    }
}
*/

int kill(pid_t pid, int sig);
static bool usr1_lock;

static void catcher(int sn) {
    usr1_lock = true;
    return;
}

static pid_t execute_job(struct job * jb) {
    int i, pinp = 0, fd, st, res;
    pid_t ctl;
    int pp[2];
    if ((ctl = fork()) == 0) {
        if (debug) fprintf(stderr, "controller pid - %d\ncommand output------------------\n", getpid());
        signal(SIGTSTP, SIG_DFL);
        res = 0;
        for (i = 0; i < jb->commandsc; i++) {
            if (i != jb->commandsc - 1) {
                pipe(pp);
            }
            if ((jb->commands[i]->pid = fork()) == 0) {
                if (pinp != 0){  
                    dup2(pinp, 0); 
                    close(pinp);
                }
                if (i != jb->commandsc - 1) {
                    dup2(pp[1], 1);
                    close(pp[1]);
                    close(pp[0]);
                }
                if (jb->commands[i]->input != NULL) {
                    fd = open(jb->commands[i]->input, O_CREAT | O_RDONLY, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
                    dup2(fd, 0);
                    close(fd);
                }
                if (jb->commands[i]->output != NULL) {
                    if (jb->commands[i]->out_append)
                        fd = open(jb->commands[i]->output, O_CREAT | O_APPEND | O_WRONLY, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
                    else 
                        fd = open(jb->commands[i]->output, O_CREAT | O_WRONLY, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
                    dup2(fd, 1);
                    close(fd);
                }
                exec_com(jb->commands[i]);           
            }
            if (i != jb->commandsc - 1)
                close(pp[1]);
            close(pinp);
            pinp = pp[0];
        }
        foreground = jb;
        while (errno != ECHILD) {
            if (wait(&st) == jb->commands[jb->commandsc - 1]->pid)
                res = WEXITSTATUS(st);
        }
        if (debug) fprintf(stderr, "tgetpgrp == %d, pgrp == %d, ppgrp == %d\n", tcgetpgrp(tty_fd), getpgid(getpid()), getpgid(getppid()));
        _exit(res);
    }
    setpgid(ctl, ctl);
    if (is_interactive && !jb->background)
        tcsetpgrp(tty_fd, getpgid(ctl));
    kill(-ctl, SIGCONT);
    return ctl;
}

static void add_background_job(struct job * x, pid_t ctl) {
    increase((void**)&background_jobs, &bjsz, &bjrsz, sizeof(struct job *));
    increase((void**)&background, &bsz, &brsz, sizeof(struct job *));
    if (errno == ENOMEM)
        exit_shell();
    background[background_jobs_n] = ctl;
    background_jobs[background_jobs_n] = x;
    background_jobs_n++;
}
/*

*/
void zombie_clr() {
    int st, i, res;
    for (i = 0; i < background_jobs_n; i++) {
        res = waitpid(background[i], &st, WNOHANG);
        if (res != 0 && res != -1) {
            printf("job %d exited with %d\n", i, WEXITSTATUS(st));
            status = WEXITSTATUS(st);
            delete_pid(background[i--]);
        }
    }
    errno = 0;
}

void execute(struct job* x) {
    if (builtin_hook(x)) {
        free_job(x);
        return;
    }
    if (debug) {
        printf("my group - %d, term control - %d\n", getpgrp(), tcgetpgrp(tty_fd));
        print_job_desc(x);
        fflush(stdout);
    }
    if (x->background) {
        add_background_job(x, execute_job(x));
    } else {
        pid_t ctl = execute_job(x);
        int st;
        waitpid(ctl, &st, WUNTRACED);
        tcsetpgrp(tty_fd, getpgid(getpid()));
        if (WIFSTOPPED(st)) {
            add_background_job(x, ctl);
            printf("command went to background\n");
            fflush(stdout);
        } else {
            if (WEXITSTATUS(st) != 0)
                printf("command exited with status %d\n", WEXITSTATUS(st));
            fflush(stdout);
            status = WEXITSTATUS(st);
            free_job(x);
        }
    }
    if (!x->background)
        tcsetpgrp(tty_fd, getpgrp());
}

static void clr_signals() {
    signal(SIGTSTP, SIG_DFL);
    signal(SIGCHLD, SIG_DFL);
    signal(SIGINT, SIG_DFL);
    signal(SIGUSR1, SIG_DFL);
    signal(SIGUSR2, SIG_DFL);
    signal(SIGTTOU, SIG_DFL);
}

void init_shell(int argc, char ** argv) {
    int i;
    char * buffer;
    background_jobs_n = 0;

    debug = false;
    for (i = 0; i < argc; i++)
        if (strcmp("--debug", argv[i]) == 0)
            debug = true;
    if (debug) fprintf(stderr, "%d - PID\n", getpid());
    if (is_interactive) {
        tty_fd = open(ctermid(NULL), O_RDONLY, 0);
        tcsetpgrp(tty_fd, getpgrp());
    }
    bsz = brsz = bjsz = bjrsz = 0;
    signal(SIGTTOU, SIG_IGN);
    signal(SIGTSTP, SIG_IGN);
    signal(SIGINT, SIG_IGN);
    /*signal(SIGTSTP, signal_handler);
    signal(SIGINT, signal_handler);
    signal(SIGCHLD, signal_handler);*/
    background = NULL;
    foreground = NULL;
    background_jobs = NULL;
    rvars_n = rvars_sz = rvars_rsz = 0;
    rvars = NULL;
    status = 0;

    buffer = (char*)malloc(10 * sizeof(char));
    if (buffer == NULL) exit(3);
    sprintf(buffer, "%d", argc);
    setvar("#", buffer);

    for (i = 0; i < argc; i++) {
        buffer = (char*)malloc(10 * sizeof(char));
        if (buffer == NULL) exit(3);
        sprintf(buffer, "%d", i);
        setvar(buffer, strdup(argv[i]));
    }
    
    buffer = (char*)malloc(10 * sizeof(char));
    if (buffer == NULL) exit(3);
    sprintf(buffer, "%d", getuid());
    setvar("UID", buffer);

    buffer = (char*)malloc(10 * sizeof(char));
    if (buffer == NULL) exit(3);
    sprintf(buffer, "%d", getpid());
    setvar("PID", buffer);

    buffer = (char*)malloc(40 * sizeof(char));
    if (buffer == NULL) exit(3);
    gethostname(buffer, 39);
    errno = 0;
    setvar("HOSTNAME", buffer);

    buffer = (char*)malloc(200 * sizeof(char));
    if (buffer == NULL) exit(3);
    readlink("/proc/self/exe", buffer, sizeof(char) * 200);
    errno = 0;
    setvar("SHELL", buffer);
    
}

void setvar(char * name, char * val){
    size_t i;
    if (val == NULL) return;
    for (i = 0; i < rvars_n; i++) {
        if (strcmp(rvars[i].name, name) == 0) {
            free(rvars[i].value);
            rvars[i].value = val;
        }
    }
    increase((void**)&rvars, &rvars_sz, &rvars_rsz, sizeof(struct variable));
    if (errno != 0) return;
    rvars[rvars_n].name = name;
    rvars[rvars_n].value = val;
    rvars_n++;
}

char * findvar(char * name) {
    char * tmp, *buffer;
    size_t i;
    if (strcmp(name, "?") == 0) {
        buffer = (char*)malloc(10 * sizeof(char));
        if (buffer == NULL) exit(3);
        sprintf(buffer, "%d", status);
        return buffer;
    }
    for (i = 0; i < rvars_n; i++) {
        if (strcmp(rvars[i].name, name) == 0)
            return rvars[i].value;
    }
    tmp = getenv(name);
    if (tmp == NULL) {
       return "";
    }
    else return tmp;
}

void exit_shell() {
    /*free(background_jobs);*/
    exit(0);
}
