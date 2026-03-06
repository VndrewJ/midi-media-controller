#include <unity.h>
#include <stdint.h>

#include "logic_pot.h"

void setUp(void) {
    // This is run before EACH TEST
}

void tearDown(void) {
    // This is run after EACH TEST
}

void test_convert_pot_value_to_midi_cc(void) {
    // Example test case for convert_pot_value_to_midi_cc
    TEST_ASSERT_EQUAL_UINT8(0, convert_pot_value_to_midi_cc(0));
    TEST_ASSERT_EQUAL_UINT8(127, convert_pot_value_to_midi_cc(4095));
    TEST_ASSERT_EQUAL_UINT8(64, convert_pot_value_to_midi_cc(2048));
}

void test_smooth_pot_value_fp_1(void) {
    midi_adc_t adc = {0, 0, 0};
    prime_accumulator(&adc); // Sets everything to 0

    adc.raw_data_input = 1000;
    uint16_t first_smoothed = smooth_pot_value_fp(&adc);
    // Assertion: It moved away from 0, but is nowhere near 1000 yet
    TEST_ASSERT(first_smoothed == 125); 

    adc.raw_data_input = 2000;
    uint16_t second_smoothed = smooth_pot_value_fp(&adc);
    // Assertion: It's higher than the first result, but still trailing the input
    TEST_ASSERT(second_smoothed == 359); 
}

void test_filter_stability(void) {
    midi_adc_t adc;
    uint16_t initial_val = 2048;

    adc.raw_data_input = initial_val;
    
    // Prime at midpoint
    prime_accumulator(&adc); 

    // Feed the same value 10 times
    for (int i = 0; i < 10; i++) {
        adc.raw_data_input = initial_val;
        uint16_t smoothed = smooth_pot_value_fp(&adc);
        
        TEST_ASSERT_EQUAL_UINT16(initial_val, smoothed);
    }
}

void test_filter_convergence(void) {
    midi_adc_t adc;
    int initial_val = 500;
    int final_val = 4095;
    
    adc.raw_data_input = initial_val;
    prime_accumulator(&adc); 

    // 1. Simulate the movement (The Ramp)
    for (int i = initial_val; i <= final_val; i += 5) {
        adc.raw_data_input = i;
        smooth_pot_value_fp(&adc);
    }

    // At this point, last_smoothed will be ~4060 (The Lag)
    TEST_ASSERT_LESS_THAN_UINT16(final_val, adc.last_smoothed);

    // 2. Simulate holding the knob at max (Settling)
    adc.raw_data_input = final_val;
    for (int i = 0; i < 50; i++) {
        smooth_pot_value_fp(&adc);
    }

    // NOW it should reach the maximum value exactly
    TEST_ASSERT_EQUAL_UINT16(final_val, adc.last_smoothed);
}

void test_apply_hysteresis(void) {
    // Example test case for apply_hysteresis
    TEST_ASSERT_EQUAL_UINT8(10, apply_hysteresis(10, 11)); // No change, within threshold
    TEST_ASSERT_EQUAL_UINT8(10, apply_hysteresis(10, 9));  // No change, within threshold
    TEST_ASSERT_EQUAL_UINT8(12, apply_hysteresis(10, 12)); // Change, exceeds threshold
    TEST_ASSERT_EQUAL_UINT8(8, apply_hysteresis(10, 8));   // Change, exceeds threshold
}

int main(void) {
    UNITY_BEGIN();

    RUN_TEST(test_convert_pot_value_to_midi_cc);
    RUN_TEST(test_smooth_pot_value_fp_1);
    RUN_TEST(test_filter_stability);
    RUN_TEST(test_filter_convergence);
    RUN_TEST(test_apply_hysteresis);
    return UNITY_END();
}