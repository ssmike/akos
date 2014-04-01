#include <errno.h>
#include <assert.h>
#include <stdlib.h>
#include "memmove.h"


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
