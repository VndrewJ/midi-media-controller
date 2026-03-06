#include "ble_manager.h"
#include <esp_timer.h>

uint64_t uptime_ms() {
    // Uses divide instead of bit shift due to accumulation error ~ 1h of 3.6s
    return (uint64_t)esp_timer_get_time() / 1000ULL;    
}