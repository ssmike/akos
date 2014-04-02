#include "shell_structs.h"
#include "argparse.h"
#include "memmove.h"
#include "exec.h"
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>


char * s = NULL;
size_t s_ss, s_rs;

void push_back(char x) {
    increase((void**)&s, &s_ss, &s_rs, sizeof(char));
    if (errno != 0) {
        fprintf(stderr, "memory allocation error");
        exit(3);
    }
    s[(s_ss/sizeof(char)) - 1] = x;
}

static int type(char x) {
    if (x == '\'') return 1;
    if (x == '\"') return 2;
    return 0;
}

int main(int argc, char ** argv) {
    int quot = 0;
    bool slash = false;
    bool comment = false;
    init_shell(argc, argv);
    s_ss = s_rs = 0;
    s = NULL;
    while(!feof(stdin)) {
        char c = getchar();
        if (comment) {
            if (c == '\n') comment = false;
            continue;
        }
        if (slash) {
            if (c == '\n') {
                s[(s_ss/sizeof(char)) - 1] = ' ';
            } else push_back(c);
            slash = false;
            continue;
        } 
        if (quot != 0) {
            push_back(c);
            if (quot == type(c))
                quot = 0;
            continue;
        }
        if (c == '\n' || c == ';') {
            struct job * jb = parse(s);
            if (jb != NULL) {
                execute(jb);
            } else {
                fprintf(stderr, "\n%s\n", PARSE_ERROR_MESSAGE);
                fflush(stderr);
            }
            free(s);
            s = NULL;
            s_ss = s_rs = 0;
            continue;
        }
        if (type(c) != 0)
            quot = type(c);
        if (c == '\\')
            slash = true;
        push_back(c);
    }
}
