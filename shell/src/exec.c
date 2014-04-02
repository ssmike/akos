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
#include "builtins.h"

struct variable{
    char * name;
    char * value;
};

static void signal_handler(int sn) {
    if (sn == SIGTTIN) {
        fputs("SIGTTIN", stderr);
    }
    if (sn == SIGTSTP) {
        fputs("SIGTSTP", stderr);
    }
    if (sn == SIGINT) {
        _exit(2);
    }
}

struct pid_t ** background;
struct job * foreground;
size_t bsz, brsz;
pid_t for_c_pid;

size_t rvars_sz, rvars_rsz, rvars_n;
struct variable * rvars;
static int tty_fd;

int status;

static bool builtin_hook(struct job * x) {
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

static void exec_com(struct command * cs) {
    if (builtin_find(cs->name)) builtin_exec(cs);
    execvp(cs->name, cs->args);
    exit(10);
}

static void controller_signal_handler(int snum) {
    
}

extern int setpgrp(void);

static pid_t execute_job(struct job * jb) {
    int i, pinp = 0, fd, st, res;
    pid_t ctl;
    int pp[2];
    if ((ctl = fork()) == 0) {
        setpgid(0, 0);
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
        exit(st);
    }
    if (!jb->background) {
        tcsetpgrp(tty_fd, getpgid(ctl));
    }
    return ctl;
}

void execute(struct job* x) {
    if (builtin_hook(x)) {
        free_job(x);
        return;
    }
    print_job_desc(x);
    fflush(stdout);
    waitpid(execute_job(x), 0, 0);
    tcsetpgrp(tty_fd, getpgid(0));
    free_job(x);
}

extern int gethostname(char *name, size_t len);
extern ssize_t readlink(const char *path, char *buf, size_t bufsiz);
extern char *ctermid(char *);

void init_shell(int argc, char ** argv) {
    int i;
    char * buffer;
    tty_fd = open(ctermid(NULL), O_RDONLY, 0);
    bsz = brsz = 0;
    signal(SIGTSTP, signal_handler);
    signal(SIGINT, signal_handler);
    signal(SIGTTIN, signal_handler);
    background = NULL;
    foreground = NULL;
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
        printf(buffer, "%d", status);
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
}
