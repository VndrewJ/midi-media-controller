#include "gpio_driver.h"

void gpio_init() {
    // Configure LED pin
    gpio_config(&led_config);
    led_state = false; // Initialize LED state to off
    // Configure button pin
    gpio_config(&button_config);
    mute_button_state = false; // Initialize mute button state to off
    // Configure potentiometer pin
    gpio_config(&pot_config);
}

void toggle_led() {
    // Toggle the LED state
    gpio_set_level(LED_PIN, !led_state);
}

void toggle_mute_button() {
    // Toggle the mute button state
    gpio_set_level(BUTTON_PIN, !mute_button_state);
}

