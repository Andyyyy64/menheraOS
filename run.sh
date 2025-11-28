#!/bin/bash

set -e

CROSS_PREFIX="riscv64-unknown-elf-"
CC="${CROSS_PREFIX}gcc"
OBJCOPY="${CROSS_PREFIX}objcopy"
OBJDUMP="${CROSS_PREFIX}objdump"

BUILD_DIR="build"
DISK_DIR="disk"

CFLAGS="-Wall -Wextra -O2 -g -ffreestanding -fno-common -nostdlib -mno-relax"
CFLAGS="$CFLAGS -march=rv64gc -mabi=lp64d -mcmodel=medany -Iinclude"
LDFLAGS="-nostdlib -Wl,--gc-sections"
LDFLAGS="-nostdlib -Wl,--gc-sections"

mkdir -p "$BUILD_DIR"

echo "Building kernel..."

echo "  Compiling start.S..."
$CC $CFLAGS -c start.S -o "$BUILD_DIR/start.o"

echo "  Compiling kernel.c..."
$CC $CFLAGS -c kernel.c -o "$BUILD_DIR/kernel.o"

echo "  Compiling common.c..."
$CC $CFLAGS -c common.c -o "$BUILD_DIR/common.o"

echo "  Linking kernel..."
$CC $LDFLAGS -T kernel.ld "$BUILD_DIR/start.o" "$BUILD_DIR/kernel.o" "$BUILD_DIR/common.o" -o "$BUILD_DIR/kernel.elf"

echo "  Generating disassembly..."
$OBJDUMP -d "$BUILD_DIR/kernel.elf" > "$BUILD_DIR/kernel.dis"

echo "Kernel build complete!"
ls -la "$BUILD_DIR/kernel.elf"

if [ "$1" == "run" ]; then
    echo "Starting QEMU..."
    echo "Press Ctrl+A, X to exit QEMU"
    echo ""
    
    qemu-system-riscv64 \
        -M virt \
        -m 128M \
        -nographic \
        -bios default \
        -kernel "$BUILD_DIR/kernel.elf"
elif [ "$1" == "debug" ]; then
    echo "Starting QEMU with GDB server..."
    echo "Connect with: ${CROSS_PREFIX}gdb $BUILD_DIR/kernel.elf"
    echo "Then in GDB: target remote :1234"
    echo ""
    
    qemu-system-riscv64 \
        -M virt \
        -m 128M \
        -nographic \
        -bios default \
        -kernel "$BUILD_DIR/kernel.elf" \
        -S -s
else
    echo ""
    echo "Usage:"
    echo "  $0        - Build only"
    echo "  $0 run    - Build and run"
    echo "  $0 debug  - Build and run with GDB server"
fi
