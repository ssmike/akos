#include "getss.h"

int main() {
    char * s;
    int code;
    int c;
    c = getss(&s);
    printf("%d \"%s\"\n", c, s);

    if ((code = getss(&s)) < 0) {
        printf("%d\n", code);
        return 0;
    }
    printf("second line - \"%s\"\n", s);
    return 0; 
}
