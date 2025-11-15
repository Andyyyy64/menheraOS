#include <stdint.h>
#include <stddef.h>
#include "common.h"  // 共通ヘッダ追加

#define UART_BASE 0x10000000 // UARTのベースアドレス QEMUのvirt machineでは0x10000000だよ
#define UART_THR  (UART_BASE + 0)  // Transmit Holding Register 送信データレジスタ
#define UART_LSR  (UART_BASE + 5)  // Line Status Register 送信可能か確認
#define UART_LSR_THRE (1 << 5)     // Transmit Holding Register Empty 送信可能か確認

// MMIO 読み書き関数
static inline uint8_t mmio_read8(uintptr_t addr) {
    return *(volatile uint8_t *)addr; // コンパイラの最適化を防ぎつつ64bitで読み込む
}

static inline void mmio_write8(uintptr_t addr, uint8_t value) {
    *(volatile uint8_t *)addr = value;
}

void uart_init(void) {
    // QEMU の virtmachine では UART は既に初期化済みだから特別な処理はまだいらない
}

// uartの送信は非同期なので送信レジスタからのポーリングでやる
void uart_putchar(char c) {
    // 送信レジスタが空になるまで待機
    while ((mmio_read8(UART_LSR) & UART_LSR_THRE) == 0) {
        // たいき
    }
    mmio_write8(UART_THR, c);
}

// 文字列出力,大体CRLFを期待している
void uart_puts(const char *s) {
    while (*s) {
        if (*s == '\n') {
            uart_putchar('\r');  // CRLF
        }
        uart_putchar(*s);
        s++;
    }
}

// printfもどき
void printf(const char *fmt, ...) {
    uart_puts(fmt);
}

// トラップハンドラ: Sモードからのトラップ処理
void trap_handler(void) {
    uint64_t scause, stval, sepc;
    
    // CSRから原因と値を取得
    asm volatile (
        "csrr %0, scause\n"
        "csrr %1, stval\n"
        "csrr %2, sepc\n"
        : "=r"(scause), "=r"(stval), "=r"(sepc)
    );
    
    if (scause == TRAP_CAUSE_ENV_CALL_SMODE) {
        // ecall from S-mode: 簡単なシステムコール扱い（今はテスト）
        printf("Trap: ecall from S-mode detected! (a0 = %x)\n", a0);  // a0を引数として表示（暫定）
        
        // sepcを+4してecall命令をスキップ（復帰時次へ）
        sepc += 4;
        asm volatile("csrw sepc, %0" : : "r"(sepc));
        
        // ここで本格的なシステムコール処理を追加予定
    } else if (scause == TRAP_CAUSE_ILLEGAL_INSTR) {
        printf("Trap: Illegal instruction at 0x%x (stval=0x%x)\n", sepc, stval);
        // エラー処理: ハルト
        while(1) asm volatile("wfi");
    } else {
        printf("Unknown trap: scause=0x%x, stval=0x%x\n", scause, stval);
        while(1) asm volatile("wfi");
    }
    
    // ユーザモード復帰フラグを設定（SPIE=1で中断有効、SPP=0でUモード復帰）
    uint64_t sstatus;
    asm volatile("csrr %0, sstatus" : "=r"(sstatus));
    sstatus |= SSTATUS_SPIE;  // 中断有効
    sstatus &= ~SSTATUS_SPP;  // Uモードへ（ただし今はSモードテスト）
    asm volatile("csrw sstatus, %0" : : "r"(sstatus));
    
    // sretで復帰
    asm volatile("sret");
}

// カーネルのエントリポイント、start.Sから呼び出されるよ
void kmain(uint64_t hartid, void *dtb) {
    uart_init();
    
    printf("RISC-V Kernel\n");
    printf("Hello World from S-mode!\n");

    printf("Hart ID: ");
    // hartid を16進数で表示
    char hex[] = "0123456789abcdef";
    for (int i = 15; i >= 0; i--) {
        uart_putchar(hex[(hartid >> (i * 4)) & 0xf]);
    }
    printf("\n");
    
    printf("DTB Address: ");
    uintptr_t dtb_addr = (uintptr_t)dtb;
    for (int i = 15; i >= 0; i--) {
        uart_putchar(hex[(dtb_addr >> (i * 4)) & 0xf]);
    }
    printf("\n");
    
    // テスト: ecallでトラップ誘発
    printf("Testing trap handler with ecall...\n");
    asm volatile("ecall");  // Sモードからecall: トラップ発生
    
    printf("System halted.\n");  // ここには到達しないはず
    
    // 無限ループ
    while (1) {
        asm volatile("wfi");
    }
}
