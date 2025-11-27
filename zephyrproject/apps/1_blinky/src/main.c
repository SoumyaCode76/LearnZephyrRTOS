#include <zephyr/kernel.h>
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
    //3. Make sure the device is ready
    //4. Configure pin as output push-pull
    while(1)
    {
        //5. Toggle LED pin
        //6. Delay
        return 0;
    }
}

 