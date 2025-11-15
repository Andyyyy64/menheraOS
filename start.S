# OpenSBI から制御を受け取り、カーネルを起動する
#
.section .text.start
.global _start

_start:
    # OpenSBI から a0=hartid, a1=DTB が渡される
    # スタックポインタを設定
    la sp, __stack_top
    
    # .bss セクションをゼロクリア
    la t0, __bss_start
    la t1, __bss_end
clear_bss:
    beq t0, t1, clear_bss_done
    sw zero, 0(t0)
    addi t0, t0, 4
    j clear_bss
clear_bss_done:

    # トラップベクタを設定: DIRECTモードでtrap_handlerを登録
    la t0, trap_handler
    csrw stvec, t0  # stvec = trap_handler のアドレス

    # カーネルメイン関数を呼び出し
    # a0, a1 はそのまま渡す
    call kmain
    
    # カーネルが戻ってきた場合は無限ループ
halt:
    wfi
    j halt

# トラップハンドラの定義（kernel.cからリンクされる）
.global trap_handler
trap_handler:
    # ここはkernel.cの関数にジャンプ（アセンブリで直接実装せずC関数呼び出し）
    jal ra, trap_handler  # C側のtrap_handlerを呼び出し（ただし今はC関数なので調整）
    # 注意: 実際はC関数なので、call trap_handler で呼び出し、戻ったらsret

.section .bss
.align 12
__stack_bottom:
    .space 4096 * 4  # 16KB のスタック
__stack_top:
