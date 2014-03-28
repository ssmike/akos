#include "getss.h"
#include "argparse.h"
#include <stdio.h>

int main() {
    char * s;
    int ii = 0;
    while (!feof(stdin)) {
        int n, i;
        struct job * jb;
        char ** ss; 
        if (getss(&s) < 0) continue;
        ss = parseCTokens(s, &n);
        puts("tokens\n");
        printf("%d\n", n);
        for (i = 0; i < n; i++)
            puts(ss[i]);
        printf("\n----job %d----\n", ii++);
        jb = parse(s);
        if (jb == NULL) {
            printf("%s\n", PARSE_ERROR_MESSAGE);
            return 0;
        }
        print_job_desc(jb);
        
    }
    return 0;    
}
