#include <unity.h>
#include <stdint.h>
#include <time.h>

#include "logic_pot.h"
#include "midi_protocol.h"

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

// Helper function that generates synthetic uptime_ms values for testing mask_13bit
uint64_t generate_uptime_ms(uint64_t base, uint64_t offset) {
    return base + offset;

}

void test_mask_13bit(void) {
    // Example test case for mask_13bit
    TEST_ASSERT_EQUAL_UINT16(0, Midi_Message::mask_13bit(0));
    TEST_ASSERT_EQUAL_UINT16(1, Midi_Message::mask_13bit(1));
    TEST_ASSERT_EQUAL_UINT16(127, Midi_Message::mask_13bit(127));
    TEST_ASSERT_EQUAL_UINT16(8191, Midi_Message::mask_13bit(8191));  // Last valid value before wrap
    TEST_ASSERT_EQUAL_UINT16(0, Midi_Message::mask_13bit(8192));     // Wraps around with some jitter
    TEST_ASSERT_EQUAL_UINT16(1234, Midi_Message::mask_13bit(1234));   // Random Value
    TEST_ASSERT_EQUAL_UINT16(1864, Midi_Message::mask_13bit(10056));   // Wrap with offset (10056 % 8192 = 1864)
}

void test_build_cc(void) {
    // Example test case for build_cc
    uint16_t ts_13bit = 1234;
    BLE_MIDI_Packet pkt = Midi_Message::build_cc(Midi_CC::VOLUME, 64, ts_13bit);

    TEST_ASSERT_EQUAL_UINT8(0x80 | (ts_13bit >> 7), pkt.data[0]);   // Header byte
    TEST_ASSERT_EQUAL_UINT8(0x80 | (ts_13bit & 0x7F), pkt.data[1]); // Timestamp LSB
    TEST_ASSERT_EQUAL_UINT8(0xB0 | MIDI_CHANNEL, pkt.data[2]);      // Status byte
    TEST_ASSERT_EQUAL_UINT8((uint8_t)Midi_CC::VOLUME, pkt.data[3]); // CC number
    TEST_ASSERT_EQUAL_UINT8(64, pkt.data[4]);                       // CC value
}


int main(void) {
    UNITY_BEGIN();

    // adc logic tests
    RUN_TEST(test_convert_pot_value_to_midi_cc);
    RUN_TEST(test_smooth_pot_value_fp_1);
    RUN_TEST(test_filter_stability);
    RUN_TEST(test_filter_convergence);
    RUN_TEST(test_apply_hysteresis);
    // midi protocol tests
    RUN_TEST(test_mask_13bit);
    RUN_TEST(test_build_cc);


    return UNITY_END();
}