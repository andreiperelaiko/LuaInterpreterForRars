#include "lib/string.h"
#include "lib/memory.h"

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

char* str_concat(const char* a, const char* b){
    unsigned int lena = strlen(a);
    unsigned int lenb = strlen(b);
    unsigned int len = lena + lenb;
    char* buffer = malloc(sizeof(*buffer) * (len + 1));
    for (int i = 0; i < lena; i++){
        buffer[i] = a[i];
    }
    for (int i = 0; i < lenb; i++){
        buffer[lena + i] = b[i];
    }
    buffer[len] = '\0';
    return buffer;
}