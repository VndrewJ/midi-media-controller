#ifndef BLE_MANAGER_H
#define BLE_MANAGER_H

#include <stdint.h>
#include <NimBLEDevice.h>
#include "midi_protocol.h"

#define MIDI_SERVICE_UUID        "03B80E5A-EDE8-4B33-A751-6CE34EC4C700"
#define MIDI_CHARACTERISTIC_UUID "7772E5DB-3868-4112-A1A9-F2669D106BF3"
#define DEVICE_NAME              "ESP32 MIDI Controller"

extern bool ble_connected;

// Initialise BLE MIDI service and start advertising.
void ble_manager_init();

// Callbacks for BLE connection events
void ble_on_connect(NimBLEServer* pServer, ble_gap_conn_desc* desc);
void ble_on_disconnect(NimBLEServer* pServer);

// Get the current uptime in milliseconds since the device started. Uses esp timer
uint64_t uptime_ms();

// Send a BLE MIDI CC packet if a client is connected.
void ble_send_midi_cc(Midi_CC cc, uint8_t value);

#endif // BLE_MANAGER_H