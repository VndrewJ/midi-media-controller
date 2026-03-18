#include "ble_manager.h"
#include "midi_protocol.h"
#include <esp_timer.h>

bool ble_connected = false;
static NimBLECharacteristic* s_pCharacteristic = nullptr;

void MyServerCallbacks::onConnect(NimBLEServer* pServer, NimBLEConnInfo& connInfo) {
    ble_connected = true;
    pServer->updateConnParams(connInfo.getConnHandle(), 6, 12, 0, 100);
    printf("Client Connected\n");
}

void MyServerCallbacks::onDisconnect(NimBLEServer* pServer, NimBLEConnInfo& connInfo, int reason) {
    ble_connected = false;
    pServer->getAdvertising()->start();
    printf("Client Disconnected - Restarted Advertising\n");
}

void ble_manager_init() {
    // Initialize NimBLE stack
    NimBLEDevice::init(DEVICE_NAME);
    NimBLEServer* pServer = NimBLEDevice::createServer();
    pServer->setCallbacks(new MyServerCallbacks());
    NimBLEService* pService = pServer->createService(MIDI_SERVICE_UUID);
    s_pCharacteristic = pService->createCharacteristic(
        MIDI_CHARACTERISTIC_UUID,
        NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::WRITE | NIMBLE_PROPERTY::NOTIFY
    );
    pService->start();
    NimBLEAdvertising* pAdvertising = NimBLEDevice::getAdvertising();
    pAdvertising->addServiceUUID(MIDI_SERVICE_UUID);
    pAdvertising->start();
    ble_connected = false;
}

void ble_on_connect(NimBLEServer* pServer, ble_gap_conn_desc* desc) {
    // max interval: 15ms, min interval: 7.5ms, latency: 0, supervision timeout: 1s
    ble_connected = true;
    pServer->updateConnParams(desc->conn_handle, 6, 12, 0, 100);
}

uint64_t uptime_ms() {
    // Uses divide instead of bit shift due to accumulation error ~ 1h of 3.6s
    return (uint64_t)esp_timer_get_time() / 1000ULL;
}

void ble_send_midi_cc(Midi_CC cc, uint8_t value) {
    if (!ble_connected || s_pCharacteristic == nullptr) return;
    uint16_t ts = Midi_Message::mask_13bit(uptime_ms());
    BLE_MIDI_Packet_t pkt = Midi_Message::build_cc(cc, value, ts);
    // s_pCharacteristic->setValue(pkt.data, sizeof(pkt.data));
    s_pCharacteristic->setValue(&pkt.data[0], 5);
    s_pCharacteristic->notify();
}