#include "exec.h"
#include "argparse.h"
#include "memmove.h"
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>

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
    int i, n, j;
    size_t rsz = 0, r_rsz = 0;
    int qts = 0;
    int ppos = 0;
    char ** res = NULL;
    bool c_slsh = false, pr_slsh = false;
    *sz = 0;
    n = strlen(x);
    for (i = 0; i <= n; i++) {
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
        if (!c_slsh && i == n) push = true;
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
    if (errno == ENOMEM) return NULL;
    return res;
}

struct command * parse_command(char ** x, int n) { 
#define fail(s) { \
                    PARSE_ERROR_MESSAGE=s; \
                    free_command(cd);  \
                    return NULL;      \
                }
    int i, tst;
    size_t cd_ss = sizeof(char*), cd_rs = sizeof(char*);
    struct command * cd = trymalloc(sizeof(struct command));
    if (errno != 0) return NULL;
    cd->argc = 1;
    cd->args = malloc(sizeof(char*));
    if (cd->args == NULL) {
        free_command(cd);
        errno = ENOMEM;
        return NULL;
    }
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
                replace_vars(&cd->input);
                if (cd->input == NULL) {
                    free_command(cd);
                    return NULL;
                }
           } else if (strcmp(x[i], ">") == 0) {
                if (cd->output != NULL) fail("too many redirects");
                cd->output = strdup(x[i + 1]);
                replace_vars(&cd->output);
                if (cd->output == NULL) {
                    free_command(cd);
                    return NULL;
                }
           } else if (strcmp(x[i], ">>") == 0) {
                if (cd->output != NULL) fail("too many redirects");
                cd->output = strdup(x[i + 1]);
                cd->out_append = true;
                replace_vars(&cd->output);
                if (cd->output == NULL) {
                    errno = ENOMEM;
                    free_command(cd);
                    return NULL;
                }
           } else fail("invalid token");
           i++;
        } else {
            if (i == 0) {
                cd->name = strdup(x[i]);
                cd->args[0] = strdup(x[i]);
                tst = replace_vars(&(cd->name)) || replace_vars(&(cd->args[0]));
                if (tst != 0 || cd->name == NULL || cd->args[0] == NULL) {
                    free_command(cd);
                    return NULL;
                }
            } else {
                increase((void**)&(cd->args), &cd_ss, &cd_rs, sizeof(char*));
                if (errno == 0) {
                    cd->args[cd->argc] = strdup(x[i]);
                    tst = replace_vars(&cd->args[cd->argc]);
                    cd->argc += 1;
                }
                if (errno != 0 || tst != 0 || cd->args[cd->argc - 1] == NULL) {
                    free_command(cd);
                    return NULL;
                }
            }
        }
    }
    increase((void**)&(cd->args), &cd_ss, &cd_rs, sizeof(char*));
    truncate_mem((void**)&(cd->args), &cd_ss, &cd_rs);
    if (errno != 0) {
        free_command(cd);
        return NULL;
    } else {
        cd->args[cd->argc] = NULL;
    }
    return cd;
#undef fail
}



struct job * parse(char * x) {
    int n, i, j;
    size_t cds_rs = 0, cds_ss = 0;
    struct job * res = trymalloc(sizeof(struct job));
    char ** tokens;
    if (res == NULL) {
        errno = ENOMEM;
        return NULL;
    }
    if (x == NULL) {
        PARSE_ERROR_MESSAGE = "empty string";
        return NULL;
    }
    tokens = parseCTokens(x, &n);
    res->commandsc = 0;
    res->commands = NULL;
    res->background = false;
    if (n == 0) {
        PARSE_ERROR_MESSAGE = "empty string";
        free_job(res);
        return NULL;
    }
    PARSE_ERROR_MESSAGE = "ok";
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
    int i;
    printf("background: %d\n", jb->background);
    printf("commands : %d\n", jb->commandsc);
    for (i = 0; i < jb->commandsc; i++) {
        print_command_desc(jb->commands[i]);
    }
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
    int n = strlen(*x), i, j, k, tlen, buf;
    char * ans = (char*)malloc(sizeof(char));
    size_t anssz = sizeof(char);
    size_t ansrsz = sizeof(char);
    int quott = 0;
    bool slash = false;
    char * tmp, * tmp2;
    char carr[10];
    if (ans == NULL) {
        errno = ENOMEM;
        return -1;
    }
    for (i = 0; i < n; i++) {
        if (slash) {
            push_back(&ans, &anssz, &ansrsz, (*x)[i]);
            slash = false;
            continue;
        }
        if ((*x)[i] == '$' && (quott & 1) == 0 && !slash) {
            if (i != n - 1 && (*x)[i + 1] == '{') {
                j = i + 2;
                while (j < n && (*x)[j] != '}') j++;
                if (j == n) {
                    PARSE_ERROR_MESSAGE = "bad substitution";
                    free(ans);
                    return -1;
                }
                if ((tmp = strndup((*x) + i + 2, j - i - 2)) == NULL) {
                    errno = ENOMEM;
                    free(ans);
                    return -1;
                }
                tmp2 = findvar(tmp);
                tlen = strlen(tmp2);
                free(tmp);
                for (k = 0; k < tlen; k++)
                    push_back(&ans, &anssz, &ansrsz, tmp2[k]);
                i = j;
            } else {
                j = i + 1;
                while (j < n && isChar((*x)[j])) j++;
                if ((tmp = strndup((*x) + i + 1, j - i - 1)) == NULL) {
                    errno = ENOMEM;
                    free(ans);
                    return -1;
                }
                tmp2 = findvar(tmp);
                tlen = strlen(tmp2);
                free(tmp);
                for (k = 0; k < tlen; k++)
                    push_back(&ans, &anssz, &ansrsz, tmp2[k]);
                i = j - 1;
            }
            continue;
        }
        if ((*x)[i] == '%' && quott == 0) {
            for (j = i + 1; j < n && (*x)[j] >= '0' && (*x)[j] <= '9'; j++);
            if ((tmp = strndup((*x) + i + 1, j - i - 1)) == NULL) {
                errno = ENOMEM;
                free(ans);
                return -1;
            }
            buf = -1;
            sscanf(tmp, "%d", &buf);
            if (buf < 0 || buf > background_jobs_n) { 
                carr[0] = '1';
                carr[1] = '\0';
            }
            else sprintf(carr, "%d", -background_jobs[buf]->ctl_grp);
            tlen = strlen(carr);
            for (k = 0; k < tlen; k++)
                push_back(&ans, &anssz, &ansrsz, carr[k]);
            i = j - 1;
            continue;
        }
        if (quott) {
            if ((quott & 1) == 0 && (*x)[i] == '\\')
                slash = true;                
            if (type((*x)[i]) == quott)
                quott = false;
            else push_back(&ans, &anssz, &ansrsz, (*x)[i]);
            continue;
        }

        if (type((*x)[i]) != 0) {
            quott = type((*x)[i]);
            continue;
        }
        if ((*x)[i] == '\\') {
            slash = true;
            continue;
        }
        push_back(&ans, &anssz, &ansrsz, (*x)[i]);
    }
    /*push_back(&ans, &anssz, &ansrsz, '\0');*/
    truncate_mem((void**)&ans, &anssz, &ansrsz);
    if (errno != 0) {
        free(ans);
        return -1;
    }
    free(*x);
    (*x) = ans;
    return 0;
}
