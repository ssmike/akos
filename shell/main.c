#include "shell_structs.h"
#include "argparse.h"
#include "memmove.h"
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
}

int type(char x) {
    if (x == '\'') return 1;
    if (x == '\"') return 2;
    return 0;
}

int main(int argc, char ** argv) {
    int quotes = 0;
    bool slash = false;
    bool comment = false;
    s_ss = s_rs = 0;
    while(!feof(stdin)) {
        char c = getchar();
        if (comment) {
            if (c == '\n') comment = false;
            continue;
        }
        if (slash) {
            push_back(c);
            continue;
        } 
        if (quotes != 0) {
            push_back(c);
            if (quotes == type(c))
                quotes = 0;
            continue;
        }             
        if (c == '\n' || c == ';') {
            
        }
        if (type(c) != 0) {
            
        }
    }
}
