#include "exec.h"
#include "shell_structs.h"
#include <string.h>
#include "memmove.h"
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>

/*for print job desc (debug) */
#include "argparse.h"
#include <stdio.h>

struct variable{
    char * name;
    char * value;
};

size_t rvars_sz, rvars_rsz, rvars_n;
struct variable * rvars;

int status;

void execute(struct job* x) {
/*    int i, j;
    for (i = 0; i < x->commandsc; i++) {
        replace_vars(&(x->commands[i]->name));
        for (j = 0; j < x->commands[i]->argc; j++)
            replace_vars(&(x->commands[i]->args[j]));
    }*/
    print_job_desc(x);
    fflush(stdout);
    free_job(x);
}

extern int gethostname(char *name, size_t len);
extern ssize_t readlink(const char *path, char *buf, size_t bufsiz);

void init_shell(int argc, char ** argv) {
    int i;
    char * buffer;
    rvars_n = rvars_sz = rvars_rsz = 0;
    rvars = NULL;
    status = 0;

    buffer = (char*)malloc(10 * sizeof(char));
    if (buffer == NULL) exit(3);
    sprintf(buffer, "%d", argc);
    setvar("#", buffer);

    for (i = 0; i <= argc; i++) {
        buffer = (char*)malloc(10 * sizeof(char));
        if (buffer == NULL) exit(3);
        sprintf(buffer, "%d", i);
        setvar(buffer, argv[i]);
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
    
   /*setvar("");*/
}

void setvar(char * name, char * val){
    if (val == NULL) return;
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
    if (tmp == NULL) return "";
    else return tmp;
}
