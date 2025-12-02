/* カーネルヘッダファイル: カーネル専用の構造体と定数定義 */
#include "common.h"

// 例外/トラップ関連定数
#define TRAP_CAUSE_ILLEGAL_INSTRUCTION 0x2
#define TRAP_CAUSE_ECALL_FROM_U 0x8
#define TRAP_CAUSE_ECALL_FROM_S 0x9
#define TRAP_CAUSE_PAGE_FAULT 0xc  // Load/Store/Instruction

// システムコール番号
#define SYS_PUTCHAR 1
#define SYS_GETCHAR 2
#define SYS_EXIT 3
#define SYS_YIELD 4

#define PROCS_MAX 8       // 最大プロセス数

#define PROC_UNUSED   0   // 未使用のプロセス管理構造体
#define PROC_RUNNABLE 1   // 実行可能なプロセス

struct process {
    int pid;             // プロセスID
    int state;           // プロセスの状態: PROC_UNUSED または PROC_RUNNABLE
    vaddr_t sp;          // コンテキストスイッチ時のスタックポインタ
    uint8_t stack[8192]; // カーネルスタック
};

// ページテーブルエントリ構造体 (Sv39)
#define PTE_V (1 << 0) // Valid
#define PTE_R (1 << 1) // Read
#define PTE_W (1 << 2) // Write
#define PTE_X (1 << 3) // Execute
#define PTE_U (1 << 4) // User
#define SATP_SV39 (8ULL << 60)
#define SATP_MAKE(ppn) (SATP_SV39 | (((uint64_t)ppn) >> 12))

typedef uint64_t pte_t;

// ページテーブル構造体
typedef pte_t *pagetable_t;  // 512エントリの配列を指すポインタ

struct sbiret {
    long error;
    long value;
};

struct trap_frame {
    uint64_t ra;
    uint64_t gp;
    uint64_t tp;
    uint64_t t0;
    uint64_t t1;
    uint64_t t2;
    uint64_t t3;
    uint64_t t4;
    uint64_t t5;
    uint64_t t6;
    uint64_t a0;
    uint64_t a1;
    uint64_t a2;
    uint64_t a3;
    uint64_t a4;
    uint64_t a5;
    uint64_t a6;
    uint64_t a7;
    uint64_t s0;
    uint64_t s1;
    uint64_t s2;
    uint64_t s3;
    uint64_t s4;
    uint64_t s5;
    uint64_t s6;
    uint64_t s7;
    uint64_t s8;
    uint64_t s9;
    uint64_t s10;
    uint64_t s11;
    uint64_t sp;
} __attribute__((packed));

#define READ_CSR(reg)                                                          \
    ({                                                                         \
        unsigned long __tmp;                                                   \
        __asm__ __volatile__("csrr %0, " #reg : "=r"(__tmp));                  \
        __tmp;                                                                 \
    })

#define WRITE_CSR(reg, value)                                                  \
    do {                                                                       \
        uint64_t __tmp = (value);                                              \
        __asm__ __volatile__("csrw " #reg ", %0" ::"r"(__tmp));                \
    } while (0)

#define PANIC(fmt, ...)                                                        \
    do {                                                                       \
        printf("PANIC: %s:%d: " fmt "\n", __FILE__, __LINE__, ##__VA_ARGS__);  \
        while (1) {}                                                           \
    } while (0)
