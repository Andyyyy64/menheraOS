/* RISC-V CSRレジスタ定数 */
#define CSR_STVEC    0x141
#define CSR_SSTATUS  0x100
#define CSR_SCAUSE   0x142
#define CSR_SEPC     0x141
#define CSR_STVAL    0x143

/* トラップ原因（scause） */
#define TRAP_CAUSE_ENV_CALL_SMODE 8  // ecall from S-mode (テスト用)
#define TRAP_CAUSE_ILLEGAL_INSTR 2

/* トラップコンテキスト構造体: ユーザからカーネルへのコンテキスト保存 */
typedef struct trap_context {
    uint64_t ra;     // 戻りアドレス
    uint64_t sp;     // スタックポインタ
    uint64_t gp;     // グローバルポインタ
    uint64_t tp;     // スレッドポインタ
    uint64_t t0, t1, t2;  // テンポラリレジスタ
    uint64_t s0, s1;      // 保存レジスタ
    uint64_t a0, a1, a2, a3, a4, a5, a6, a7;  // 引数/戻り値レジスタ
    uint64_t s2, s3, s4, s5, s6, s7, s8, s9;  // 保存レジスタ
    uint64_t s10, s11;                        // 保存レジスタ
    uint64_t t3, t4, t5, t6;                  // テンポラリレジスタ
} trap_context_t;

/* モードフラグ */
#define SSTATUS_SPP (1UL << 8)   // Supervisor Previous Privilege
#define SSTATUS_SPIE (1UL << 5)  // S Previous Interrupt Enable
