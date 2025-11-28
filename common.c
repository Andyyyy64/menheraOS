#include "common.h"
#include "kernel.h"

// バイト単位でメモリをクリア
void *memset(void *ptr, int value, uint64_t size) {
    uint8_t *p = (uint8_t *)ptr;
    uint8_t val = (uint8_t)value;
    for (uint64_t i = 0; i < size; i++) {
        p[i] = val;
    }
    return ptr;
}

// バイト単位でメモリをコピー
void *memcpy(void *dest, const void *src, uint64_t size) {
    uint8_t *d = (uint8_t *)dest;
    const uint8_t *s = (const uint8_t *)src;
    for (uint64_t i = 0; i < size; i++) {
        d[i] = s[i];
    }
    return dest;
}

// NULL終端文字列の長さを返す
uint64_t strlen(const char *str) {
    uint64_t len = 0;
    while (str[len] != '\0') {
        len++;
    }
    return len;
}

// 文字列を比較し、等しければ0を返す
int strcmp(const char *s1, const char *s2) {
    while (*s1 && (*s1 == *s2)) {
        s1++;
        s2++;
    }
    return *(const uint8_t *)s1 - *(const uint8_t *)s2;
}

char *strcpy(char *dst, const char *src) {
    char *d = dst;
    while (*src)
        *d++ = *src++;
    *d = '\0';
    return dst;
}

// 簡易printf関数
void printf(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);

    const char *p = fmt;
    while (*p) {
        if (*p == '%') {
            p++;
            switch (*p) {
                case 'd': {
                    uint64_t ival = va_arg(args, uint64_t);
                    int val = (int)ival;
                    // int to str
                    char buf[32];
                    int i = 0, neg = 0;
                    if (val < 0) { neg = 1; val = -val; }
                    do {
                        buf[i++] = (val % 10) + '0';
                        val /= 10;
                    } while (val > 0);
                    if (neg) buf[i++] = '-';
                    while (--i >= 0) putchar(buf[i]);  // putcharはkernelで定義予定～～
                    break;
                }
                case 'x': {
                    uint64_t val = va_arg(args, uint64_t);
                    char buf[32];
                    int i = 0;
                    if (val == 0) { putchar('0'); break; }
                    while (val > 0) {
                        int digit = val % 16;
                        buf[i++] = digit < 10 ? '0' + digit : 'a' + digit - 10;
                        val /= 16;
                    }
                    while (--i >= 0) putchar(buf[i]);
                    break;
                }
                case 's': {
                    uint64_t sval = va_arg(args, uint64_t);
                    const char *str = (const char *)sval;
                    uint64_t len = strlen(str);
                    for (uint64_t i = 0; i < len; i++) {
                        putchar(str[i]);
                    }
                    break;
                }
                default:
                    putchar(*p);
                    break;
            }
            p++;
        } else {
            putchar(*p);
            p++;
        }
    }
    va_end(args);
}
