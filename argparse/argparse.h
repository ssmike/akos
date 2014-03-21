#ifndef ARGPARSE_H
#define ARGPARSE_H

#include <stdbool.h>
#include <stdlib.h>


struct command{
    char * input, * output;
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
extern char *strndup(const char *, size_t);
char *strdup(const char *s);
char ** parseCTokens(char * x, int * sz);
char * PARSE_ERROR_MESSAGE;
struct command * parse_command(char ** x, int n);
void print_command_desc(struct command *);

#endif
