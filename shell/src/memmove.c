#include <errno.h>
#include <assert.h>
#include <stdlib.h>
#include "memmove.h"
#include "shell_structs.h"

#include <stdio.h>

#define PAGE_EXP_LIMIT 4000

size_t pagesz;

void push_back(char ** s, size_t * s_ss, size_t * s_rs, char x) {
    increase((void**)s, s_ss, s_rs, sizeof(char));
    if (errno == ENOMEM) {
        fprintf(stderr, "memory allocation error");
        exit(3);
    }
    (*s)[(*s_ss/sizeof(char)) - 2] = x;
    (*s)[(*s_ss/sizeof(char)) - 1] = '\0';
}

void free_command(struct command * cm) {
    int i;
    if (cm->args != NULL)
        for (i = 0; i < cm->argc; i++)
            if (cm->args[i] != NULL)
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

void free_job(struct job * jb) {
    int i;
    if (jb->commands != NULL) {
        for (i = 0; i < jb->commandsc; i++)
            free_command(jb->commands[i]);
        free(jb->commands);
    }
    free(jb);
}


void increase(void ** arr, size_t * cur_sz, size_t * real_sz, size_t delta) {
    size_t newsz = 0;
    void * tmp;
    if (*cur_sz + delta <= *real_sz) {
        *cur_sz += delta;
        return;
    }
    if (*real_sz >= pagesz * PAGE_EXP_LIMIT) {
        if (*real_sz % pagesz == 0)
            newsz = *real_sz + pagesz;
        else
            newsz = *real_sz + (pagesz - (*real_sz % pagesz));
    } else {
        newsz = *real_sz * 2;
    }
    newsz = *cur_sz + delta;
    tmp = realloc((void*)(*arr), newsz);
    if (tmp == NULL) {
        errno = ENOMEM;
        return;
    }
    *cur_sz = newsz;
    *arr = tmp;
    *real_sz = newsz;
    *arr = tmp;
}

void truncate_mem(void ** arr, size_t * cur_sz, size_t * real_sz) {
    /*assert(*cur_sz == *real_sz);*/
    void * tmp = realloc(*arr, *cur_sz);
    if (tmp == NULL) {
        errno = ENOMEM;
        return;
    }
    *real_sz = *cur_sz;
    *arr = tmp;
}
