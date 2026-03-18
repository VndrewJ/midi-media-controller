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

    while (1) {
        xQueueReceive(args->tick_queue, &tick, portMAX_DELAY);

        if (debounce_btn(&btn_state, gpio_get_level(BUTTON_PIN))) {
            if (btn_state.stable) {
                midi_queue_item_t item = {Midi_CC::MUTE, 127};
                xQueueSend(args->midi_queue, &item, 0);
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