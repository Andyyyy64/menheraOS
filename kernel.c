#include "kernel.h"
#include "common.h"

extern char __bss_start[], __bss_end[], __stack_top[];
extern char __free_ram[], __free_ram_end[];
extern char __kernel_base[]; // kernel.ld で定義が必要 (0x80200000)

struct process procs[PROCS_MAX];
struct process *current_proc; // 現在実行中のプロセス
struct process *idle_proc;    // アイドルプロセス

struct sbiret sbi_call(long arg0, long arg1, long arg2, long arg3, long arg4,
                       long arg5, long fid, long eid) {
    register long a0 __asm__("a0") = arg0;
    register long a1 __asm__("a1") = arg1;
    register long a2 __asm__("a2") = arg2;
    register long a3 __asm__("a3") = arg3;
    register long a4 __asm__("a4") = arg4;
    register long a5 __asm__("a5") = arg5;
    register long a6 __asm__("a6") = fid;
    register long a7 __asm__("a7") = eid;

    __asm__ __volatile__("ecall"
                         : "=r"(a0), "=r"(a1)
                         : "r"(a0), "r"(a1), "r"(a2), "r"(a3), "r"(a4), "r"(a5),
                           "r"(a6), "r"(a7)
                         : "memory");
    return (struct sbiret){.error = a0, .value = a1};
}

void putchar(char ch) {
    sbi_call(ch, 0, 0, 0, 0, 0, 0, 1 /* Console Putchar */);
}

paddr_t alloc_pages(uint32_t n) {
    static paddr_t next_paddr = (paddr_t) __free_ram;
    paddr_t paddr = next_paddr;
    next_paddr += n * PAGE_SIZE;

    if (next_paddr > (paddr_t) __free_ram_end)
        PANIC("out of memory");

    memset((void *) paddr, 0, n * PAGE_SIZE);
    return paddr;
}

void handle_trap(struct trap_frame *f) {
    uint64_t scause = READ_CSR(scause);
    uint64_t stval = READ_CSR(stval);
    uint64_t user_pc = READ_CSR(sepc);

    PANIC("unexpected trap scause=%lx, stval=%lx, sepc=%lx\n", scause, stval, user_pc);
}

__attribute__((naked))
__attribute__((aligned(4)))
void kernel_entry(void) {
    __asm__ __volatile__(
        "csrw sscratch, sp\n"
        "addi sp, sp, -8 * 31\n"
        "sd ra,  8 * 0(sp)\n"
        "sd gp,  8 * 1(sp)\n"
        "sd tp,  8 * 2(sp)\n"
        "sd t0,  8 * 3(sp)\n"
        "sd t1,  8 * 4(sp)\n"
        "sd t2,  8 * 5(sp)\n"
        "sd t3,  8 * 6(sp)\n"
        "sd t4,  8 * 7(sp)\n"
        "sd t5,  8 * 8(sp)\n"
        "sd t6,  8 * 9(sp)\n"
        "sd a0,  8 * 10(sp)\n"
        "sd a1,  8 * 11(sp)\n"
        "sd a2,  8 * 12(sp)\n"
        "sd a3,  8 * 13(sp)\n"
        "sd a4,  8 * 14(sp)\n"
        "sd a5,  8 * 15(sp)\n"
        "sd a6,  8 * 16(sp)\n"
        "sd a7,  8 * 17(sp)\n"
        "sd s0,  8 * 18(sp)\n"
        "sd s1,  8 * 19(sp)\n"
        "sd s2,  8 * 20(sp)\n"
        "sd s3,  8 * 21(sp)\n"
        "sd s4,  8 * 22(sp)\n"
        "sd s5,  8 * 23(sp)\n"
        "sd s6,  8 * 24(sp)\n"
        "sd s7,  8 * 25(sp)\n"
        "sd s8,  8 * 26(sp)\n"
        "sd s9,  8 * 27(sp)\n"
        "sd s10, 8 * 28(sp)\n"
        "sd s11, 8 * 29(sp)\n"

        "csrr a0, sscratch\n"
        "sd a0, 8 * 30(sp)\n"

        "mv a0, sp\n"
        "call handle_trap\n"

        "ld ra,  8 * 0(sp)\n"
        "ld gp,  8 * 1(sp)\n"
        "ld tp,  8 * 2(sp)\n"
        "ld t0,  8 * 3(sp)\n"
        "ld t1,  8 * 4(sp)\n"
        "ld t2,  8 * 5(sp)\n"
        "ld t3,  8 * 6(sp)\n"
        "ld t4,  8 * 7(sp)\n"
        "ld t5,  8 * 8(sp)\n"
        "ld t6,  8 * 9(sp)\n"
        "ld a0,  8 * 10(sp)\n"
        "ld a1,  8 * 11(sp)\n"
        "ld a2,  8 * 12(sp)\n"
        "ld a3,  8 * 13(sp)\n"
        "ld a4,  8 * 14(sp)\n"
        "ld a5,  8 * 15(sp)\n"
        "ld a6,  8 * 16(sp)\n"
        "ld a7,  8 * 17(sp)\n"
        "ld s0,  8 * 18(sp)\n"
        "ld s1,  8 * 19(sp)\n"
        "ld s2,  8 * 20(sp)\n"
        "ld s3,  8 * 21(sp)\n"
        "ld s4,  8 * 22(sp)\n"
        "ld s5,  8 * 23(sp)\n"
        "ld s6,  8 * 24(sp)\n"
        "ld s7,  8 * 25(sp)\n"
        "ld s8,  8 * 26(sp)\n"
        "ld s9,  8 * 27(sp)\n"
        "ld s10, 8 * 28(sp)\n"
        "ld s11, 8 * 29(sp)\n"
        "ld sp,  8 * 30(sp)\n"
        "sret\n"
    );
}

