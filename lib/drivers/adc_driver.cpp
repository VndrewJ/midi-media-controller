#include "adc_driver.h"

adc_continuous_handle_t adc_handle = NULL;

void adc_init() {
    // 1. Define Handle Config
    adc_continuous_handle_cfg_t adc_handle_config = {
        .max_store_buf_size = 1024,
        .conv_frame_size = 256,
        .flags = {},
    };
    ESP_ERROR_CHECK(adc_continuous_new_handle(&adc_handle_config, &adc_handle));

    // 2. Define Pattern
    adc_digi_pattern_config_t adc_pattern = {
        .atten = ADC_ATTEN_DB_12,
        .channel = ADC_CHANNEL_4,                   // GPIO32 is ADC1 Channel 4
        .unit = ADC_UNIT_1,
        .bit_width = SOC_ADC_DIGI_MAX_BITWIDTH,
    };

    // 3. Apply Main Config
    adc_continuous_config_t adc_config = {
        .pattern_num = 1,
        .adc_pattern = &adc_pattern, 
        .sample_freq_hz = 20000,
        .conv_mode = ADC_CONV_SINGLE_UNIT_1,
        .format = ADC_DIGI_OUTPUT_FORMAT_TYPE1,
    };

    ESP_ERROR_CHECK(adc_continuous_config(adc_handle, &adc_config));
}