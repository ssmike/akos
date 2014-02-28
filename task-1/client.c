#include "getss.h"

int main() {
    char * s;
    int c = getss(&s);
    printf("%d \"%s\"\n", c, s);
    getss(&s);
    printf("second line - \"%s\"\n", s);
    return 0; 
}
