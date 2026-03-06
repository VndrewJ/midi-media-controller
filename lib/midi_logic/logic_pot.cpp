#include "logic_pot.h"

uint8_t convert_pot_value_to_midi_cc(int pot_value) {
    return (uint8_t)(pot_value >> 5); // Scale down from 0-4095 to 0-127
}

void prime_accumulator(midi_adc_t* adc) {
    adc->accumulator = ((uint32_t)adc->raw_data_input) << SMOOTHING_SHIFT; // Start with the first input as the initial smoothed value
    adc->last_smoothed = adc->raw_data_input; // Set the last smoothed value to the initial input
}

uint16_t smooth_pot_value_fp(midi_adc_t* adc) {
    // z(i) = z(i-1) - (z(i-1) / 8) + x(i)
    // 1. Logic: New Accumulator = Old Accumulator - (Old Accumulator / 8) + New Input
    // 8 is the smoothing factor
    adc->accumulator = adc->accumulator - (adc->accumulator >> SMOOTHING_SHIFT) + adc->raw_data_input;

    // 2. Extract the 12-bit smoothed value (y) from the accumulator (z)
    // Since z is essentially 8 * y, we shift right by 3 to get y.
    adc->last_smoothed = (uint16_t)(adc->accumulator >> SMOOTHING_SHIFT);

    return adc->last_smoothed;
}

uint8_t apply_hysteresis(uint8_t current_value, uint8_t new_value) {
    if (abs(new_value - current_value) >= HYSTERESIS_THRESHOLD) {
        return new_value; // Update if the change exceeds the threshold
    }
    return current_value; // Otherwise, keep the current value
}