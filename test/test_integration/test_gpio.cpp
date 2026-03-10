#include <stdio.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "logic_pot.h"
#include "adc_driver.h"
#include "gpio_driver.h"

#define READ_LEN 256

// Task: Read potentiometer via ADC continuous mode, apply smoothing + hysteresis,
//       and print MIDI CC value only when it changes.
static void pot_task(void* arg) {
    uint8_t result[READ_LEN] = {0};
    uint32_t bytes_read = 0;

    midi_adc_t adc_state = {0, 0, 0};
    bool primed = false;
    uint8_t last_midi_cc = 0xFF; // 0xFF is out of MIDI range — guarantees first value is printed

    while (1) {
        esp_err_t ret = adc_continuous_read(adc_handle, result, READ_LEN, &bytes_read, pdMS_TO_TICKS(1000));
        if (ret == ESP_OK) {
            // Each sample is 4 bytes (adc_digi_output_data_t with TYPE1 format)
            for (int i = 0; i + 3 < (int)bytes_read; i += 4) {
                adc_digi_output_data_t* sample = (adc_digi_output_data_t*)&result[i];
                if (sample->type1.channel != ADC_CHANNEL_4) {
                    continue;
                }

                adc_state.raw_data_input = sample->type1.data;

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
        }
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

// Task: Poll the mute button and print its state on every change.
static void button_task(void* arg) {
    int last_state = gpio_get_level(BUTTON_PIN);

    while (1) {
        int current_state = gpio_get_level(BUTTON_PIN);
        if (current_state != last_state) {
            last_state = current_state;
            // Button uses PULLUP — low means physically pressed
            printf("[BTN] Button %s\n", current_state == 0 ? "PRESSED" : "RELEASED");
        }
        vTaskDelay(pdMS_TO_TICKS(20));
    }
}

extern "C" void app_main() {
    gpio_init();
    adc_init();
    adc_continuous_start(adc_handle);

    printf("=== GPIO & Pot Integration Test ===\n");
    printf("Actuate the button or potentiometer to see live values.\n\n");

    xTaskCreate(pot_task,    "pot_task",    4096, NULL, 5, NULL);
    xTaskCreate(button_task, "button_task", 2048, NULL, 5, NULL);
}