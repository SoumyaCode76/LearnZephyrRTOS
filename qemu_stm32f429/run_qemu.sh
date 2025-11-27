#!/bin/bash
# Script to run a Zephyr binary on QEMU STM32F429 emulation

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

if [ -z "$1" ]; then
    echo "Usage: $0 <path_to_zephyr.elf>"
    echo "Example: $0 zephyr.elf"
    exit 1
fi

BINARY="$1"

if [ ! -f "$BINARY" ]; then
    echo "Error: Binary file '$BINARY' not found"
    exit 1
fi

echo "Starting QEMU with STM32 (Cortex-M3) emulation..."
echo "Binary: $BINARY"
echo "Press Ctrl+A then X to exit QEMU"
echo ""

qemu-system-arm \
    -cpu cortex-m3 \
    -machine lm3s6965evb \
    -nographic \
    -kernel "$BINARY"
