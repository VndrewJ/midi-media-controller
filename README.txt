## Midi Media Controller For ESP32
Controls media using MIDI CC instead of HID for more precise control over control parameters such as volume.

# Specifications
- Utilise BLE for communications 
- Use MIDI CC
- Mute Button
- Potentiometer for volume control for individual apps
- Real-time or near real time
- Modular to add more features over time
- Split peripheral scanning and BLE onto different cores so they dont conflict
- Use queue to pass data from ISR to RTOS task

# Usage
- Needs MIDI Mixer Software to control windows applications


# TODO
1. Implement BLE MIDI Driver using core 1 of MCU
    a. Configure BLE for fast interval (7.5ms-15ms)
2. Implement 1ms timer for high freq polling of peripherals
    a. Mute Button
    b. Potentiometer    
        i. Calibration
        ii. Noise Filter
        iii. Circular buffer and average last x readings. Send MIDI CC if theres a change of > 1


