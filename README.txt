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
1. Native logic DONE
    a. Potentiometer logic
    b. midi CC messages
2. Peripheral Drivers (core 1)
    a. mute button DONE
    b. timer setup for high freq polling TODO
    b. adc + potentiometer DONE
3. BLE driver (core 0)
    a. configure for fast interval (7.5-15ms)




