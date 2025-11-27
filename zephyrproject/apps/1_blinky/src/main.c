#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>
#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/gpio.h>


//1. Check for board led0 alias
#if DT_NODE_HAS_STATUS(DT_ALIAS(led0), okay)
#define LED0_NODE DT_ALIAS(led0)
#else
#warning "Board does not define led0 as a devicetree alias. Please add in the board overlay file."
#endif
//2. Build a typed description of the LED pin from the devicetree
#ifdef LED0_NODE
static const struct gpio_dt_spec led = GPIO_DT_SPEC_GET(LED0_NODE, gpios);
#endif
int main(void)
{
    //3. Make sure the device is ready (i.e. the device/peripheral is initialized)
    #ifdef LED0_NODE
    // if(!gpio_is_ready_dt(&led))
    // {
    //     printk("Error: not ready\n");
    //     return -1;
    // }
    if(gpio_pin_configure(led.port, led.pin, GPIO_OUTPUT_INACTIVE) != 0)
    {
        printk("GPIO pin configuration failed\n");
        return -1;
    }
    //4. Configure pin as output push-pull
    while(1)
    {
        //5. Toggle LED pin
        //6. Delay
        return 0;
    }
    #else
    printk("Error: No LED0 node found in devicetree\n");
    return -1;
    #endif
}

 