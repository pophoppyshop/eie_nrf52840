/*
main.c
*/

#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>

#define LED0_NODE DT_ALIAS(led0)
#define LED1_NODE DT_ALIAS(led1)
#define LED2_NODE DT_ALIAS(led2)
#define LED3_NODE DT_ALIAS(led3)

static const struct gpio_dt_spec led0 = GPIO_DT_SPEC_GET(LED0_NODE, gpios);
static const struct gpio_dt_spec led1 = GPIO_DT_SPEC_GET(LED1_NODE, gpios);
static const struct gpio_dt_spec led2 = GPIO_DT_SPEC_GET(LED2_NODE, gpios);
static const struct gpio_dt_spec led3 = GPIO_DT_SPEC_GET(LED3_NODE, gpios);

void TurnOnLED(const struct gpio_dt_spec led, int duration_ms) {
    gpio_pin_toggle_dt(&led);

    k_msleep(duration_ms);

    gpio_pin_toggle_dt(&led);
}

int main(void) {
    int ret;

    if (!gpio_is_ready_dt(&led0) || !gpio_is_ready_dt(&led1) 
    || !gpio_is_ready_dt(&led2) || !gpio_is_ready_dt(&led3)) {
        return -1;
    }

    ret = gpio_pin_configure_dt(&led0, GPIO_OUTPUT_ACTIVE);

    if (ret < 0) {
        return ret;
    }

    ret = gpio_pin_configure_dt(&led1, GPIO_OUTPUT_ACTIVE);

    if (ret < 0) {
        return ret;
    }

    ret = gpio_pin_configure_dt(&led2, GPIO_OUTPUT_ACTIVE);

    if (ret < 0) {
        return ret;
    }

    ret = gpio_pin_configure_dt(&led3, GPIO_OUTPUT_ACTIVE);

    if (ret < 0) {
        return ret;
    }

    // Turn off all LEDs
    gpio_pin_toggle_dt(&led0);
    gpio_pin_toggle_dt(&led1);
    gpio_pin_toggle_dt(&led2);
    gpio_pin_toggle_dt(&led3);
    

    while (1) {
        
        for (int i =0; i < 5; i++){
            TurnOnLED(led0, 100);
            TurnOnLED(led1, 100);
            TurnOnLED(led3, 100);
            TurnOnLED(led2, 100);
        }

        for (int i =0; i < 5; i++){
            TurnOnLED(led2, 100);
            TurnOnLED(led3, 100);
            TurnOnLED(led1, 100);
            TurnOnLED(led0, 100);
        }
        
    }

    return 0;
}