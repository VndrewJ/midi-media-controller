#include <gpio_driver.h>


void app_main() {
    // Initialize GPIO pins
    gpio_init();
    asm("nop");
}