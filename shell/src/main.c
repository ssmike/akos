#include "shell_structs.h"
#include "argparse.h"
#include "memmove.h"
#include "exec.h"
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>


char * s = NULL;
size_t s_ss, s_rs;

static int type(char x) {
    if (x == '\'') return 1;
    if (x == '\"') return 2;
    return 0;
}

static void exec_s() {
    struct job * jb = parse(s);
    free(s);
    if (jb != NULL) {
        execute(jb);
    } else {
        fprintf(stderr, "\n%s\n", PARSE_ERROR_MESSAGE);
        fflush(stderr);
    }
    s = (char*)malloc(sizeof(char));
    if (s == NULL) exit(3);
    s[0] = '\0';
    s_ss = s_rs = sizeof(char);
    if(errno == ENOMEM) {
        free(s);
        exit(3);
    }
}

int main(int argc, char ** argv) {
    int quot = 0;
    bool slash = false;
    bool comment = false;
    init_shell(argc, argv);
    s_ss = s_rs = sizeof(char);
    s = (char*)malloc(sizeof(char));
    if (s == NULL) exit(3);
    s[0] = '\0';
    while(!feof(stdin)) {
        char c = getchar();
        if (comment) {
            if (c == '\n') {
                comment = false;
                exec_s();
            }
            continue;
        }
        if (slash) {
            if (c == '\n') {
                s[(s_ss/sizeof(char)) - 2] = ' ';
            } else push_back(&s, &s_ss, &s_rs, c);
            slash = false;
            continue;
        } 
        if (quot != 0) {
            push_back(&s, &s_ss, &s_rs, c);
            if (quot == type(c))
                quot = 0;
            continue;
        }
        if (c == '\n' || c == ';') {
            exec_s();
            continue;
        }
        if (type(c) != 0)
            quot = type(c);
        if (c == '\\')
            slash = true;
        if (c == '#') {
            comment = true;
            continue;
        }
        push_back(&s, &s_ss, &s_rs, c);
    }
    exit_shell();
}
