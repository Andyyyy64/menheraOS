/* 共通ヘッダファイル: カーネルとユーザー空間で共有される基本定義 */

// 基本型定義 (RISC-V 64-bit 向け)
typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int uint32_t;
typedef unsigned long uint64_t;
typedef char int8_t;
typedef short int16_t;
typedef int int32_t;
typedef long int64_t;
typedef uint64_t uintptr_t;
typedef int64_t intptr_t;
typedef uint64_t size_t;

// NULL定義
#define NULL ((void*)0)

// 真偽値定義
typedef uint8_t bool;
#define true 1
#define false 0

// ページサイズ定数 (RISC-V Sv39)
#define PAGE_SIZE 4096
#define PAGE_SHIFT 12

// 仮想アドレス関連定数
#define USER_BASE 0x40000000  // ユーザー空間の仮想アドレスベース
#define KERNEL_BASE 0x80000000  // カーネル空間の仮想アドレスベース

#define va_list  __builtin_va_list
#define va_start __builtin_va_start
#define va_end   __builtin_va_end
#define va_arg   __builtin_va_arg

typedef uint64_t paddr_t;
typedef uint64_t vaddr_t;

#define align_up(value, align)   __builtin_align_up(value, align)
#define is_aligned(value, align) __builtin_is_aligned(value, align)
#define offsetof(type, member)   __builtin_offsetof(type, member)

void *memset(void *buf, int c, size_t n);
void *memcpy(void *dst, const void *src, size_t n);
char *strcpy(char *dst, const char *src);
int strcmp(const char *s1, const char *s2);
void printf(const char *fmt, ...);
void putchar(char ch);
