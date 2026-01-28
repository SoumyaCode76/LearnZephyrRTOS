#include <zephyr/sys/printk.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/device.h>
#include <zephyr/devicetree.h>

int main(void)
{
    int count = 0;
    while(1)
    {
        printk("Hello World! %d\n", count++);
        k_msleep(2000);
    }
    return 0;
}