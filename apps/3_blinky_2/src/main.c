#include <zephyr/sys/printk.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/device.h>
#include <zephyr/devicetree.h>

// DT indices must be compile-time literals, so build the array at
// preprocessing time with DT_FOREACH_PROP_ELEM instead of a runtime loop.
#define LED_DEVICE_GET(node_id, prop, idx)          DEVICE_DT_GET_BY_IDX(node_id, prop, idx),
#define LED_PIN_GET(node_id, prop, idx)             DT_PHA_BY_IDX(node_id, prop, idx, pin),
#define LED_PIN_FLAG_GET(node_id, prop, idx)        DT_PHA_BY_IDX(node_id, prop, idx, flags),

const struct device* external_leds[] = {
    DT_FOREACH_PROP_ELEM(DT_ALIAS(user_leds), gpios, LED_DEVICE_GET)
};

const uint8_t external_led_pin[] = {
    DT_FOREACH_PROP_ELEM(DT_ALIAS(user_leds), gpios, LED_PIN_GET)
};

const uint8_t external_led_pin_flags[] = {
    DT_FOREACH_PROP_ELEM(DT_ALIAS(user_leds), gpios, LED_PIN_FLAG_GET)
};

#define NUM_OF_LEDS ARRAY_SIZE(external_leds)

int main(void)
{
    for(uint8_t ix = 0; ix < NUM_OF_LEDS; ix++)
    {
        if((device_is_ready(external_leds[ix]) == false))
        {
            printk("Device at %p not ready. Exiting thread!\n", external_leds[ix]);
            break;
        }
        if(gpio_pin_configure(external_leds[ix], external_led_pin[ix], GPIO_OUTPUT) < 0)
        {
            printk("Pin configuration failed. Exiting...\n");
            break;
        }
    }
    while(1)
    {
        gpio_pin_toggle(external_leds[0], external_led_pin[0]);
        k_msleep(200);
        gpio_pin_toggle(external_leds[1], external_led_pin[1]);
        k_msleep(500);
        gpio_pin_toggle(external_leds[2], external_led_pin[2]);
        k_msleep(1000);
    }
    return 0;
}