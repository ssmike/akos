#include "argparse.h"
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>

static void increase(void ** arr, int * cur_sz, int * real_sz, int delta) {
    void * tmp;
    assert(*cur_sz == *real_sz);
    *real_sz += delta;
    *cur_sz += delta;
    tmp = realloc(*arr, *cur_sz);
    if (tmp == NULL) {
        errno = ENOMEM;
        return;
    }
    *arr = tmp;
}
static void truncate_mem(void ** arr, int * cur_sz, int * real_sz) {
    assert(*cur_sz == *real_sz);
}

static bool bracket(char c) {
    return c == '<' || c == '>';
}

static char * truncate(char * s) {
    int n;
    int i, j;
    char * tmp;
    if (s == NULL){
        errno = ENOMEM;
        return NULL;
    }
    n = strlen(s);
    i = 0; j = n;
    while (i < n && s[i] == ' ') i++;
    while (j > 0 && s[j - 1] == ' ') j--;
    tmp = strndup(s + i, j - i);
    if (tmp != NULL)
        free(s);
    else errno = ENOMEM;
    return tmp;
}

static bool quotes(char x) {
    return x == '\"' || x == '\'';
}

static bool allspaces(char * x, int n) {
    int i;
    for (i = 0; i < n; i++)
        if (x[i] != ' ') return false;
    return true;
}

char ** parseCTokens(char * x, int * sz) {
    int i, n, rsz = 0, r_rsz = 0, j;
    int qts = 0;
    int ppos = 0;
    char ** res = NULL;
    *sz = 0;
    n = strlen(x);
    for (i = 0; i <= n; i++) {
        bool push = false;
        /*quotes*/
        if (quotes(x[i]) && !qts) push = true;
        if (i > 0 && !qts && quotes(x[i])) push = true;
        if (quotes(x[i])) {
            if (x[i] == '\'') qts ^= 1;
            if (x[i] == '\"') qts ^= 2;
        }
        /*tasks delimiters*/
        if (!qts && x[i] == '|') push = true;
        if (!qts && i > 0 && x[i - 1] == '|') push = true;
        /*redirections*/
        if (!qts && bracket(x[i])) push = true;
        if (!qts && i != 0 && bracket(x[i]) && !bracket(x[i - 1])) push = true;
        /*end of the line*/
        if (i == n) push = true;
        /*spaces*/
        if (!qts && x[i] == ' ') push = true;
        /*backgrounds*/
        if (!qts && x[i] == '&') push = true;
        if (!qts && i > 0 && x[i - 1] == '&') push = true;

        if (push) {
            if (allspaces(x + ppos, i - ppos)) continue;
            increase((void**)&res, &rsz, &r_rsz, sizeof(char *));
            if (errno == 0) {
                res[*sz] = truncate(strndup(x + ppos, i - ppos));
                *sz += 1;
            }
            if (errno != 0) {
                for (j = 0; j < *sz; j++)
                    free(res[j]);
                free(res);
                return NULL;
            }
            ppos = i;
        }
    }
    truncate_mem((void**)&res, &rsz, &r_rsz);
    if (errno != 0) {
        for (j = 0; j < *sz; j++)
            free(res[j]);
        free(res);
        return NULL;
    }
    return res;
}

static void * trymalloc(size_t size) {
    void * res = malloc(size);
    if (res == NULL) errno = ENOMEM;
    return NULL;
}

static struct command * parse_command(char ** x, int n) { 
    int i, j  = 0;

}

static void free_command(struct command * cm) {
    int i;
    for (i = 0; i < cm->argc; i++)
        free(cm->args[i]);
    if (cm->input != NULL)
        free(cm->input);
    if (cm->output == NULL)
        free(cm->output);
    free(cm->args);
    free(cm);
}

static void free_job(struct job * jb) {
    int i;
    for (i = 0; i < jb->commandsc; i++)
        free_command(jb->commands[i]);
    free(jb->commands);
    free(jb);
}

struct job * parse(char * x) {
    int n, i, j, cds_rs = 0, cds_ss = 0, ppos = 0;
    int rescdss = 0, rescdsr = 0;
    struct job * res = trymalloc(sizeof(struct job *)), * tmp;
    char ** tokens = parseCTokens(x, &n);
    res->commandsc = 0;
    res->commands = NULL;
    res->background = false;
    PARSE_ERROR_MESSAGE = "All is ok";
    if (errno != 0) return NULL;
    for (i = 0; i < n; i++) {
        if (strcmp(tokens[i], "|") == 0) {
            for (j = i + 1; j < n && strcmp(tokens[i], "|") != 0 && strcmp(tokens[i], "&") != 0; j++);
            if (i == 0 || i == n - 1 || j == i + 1) {
                PARSE_ERROR_MESSAGE = "invalid '|' use";
                free_job(res);
                return NULL;
            }
            increase((void**)&(res->commands), &cds_ss, &cds_rs, sizeof(struct command *));
            if (errno == 0) {
                res->commands[res->commandsc] = parse_command(tokens + i + 1, j - i - 1);
                if (res->commands[res->commandsc] == NULL) {
                    free_job(res);
                    return NULL;
                }
                res->commandsc += 1;
            } else {
                free_job(res);
                return NULL;
            }
        }
        if (strcmp(tokens[i], "&") == 0) {
            if (i != n - 1 || res->background) {
                PARSE_ERROR_MESSAGE = "invalid '&' use";
                free_job(res);
                return NULL;
            }
            res->background = true;
        }
    }
    truncate_mem((void**)&res->commands, &cds_ss, &cds_rs);
    if (errno != 0) {
        free_job(res);
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
