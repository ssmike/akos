#ifndef BUILTINS_H
#define BUILTINS_H

#define builtins_n 1
#include <shell_structs.h>

typedef int (*builtin)(struct command *);
extern char * builtin_names[builtins_n];
extern builtin functions[builtins_n];


#endif
