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

bool debug;
bool is_interactive;
int background_jobs_n;
struct job ** background_jobs;
struct job * foreground;
size_t bsz, brsz, bjsz, bjrsz;
pid_t for_c_pid;

size_t rvars_sz, rvars_rsz, rvars_n;
struct variable * rvars;
int tty_fd;

int status;

int findpid(pid_t p) {
    int i;
    for (i = 0; i < background_jobs_n; i++) {
        if (background_jobs[i]->ctl_grp == p)
            return i;
    }
    return -1;
}

/*
 * returns latest status from waitpid if all exited
 * else return status of latest stopped child
 */
int waitForJob(struct job * x) {
    int i, status, stopped_st;
    int stopped = 0, exited = 0;
    while (stopped + exited != x->commandsc) {
        int tmp;
        waitpid(-x->ctl_grp, &tmp, WUNTRACED);
        if (WIFSTOPPED(tmp)) {
            stopped++;
            stopped_st = tmp;
        } else {
            exited++;
            status = tmp;
        }
    }
    if (stopped != 0)
        return stopped_st;
    else
        return status;
}

void delete_pid(pid_t p) {
    int i, j;
    size_t a, b;
    for (i = 0; background_jobs[i]->ctl_grp != p && i < background_jobs_n; i++);
    if (i == background_jobs_n) return;
    for (j = i; j < background_jobs_n - 1; j++)
        background_jobs[j] = background_jobs[j + 1];
    background_jobs_n--;
    bsz -= sizeof(pid_t);
    bjsz -= sizeof(struct job *);
    if (bsz < brsz / 2) {
        a = brsz / 2;
        b = bjrsz / 2;
        truncate_mem((void**)&background_jobs, &a, &bjrsz);
        if (errno == ENOMEM) {
            printf("failed to allocate memory\n");
            exit_shell();
        }
    }
}


static void clr_signals();
static void exec_com(struct command * cs) {
    clr_signals();
    if (builtin_find(cs->name)) builtin_exec(cs);
    execvp(cs->name, cs->args);
    printf("command not found\n");
    exit(10);
}

int kill(pid_t pid, int sig);

static pid_t execute_job(struct job * jb) {
    int i, pinp = 0, fd, st, res;
    pid_t ctl_pgrp = -1;
    int pp[2];
    {
        if (debug) fprintf(stderr, "controller pid - %d\ncommand output------------------\n", getpid());
        res = 0;
        for (i = 0; i < jb->commandsc; i++) {
            if (i != jb->commandsc - 1) {
                pipe(pp);
            }
            if ((jb->commands[i]->pid = fork()) == 0) {
                clr_signals();
                close(tty_fd);
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
            if (ctl_pgrp == -1)
                ctl_pgrp = jb->commands[i]->pid;
            setpgid(jb->commands[i]->pid, ctl_pgrp);
            if (i != jb->commandsc - 1)
                close(pp[1]);
            if (pinp != 0)
                close(pinp);
            pinp = pp[0];
        }
    }
    if (is_interactive && !jb->background)
        tcsetpgrp(tty_fd, ctl_pgrp);
    kill(-ctl_pgrp, SIGCONT);
    jb->ctl_grp = ctl_pgrp;
    return ctl_pgrp;
}

static void add_background_job(struct job * x, pid_t ctl) {
    increase((void**)&background_jobs, &bjsz, &bjrsz, sizeof(struct job *));
    if (errno == ENOMEM)
        exit_shell();
    background_jobs[background_jobs_n] = x;
    background_jobs_n++;
}

void zombie_clr() {
    int st, i, res, j;
    for (i = 0; i < background_jobs_n; i++) {
        bool all_exited = true;
        for (j = 0; j < background_jobs[i]->commandsc; j++) {
            if (background_jobs[i]->commands[j]->pid != -1) {
                res = waitpid(background_jobs[i]->commands[j]->pid, &st, WNOHANG);
                if (res != 0 && res != -1) {                    
                    background_jobs[i]->commands[j]->pid = -1;
                } else all_exited = false;
            }
        }
        if (all_exited) {
            status = WEXITSTATUS(st);
            printf("job %d exited with status %d\n", i, WEXITSTATUS(st));
            delete_pid(background_jobs[i]->ctl_grp);
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
        int st = waitForJob(x);
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
    int i, n;
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
    foreground = NULL;
    background_jobs = NULL;
    rvars_n = rvars_sz = rvars_rsz = 0;
    rvars = NULL;
    status = 0;
    pagesz = sysconf(_SC_PAGESIZE);

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

    buffer = (char*)malloc(202 * sizeof(char));
    memset(buffer, 0, sizeof(buffer));
    if (buffer == NULL) exit(3);
    n = readlink("/proc/self/exe", buffer, 200);
    buffer[n] = '\0';
    if (errno == ENAMETOOLONG)
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
