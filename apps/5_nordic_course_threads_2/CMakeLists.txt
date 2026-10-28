# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.20.0)

if(BOARD STREQUAL "stm32f429i_disc1")
    set(DTC_OVERLAY_FILE "boards/stm32f429i_board.overlay")
    set(CONF_FILE "boards/stm32f429i_board.conf")
elseif(BOARD STREQUAL "nucleo_f103rb")
    set(DTC_OVERLAY_FILE "boards/${BOARD}_picsimlab.overlay")
    set(CONF_FILE "boards/${BOARD}_picsimlab.conf")
elseif(BOARD STREQUAL "native_sim")
    set(DTC_OVERLAY_FILE "boards/${BOARD}_board.overlay")
    set(CONF_FILE "boards/${BOARD}_board.conf")
endif()

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(blinky_2)

target_sources(app PRIVATE src/main.c)
