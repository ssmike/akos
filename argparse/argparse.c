#include "argparse.h"
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>

static void increase(void ** arr, int * cur_sz, int * real_sz, int delta) {
    void * tmp;
    assert(cur_sz == real_sz);
    *real_sz += delta;
    *cur_sz += delta;
    tmp = realloc(*arr, *cur_sz);
    if (tmp == 0) {
        errno = ENOMEM;
        free(arr);
        return;
    }
    *arr = tmp;
}

static char ** parseCTokens(char * x, int * sz) {
    int i, n;
    n = strlen(x);
     
}

static struct command * parse_command(char * x) { 
   int i, j, n = 0;
   char ** tokens = parseCTokens(x, &n); 
   struct command * res = (struct command *) malloc(sizeof(struct command));
   if (res == NULL) {
       errno = ENOMEM;
       return NULL;
   }
   res->ouput = res->input = NULL;
   res->out_append = false;
   res->args = NULL;
   res->argc = 0;
   for (i = 0; i < n; i++) {
       
   }
}

struct job * parse(char * x) {
    char ** commands = NULL;
    int n, i, j, cds_rs = 0, cds_ss = 0;
    int rescdss = 0, rescdsr = 0;
    struct job * res, * tmp;
    n = strlen(x);
    for (i = 0; i < n; i++) {
        if (x[i] != '|') continue;
        for (j = i + 1; j < n && x[j] != '|' && x[j] != '&'; j++);
        increase((void**)(&commands), &cds_ss, &cds_rs, sizeof(char*));
        if (errno != 0) {
            free(commands);
            return NULL; 
        }
        commands[(cds_ss/(sizeof(char*))) - 1] = strndup(x + i + 1, j - i - 1);
    }
    for (i = n - 1; i >= 0 && x[i] != '&'; i--);
    res = (struct job*) malloc(sizeof(struct job));
    if (res == NULL) {
        free(commands);
        return NULL;
    }
    res->background = (i >= 0 && x[i] == '&');
    res->commandsc = cds_ss;
    res->commands = (struct command**)malloc(sizeof(struct command *) * cds_ss/sizeof(char *));
    if (res->commands == NULL) {
        errno = ENOMEM;
        free(commands);
        free(res);
        return NULL;
    }
    for (i = 0; i < cds_ss; i++) {
        increase((void**)&(res->commands), &rescdss, &rescdsr, sizeof(struct command *));
        if (errno != 0) {
            errno = ENOMEM;
            free(commands);
            free(res->commands);
            free(res);
            return NULL;
        }
        res->commands[rescdss/sizeof(struct command *) - 1] = parse_command(commands[i]);
    }
    free(commands);
    tmp = realloc(res->commands, res->commandsc * sizeof(struct command *));
    if (tmp == NULL) {
        free(res->commands);
        free(res);
        errno = ENOMEM;
        return NULL;
    }
    return res;
}


void print_job_desc(struct job * jb) {
    int i, j, k;
    printf("background: %d\n", jb->background);
    printf("commands : %d\n", jb->commandsc);
    for (i = 0; i < jb->commandsc; i++) {
        printf("name : %s\n", jb->commands[i]->name);
        printf("args: %d\n", jb->commands[i]->argc);
        for (k = 0; k < jb->commands[i]->argc; k++)
            printf("arg[%d]: %s\n", k, jb->commands[i]->args[k]);
    }
}
