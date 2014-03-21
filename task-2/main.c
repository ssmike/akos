#include <stdio.h>
#include "getss.h"
#include <stdbool.h>

int main(int argc, char ** argv) {
    char * s;
    bool found;
    int i, n, k, j, len;
    if (argc != 2 || sscanf(argv[1], "-%d", &k) != 1) {
        printf("invalid arguments\n");
        return 1;
    }
    n = getss(&s);
    if (n < 0) {
        printf("something went wrong\n");
        return 1;
    }
    for (i = 0; i < n;) {
        found = false;
        for (len = k; len > 0; len--) {
            if (i + len * 2 <= n) {
                found = true;
                for (j = 0; j < len; j++)
                     if (s[i + j] != s[i + len + j]) {
                         found = false;
                         break;
                     }
            }
            if (found) {
                i += 2 * len;
                break;
            }
        }
        if (!found) {
            printf("%c", s[i]);
            i++;
        }
    }
    return 0;
}
