#include "argparse.h"
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>
void increase(void * arr, int * cur_sz, int * real_sz, int delta) {
    assert(cur_sz == real_sz);
    *real_sz += delta;
    *cur_sz += delta;
    realloc(arr, *cur_sz);
}

char ** parseCTokens(char * x) {
    int i, n;
    n = strlen(x);
    for ()   
}

struct command * parse_command(char * x) { 
}

struct job * parse(char * x) {
    char * * commands = NULL;
    int n, i, j, cds_rs = 0, cds_ss = 0;
    int rescdss = 0, rescdsr = 0;
    struct job * res;
    n = strlen(x);
    for (i = 0; i < n; i++) {
        if (x[i] != '|') continue;
        for (j = i + 1; j < n && x[j] != '|' && x[j] != '&'; j++);
        increase(commands, &cds_ss, &cds_rs, sizeof(char*));
        commands[(cds_ss/sizeof(char*)) - 1] = strndupa(x, j - i - 1);
    }
    for (i = n - 1; i >= 0 && x[i] != '&'; i--);
    res = (struct job*) malloc(sizeof(struct job));
    res->background = (i >= 0 && x[i] == '&');
    res->commandsc = cds_ss;
    res->commands = (struct command**)malloc(sizeof(struct command *) * cds_ss/sizeof(char *));
    for (i = 0; i < cds_ss; i++) {
        increase(res->commands, &rescdss, &rescdsr, sizeof(struct command *));
        res->commands[rescdss/sizeof(struct command *) - 1] = parse_command(commands[i]);
    }
    free(commands);
    realloc(res->commands, res->commandsc * sizeof(struct command *));
    return res;
}


void print_job_desc(struct job * jb) {
    int i, j, k;
    printf("background: %d\n", jb->background);
    printf("commands : %d\n", jb->commandsc);
    for (i = 0; i < jb->commandsc; i++) {
        printf("name : %s\n", jb->commands[i]->name);
        printf("args: %d\n", jb->commands[i]->name);
        for (k = 0; k < jb->commands[i]->argc; k++)
            printf("arg[%d]: %s\n", k, jb->commands[i]->args[k]);
    }
}
