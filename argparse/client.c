#include "getss.h"
#include "argparse.h"
#include <stdio.h>

int main() {
    char * s;
    if (getss(&s) >= 0) {
        int n, i;
        char ** ss = parseCTokens(s, &n); 
        printf("%d\n", n);
        for (i = 0; i < n; i++)
            puts(ss[i]);
    }
    return 0;    
}
