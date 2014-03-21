#include "getss.h"
#include "argparse.h"
#include <stdio.h>

int main() {
    char * s;
    if (getss(&s) >= 0) {
        int n, i;
        struct job * jb;
        char ** ss = parseCTokens(s, &n); 
        puts("tokens\n");
        printf("%d\n", n);
        for (i = 0; i < n; i++)
            puts(ss[i]);
        puts("command\n");
        /*
        struct command * cs = parse_command(ss, n);
        if (cs == NULL) {
            printf("%s\n", PARSE_ERROR_MESSAGE);
            return 0;
        }
        print_command_desc(cs);
        */
        jb = parse(s);
        if (jb == NULL) {
            printf("%s\n", PARSE_ERROR_MESSAGE);
            return 0;
        }
        print_job_desc(jb);
        
    }
    return 0;    
}
