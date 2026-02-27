/*
 * Copyright (c) 2022 Libre Solar Technologies GmbH
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/gpio.h>
#include <string.h>

volatile bool led_state = false;

struct k_timer timer;

void create_timestamp(const uint32_t count)
{
	uint32_t c = count;
	uint8_t seconds = c % 60;
	uint8_t minutes = ((c / 60) % 60);
	uint32_t hours = c / 3600;
	uint8_t time_stamp[6] = {0};

	time_stamp[0] = 0x30 + hours/10;
	time_stamp[1] = 0x30 + hours%10;

	time_stamp[2] = 0x30 + minutes/10;
	time_stamp[3] = 0x30 + minutes%10;

	time_stamp[4] = 0x30 + seconds/10;
	time_stamp[5] = 0x30 + seconds%10;

	printk("[%c%c:%c%c:%c%c.000] ", time_stamp[0], time_stamp[1], time_stamp[2], time_stamp[3], time_stamp[4], time_stamp[5]);
}

void timer_handler(struct k_timer *timer_id)
{
	static uint32_t count = 0;
	led_state = !led_state;
	count++;
	create_timestamp(count);
	printk("LED state: %d\n", led_state);
	k_timer_start(&timer, K_SECONDS(1), K_SECONDS(1));
}

int main(void)
{
	k_timer_init(&timer, timer_handler, NULL);
	k_timer_start(&timer, K_SECONDS(1), K_SECONDS(1));
	return 0;
}
