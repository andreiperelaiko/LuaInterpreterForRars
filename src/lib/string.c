#include "lib/string.h"

unsigned int strlen(const char* s){
    unsigned int n = 0;
    while (s[n]) n++;
    return n;
}

int str_eq(const char* a, const char* b){
    if (a == 0 && b == 0) return 1;
    if (a == 0 || b == 0) return 0;
    while (*a && *b){
        if (*a != *b) return 0;
        a++; b++;
    }
    return *a == *b;
}
