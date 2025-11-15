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
