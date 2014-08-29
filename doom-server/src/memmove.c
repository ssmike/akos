#include <errno.h>
#include <assert.h>
#include <stdlib.h>
#include <unistd.h>
#include "memmove.h"

#include <stdio.h>

#define PAGE_EXP_LIMIT 4000

size_t pagesz;

int push_back(char ** s, size_t * s_ss, size_t * s_rs, char x) {
    if (increase((void**)s, s_ss, s_rs, sizeof(char)) != 0) {
        return 1;
    }
    (*s)[(*s_ss/sizeof(char)) - 2] = x;
    (*s)[(*s_ss/sizeof(char)) - 1] = '\0';
}

int increase(void ** arr, size_t * cur_sz, size_t * real_sz, size_t delta) {
    size_t newsz = *real_sz;
    void * tmp;
    if (*cur_sz + delta <= *real_sz) {
        *cur_sz += delta;
        return 0;
    }
    while (newsz < *cur_sz + delta) {
        if (newsz >= pagesz * PAGE_EXP_LIMIT || *real_sz == 0) {
            if (newsz % pagesz == 0)
                newsz += pagesz;
            else
                newsz += (pagesz - (*real_sz % pagesz));
        } else {
            newsz = *real_sz * 2;
        }
    }
    tmp = realloc((void*)(*arr), newsz);
    if (tmp == NULL) {
        return 1;
    }
    *cur_sz += delta;
    *arr = tmp;
    *real_sz = newsz;
    *arr = tmp;
    return 0;
}

int truncate_mem(void ** arr, size_t * cur_sz, size_t * real_sz) {
    void * tmp;
    if (*cur_sz == 0) {
        if (*real_sz == 0)
            return 0;
        else {
            *real_sz = 0;
            free(*arr);
            *arr = NULL;
            return 0;
        }
    }
    tmp = realloc(*arr, *cur_sz);
    if (tmp == NULL) {
        return 1;
    }
    *real_sz = *cur_sz;
    *arr = tmp;
}


void * ASSERT_NULL(void * a, char * msg) {
    if (a == NULL) {
        fprintf(stderr, "NULL pointer exception in %s\n", msg);
        _exit(0);
    }
    return a;
}
