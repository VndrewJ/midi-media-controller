#ifndef MIDI_PROTOCOL_H
#define MIDI_PROTOCOL_H

#include <stdint.h>

// Default MIDI Channel, add more later if needed
#define MIDI_CHANNEL 0

enum class Midi_CC : uint8_t {
    VOLUME = 7,
    MUTE   = 102,
};

// ── BLE MIDI packet ──────────────────────────────────────────────────────────
// For this use case, we only need 5 bytes. Can update later if we want to support more message types.
//   data[0]  Header byte       : 0x80 | ts[12:7]
//   data[1]  Timestamp LSB     : 0x80 | ts[6:0]
//   data[2]  Status byte       : 0xB0 | channel
//   data[3]  CC number         : (uint8_t)Midi_CC
//   data[4]  CC value          : 0-127
struct BLE_MIDI_Packet {
    uint8_t data[5];
};

class Midi_Message {
public:
    // Mask a raw uptime_ms value to the 13-bit range required by BLE MIDI.
    // Call with the result of ble_manager's uptime_ms()
    static uint16_t mask_13bit(uint64_t uptime_ms);

    // Build a ready-to-send BLE MIDI CC packet.
    // ts_13bit: result of mask_13bit(), must be provided by the caller.
    static BLE_MIDI_Packet build_cc(Midi_CC cc, uint8_t value, uint16_t ts_13bit, uint8_t channel = MIDI_CHANNEL);
};

#endif // MIDI_PROTOCOL_H