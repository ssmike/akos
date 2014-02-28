#include "getss.h"
#include <stdlib.h>

int getss(char ** res) {
    return fgetss(stdin, res);
}

int getss_(char ** res, int lim) {
    return fgetss_(stdin, res, lim);
}

int fgetss(FILE * fin, char ** res) {
    return fgetss_(fin, res, GETSS_DEFAULT_LIM);
}

int fgetss_(FILE * fin, char ** res, int lim) {
    int sz = 0;
    int allsz = 1;
    char * buf = malloc(sizeof(char));
    int cchar;
    while (sz < lim && (cchar = fgetc(fin)) != '\n') {
       if (sz + 1 > allsz) {
           buf = realloc(buf, allsz * 2 * sizeof(char));
           allsz *= 2;
       }
       if (buf == 0) {
           return -1e+9;
       }
       buf[sz++] = cchar;
    }
    if (sz >= lim) {
        *res = buf;
        return -sz;
    }
    buf = realloc(buf, (sz + 1) * sizeof(char));
    buf[sz] = '\0';
    *res = buf;
    return sz;
}
