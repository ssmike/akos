#include <errno.h>
#include <assert.h>
#include <stdlib.h>
#include "memmove.h"
#include "shell_structs.h"

#include <stdio.h>

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

void truncate_mem(void ** arr, size_t * cur_sz, size_t * real_sz) {
    assert(*cur_sz == *real_sz);
}
