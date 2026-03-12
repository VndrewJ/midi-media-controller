
#ifndef LOGIC_POT_H
#define LOGIC_POT_H

#include <stdint.h>
#include <stdlib.h>

#define SMOOTHING_SHIFT 3                 // Change this for smooth factor (e.g., 3 for 1/8th, 4 for 1/16th, etc.)
#define HYSTERESIS_THRESHOLD 2            // Minimum change in MIDI CC value to consider it a valid update
#define BTN_DEBOUNCE_MS      20            // Ticks (ms) before a button edge is accepted

/*
    @brief Structure to hold the raw potentiometer input, the accumulator for smoothing, and the last smoothed value.
    @param raw_data_input The latest raw ADC reading from the potentiometer (0-4095).
    @param accumulator A high-resolution accumulator that maintains a history of inputs for smoothing.
    @param last_smoothed The last smoothed value that will be used to calculate the next MIDI CC value.
*/
struct midi_adc_t {
    uint16_t raw_data_input;   // x(i): 0-4095
    uint32_t accumulator;      // z(i): High-res history (persistent)
    uint16_t last_smoothed;    // y(i): The 12-bit result for the next loop
};

// Convert a potentiometer value (0-4095) to a MIDI CC value (0-127)
uint8_t convert_pot_value_to_midi_cc(int pot_value);

// Prime the accumulator with the first raw input to ensure it starts at a reasonable value
void prime_accumulator(midi_adc_t* adc);

// Smooth the potentiometer value to avoid abrupt changes in MIDI CC values
uint16_t smooth_pot_value_fp(midi_adc_t* adc);

// Hysteresis function to prevent rapid toggling of MIDI CC values around a threshold
uint8_t apply_hysteresis(uint8_t current_value, uint8_t new_value);

/*
    @brief State for a single debounced button.
    @param stable  The last accepted stable level (0 = pressed, 1 = released).
    @param counter Number of consecutive ticks the raw level has differed from stable.
*/
struct midi_btn_t {
    int stable;
    int counter;
};

// Initialise debounce state with the current pin level.
void init_btn(midi_btn_t* btn, int initial_level);

// Checks if button is debounced 
bool debounce_btn(midi_btn_t* btn, int raw_level);

#endif // LOGIC_POT_H