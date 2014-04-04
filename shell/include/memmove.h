#ifndef MEMMOVE_H
#define MEMMOVE_H
#include "shell_structs.h"

extern size_t pagesz;
void increase(void ** arr, size_t * cur_sz, size_t * real_sz, size_t delta);
void truncate_mem(void ** arr, size_t * cur_sz, size_t * real_sz);
void free_command(struct command * cm);
void free_job(struct job * jb);
void push_back(char ** s, size_t * s_ss, size_t * s_rs, char x);

#endif
