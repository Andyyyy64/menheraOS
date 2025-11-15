/* カーネルヘッダファイル: カーネル専用の構造体と定数定義 */

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

// プロセス状態
typedef enum {
    PROC_STATE_READY,
    PROC_STATE_RUNNING,
    PROC_STATE_SLEEPING,
    PROC_STATE_ZOMBIE
} proc_state_t;

// プロセス構造体 (将来の拡張用)
typedef struct process {
    int pid;
    proc_state_t state;
    uint64_t context[32];  // レジスタコンテキスト (簡易)
    uint64_t *page_table;  // ページテーブルルート
    uint64_t stack_base;   // スタックベース
    uint64_t stack_size;
} process_t;

// ページテーブルエントリ構造体 (Sv39)
typedef union pte {
    uint64_t bits;
    struct {
        uint64_t v : 1;    // Valid
        uint64_t r : 1;    // Read
        uint64_t w : 1;    // Write
        uint64_t x : 1;    // Execute
        uint64_t u : 1;    // User
        uint64_t g : 1;    // Global
        uint64_t a : 1;    // Accessed
        uint64_t d : 1;    // Dirty
        uint64_t reserved : 44;
        uint64_t ppn : 26; // Physical Page Number
    } fields;
} pte_t;

// ページテーブル構造体
typedef uint64_t pte_t *pagetable_t;  // 512エントリの配列を指すポインタ
