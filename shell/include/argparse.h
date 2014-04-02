#ifndef ARGPARSE_H
#define ARGPARSE_H

#include <stdbool.h>
#include <stdlib.h>
#include "shell_structs.h"



struct job * parse(char *);
void print_job_desc(struct job *);
extern char *strndup(const char *, size_t);
char *strdup(const char *s);
/*char *strndup(const char *s, int n);*/
char ** parseCTokens(char * x, int * sz);
char * PARSE_ERROR_MESSAGE;
struct command * parse_command(char ** x, int n);
void print_command_desc(struct command *);
int replace_vars(char **);

#endif
