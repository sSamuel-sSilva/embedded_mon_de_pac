#ifndef BLE_H
#define BLE_H

#include <Arduino.h>
#include <NimBLEDevice.h>
#include <nvs_flash.h>
#include <stdbool.h>

enum ble_status {
  BLE_CONNECTED = 0,
  BLE_DISCONNECTED = 1,
  BLE_ADVERSITING = 2
};

enum ble_chars {
  RFID_CHAR = 0,
  VITALS_CHAR = 1
};


bool ble_init_all();
bool send_data(uint8_t* data, size_t size, ble_chars chrs);
void ble_check_ping();
ble_status get_ble_status();

#endif