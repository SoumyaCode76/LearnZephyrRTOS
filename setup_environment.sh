#!/bin/bash
# Zephyr RTOS Environment Setup Script
# Source this file to set up your Zephyr development environment

# Set Zephyr base directory
export ZEPHYR_BASE=/workspaces/LearnZephyrRTOS/zephyr

# Add Zephyr SDK to PATH
export ZEPHYR_SDK_INSTALL_DIR=/workspaces/LearnZephyrRTOS/zephyr-sdk-0.17.4

# Add west to PATH (if not already in PATH)
export PATH="$HOME/.local/bin:$PATH"

# Set up CMake package registry (already done by setup.sh, but included for reference)
# The SDK is registered at: ~/.cmake/packages/Zephyr-sdk

echo "Zephyr environment configured:"
echo "  ZEPHYR_BASE: $ZEPHYR_BASE"
echo "  ZEPHYR_SDK_INSTALL_DIR: $ZEPHYR_SDK_INSTALL_DIR"
echo "  West version: $(west --version)"
echo ""
echo "Ready to build Zephyr applications!"
