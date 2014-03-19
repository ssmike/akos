#include <getss.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>

int main() {
    char * or;
    char * pt;
    char * tar;
    int orl, ptl;
    int i, j;
    if (!getss(&or) || !getss(&pt) || !getss(&tar)) {
        fprintf(stderr, "Weird error");
        return 1;
    }
    orl = strlen(or);
    ptl = strlen(pt);
    for (i = 0; i < orl;) {
        bool fl = true;
        if (i + ptl > orl)
            fl = false;
        else {
            for (j = 0; j < ptl; j++) {
                if (pt[j] != or[i + j]) {
                    fl = false;
                    break;
                }
            }
        }
        if (fl) {
            fprintf(stdout, "%s", tar);
            i += ptl;
        } else {
            fprintf(stdout, "%c", or[i]);
            i++;
        }
    }
}
