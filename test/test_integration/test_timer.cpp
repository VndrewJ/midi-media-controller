#include <stdio.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

#include "logic_pot.h"
#include "adc_driver.h"
#include "gpio_driver.h"

#define READ_LEN       256
#define BTN_DEBOUNCE_MS 20  // Ticks (ms) before a button edge is accepted

// Peripheral scan task — pinned to core 1, leaving core 0 free for BLE.
// Wakes on every 1ms tick posted to tick_queue by the gptimer ISR.
static void scan_task(void* arg) {
    QueueHandle_t tick_queue = (QueueHandle_t)arg;
    uint32_t tick = 0;

    // ADC state
    uint8_t  adc_buf[READ_LEN] = {0};
    uint32_t bytes_read = 0;
    midi_adc_t adc_state = {0, 0, 0};
    bool    primed      = false;
    uint8_t last_midi_cc = 0xFF;

    // Button debounce state
    int btn_stable   = gpio_get_level(BUTTON_PIN);
    int btn_debounce = 0;

    while (1) {
        // Block until the ISR posts the next 1ms tick
        xQueueReceive(tick_queue, &tick, portMAX_DELAY);

        // ── Potentiometer ────────────────────────────────────────────────
        esp_err_t ret = adc_continuous_read(adc_handle, adc_buf, READ_LEN,
                                            &bytes_read, 0); // non-blocking
        if (ret == ESP_OK) {
            for (int i = 0; i + 3 < (int)bytes_read; i += 4) {
                adc_digi_output_data_t* s = (adc_digi_output_data_t*)&adc_buf[i];
                if (s->type1.channel != ADC_CHANNEL_4) continue;
                adc_state.raw_data_input = s->type1.data;
            }

            if (!primed) {
                prime_accumulator(&adc_state);
                primed = true;
            }

            uint16_t smoothed  = smooth_pot_value_fp(&adc_state);
            uint8_t  midi_cc   = convert_pot_value_to_midi_cc(smoothed);
            uint8_t  stable_cc = apply_hysteresis(last_midi_cc, midi_cc);

            if (stable_cc != last_midi_cc) {
                last_midi_cc = stable_cc;
                printf("[POT] Raw: %4u  Smoothed: %4u  MIDI CC: %3u\n",
                       adc_state.raw_data_input, smoothed, stable_cc);
            }
        }

        // ── Button (debounced) ───────────────────────────────────────────
        int btn_raw = gpio_get_level(BUTTON_PIN);
        if (btn_raw == btn_stable) {
            btn_debounce = 0;
        } else if (++btn_debounce >= BTN_DEBOUNCE_MS) {
            btn_stable   = btn_raw;
            btn_debounce = 0;
            // Pullup: LOW = pressed
            printf("[BTN] %s\n", btn_stable == 0 ? "PRESSED" : "RELEASED");
        }
    }
}

extern "C" void app_main() {
    gpio_init();
    adc_init();
    adc_continuous_start(adc_handle);

    // Single-item queue — ISR overwrites it each tick so ticks never pile up
    QueueHandle_t tick_queue = xQueueCreate(1, sizeof(uint32_t));

    // Start the 1ms timer; ISR will post to tick_queue immediately
    timer_init(tick_queue);

    printf("=== Timer-Driven Peripheral Scan (1ms tick, core 1) ===\n\n");

    // Pin scan_task to core 1 — BLE stack (NimBLE) will run on core 0
    xTaskCreatePinnedToCore(scan_task, "scan_task", 4096, tick_queue, 5, NULL, 1);
}