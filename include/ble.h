#ifndef BLE_H
#define BLE_H

#include <Arduino.h>
#include <NimBLEDevice.h>
#include <nvs_flash.h>
#include <stdbool.h>

enum BLE_STATUS_T {
  CONNECTED = 0,
  DISCONNECTED = 1,
  ADVERSITING = 2
};

enum BLE_CHARS {
  RFID_CHAR = 0,
  VITALS_CHAR = 1
};


bool ble_init_all();
bool send_data(uint8_t* data, size_t size, BLE_CHARS chrs);
void check_ping();
BLE_STATUS_T get_status();

#endif