#ifndef ARGPARSE_H
#define ARGPARSE_H

#include <stdbool.h>
#include <stdlib.h>


struct command{
    char * input, * ouput;
    bool out_append;
    char * name;
    char ** args;
    int argc;
};

struct job{
    struct command ** commands; 
    int commandsc;
    bool background;
};

struct job * parse(char *);
void print_job_desc(struct job *);
char *strndup(const char *, size_t);

#endif
