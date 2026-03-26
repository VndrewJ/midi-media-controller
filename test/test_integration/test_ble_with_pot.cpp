#include <stdio.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

#include "logic_pot.h"
#include "adc_driver.h"
#include "gpio_driver.h"
#include "ble_manager.h"

#define READ_LEN 256

struct midi_queue_item_t {
    Midi_CC cc;
    uint8_t value;
};

struct scan_task_args_t {
    QueueHandle_t tick_queue;
    QueueHandle_t midi_queue;
};

// Scans peripherals every 1ms tick posted by the gptimer ISR
static void scan_task(void* arg) {
    scan_task_args_t* args = (scan_task_args_t*)arg;
    uint32_t tick = 0;
    midi_btn_t btn_state;
    init_btn(&btn_state, gpio_get_level(BUTTON_PIN));

    // ADC state
    uint8_t  adc_buf[READ_LEN] = {0};
    uint32_t bytes_read = 0;
    midi_adc_t adc_state = {0, 0, 0};
    bool    primed      = false;
    uint8_t last_midi_cc = 0xFF;

    while (1) {
        xQueueReceive(args->tick_queue, &tick, portMAX_DELAY);

        // ── Potentiometer ────────────────────────────────────────────────
        esp_err_t ret = adc_continuous_read(adc_handle, adc_buf, READ_LEN,
                                            &bytes_read, 0); 
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
                midi_queue_item_t item = {Midi_CC::VOLUME, stable_cc}; 
                if (!xQueueSend(args->midi_queue, &item, 0)) {
                    printf("[POT] MIDI queue full, message dropped!\n");
                }
                // printf("[POT] Raw: %4u  Smoothed: %4u  MIDI CC: %3u\n",
                //        adc_state.raw_data_input, smoothed, stable_cc);
            }
        }

        if (debounce_btn(&btn_state, gpio_get_level(BUTTON_PIN))) {
            if (btn_state.stable) {
                midi_queue_item_t item = {Midi_CC::VOLUME, 0};
                if (!xQueueSend(args->midi_queue, &item, 0)) {
                    printf("[BTN] MIDI queue full, message dropped!\n");
                }
            }
        }
    }
}

static void ble_task(void* arg) {
    scan_task_args_t* args = (scan_task_args_t*)arg;
    midi_queue_item_t item;

    while (1) {
        // Wait for a command from scan_task
        if (xQueueReceive(args->midi_queue, &item, portMAX_DELAY)) {
            ble_send_midi_cc(item.cc, item.value); 
        }
    }
}

extern "C" void app_main() {
    gpio_init();
    ble_manager_init(); // Initialize BLE before starting tasks
    adc_init();
    adc_continuous_start(adc_handle);
    static scan_task_args_t args = {nullptr, nullptr};

    // Single-item queue — ISR overwrites it each tick so ticks never pile up
    args.tick_queue = xQueueCreate(1, sizeof(uint32_t));

    // Midi queue runs on core 0 with the BLE stack
    args.midi_queue = xQueueCreate(8, sizeof(midi_queue_item_t));

    // Start the 1ms timer; ISR will post to tick_queue immediately
    timer_init(args.tick_queue);

    printf("=== Timer-Driven Peripheral Scan (1ms tick, core 1) ===\n\n");

    // Pin scan_task to core 1 — BLE stack (NimBLE) will run on core 0
    xTaskCreatePinnedToCore(scan_task, "scan_task", 4096, &args, 5, NULL, 1);
    xTaskCreatePinnedToCore(ble_task, "ble_task", 4096, &args, 5, NULL, 0);
}