#include <stdint.h>
#include <stdlib.h>

struct midi_adc_t {
    long accumulator;    // The "running total" for smoothing
    int lastMidiValue;   // For Hysteresis/Change detection
    int filterShift;     // Smoothing strength (e.g., 4)
};

// Convert a potentiometer value (0-4095) to a MIDI CC value (0-127)
uint8_t convert_pot_value_to_midi_cc(int pot_value);

// Smooth the potentiometer value to avoid abrupt changes in MIDI CC values
uint8_t smooth_pot_value(uint8_t current_value, uint8_t new_value, float smoothing_factor);

// Hysteresis function to prevent rapid toggling of MIDI CC values around a threshold
uint8_t apply_hysteresis(uint8_t current_value, uint8_t new_value, uint8_t threshold);