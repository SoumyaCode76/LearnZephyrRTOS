# QEMU STM32F429 Emulation

This folder is dedicated to running Zephyr RTOS applications on QEMU emulating the STM32F429 MCU.

## Board Target

For STM32F429 emulation in Zephyr, use one of these boards:
- `qemu_cortex_m4` - Generic Cortex-M4 board (closest to STM32F4)
- `stm32f429i_disc1` - Actual STM32F429I Discovery board (for hardware)

## Building Applications

```bash
# Navigate to your app directory
cd /workspaces/LearnZephyrRTOS/zephyrproject/apps/your_app

# Build for QEMU Cortex-M4
west build -b qemu_cortex_m4 .
```

## Running on QEMU

After building, run the application:

```bash
west build -t run
```

Or manually with QEMU:

```bash
qemu-system-arm -cpu cortex-m4 \
    -machine lm3s6965evb \
    -nographic \
    -kernel build/zephyr/zephyr.elf
```

## Copying Binaries Here

To organize your built binaries:

```bash
# Copy the ELF file
cp build/zephyr/zephyr.elf /workspaces/LearnZephyrRTOS/qemu_stm32f429/

# Copy the binary
cp build/zephyr/zephyr.bin /workspaces/LearnZephyrRTOS/qemu_stm32f429/

# Copy the hex file
cp build/zephyr/zephyr.hex /workspaces/LearnZephyrRTOS/qemu_stm32f429/
```

## Running Binaries from This Folder

```bash
cd /workspaces/LearnZephyrRTOS/qemu_stm32f429
qemu-system-arm -cpu cortex-m4 -machine lm3s6965evb -nographic -kernel zephyr.elf
```

## Exit QEMU

Press `Ctrl+A` then `X` to exit QEMU.
