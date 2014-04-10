#include "shell_structs.h"
#include "argparse.h"
#include "memmove.h"
#include "exec.h"
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>

int geterrno() {
    return errno;
}

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
        if (is_interactive) {
            fprintf(stderr, "\n%s\n", PARSE_ERROR_MESSAGE);
            fflush(stderr);
        }
    }
    s = (char*)malloc(sizeof(char));
    if (s == NULL) exit(3);
    s[0] = '\0';
    s_ss = s_rs = sizeof(char);
    if(errno == ENOMEM) {
        free(s);
        exit(3);
    }
    zombie_clr();
}

#include "getss.h"

int getc_buf_len;
char * getc_buf = NULL;
int getc_buf_pos;

bool allspaces_s() {
    int slen = strlen(s), i;
    for (i = 0; i < slen; i++) {
        if (s[i] != ' ')
            return false;
    }
    return true;
}

char prgetc() {
    while (getc_buf_pos == getc_buf_len || getc_buf == NULL) {
        if (is_interactive) {
            if (allspaces_s()) printf("> ");
            else printf("| ");
            fflush(stdout);
        }
        free(getc_buf);
        if ((getc_buf_len = getss(&getc_buf)) < 0) {
            printf("failed to read\n");
            exit(3);
        }
        getc_buf_pos = 0;
    }
    return getc_buf[getc_buf_pos++];
}

int main(int argc, char ** argv) {
    char c;
    int quot = 0;
    bool slash = false;
    bool comment = false;
    bool prevS = false;
    
    is_interactive = isatty(0);
    init_shell(argc, argv);
    s_ss = s_rs = sizeof(char);
    s = (char*)malloc(sizeof(char));
    if (s == NULL) exit(3);
    s[0] = '\0';
    while(!feof(stdin)) {
        if (errno == EINTR) errno = 0;
        c = prgetc();
        if (comment) {
            prevS = false;
            if (c == '\n') {
                comment = false;
                exec_s();
            }
            continue;
        }
        if (slash) {
            prevS = false;
            if (c == '\n') {
                s[(s_ss/sizeof(char)) - 2] = ' ';
            } else push_back(&s, &s_ss, &s_rs, c);
            slash = false;
            continue;
        }
        if ((quot == type('\"') || quot == 0) && c == '$') {
           prevS = true; 
        }
        if (quot != 0) {
            prevS = false;
            push_back(&s, &s_ss, &s_rs, c);
            if (quot == type(c))
                quot = 0;
            continue;
        }
        if (c == '\n' || c == ';') {
            prevS = false;
            exec_s();
            continue;
        }
        if (type(c) != 0)
            quot = type(c);
        if (c == '\\')
            slash = true;
        if (c == '#' && !prevS) {
            comment = true;
            continue;
        }
        if (c != '$')
            prevS = false;
        push_back(&s, &s_ss, &s_rs, c);
    }
    exit_shell();
}
