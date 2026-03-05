#include "logic_pot.h"

uint8_t convert_pot_value_to_midi_cc(int pot_value) {
    return (uint8_t)(pot_value >> 5) & 0x7F; // Scale down from 0-4095 to 0-127
}

uint16_t smooth_pot_value(uint16_t current_value, uint16_t new_value, float smoothing_factor) {
    //TODO
}