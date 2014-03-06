#include "getss.h"
#include <stdlib.h>

/*TODO
 * 
 * Можно было бы и макросами сделать
 * наверно.  
 */

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
    if (feof(stdin)) {
        return EOF_ERROR;
    }
    *res = NULL;
    int sz = 0;
    int allsz = 1;
    char * buf = malloc(sizeof(char)), * tmp;
    if (buf == NULL) return ALLOCATION_FAILED;
    int cchar;
    while (sz < lim && (cchar = fgetc(fin)) != '\n'&& !feof(fin)) {
        if (sz + 1 > allsz) {
            tmp = realloc(buf, allsz * 2 * sizeof(char));
            if (tmp == NULL) {
                *res = buf;
                return ALLOCATION_FAILED;
            }
            allsz *= 2;
        } 
        buf[sz++] = cchar;
    }
    if (sz >= lim) {
        *res = buf;
        return LIMIT_EXCEEDED;
    }
    buf = realloc(buf, (sz + 1) * sizeof(char));
    if (tmp == NULL) {
        *res = buf;
        return ALLOCATION_FAILED;
    }
    buf[sz] = '\0';
    *res = buf;
    return sz;
}
