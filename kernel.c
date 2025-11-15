#include <stdint.h>
#include <stddef.h>

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
    
    printf("System halted.\n");
    
    // 無限ループ
    while (1) {
        asm volatile("wfi");
    }
}
