#include "adc_driver.h"

void adc_init() {
    ESP_ERROR_CHECK(adc_continuous_new_handle(&adc_config, &adc_handle));
}