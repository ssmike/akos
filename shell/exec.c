#include "exec.h"
#include "shell_structs.h"
#include <string.h>
#include "memmove.h"
#include <errno.h>
#include <stdlib.h>

/*for print job desc (debug) */
#include "argparse.h"
#include <stdio.h>



void execute(struct job* x) {
/*    int i, j;
    for (i = 0; i < x->commandsc; i++) {
        replace_vars(&(x->commands[i]->name));
        for (j = 0; j < x->commands[i]->argc; j++)
            replace_vars(&(x->commands[i]->args[j]));
    }*/
    print_job_desc(x);
    fflush(stdout);
}

void init_shell(int argc, char ** argv) {
}


static void push_back(char ** s, size_t * s_ss, size_t * s_rs, char x) {
    increase((void**)s, s_ss, s_rs, sizeof(char));
    if (errno != 0) {
        fprintf(stderr, "memory allocation error");
        exit(3);
    }
    (*s)[(*s_ss/sizeof(char)) - 1] = x;
}

static char * findvar(char * name) {
    return getenv(name);
}

static int type(char x) {
    if (x == '\'') return 1;
    if (x == '\"') return 2;
    return 0;
}

static bool isChar(char x) {
    return x != '\"' && x != '\'' && x != '}' && x != '{' && x != '$' && x != '\\';
}

int replace_vars(char ** x) {
    int n = strlen(*x), i;
    char * ans = NULL;
    size_t anssz = 0;
    size_t ansrsz = 0;
    int quott = 0;
    bool slash = false;
    for (i = 0; i < n; i++) {
        if (slash) {
            
        }
        if (quott) {
            if (type(*x[i]) == quott)
                quott = false;
            else push_back(&ans, &anssz, &ansrsz, *x[i]);
        }
    }
    *x = ans;
    return 0;
}
