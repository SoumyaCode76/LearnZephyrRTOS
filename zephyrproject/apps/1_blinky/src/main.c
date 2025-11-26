#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/gpio.h>

int main(void)
{
    const struct device *led;
    int ret;

    /* Get the device binding for the LED */
    led = DEVICE_DT_GET(DT_ALIAS(led0));
    if (!device_is_ready(led)) {
        return -1;
    }

    /* Configure the LED pin as output */
    ret = gpio_pin_configure(led, DT_GPIO_PIN(DT_ALIAS(led0), gpios),
                             GPIO_OUTPUT_ACTIVE | DT_GPIO_FLAGS(DT_ALIAS(led0), gpios));
    if (ret < 0) {
        return -1;
    }

    while (1) {
        /* Toggle the LED state */
        gpio_pin_toggle(led, DT_GPIO_PIN(DT_ALIAS(led0), gpios));
        printk("LED toggled\n");
        k_msleep(1000); /* Sleep for 1000 milliseconds */
    }

    return 0;
}