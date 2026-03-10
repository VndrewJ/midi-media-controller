#include "gpio_driver.h"

static bool led_state = false;
static bool mute_button_state = false;

void gpio_init() {
    // 1. LED Configuration
    gpio_config_t led_conf = {
        .pin_bit_mask = (1ULL << LED_PIN),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE
    };
    gpio_config(&led_conf);

    // 2. Button Configuration
    gpio_config_t btn_conf = {
        .pin_bit_mask = (1ULL << BUTTON_PIN),
        .mode = GPIO_MODE_INPUT, // Changed from INPUT_OUTPUT to just INPUT
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE
    };
    gpio_config(&btn_conf);

    // 3. Potentiometer Pin (GPIO Side)
    // Note: Usually, when using ADC Continuous mode, 
    // you don't actually need to call gpio_config for the POT_PIN.
    // The ADC driver handles the IO MUX routing for you.
}

void toggle_led() {
    led_state = !led_state; // Update the tracker
    gpio_set_level(LED_PIN, led_state);
}