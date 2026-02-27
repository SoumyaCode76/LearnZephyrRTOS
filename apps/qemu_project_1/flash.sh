#!/bin/bash
# Script to flash the application to hardware using west flash

# Set colors for output
GREEN='\033[0;32m'
BLUE='\033[0;34m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

echo -e "${BLUE}=== West Flash Guide ===${NC}"
echo ""

# Ensure environment is set up
if [ -z "$ZEPHYR_BASE" ]; then
    echo "Setting up Zephyr environment..."
    source /workspaces/LearnZephyrRTOS/setup_environment.sh
fi

# Check if a build exists
if [ ! -d "build" ]; then
    echo -e "${YELLOW}No build directory found. Building first...${NC}"
    echo "Usage: ./flash.sh <board_name>"
    echo "Example: ./flash.sh stm32f429i_disc1"
    
    if [ -z "$1" ]; then
        echo ""
        echo -e "${YELLOW}Please specify a board name:${NC}"
        echo "  ./flash.sh stm32f429i_disc1"
        echo "  ./flash.sh nucleo_f429zi"
        echo "  ./flash.sh nrf52840dk_nrf52840"
        exit 1
    fi
    
    BOARD=$1
    echo ""
    echo -e "${GREEN}Building for board: ${BOARD}${NC}"
    west build -b ${BOARD} -p auto
    
    if [ $? -ne 0 ]; then
        echo "Build failed!"
        exit 1
    fi
fi

echo ""
echo -e "${BLUE}=== Flashing Options ===${NC}"
echo ""
echo -e "${GREEN}1. Standard Flash (auto-detect runner):${NC}"
echo "   west flash"
echo ""
echo -e "${GREEN}2. Specify Flash Runner:${NC}"
echo "   west flash --runner openocd    # For STM32 with OpenOCD"
echo "   west flash --runner jlink      # For J-Link debugger"
echo "   west flash --runner pyocd      # For PyOCD"
echo "   west flash --runner nrfjprog   # For Nordic nRF devices"
echo ""
echo -e "${GREEN}3. Flash with specific options:${NC}"
echo "   west flash --runner openocd --board-dir=/path/to/board"
echo ""
echo -e "${YELLOW}Press Enter to attempt flashing now, or Ctrl+C to cancel${NC}"
read

echo -e "${GREEN}Attempting to flash...${NC}"
west flash

if [ $? -eq 0 ]; then
    echo -e "${GREEN}Flash successful!${NC}"
else
    echo ""
    echo -e "${YELLOW}Flash failed. Common issues:${NC}"
    echo "  - Hardware not connected"
    echo "  - Wrong debugger/runner specified"
    echo "  - Missing udev rules (Linux)"
    echo "  - Debugger drivers not installed"
    echo ""
    echo "Try specifying the runner manually:"
    echo "  west flash --runner openocd"
fi
