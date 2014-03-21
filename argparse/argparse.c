#include "argparse.h"
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>

static void free_command(struct command * cm) {
    int i;
    if (cm->args != NULL)
        for (i = 0; i < cm->argc; i++)
            free(cm->args[i]);
    if (cm->name != NULL)
        free(cm->name);
    if (cm->input != NULL)
        free(cm->input);
    if (cm->output != NULL)
        free(cm->output);
    if (cm->args != NULL)
        free(cm->args);
    free(cm);
}

static void free_job(struct job * jb) {
    int i;
    if (jb->commands != NULL) {
        for (i = 0; i < jb->commandsc; i++)
            free_command(jb->commands[i]);
        free(jb->commands);
    }
    free(jb);
}

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
static int qtype(char x) {
    if (x == '\'') return 1;
    if (x == '\"') return 2;
}

char ** parseCTokens(char * x, int * sz) {
    int i, n, rsz = 0, r_rsz = 0, j;
    int qts = 0;
    int ppos = 0;
    char ** res = NULL;
    bool c_slsh = false, pr_slsh = false;
    *sz = 0;
    n = strlen(x);
    for (i = 0; i <= n && (c_slsh || qts || x[i - 1] != '#'); i++) {
        bool push = false;
        /*quotes*/
        /*if (!c_slsh && quotes(x[i]) && !qts) push = true;*/
        /*if (!pr_slsh && i > 0 && !qts && quotes(x[i - 1])) push = true;*/
        if (!c_slsh && (qts == 0 || qts == qtype(x[i])) && quotes(x[i])) {
            qts ^= qtype(x[i]);
        }
        /*tasks delimiters*/
        if (!c_slsh && !qts && x[i] == '|') push = true;
        if (!pr_slsh && !qts && i > 0 && x[i - 1] == '|') push = true;
        /*redirections*/
        if (!pr_slsh && !qts && i != 0 && bracket(x[i - 1]) && !bracket(x[i])) push = true;
        if (!c_slsh && !qts && i != 0 && bracket(x[i]) && !bracket(x[i - 1])) push = true;
        /*end of the line*/
        if (!c_slsh && i == n || x[i] == '#') push = true;
        /*spaces*/
        if (!c_slsh && !qts && x[i] == ' ') push = true;
        /*backgrounds*/
        if (!c_slsh && !qts && x[i] == '&') push = true;
        if (!pr_slsh && !qts && i > 0 && x[i - 1] == '&') push = true;

        if (push && !allspaces(x + ppos, i - ppos)) {
            /*if (allspaces(x + ppos, i - ppos)) continue;*/
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

        if (!c_slsh && x[i] == '\\') {
            pr_slsh = c_slsh;
            c_slsh = true;
        } else {
            pr_slsh = c_slsh;
            c_slsh = false;
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
    if (errno != 0) return NULL;
    return res;
}

struct command * parse_command(char ** x, int n) { 
#define fail(s) { \
                    PARSE_ERROR_MESSAGE=s; \
                    free_command(cd);  \
                    return NULL;      \
                }
    int i, j, cd_ss = 0, cd_rs = 0;
    struct command * cd = trymalloc(sizeof(struct command));
    if (errno != 0) return NULL;
    cd->argc = 0;
    cd->args = NULL;
    cd->out_append = false;
    cd->output = NULL;
    cd->input = NULL;
    cd->name = NULL;
    for (i = 0; i < n; i++) {
        if (bracket(x[i][0])) {
           if (i == n - 1) fail("syntax error at last token");
           if (strcmp(x[i], "<") == 0) {
                if (cd->input != NULL) fail("too many redirects");
                cd->input = strdup(x[i + 1]);
                if (cd->input == NULL) {
                    free_command(cd);
                    return NULL;
                }
           } else if (strcmp(x[i], ">") == 0) {
                if (cd->output != NULL) fail("too many redirects");
                cd->output = strdup(x[i + 1]);
                if (cd->output == NULL) {
                    free_command(cd);
                    return NULL;
                }
           } else if (strcmp(x[i], ">>") == 0) {
                if (cd->output != NULL) fail("too many redirects");
                cd->output = strdup(x[i + 1]);
                cd->out_append = true;
                if (cd->output == NULL) {
                    errno = ENOMEM;
                    free_command(cd);
                    return NULL;
                }
           } else fail("invalid token");
           i++;
        } else {
           if (i == 0) {
              if ((cd->name = strdup(x[i])) == NULL) {
                  free_command(cd);
                  return NULL;
              }
           } else {
               increase((void**)&(cd->args), &cd_ss, &cd_rs, sizeof(char*));
               if (errno == 0) {
                   cd->args[cd->argc] = strdup(x[i]);
                   cd->argc += 1;
               }
               if (errno != 0 || cd->args[cd->argc - 1] == NULL) {
                   free_command(cd);
                   return NULL;
               }
           }
        }
    }
    
    if (errno != 0) {
        free_command(cd);
        return NULL;
    }
    return cd;
#undef fail
}



struct job * parse(char * x) {
    int n, i, j, cds_rs = 0, cds_ss = 0, ppos = 0;
    int rescdss = 0, rescdsr = 0;
    struct job * res = trymalloc(sizeof(struct job)), * tmp;
    char ** tokens = parseCTokens(x, &n);
    if (n == 0) {
        PARSE_ERROR_MESSAGE = "empty string";
        free_job(res);
        return NULL;
    }
    res->commandsc = 0;
    res->commands = NULL;
    res->background = false;
    PARSE_ERROR_MESSAGE = "All is ok";
    if (errno != 0) return NULL;
    for (i = -1; i < n; i++) {
        if (i == -1 || strcmp(tokens[i], "|") == 0) {
            for (j = i + 1; j < n && strcmp(tokens[j], "|") != 0 && strcmp(tokens[j], "&") != 0; j++);
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
        } else
        if (strcmp(tokens[i], "&") == 0) {
            if (i != n - 1 || res->background) {
                PARSE_ERROR_MESSAGE = "invalid '&' use";
                free_job(res);
                return NULL;
            }
            res->background = true;
        }
    }
    for (i = 0; i < n; i++)
        free(tokens[i]);
    free(tokens);
    truncate_mem((void**)&res->commands, &cds_ss, &cds_rs);
    if (errno != 0) {
        free_job(res);
        return NULL;
    }
    return res;
}

void print_command_desc(struct command * jb) {
    int k;
    printf("-------command\n");
    printf("name : %s\n", jb->name);
    printf("args: %d\n", jb->argc);
    if (jb->input != NULL)
        printf("input redirected from : %s\n", jb->input);
    if (jb->output != NULL)
        printf("output redirected to : %s\n", jb->output);
    if (jb->out_append) {
        printf("output append mode\n");
    }
    for (k = 0; k < jb->argc; k++)
        printf("arg[%d]: %s\n", k, jb->args[k]);
}


void print_job_desc(struct job * jb) {
    int i, j, k;
    printf("background: %d\n", jb->background);
    printf("commands : %d\n", jb->commandsc);
    for (i = 0; i < jb->commandsc; i++) {
        print_command_desc(jb->commands[i]);
    }
}
