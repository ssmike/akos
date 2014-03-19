#ifndef ARGPARSE_H
#define ARGPARSE_H

#include <sys/queue.h>
#include <stdbool.h>


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

#endif
