#include <zephyr/sys/printk.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/kernel.h>

struct k_timer my_timer;

void expiry_func(struct k_timer *timer_id)
{
    printk("Timer expired! at: %d\n", (k_uptime_get_32() / 1000));
}

int main(void)
{
    int count = 0;
    k_timer_init(&my_timer, expiry_func, NULL);
    k_timer_start(&my_timer, K_SECONDS(5), K_SECONDS(70));
    while(1)
    {
        // printk("Hello World! %d\n", count++);
        k_sleep(K_SECONDS(100));
    }
    return 0;
}