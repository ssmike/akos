#define _POSIX_SOURCE 1
#include "builtins.h"
#include <signal.h>
#include "exec.h"
#include <stdio.h>
#include "argparse.h"
#include <unistd.h>
#include <errno.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <limits.h>
#include <sys/wait.h>
#include <getss.h>
#include <memmove.h>
#include <stdlib.h>

int jobs(struct command * t);
int pwd(struct command * t);
int mcat(struct command * t);
int msort(struct command * t);

char * builtin_names[builtins_n] = {"jobs", "pwd", "mcat", "msort"};
builtin functions[builtins_n] = {&jobs, &pwd, &mcat, &msort};

int mcat(struct command * t) {
    char c; 
    FILE * fin = stdin;
    if (t->argc > 2) return 2;
    if (t->argc > 1) {
        fin = fopen(t->args[1], "r");
        if (fin == NULL) return 2;
    }
    while ((c = fgetc(fin)) != EOF) {
        fputc(c, stdout);
    }
    return 0;
}

int strcmpv(const void * a, const void * b) {
    return strcmp(*((char **) a), *((char **) b));
}

int msort(struct command * t) {
    char ** lines = NULL;
    size_t lsz = 0, lrsz = 0;
    bool reversed = false;
    bool unique = false;
    FILE * fin = stdin;
    int i;
    for (i = 1; i < t->argc; i++) {
        if (strcmp(t->args[i], "-d") == 0) {
            reversed = true;
        } else
        if (strcmp(t->args[i], "-u") == 0) {
            unique = true;
        } else {
            fin = fopen(t->args[i], "r");
        }
    }
    while (!feof(stdin)) {
        char * ss;
        if (fgetss(fin, &ss) < 0)
            return 2;
        if (ss == NULL) break;
        increase((void*)&lines, &lsz, &lrsz, sizeof(char *));
        lines[lsz/sizeof(char*) - 1] = ss;
        if (errno == ENOMEM)
            return 2;
    }
    qsort((void*)lines, lsz / sizeof(char *), sizeof(char*), strcmpv);
    if (reversed) {
        int i = 0, j = lsz / sizeof(char *) - 1;
        while (i < j) {
            char * z = lines[i];
            lines[i] = lines[j];
            lines[j] = z;
            i++; j--;
        }
    }
    for (i = 0; i < lsz / sizeof(char *); i++) {
        if (!unique || i == 0 || strcmp(lines[i], lines[i - 1]) != 0) {
            lines[i][strlen(lines[i]) - 1] = '\0';
            if (strlen(lines[i]) != 0)
                puts(lines[i]);
        }
    }
    fflush(stdout);
    return 0;
}

int jobs(struct command * t) {
    int i;
    for (i = 0; i < background_jobs_n; i++) {
        printf("background job group id - %d\n", background_jobs[i]->ctl_grp);
        printf("id : %d\n", i);
        print_job_desc(background_jobs[i]);
    }
    return 0;
}

int pwd(struct command * t) {
    printf("%s\n", getenv("PWD"));
    return 0;
}

static char cwdbuf[PATH_MAX + 1];

bool builtin_hook(struct job * x) {
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
                dd = -background_jobs[background_jobs_n - 1]->ctl_grp;
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
            st = waitForJob(background_jobs[findpid(-dd)]);
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
                if (background_jobs_n >= 1)
                    dd = -background_jobs[background_jobs_n - 1]->ctl_grp;
                else dd = 1;
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
            st = waitForJob(background_jobs[findpid(-dd)]);
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

bool builtin_find(char * s) {
    int i;
    for (i = 0; i < builtins_n; i++) {
        if (strcmp(s, builtin_names[i]) == 0) {
            return true;
        }
    }
    return false;
}

void builtin_exec(struct command * cs) {
    int i;
    for (i = 0; i < builtins_n; i++) {
        if (strcmp(cs->name, builtin_names[i]) == 0) {
            exit((*functions[i])(cs));
        }
    }
    free(cs);
    exit(10); 
}
