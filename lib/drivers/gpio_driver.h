#pragma once
#include <driver/gpio.h>
#include <driver/gptimer.h>
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"

#define LED_PIN    GPIO_NUM_2
#define BUTTON_PIN GPIO_NUM_4

extern gptimer_handle_t timer_handle;

// Initialise GPIO pins for LED and button
void gpio_init();

// Initialise 1ms GP timer; overwrites tick_queue from ISR each tick
void timer_init(QueueHandle_t tick_queue);

// Toggle the LED state
void toggle_led();

// Toggle Mute Button state
void toggle_mute_button();
