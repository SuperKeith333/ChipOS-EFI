#include <stddef.h>
#include <stdint.h>

__attribute__((noinline))
int memcmp(const void* a, const void* b, size_t n) {
    const uint8_t* x = a;
    const uint8_t* y = b;
    while (n--) {
        if (*x != *y)
            return *x - *y;
        x++;
        y++;
    }
    return 0;
}

char* strchr(const char* s, int c) {
    char ch = (char)c;
    while (*s) {
        if (*s == ch)
            return (char*)s;
        s++;
    }
    return ch == '\0' ? (char*)s : NULL;
}

__attribute__((optimize("no-tree-loop-distribute-patterns"), noinline))
char* strcat(char* dst, const char* src) {
    char* d = dst;
    while (*d) d++;                 // find the end of dst
    while ((*d++ = *src++));        // copy src including the terminator
    return dst;
}

__attribute__((optimize("no-tree-loop-distribute-patterns"), noinline))
char* strncat(char* dst, const char* src, size_t n) {
    char* d = dst;
    while (*d) d++;
    while (n && *src) {             // copy at most n chars
        *d++ = *src++;
        n--;
    }
    *d = '\0';                      // strncat always terminates
    return dst;
}

__attribute__((optimize("no-tree-loop-distribute-patterns"), noinline))
char* strcpy(char* dst, const char* src) {
    char* d = dst;
    while ((*d++ = *src++));
    return dst;
}