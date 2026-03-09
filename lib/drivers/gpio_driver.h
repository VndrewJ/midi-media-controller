#include <driver/gpio.h>

#define LED_PIN GPIO_NUM_2
#define BUTTON_PIN GPIO_NUM_4
#define POT_PIN GPIO_NUM_36

static bool led_state;
static bool mute_button_state;

gpio_config_t led_config = {
    .pin_bit_mask = (1ULL << LED_PIN),
    .mode = GPIO_MODE_OUTPUT,
    .pull_up_en = GPIO_PULLUP_DISABLE,
    .pull_down_en = GPIO_PULLDOWN_DISABLE,
    .intr_type = GPIO_INTR_DISABLE,
};

gpio_config_t button_config = {
    .pin_bit_mask = (1ULL << BUTTON_PIN),
    .mode = GPIO_MODE_INPUT_OUTPUT,
    .pull_up_en = GPIO_PULLUP_ENABLE,
    .pull_down_en = GPIO_PULLDOWN_DISABLE,
    .intr_type = GPIO_INTR_DISABLE,
};

gpio_config_t pot_config = {
    .pin_bit_mask = (1ULL << POT_PIN),
    .mode = GPIO_MODE_INPUT,
    .pull_up_en = GPIO_PULLUP_DISABLE,
    .pull_down_en = GPIO_PULLDOWN_DISABLE,
    .intr_type = GPIO_INTR_DISABLE,
};

// Initialise gpio pins for LED, button and potentiometer
void gpio_init();

// Toggle the LED state
void toggle_led();

// Toggle Mute Button state
void toggle_mute_button();