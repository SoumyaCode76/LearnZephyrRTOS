#include <zephyr/sys/printk.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/device.h>
#include <zephyr/devicetree.h>

int main(void)
{
    int count = 0;
    // printk("Hello World! %d\n", count++);
    while(1)
    {
        k_msleep(10000000);
    }
    return 0;
}