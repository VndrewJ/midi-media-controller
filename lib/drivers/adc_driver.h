#include <esp_adc/adc_continuous.h>

adc_continuous_handle_t adc_handle = NULL;
adc_continuous_handle_cfg_t adc_config = {
    .max_store_buf_size = 1024,
    .conv_frame_size = 1024,
};
// Add esp32 calibration function

// Initialise ADC for potentiometer input
void adc_init();