__attribute__((naked)) void switch_context(uint64_t *prev_sp,
                                           uint64_t *next_sp) {
    __asm__ __volatile__(
        // 実行中プロセスのスタックへレジスタを保存
        "addi sp, sp, -13 * 8\n"
        "sd ra,  0  * 8(sp)\n"
        "sd s0,  1  * 8(sp)\n"
        "sd s1,  2  * 8(sp)\n"
        "sd s2,  3  * 8(sp)\n"
        "sd s3,  4  * 8(sp)\n"
        "sd s4,  5  * 8(sp)\n"
        "sd s5,  6  * 8(sp)\n"
        "sd s6,  7  * 8(sp)\n"
        "sd s7,  8  * 8(sp)\n"
        "sd s8,  9  * 8(sp)\n"
        "sd s9,  10 * 8(sp)\n"
        "sd s10, 11 * 8(sp)\n"
        "sd s11, 12 * 8(sp)\n"

        // スタックポインタの切り替え
        "sd sp, (a0)\n"
        "ld sp, (a1)\n"

        // 次のプロセスのスタックからレジスタを復元
        "ld ra,  0  * 8(sp)\n"
        "ld s0,  1  * 8(sp)\n"
        "ld s1,  2  * 8(sp)\n"
        "ld s2,  3  * 8(sp)\n"
        "ld s3,  4  * 8(sp)\n"
        "ld s4,  5  * 8(sp)\n"
        "ld s5,  6  * 8(sp)\n"
        "ld s6,  7  * 8(sp)\n"
        "ld s7,  8  * 8(sp)\n"
        "ld s8,  9  * 8(sp)\n"
        "ld s9,  10 * 8(sp)\n"
        "ld s10, 11 * 8(sp)\n"
        "ld s11, 12 * 8(sp)\n"
        "addi sp, sp, 13 * 8\n"
        "ret\n"
    );
}

void map_page(pagetable_t table, vaddr_t vaddr, paddr_t paddr, uint64_t flags) {
    if (!is_aligned(vaddr, PAGE_SIZE))
        PANIC("unaligned vaddr %x", vaddr);
    if (!is_aligned(paddr, PAGE_SIZE))
        PANIC("unaligned paddr %x", paddr);

    pte_t *pte = &table[(vaddr >> 30) & 0x1ff]; // Level 2 (1GB)
    if (!(*pte & PTE_V)) {
        // 新しいページテーブルを確保して、テーブルを設定する
        paddr_t pt_paddr = alloc_pages(1);
        *pte = ((pt_paddr / PAGE_SIZE) << 10) | PTE_V;
    }

    pagetable_t table1 = (pagetable_t) ((*pte >> 10) * PAGE_SIZE);
    pte = &table1[(vaddr >> 21) & 0x1ff]; // Level 1 (2MB)
    if (!(*pte & PTE_V)) {
        // 新しいページテーブルを確保して、テーブルを設定する
        paddr_t pt_paddr = alloc_pages(1);
        *pte = ((pt_paddr / PAGE_SIZE) << 10) | PTE_V;
    }

    pagetable_t table0 = (pagetable_t) ((*pte >> 10) * PAGE_SIZE);
    pte = &table0[(vaddr >> 12) & 0x1ff]; // Level 0 (4KB)
    *pte = ((paddr / PAGE_SIZE) << 10) | flags | PTE_V;
}

struct process *create_process(uint64_t pc) {
    // 空いているプロセス管理構造体を探す
    struct process *proc = NULL;
    int i;
    for (i = 0; i < PROCS_MAX; i++) {
        if (procs[i].state == PROC_UNUSED) {
            proc = &procs[i];
            break;
        }
    }

