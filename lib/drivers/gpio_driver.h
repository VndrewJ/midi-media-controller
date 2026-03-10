#pragma once
#include <driver/gpio.h>

#define LED_PIN GPIO_NUM_2
#define BUTTON_PIN GPIO_NUM_4
#define POT_PIN GPIO_NUM_36

// Initialise gpio pins for LED, button and potentiometer
void gpio_init();

// Toggle the LED state
void toggle_led();

// Toggle Mute Button state
void toggle_mute_button();