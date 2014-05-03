#ifndef BUILTINS_H
#define BUILTINS_H

#define builtins_n 5
#include <shell_structs.h>

typedef int (*builtin)(struct command *);
extern char * builtin_names[builtins_n];
extern builtin functions[builtins_n];

bool builtin_hook(struct job * x);
bool builtin_find(char * s);
void builtin_exec(struct command * cs);


#endif
