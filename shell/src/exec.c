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

/*for print job desc (debug) */
#include "argparse.h"
#include <stdio.h>


extern int setpgrp(void);
extern pid_t getpgid(pid_t);
extern int gethostname(char *name, size_t len);
extern ssize_t readlink(const char *path, char *buf, size_t bufsiz);
extern char *ctermid(char *);

struct variable{
    char * name;
    char * value;
};

static void signal_handler(int sn) {
    if (sn == SIGTSTP) {
        fputs("SIGTSTP", stderr);
    }
    if (sn == SIGINT) {
        /*_exit(2);*/
    }
}

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

static bool builtin_hook(struct job * x) {
    if (x->commandsc == 1) {
        if (strcmp(x->commands[0]->name, "exit") == 0) {
            exit_shell();
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
        if (strcmp(cs->args[i], builtin_names[i]) == 0) {
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
    _exit(10);
}

static void controller_signal_handler(int sn) {
    if (sn == SIGTSTP) {
        tcsetpgrp(tty_fd, getpgid(getppid()));
    }
    if (sn == SIGINT) {
        tcsetpgrp(tty_fd, getpgid(getppid()));
        _exit(2);
    }
}


int kill(pid_t pid, int sig);
static bool usr1_lock;

static void catcher(int sn) {
    usr1_lock = true;
}

static pid_t execute_job(struct job * jb) {
    int i, pinp = 0, fd, st, res;
    pid_t ctl;
    int pp[2];
    usr1_lock = false;
    signal(SIGUSR1, catcher);
    if ((ctl = fork()) == 0) {
        fprintf(stderr, "controller pid - %d\n", getpid());
        if (!usr1_lock) pause();
        res = true;
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
                        fd = open(jb->commands[i]->input, O_CREAT | O_APPEND | O_WRONLY, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
                    else 
                        fd = open(jb->commands[i]->input, O_CREAT | O_WRONLY, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
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
        signal(SIGTSTP, controller_signal_handler);
        signal(SIGINT, controller_signal_handler);
        for (i = 0; i < jb->commandsc; i++) {
            if (wait(&st) == jb->commands[jb->commandsc - 1]->pid)
                res = st;
        }
        if (is_interactive)
            tcsetpgrp(tty_fd, getpgid(getppid()));
        exit(res);
    }
    setpgid(ctl, ctl);
    if (is_interactive && !jb->background)
        tcsetpgrp(tty_fd, ctl);
    kill(ctl, SIGUSR1);
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

static void delete_pid(pid_t p) {
    int i, j;
    for (i = 0; background[i] != p && i < background_jobs_n; i++);
    if (i == background_jobs_n) return;
    for (j = i; j < background_jobs_n; j++)
        background[j] = background[j + 1];
    background_jobs_n--;
}

static void zombie_clr() {
    int st, i, res;
    for (i = 0; i < background_jobs_n; i++) {
        res = waitpid(background[i], &st, WNOHANG);
        if (res != 0 && res != -1) {
            printf("PID exited: %d\n", background[i]);
            status = st;
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
    printf("my group - %d, term control - %d\n", getpgrp(), tcgetpgrp(tty_fd));
    print_job_desc(x);
    fflush(stdout);
    if (x->background) {
        add_background_job(x, execute_job(x));
    } else {
        pid_t ctl = execute_job(x);
        int status;
        waitpid(ctl, &status, WUNTRACED);
        if (WIFSTOPPED(status)) {
            add_background_job(x, ctl);
        } else {
            free_job(x);
        }
    }
    if (!x->background)
        tcsetpgrp(tty_fd, getpgrp());
    zombie_clr();
}

static void clr_signals() {
    signal(SIGTSTP, SIG_DFL);
    signal(SIGCHLD, SIG_DFL);
    signal(SIGINT, SIG_DFL);
    signal(SIGUSR1, SIG_DFL);
    signal(SIGUSR2, SIG_DFL);
}

void init_shell(int argc, char ** argv) {
    int i;
    char * buffer;
    is_interactive = isatty(0);
    background_jobs_n = 0;
    fprintf(stderr, "%d - PID\n", getpid());

    if (is_interactive) {
        tty_fd = open(ctermid(NULL), O_RDONLY, 0);
        tcsetpgrp(tty_fd, getpgrp());
    }
    bsz = brsz = bjsz = bjrsz = 0;
    signal(SIGTSTP, signal_handler);
    signal(SIGINT, signal_handler);
    signal(SIGCHLD, signal_handler);
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
