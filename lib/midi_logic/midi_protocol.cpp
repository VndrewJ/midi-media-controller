#include "midi_protocol.h"

uint16_t Midi_Message::mask_13bit(uint64_t uptime_ms) {
    // BLE MIDI spec: 13-bit timestamp, 1ms resolution, wraps every 8192ms.
    return (uint16_t)(uptime_ms & 0x1FFF);
}

BLE_MIDI_Packet Midi_Message::build_cc(Midi_CC cc, uint8_t value, uint16_t ts_13bit, uint8_t channel) {
    BLE_MIDI_Packet pkt;
    pkt.data[0] = 0x80 | (uint8_t)(ts_13bit >> 7);   // Header:        MSB=1, ts[12:7]
    pkt.data[1] = 0x80 | (uint8_t)(ts_13bit & 0x7F); // Timestamp LSB: MSB=1, ts[6:0]
    pkt.data[2] = 0xB0 | (channel & 0x0F);            // CC status byte
    pkt.data[3] = (uint8_t)cc;                        // CC number
    pkt.data[4] = value & 0x7F;                       // CC value (7-bit)
    return pkt;
}
