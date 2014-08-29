#ifndef MEMMOVE_H
#define MEMMOVE_H

extern size_t pagesz;
int increase(void ** arr, size_t * cur_sz, size_t * real_sz, size_t delta);
int truncate_mem(void ** arr, size_t * cur_sz, size_t * real_sz);
int push_back(char ** s, size_t * s_ss, size_t * s_rs, char x);
void* ASSERT_NULL(void *, char *);

#endif
