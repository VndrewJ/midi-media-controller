#pragma once
#include <esp_adc/adc_continuous.h>

extern adc_continuous_handle_t adc_handle;

// Add esp32 calibration function

// Initialise ADC for potentiometer input
void adc_init();


