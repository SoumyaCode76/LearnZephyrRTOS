#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/logging/log.h>

// DT indices must be compile-time literals, so build the array at
// preprocessing time with DT_FOREACH_PROP_ELEM instead of a runtime loop.

LOG_MODULE_REGISTER(nordic_course_threads_1, LOG_LEVEL_DBG);

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

// Static thread configuration
#define STACKSIZE       (256)
#define THREAD0_PRIO    (6)
#define THREAD1_PRIO    (7)
#define THREAD2_PRIO    (8)

void thread0_handler_fcn(void* p1, void* p2, void* p3)
{
    while(true)
    {
        gpio_pin_toggle(external_leds[0], external_led_pin[0]);
        LOG_PRINTK("LED 1 state: %d\n", gpio_pin_get_raw(external_leds[0], external_led_pin[0]));
        // k_busy_wait(1000000);
        // k_yield();
        k_msleep(5000);
    }
}
void thread1_handler_fcn(void* p1, void* p2, void* p3)
{
    while(true)
    {
        gpio_pin_toggle(external_leds[1], external_led_pin[1]);
        LOG_PRINTK("LED 2 state: %d\n", gpio_pin_get_raw(external_leds[1], external_led_pin[1]));        
        k_busy_wait(1000000);        
        // k_yield();
        // k_msleep(8000);
    }
}

void thread2_handler_fcn(void* p1, void* p2, void* p3)
{
    while(true)
    {
        gpio_pin_toggle(external_leds[2], external_led_pin[2]);
        LOG_PRINTK("LED 3 state: %d\n", gpio_pin_get_raw(external_leds[2], external_led_pin[2]));        
        k_busy_wait(1000000);
        // k_yield();
        // k_msleep(8000);
    }
}

const k_thread_entry_t thread0_handler = thread0_handler_fcn;
const k_thread_entry_t thread1_handler = thread1_handler_fcn;
const k_thread_entry_t thread2_handler = thread2_handler_fcn;

K_THREAD_DEFINE(thread0_id, STACKSIZE, thread0_handler, NULL, NULL, NULL, THREAD0_PRIO, 0, 0);
K_THREAD_DEFINE(thread1_id, STACKSIZE, thread1_handler, NULL, NULL, NULL, THREAD1_PRIO, 0, 0);
K_THREAD_DEFINE(thread2_id, STACKSIZE, thread2_handler, NULL, NULL, NULL, THREAD2_PRIO, 0, 0);

int main(void)
{
    for(uint8_t ix = 0; ix < NUM_OF_LEDS; ix++)
    {
        if((device_is_ready(external_leds[ix]) == false))
        {
            LOG_PRINTK("Device at %p not ready. Exiting thread!\n", external_leds[ix]);
            break;
        }
        if(gpio_pin_configure(external_leds[ix], external_led_pin[ix], GPIO_OUTPUT) < 0)
        {
            LOG_PRINTK("Pin configuration failed. Exiting...\n");
            break;
        }
    }

    while(1)
    {
        k_sleep(K_FOREVER);
    }
    return 0;
}