#include "gpio_driver.h"

static bool led_state = false;
static bool mute_button_state = false;

gptimer_handle_t timer_handle = NULL;

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
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE
    };
    gpio_config(&btn_conf);
}

// ISR callback — runs in interrupt context, must stay minimal.
// Overwrites the queue so ticks never accumulate if the task is briefly slow.
static bool IRAM_ATTR timer_isr_cb(gptimer_handle_t timer,
                                   const gptimer_alarm_event_data_t* edata,
                                   void* user_ctx) {
    static uint32_t tick = 0;
    tick++;

    BaseType_t higher_task_woken = pdFALSE;
    xQueueOverwriteFromISR((QueueHandle_t)user_ctx, &tick, &higher_task_woken);
    return higher_task_woken == pdTRUE;
}

void timer_init(QueueHandle_t tick_queue) {
    gptimer_config_t timer_config = {
        .clk_src = GPTIMER_CLK_SRC_DEFAULT,
        .direction = GPTIMER_COUNT_UP,
        .resolution_hz = 1000000, // 1 MHz → 1 tick = 1 µs
    };
    ESP_ERROR_CHECK(gptimer_new_timer(&timer_config, &timer_handle));

    // Pass the queue as user context to the ISR
    gptimer_event_callbacks_t cbs = { .on_alarm = timer_isr_cb };
    ESP_ERROR_CHECK(gptimer_register_event_callbacks(timer_handle, &cbs, tick_queue));

    // Alarm every 1000 µs = 1 ms, auto-reload
    gptimer_alarm_config_t alarm_config = {
        .alarm_count = 1000,
        .reload_count = 0,
        .flags = { .auto_reload_on_alarm = true },
    };
    ESP_ERROR_CHECK(gptimer_set_alarm_action(timer_handle, &alarm_config));

    ESP_ERROR_CHECK(gptimer_enable(timer_handle));
    ESP_ERROR_CHECK(gptimer_start(timer_handle));
}

void toggle_led() {
    led_state = !led_state;
    gpio_set_level(LED_PIN, led_state);
}

void toggle_mute_button() {
    mute_button_state = !mute_button_state;
    gpio_set_level(BUTTON_PIN, mute_button_state);
}