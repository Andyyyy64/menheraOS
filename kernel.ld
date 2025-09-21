/* RISC-V カーネル用リンカスクリプト */

OUTPUT_ARCH("riscv")
ENTRY(_start)

/* QEMU virt machine のメモリマップ:
 * 0x80000000 - DRAM 開始アドレス
 * OpenSBI は 0x80000000 から少し上の位置にカーネルをロードする
 */
MEMORY {
    /* カーネルを DRAM の 0x80200000 にロード（OpenSBI の後） */
    RAM (rwx) : ORIGIN = 0x80200000, LENGTH = 128M - 2M
}

SECTIONS {
    . = 0x80200000;
    
    /* テキストセクション（実行可能コード） */
    .text : {
        *(.text.start)  /* start.S のエントリポイント */
        *(.text)
        *(.text.*)
    } > RAM /* ramメモリ領域に配置 */
    
    /* 読み取り専用データ */
    .rodata : {
        *(.rodata)
        *(.rodata.*)
    } > RAM
    
    /* 初期化済みデータ */
    .data : {
        *(.data)
        *(.data.*)
    } > RAM
    
    /* BSS セクション（未初期化データ） */
    .bss : {
        __bss_start = .;
        *(.bss)
        *(.bss.*)
        *(COMMON)
        __bss_end = .;
    } > RAM
    
    /* カーネル終了位置 */
    __kernel_end = .;
    
    /* デバッグ情報（メモリには配置しない） */
    .debug_info 0 : { *(.debug_info) }
    .debug_abbrev 0 : { *(.debug_abbrev) }
    .debug_line 0 : { *(.debug_line) }
    .debug_str 0 : { *(.debug_str) }
    .debug_ranges 0 : { *(.debug_ranges) }
}