    if (!proc)
        PANIC("no free process slots");

    // switch_context() で復帰できるように、スタックに呼び出し先保存レジスタを積む
    uint64_t *sp = (uint64_t *) &proc->stack[sizeof(proc->stack)];
    *--sp = 0;                      // s11
    *--sp = 0;                      // s10
    *--sp = 0;                      // s9
    *--sp = 0;                      // s8
    *--sp = 0;                      // s7
    *--sp = 0;                      // s6
    *--sp = 0;                      // s5
    *--sp = 0;                      // s4
    *--sp = 0;                      // s3
    *--sp = 0;                      // s2
    *--sp = 0;                      // s1
    *--sp = 0;                      // s0
    *--sp = (uint64_t) pc;          // ra

    // 各フィールドを初期化
    proc->pid = i + 1;
    proc->state = PROC_RUNNABLE;
    proc->sp = (uint64_t) sp;
    return proc;
}

void yield(void) {
    // 実行可能なプロセスを探す
    struct process *next = idle_proc;
    for (int i = 0; i < PROCS_MAX; i++) {
        struct process *proc = &procs[(current_proc->pid + i) % PROCS_MAX];
        if (proc->state == PROC_RUNNABLE && proc->pid > 0) {
            next = proc;
            break;
        }
    }

    // 現在実行中のプロセス以外に、実行可能なプロセスがない。戻って処理を続行する
    if (next == current_proc)
        return;

    // カーネルスタックの切り替え (例外ハンドラ用)
    __asm__ __volatile__(
        "csrw sscratch, %[sscratch]\n"
        :
        : [sscratch] "r" ((uint64_t) &next->stack[sizeof(next->stack)])
    );

    // コンテキストスイッチ
    struct process *prev = current_proc;
    current_proc = next;
    switch_context(&prev->sp, &next->sp);
}

void delay(void) {
    for (int i = 0; i < 30000000; i++)
        __asm__ __volatile__("nop"); // 何もしない命令
}

struct process *proc_a;
struct process *proc_b;

void proc_a_entry(void) {
    printf("starting process A\n");
    while (1) {
        putchar('A');
        yield();
        delay();
    }
}

void proc_b_entry(void) {
    printf("starting process B\n");
    while (1) {
        putchar('B');
        yield();
        delay();
    }
}

void kmain(void) {
    // start.S でBSSクリア済みだが、念のため
    memset(__bss_start, 0, (size_t) __bss_end - (size_t) __bss_start);

    WRITE_CSR(stvec, (uint64_t) kernel_entry);

    // カーネルページテーブルの構築
    pagetable_t kernel_pagetable = (pagetable_t) alloc_pages(1);
    
    // カーネルコード・データ領域をマッピング (0x80200000 ~ __free_ram_end)
    // 物理アドレス == 仮想アドレス (Identity Mapping)
    for (uint64_t p = (uint64_t) __kernel_base; p < (uint64_t) __free_ram_end; p += PAGE_SIZE) {
        map_page(kernel_pagetable, p, p, PTE_R | PTE_W | PTE_X);
    }
    
    // MMIO領域 (VirtIO, UART, PLIC, CLINT) をマッピング
    // QEMU virt machine のメモリマップに基づく
    // 0x10000000 (UART)
    map_page(kernel_pagetable, 0x10000000, 0x10000000, PTE_R | PTE_W);
    // 0x02000000 (CLINT) - 64KB
    for (uint64_t p = 0x02000000; p < 0x02000000 + 0x10000; p += PAGE_SIZE) {
        map_page(kernel_pagetable, p, p, PTE_R | PTE_W);
    }
    // 0x0c000000 (PLIC) - 4MB (0x400000)
    for (uint64_t p = 0x0c000000; p < 0x0c000000 + 0x400000; p += PAGE_SIZE) {
        map_page(kernel_pagetable, p, p, PTE_R | PTE_W);
    }
    // 0x10001000 (VirtIO) - 4KB * 8 (0x8000)
    for (uint64_t p = 0x10001000; p < 0x10001000 + 0x8000; p += PAGE_SIZE) {
        map_page(kernel_pagetable, p, p, PTE_R | PTE_W);
    }

    // ページング有効化 (satp設定)
    WRITE_CSR(satp, SATP_MAKE((uint64_t)kernel_pagetable));
    __asm__ __volatile__("sfence.vma");

    printf("\n\nHello World from S-mode! (Paging Enabled)\n");

    idle_proc = create_process((uint64_t) NULL);
    idle_proc->pid = 0; // idle
    current_proc = idle_proc;

    proc_a = create_process((uint64_t) proc_a_entry);
    proc_b = create_process((uint64_t) proc_b_entry);

    yield();
    PANIC("switched to idle process");
}
