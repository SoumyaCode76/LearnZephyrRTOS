# Zephyr RTOS Project Configuration Guide (prj.conf)

## Table of Contents
1. [Introduction](#introduction)
2. [Understanding prj.conf](#understanding-prjconf)
3. [Configuration Syntax](#configuration-syntax)
4. [Discovering Available CONFIG Options](#discovering-available-config-options)
5. [Common Configuration Categories](#common-configuration-categories)
6. [Practical Examples](#practical-examples)
7. [Configuration Priority](#configuration-priority)
8. [Best Practices](#best-practices)
9. [Troubleshooting](#troubleshooting)

---

## Introduction

The `prj.conf` file is the **project configuration file** for Zephyr RTOS applications. It uses Kconfig syntax to enable/disable features, set parameters, and configure the build system.

### Purpose
- Enable/disable kernel features
- Configure drivers and subsystems
- Set memory allocations
- Control logging and debugging
- Optimize for size or performance

---

## Understanding prj.conf

### File Location
```
my_app/
├── CMakeLists.txt
├── prj.conf           ← Your configuration file
├── app.overlay
└── src/
    └── main.c
```

### Basic Structure
```conf
# Comments start with #

# Enable a feature
CONFIG_FEATURE=y

# Disable a feature
CONFIG_FEATURE=n

# Set a numeric value
CONFIG_HEAP_MEM_POOL_SIZE=2048

# Set a string value
CONFIG_BT_DEVICE_NAME="MyDevice"
```

---

## Configuration Syntax

### Boolean Options
```conf
CONFIG_GPIO=y          # Enable GPIO driver
CONFIG_SPI=n           # Disable SPI driver
```

### Integer Options
```conf
CONFIG_MAIN_STACK_SIZE=2048
CONFIG_HEAP_MEM_POOL_SIZE=4096
CONFIG_LOG_BUFFER_SIZE=1024
```

### Hexadecimal Values
```conf
CONFIG_SRAM_BASE_ADDRESS=0x20000000
```

### String Options
```conf
CONFIG_BOARD="nucleo_f429zi"
CONFIG_BT_DEVICE_NAME="Zephyr Device"
```

### Tristate Options (y/n/m)
```conf
# y = built-in, n = disabled, m = module (rare in Zephyr)
CONFIG_NETWORKING=y
```

---

## Discovering Available CONFIG Options

### Method 1: Search Kconfig Files Directly

#### Find All Kconfig Files
```bash
# From Zephyr base directory
find $ZEPHYR_BASE -name "Kconfig*" -type f
```

#### Search for Specific CONFIG Options
```bash
# Search for GPIO-related configs
grep -r "config GPIO" $ZEPHYR_BASE --include="Kconfig*"

# Search for UART configs
grep -r "config UART" $ZEPHYR_BASE --include="Kconfig*"

# Search for specific config definition
grep -r "config MAIN_STACK_SIZE" $ZEPHYR_BASE --include="Kconfig*"
```

#### Example Output
```bash
$ grep -r "config MAIN_STACK_SIZE" $ZEPHYR_BASE --include="Kconfig*"
kernel/Kconfig:config MAIN_STACK_SIZE
kernel/Kconfig:         int "Size of stack for initialization and main thread"
```

### Method 2: Use Zephyr Documentation

#### Online Documentation
```
https://docs.zephyrproject.org/latest/kconfig.html
```

#### Generate Local Documentation
```bash
cd $ZEPHYR_BASE
cmake -B build
ninja -C build kconfig-html
# Open build/kconfig/html/index.html
```

### Method 3: Examine Board-Specific Configs

#### Check Board Defconfig
```bash
# Find your board's defconfig
find $ZEPHYR_BASE/boards -name "*defconfig" | grep your_board

# View the defconfig
cat $ZEPHYR_BASE/boards/vendor/board/board_defconfig
```

#### Example: STM32 Nucleo F429ZI
```bash
cat $ZEPHYR_BASE/boards/st/nucleo_f429zi/nucleo_f429zi_defconfig
```

### Method 4: Search by Feature Category

#### Common Kconfig Locations
```bash
# Kernel configurations
cat $ZEPHYR_BASE/kernel/Kconfig

# Driver configurations
ls $ZEPHYR_BASE/drivers/*/Kconfig

# Subsystem configurations
ls $ZEPHYR_BASE/subsys/*/Kconfig

# Architecture-specific
cat $ZEPHYR_BASE/arch/arm/Kconfig
```

### Method 5: Use grep with Context

#### Find Config with Description
```bash
# Show 5 lines after the config declaration
grep -A 5 "config UART" $ZEPHYR_BASE/drivers/serial/Kconfig
```

#### Example Output
```
config UART_CONSOLE
    bool "Console over UART"
    depends on UART_DRIVER
    select CONSOLE
    help
      Enable console over UART driver.
```

### Method 6: Check Build Output

#### View Final Configuration
```bash
# After building your project
west build -b your_board

# View the complete configuration
cat build/zephyr/.config

# Search for specific configs
grep "CONFIG_GPIO" build/zephyr/.config
```

### Method 7: Use west Commands

#### List All Kconfig Options
```bash
# Set ZEPHYR_BASE environment variable
export ZEPHYR_BASE=/path/to/zephyr

# Use Python to parse Kconfig
python3 -c "
import sys
sys.path.insert(0, '$ZEPHYR_BASE/scripts/kconfig')
import kconfiglib
kconf = kconfiglib.Kconfig()
for sym in kconf.syms.values():
    if sym.name.startswith('CONFIG_'):
        print(sym.name)
"
```

### Method 8: Search by Dependency

#### Find What Depends on a Config
```bash
# Find configs that depend on GPIO
grep -r "depends on GPIO" $ZEPHYR_BASE --include="Kconfig*"

# Find what selects a config
grep -r "select UART_CONSOLE" $ZEPHYR_BASE --include="Kconfig*"
```

### Method 9: Platform-Specific Search

#### STM32-Specific Configs
```bash
grep -r "config STM32" $ZEPHYR_BASE/soc/st --include="Kconfig*"
```

#### Nordic-Specific Configs
```bash
grep -r "config NRF" $ZEPHYR_BASE/soc/nordic --include="Kconfig*"
```

### Method 10: Use VS Code Search

If working in VS Code:
```
1. Press Ctrl+Shift+F (or Cmd+Shift+F on Mac)
2. Search for: "config YOUR_FEATURE"
3. Include: **/Kconfig*
4. Exclude: **/build/**
```

---

## Common Configuration Categories

### 1. **Kernel Configurations**

#### Thread and Stack Settings
```conf
# Main thread stack size
CONFIG_MAIN_STACK_SIZE=2048

# Idle thread stack size
CONFIG_IDLE_STACK_SIZE=512

# ISR stack size
CONFIG_ISR_STACK_SIZE=2048

# Number of thread priorities
CONFIG_NUM_PREEMPT_PRIORITIES=15
```

#### Memory Management
```conf
# Enable heap memory pool
CONFIG_HEAP_MEM_POOL_SIZE=4096

# Enable memory protection
CONFIG_MEMORY_PROTECTION=y

# Stack canaries for overflow detection
CONFIG_STACK_CANARIES=y
```

#### Timers and Scheduling
```conf
# System clock frequency (Hz)
CONFIG_SYS_CLOCK_TICKS_PER_SEC=1000

# Enable timers
CONFIG_TIMERS=y

# Tickless kernel
CONFIG_TICKLESS_KERNEL=y
```

### 2. **Driver Configurations**

#### GPIO
```conf
CONFIG_GPIO=y
```

#### UART/Serial
```conf
CONFIG_SERIAL=y
CONFIG_UART_CONSOLE=y
CONFIG_UART_INTERRUPT_DRIVEN=y
CONFIG_UART_LINE_CTRL=y
```

#### I2C
```conf
CONFIG_I2C=y
CONFIG_I2C_INIT_PRIORITY=50
```

#### SPI
```conf
CONFIG_SPI=y
CONFIG_SPI_INIT_PRIORITY=70
```

#### ADC
```conf
CONFIG_ADC=y
```

#### PWM
```conf
CONFIG_PWM=y
```

#### CAN
```conf
CONFIG_CAN=y
CONFIG_CAN_INIT_PRIORITY=80
```

#### USB
```conf
CONFIG_USB_DEVICE_STACK=y
CONFIG_USB_DEVICE_PRODUCT="Zephyr USB Device"
CONFIG_USB_DEVICE_VID=0x2FE3
CONFIG_USB_DEVICE_PID=0x0001
```

### 3. **Subsystem Configurations**

#### Logging
```conf
# Enable logging
CONFIG_LOG=y

# Set default log level
CONFIG_LOG_DEFAULT_LEVEL=3  # 0=OFF, 1=ERR, 2=WRN, 3=INF, 4=DBG

# Log buffer size
CONFIG_LOG_BUFFER_SIZE=1024

# Backend configurations
CONFIG_LOG_BACKEND_UART=y
CONFIG_LOG_BACKEND_RTT=y
```

#### Shell
```conf
CONFIG_SHELL=y
CONFIG_SHELL_BACKEND_SERIAL=y
CONFIG_SHELL_PROMPT_UART="zephyr:~$ "
```

#### Networking
```conf
CONFIG_NETWORKING=y
CONFIG_NET_IPV4=y
CONFIG_NET_IPV6=y
CONFIG_NET_TCP=y
CONFIG_NET_UDP=y

# Network buffer configuration
CONFIG_NET_BUF_DATA_SIZE=128
CONFIG_NET_PKT_RX_COUNT=10
CONFIG_NET_PKT_TX_COUNT=10
```

#### Bluetooth
```conf
CONFIG_BT=y
CONFIG_BT_PERIPHERAL=y
CONFIG_BT_CENTRAL=y
CONFIG_BT_DEVICE_NAME="Zephyr BLE"
CONFIG_BT_MAX_CONN=4
CONFIG_BT_L2CAP_TX_BUF_COUNT=8
```

#### Wi-Fi
```conf
CONFIG_WIFI=y
CONFIG_WIFI_INIT_PRIORITY=80
CONFIG_NET_L2_WIFI_SHELL=y
```

#### File Systems
```conf
CONFIG_FILE_SYSTEM=y
CONFIG_FAT_FILESYSTEM_ELM=y
CONFIG_FILE_SYSTEM_LITTLEFS=y
```

#### Storage
```conf
CONFIG_FLASH=y
CONFIG_FLASH_MAP=y
CONFIG_STREAM_FLASH=y
CONFIG_NVS=y  # Non-Volatile Storage
CONFIG_SETTINGS=y
```

### 4. **Debug and Testing**

#### Debug Options
```conf
# Enable assertions
CONFIG_ASSERT=y

# Thread monitoring
CONFIG_THREAD_MONITOR=y

# Thread name
CONFIG_THREAD_NAME=y

# Object tracing
CONFIG_TRACING=y

# Init stacks with 0xAA
CONFIG_INIT_STACKS=y

# Thread stack info
CONFIG_THREAD_STACK_INFO=y
```

#### Testing
```conf
CONFIG_ZTEST=y
CONFIG_ZTEST_NEW_API=y
```

### 5. **Power Management**

```conf
CONFIG_PM=y
CONFIG_PM_DEVICE=y
CONFIG_PM_POLICY_DEFAULT=y
```

### 6. **Security and Crypto**

```conf
CONFIG_MBEDTLS=y
CONFIG_ENTROPY_GENERATOR=y
CONFIG_TEST_RANDOM_GENERATOR=y
```

### 7. **Build and Optimization**

```conf
# Optimize for size
CONFIG_SIZE_OPTIMIZATIONS=y

# Optimize for speed
CONFIG_SPEED_OPTIMIZATIONS=y

# Debug optimizations
CONFIG_DEBUG_OPTIMIZATIONS=y

# Link Time Optimization
CONFIG_LTO=y

# Remove unused sections
CONFIG_LINKER_ORPHAN_SECTION_PLACE=y
```

---

## Practical Examples

### Example 1: Basic GPIO Blinky
```conf
# prj.conf for LED blinking application
CONFIG_GPIO=y
CONFIG_LOG=y
CONFIG_LOG_DEFAULT_LEVEL=3
```

### Example 2: UART Communication
```conf
# Enable UART for serial communication
CONFIG_SERIAL=y
CONFIG_UART_CONSOLE=y
CONFIG_UART_INTERRUPT_DRIVEN=y

# Logging over UART
CONFIG_LOG=y
CONFIG_LOG_BACKEND_UART=y
CONFIG_LOG_DEFAULT_LEVEL=4

# Larger buffer for output
CONFIG_UART_CONSOLE_BUFFER_SIZE=512
```

### Example 3: I2C Sensor Application
```conf
# Enable I2C
CONFIG_I2C=y

# Enable GPIO for interrupt pins
CONFIG_GPIO=y

# Sensor subsystem
CONFIG_SENSOR=y

# Logging
CONFIG_LOG=y
CONFIG_LOG_DEFAULT_LEVEL=3

# Printf support for float (sensor readings)
CONFIG_CBPRINTF_FP_SUPPORT=y
```

### Example 4: Bluetooth Low Energy
```conf
# Bluetooth stack
CONFIG_BT=y
CONFIG_BT_PERIPHERAL=y
CONFIG_BT_DEVICE_NAME="MyBLEDevice"

# GAP and GATT
CONFIG_BT_GAP_PERIPHERAL_PREF_PARAMS=y
CONFIG_BT_GATT_DYNAMIC_DB=y

# Connection parameters
CONFIG_BT_MAX_CONN=2
CONFIG_BT_MAX_PAIRED=2

# Buffer sizes
CONFIG_BT_L2CAP_TX_BUF_COUNT=8
CONFIG_BT_BUF_ACL_RX_SIZE=251

# Logging
CONFIG_BT_DEBUG_LOG=y
CONFIG_LOG=y
CONFIG_LOG_DEFAULT_LEVEL=3
```

### Example 5: Network Application (Ethernet)
```conf
# Networking stack
CONFIG_NETWORKING=y
CONFIG_NET_TCP=y
CONFIG_NET_UDP=y
CONFIG_NET_IPV4=y
CONFIG_NET_IPV6=n

# Ethernet
CONFIG_NET_L2_ETHERNET=y

# DHCP client
CONFIG_NET_DHCPV4=y

# Network buffers
CONFIG_NET_PKT_RX_COUNT=16
CONFIG_NET_PKT_TX_COUNT=16
CONFIG_NET_BUF_RX_COUNT=32
CONFIG_NET_BUF_TX_COUNT=32

# Network shell for debugging
CONFIG_NET_SHELL=y
CONFIG_SHELL=y

# Logging
CONFIG_LOG=y
CONFIG_NET_LOG=y
```

### Example 6: USB CDC ACM Serial
```conf
# USB device stack
CONFIG_USB_DEVICE_STACK=y
CONFIG_USB_DEVICE_PRODUCT="Zephyr USB Serial"
CONFIG_USB_DEVICE_VID=0x2FE3
CONFIG_USB_DEVICE_PID=0x0001

# CDC ACM class
CONFIG_USB_CDC_ACM=y

# Console over USB
CONFIG_UART_CONSOLE=y
CONFIG_UART_LINE_CTRL=y

# Logging
CONFIG_LOG=y
```

### Example 7: File System with SD Card
```conf
# Disk and file system
CONFIG_DISK_DRIVER_SDMMC=y
CONFIG_FILE_SYSTEM=y
CONFIG_FAT_FILESYSTEM_ELM=y

# SPI for SD card
CONFIG_SPI=y
CONFIG_GPIO=y

# Logging
CONFIG_LOG=y
CONFIG_FS_LOG_LEVEL_DBG=y
```

### Example 8: Real-Time Application
```conf
# Optimize for real-time performance
CONFIG_SPEED_OPTIMIZATIONS=y
CONFIG_NO_OPTIMIZATIONS=n

# Tickless kernel for precise timing
CONFIG_TICKLESS_KERNEL=y
CONFIG_SYS_CLOCK_TICKS_PER_SEC=10000

# High priority threads
CONFIG_NUM_PREEMPT_PRIORITIES=32

# Minimal latency
CONFIG_ZERO_LATENCY_IRQS=y

# Disable unnecessary features
CONFIG_CONSOLE=n
CONFIG_LOG=n
```

### Example 9: Low Power Application
```conf
# Power management
CONFIG_PM=y
CONFIG_PM_DEVICE=y

# Tickless kernel
CONFIG_TICKLESS_KERNEL=y

# Reduce peripheral activity
CONFIG_UART=n
CONFIG_CONSOLE=n
CONFIG_LOG=n

# Low power mode
CONFIG_PM_POLICY_DEFAULT=y
```

### Example 10: Production Build
```conf
# Size optimizations
CONFIG_SIZE_OPTIMIZATIONS=y

# Disable debug features
CONFIG_LOG=n
CONFIG_CONSOLE=n
CONFIG_SHELL=n
CONFIG_ASSERT=n
CONFIG_DEBUG=n

# Enable security features
CONFIG_STACK_CANARIES=y
CONFIG_STACK_SENTINEL=y

# Bootloader support
CONFIG_BOOTLOADER_MCUBOOT=y
```

---

## Configuration Priority

Zephyr merges configurations in this order (later overrides earlier):

1. **Board defconfig** (`boards/vendor/board/board_defconfig`)
2. **SOC defconfig** (if applicable)
3. **Kconfig defaults** (from Kconfig files)
4. **prj.conf** (your application config) ← Highest priority
5. **Overlay configs** (if using `-DOVERLAY_CONFIG=extra.conf`)

### Example: Multiple Config Files
```bash
# Build with additional config
west build -b nucleo_f429zi -- -DOVERLAY_CONFIG="debug.conf;usb.conf"
```

---

## Best Practices

### 1. **Organize Configurations**
```conf
# ============================================
# Kernel Configuration
# ============================================
CONFIG_MAIN_STACK_SIZE=2048
CONFIG_HEAP_MEM_POOL_SIZE=4096

# ============================================
# Driver Configuration
# ============================================
CONFIG_GPIO=y
CONFIG_UART_CONSOLE=y

# ============================================
# Subsystem Configuration
# ============================================
CONFIG_LOG=y
CONFIG_LOG_DEFAULT_LEVEL=3

# ============================================
# Application-Specific
# ============================================
CONFIG_MY_APP_FEATURE=y
```

### 2. **Use Comments**
```conf
# Enable GPIO for LED control
CONFIG_GPIO=y

# UART console for debugging (115200 baud)
CONFIG_UART_CONSOLE=y

# Increase stack for sensor processing
CONFIG_MAIN_STACK_SIZE=4096  # Default is 1024
```

### 3. **Create Separate Configs for Different Builds**
```
my_app/
├── prj.conf           # Base configuration
├── debug.conf         # Debug-specific
├── release.conf       # Release optimizations
└── test.conf          # Testing configuration
```

```bash
# Build with debug config
west build -b board -- -DOVERLAY_CONFIG=debug.conf

# Build release
west build -b board -- -DOVERLAY_CONFIG=release.conf
```

### 4. **Check Dependencies**
```conf
# This requires CONFIG_SPI=y
CONFIG_SPI_NOR=y

# Always enable dependencies first
CONFIG_SPI=y
CONFIG_SPI_NOR=y
```

### 5. **Validate Your Configuration**
```bash
# Build and check for warnings
west build -b your_board

# Check final configuration
grep "CONFIG_GPIO" build/zephyr/.config
```

### 6. **Use Kconfig Search Script**
Create a helper script `find_kconfig.sh`:
```bash
#!/bin/bash
# Usage: ./find_kconfig.sh CONFIG_NAME

if [ -z "$1" ]; then
    echo "Usage: $0 CONFIG_NAME"
    exit 1
fi

echo "Searching for $1 in Kconfig files..."
grep -rn "config ${1#CONFIG_}" $ZEPHYR_BASE --include="Kconfig*" | head -20
```

### 7. **Document Non-Obvious Settings**
```conf
# Required for STM32 USB DFU bootloader compatibility
CONFIG_USE_DT_CODE_PARTITION=y

# Workaround for errata 1.2.3 on this chip
CONFIG_SOC_SPECIFIC_WORKAROUND=y
```

---

## Troubleshooting

### Issue 1: Config Option Not Taking Effect

**Problem**: Set `CONFIG_FEATURE=y` but feature doesn't work

**Solutions**:
```bash
# Check if config was actually set
grep "CONFIG_FEATURE" build/zephyr/.config

# Check dependencies
grep -A 10 "config FEATURE" $ZEPHYR_BASE --include="Kconfig*" -r

# Look for dependency errors in build output
west build -b board 2>&1 | grep -i "warning\|error"
```

### Issue 2: Unknown Configuration Option

**Problem**: Build warning about unknown config

**Solutions**:
```bash
# Search for correct spelling
grep -r "config.*FEATURE" $ZEPHYR_BASE --include="Kconfig*"

# Check if it's arch/board specific
find $ZEPHYR_BASE -name "Kconfig*" -exec grep -l "CONFIG_FEATURE" {} \;
```

### Issue 3: Conflicting Configurations

**Problem**: Configs conflict with each other

**Solutions**:
```conf
# Check Kconfig dependencies
# Some configs are mutually exclusive

# Example: Can't have both
CONFIG_SIZE_OPTIMIZATIONS=y
CONFIG_SPEED_OPTIMIZATIONS=y

# Choose one or the other
```

### Issue 4: Board-Specific Limitations

**Problem**: Config works on one board but not another

**Solutions**:
```bash
# Check board's defconfig
cat $ZEPHYR_BASE/boards/vendor/board/board_defconfig

# Check if hardware supports feature
grep "depends on" $ZEPHYR_BASE/drivers/*/Kconfig
```

---

## Quick Reference: Finding Configs by Feature

### GPIO
```bash
grep -r "config GPIO" $ZEPHYR_BASE/drivers/gpio/Kconfig
```

### UART
```bash
grep -r "config UART" $ZEPHYR_BASE/drivers/serial/Kconfig
grep -r "config SERIAL" $ZEPHYR_BASE/drivers/serial/Kconfig
```

### I2C
```bash
grep -r "config I2C" $ZEPHYR_BASE/drivers/i2c/Kconfig
```

### SPI
```bash
grep -r "config SPI" $ZEPHYR_BASE/drivers/spi/Kconfig
```

### Networking
```bash
grep -r "config NET" $ZEPHYR_BASE/subsys/net/Kconfig
```

### Bluetooth
```bash
grep -r "config BT" $ZEPHYR_BASE/subsys/bluetooth/Kconfig
```

### USB
```bash
grep -r "config USB" $ZEPHYR_BASE/subsys/usb/Kconfig
```

### Logging
```bash
grep -r "config LOG" $ZEPHYR_BASE/subsys/logging/Kconfig
```

---

## Useful Commands Summary

```bash
# Find all Kconfig files
find $ZEPHYR_BASE -name "Kconfig*" -type f

# Search for specific config
grep -r "config FEATURE_NAME" $ZEPHYR_BASE --include="Kconfig*"

# View final merged config
cat build/zephyr/.config

# Search in final config
grep "CONFIG_" build/zephyr/.config | grep UART

# View board defconfig
cat $ZEPHYR_BASE/boards/vendor/board/*_defconfig

# Check config with description
grep -A 5 "config UART_CONSOLE" $ZEPHYR_BASE --include="Kconfig*" -r
```

---

## Additional Resources

- **Kconfig Documentation**: https://docs.zephyrproject.org/latest/build/kconfig/index.html
- **Configuration System**: https://docs.zephyrproject.org/latest/build/kconfig/tips.html
- **Zephyr Samples**: `$ZEPHYR_BASE/samples/` (check prj.conf in each)
- **Kconfig Language**: https://www.kernel.org/doc/html/latest/kbuild/kconfig-language.html

---

**Author**: GitHub Copilot  
**Last Updated**: December 4, 2025  
**Version**: 1.0  
**License**: Feel free to use and share this guide
