/*
 * Copyright (c) 2022 Libre Solar Technologies GmbH
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/uart.h>

#include <string.h>

/* change this to any other UART peripheral if desired */
#define UART_DEVICE_NODE DT_CHOSEN(zephyr_shell_uart)

#define MSG_SIZE 32

/* queue to store up to 10 messages (aligned to 4-byte boundary) */
K_MSGQ_DEFINE(uart_msgq, MSG_SIZE, 10, 4);

/* Define a software timer */
static struct k_timer uart_rx_timer;
/* Define a shared variable to indicate UART IDLE state */
static bool uart_idle = false;
/* Define a mutex to protect access to the shared variable */
static struct k_mutex uart_idle_mutex;


static const struct device *const uart_dev = DEVICE_DT_GET(UART_DEVICE_NODE);

/* receive buffer used in UART ISR callback */
static char rx_buf[MSG_SIZE];
static int rx_buf_pos;

/*
 * Read characters from UART until line end is detected. Afterwards push the
 * data to the message queue.
 * Also, if the UART line remains idle for more than 100 ms, push the data to the message queue
 */
void serial_cb(const struct device *dev, void *user_data)
{
	uint8_t c = 0;

	if (!uart_irq_update(uart_dev)) {
		return;
	}

	if (!uart_irq_rx_ready(uart_dev)) {
		return;
	}
	
	/* check the idle state of the UART line */
	bool is_idle = false;	
	k_mutex_lock(&uart_idle_mutex, K_FOREVER);
	is_idle = uart_idle;
	uart_idle = false;
	k_mutex_unlock(&uart_idle_mutex);
	/* if the line is idle, push the data to the message queue */
	if (is_idle) {
		/* terminate string */
		rx_buf[rx_buf_pos] = '\0';
		k_msgq_put(&uart_msgq, &rx_buf, K_NO_WAIT);
		/* reset the buffer (it was copied to the msgq) */
		rx_buf_pos = 0;
		memset(rx_buf, 0, sizeof(rx_buf));
	}

	/* read until FIFO empty */
	while (uart_fifo_read(uart_dev, &c, 1) == 1) {
			 /* reset the timer on each character received */
			k_timer_start(&uart_rx_timer, K_MSEC(1000), K_MSEC(1000));
			 /* set idle to false on each character received */
			k_mutex_lock(&uart_idle_mutex, K_FOREVER);
			uart_idle = false;
			k_mutex_unlock(&uart_idle_mutex);
			 /* if the timer is already running, this will reset the timer countdown */
			 /* if the timer is not running, this will start the timer */
			 /* when the timer expires, it will set uart_idle to true,
			  which will trigger the message to be pushed to the msgq in the main loop */		
		if ((c == '\n' || c == '\r') || rx_buf_pos >= sizeof(rx_buf)) {
			/* terminate string */
			rx_buf[rx_buf_pos] = '\0';

			/* if queue is full, message is silently dropped */
			k_msgq_put(&uart_msgq, &rx_buf, K_NO_WAIT);

			/* reset the buffer (it was copied to the msgq) */
			rx_buf_pos = 0;
			memset(rx_buf, 0, sizeof(rx_buf));
		} else if (rx_buf_pos < (sizeof(rx_buf) - 1)) {
			rx_buf[rx_buf_pos] = c;
			rx_buf_pos++;
		} 
		/* else: characters beyond buffer size are dropped */
	}
}

/*
 * Print a null-terminated string character by character to the UART interface
 */
void print_uart(char *buf)
{
	int msg_len = strlen(buf);

	for (int i = 0; i < msg_len; i++) {
		uart_poll_out(uart_dev, buf[i]);
	}
}

void uart_timer_expiry_func(struct k_timer *timer_id)
{
	/* Write to a shared variable between timer expiry func and the uart cb */
	/* The shared variable access to be protected using k_mutex */
	k_mutex_lock(&uart_idle_mutex, K_FOREVER);
	uart_idle = true;
	k_mutex_unlock(&uart_idle_mutex);
	serial_cb(uart_dev, NULL);
}

int main(void)
{
	char tx_buf[MSG_SIZE];

	if (!device_is_ready(uart_dev)) {
		printk("UART device not found!");
		return 0;
	}

	/* configure interrupt and callback to receive data */
	int ret = uart_irq_callback_user_data_set(uart_dev, serial_cb, NULL);

	if (ret < 0) {
		if (ret == -ENOTSUP) {
			printk("Interrupt-driven UART API support not enabled\n");
		} else if (ret == -ENOSYS) {
			printk("UART device does not support interrupt-driven API\n");
		} else {
			printk("Error setting UART callback: %d\n", ret);
		}
		return 0;
	}
	uart_irq_rx_enable(uart_dev);

	print_uart("Hello! I'm your echo bot.\r\n");
	print_uart("Tell me something and press enter:\r\n");

	/* initialize and start software timer */
	k_timer_init(&uart_rx_timer, uart_timer_expiry_func, NULL);
	k_timer_start(&uart_rx_timer, K_MSEC(1000), K_MSEC(1000));

	/* indefinitely wait for input from the user */
	while (k_msgq_get(&uart_msgq, &tx_buf, K_FOREVER) == 0) {
		print_uart("Echo: ");
		print_uart(tx_buf);
		print_uart("\r\n");
	}
	return 0;
}
