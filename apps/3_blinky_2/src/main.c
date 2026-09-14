#include <zephyr/sys/printk.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/logging/log.h>

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

LOG_MODULE_REGISTER(nordic_blinky, LOG_LEVEL_DBG);

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
    int exercise_num = 2;
    uint8_t data[] = {0x00, 0x01, 0x02, 0x03,
                    0x04, 0x05, 0x06, 0x07,
                    'H', 'e', 'l', 'l','o'};
    // Printf-like messages
    LOG_INF("nRF Connect SDK Fundamentals");
    LOG_INF("Exercise %d",exercise_num);    
    LOG_DBG("A log message in debug level");
    LOG_WRN("A log message in warning level!");
    LOG_ERR("A log message in Error level!");
    // Hexdump some data
    LOG_HEXDUMP_INF(data, sizeof(data),"Sample Data!"); 

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