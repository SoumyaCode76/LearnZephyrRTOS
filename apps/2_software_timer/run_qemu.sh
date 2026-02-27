#!/bin/bash
# Script to build and run the hello_world app in QEMU

# Set colors for output
GREEN='\033[0;32m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

echo -e "${BLUE}=== Building and Running Hello World in QEMU ===${NC}"

# Ensure environment is set up
if [ -z "$ZEPHYR_BASE" ]; then
    echo "Setting up Zephyr environment..."
    source /workspaces/LearnZephyrRTOS/setup_environment.sh
fi

# Navigate to the app directory
# cd /workspaces/LearnZephyrRTOS/apps/1_hello_world

echo -e "${GREEN}Building application for QEMU ARM Cortex-M3...${NC}"
west build -b qemu_cortex_m3 -p auto

if [ $? -eq 0 ]; then
    echo -e "${GREEN}Build successful! Starting QEMU...${NC}"
    echo ""
    echo "Press Ctrl+A then X to exit QEMU"
    echo ""
    sleep 10
    west build -t run
else
    echo "Build failed!"
    exit 1
fi
