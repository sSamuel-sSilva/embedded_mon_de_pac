#ifndef BLE_H
#define BLE_H

#include <NimBLEDevice.h>
#include <nvs_flash.h>
#include <Arduino.h>


#define RFID_SERVICE_UUID "0000ffe0-0000-1000-8000-00805f9b34fb"
#define RFID_CHAR_UUID "0000ffe1-0000-1000-8000-00805f9b34fb"

#define VITALS_SERVICE_UUID "0000cafe-0000-1000-8000-00805f9b34fb"
#define VITALS_CHAR_UUID "0000c001-0000-1000-8000-00805f9b34fb"

#define PING_SERVICE_UUID "0000dead-0000-1000-8000-00805f9b34fb"
#define PING_CHAR_UUID "0000beef-0000-1000-8000-00805f9b34fb"

extern NimBLEServer* server;
extern NimBLECharacteristic* rfid_char;
extern NimBLECharacteristic* ping_char;
extern NimBLECharacteristic* vitals_char;

extern bool connected;
extern unsigned long last_pong;
extern int missed_pongs;

typedef struct message_s  {
  uint32_t code;
  float hr;
  float spo;
  float temp;
} message_t;


void init_ble();
void init_rfid_service();
void init_vitals_service();
void init_pong_service();
void init_adversiting();
bool send_card(const char* card);
bool send_vitals(message_t packet);
void check_ping();


class PongCallbacks : public NimBLECharacteristicCallbacks
{
  void onWrite(NimBLECharacteristic* pChar, NimBLEConnInfo& connInfo)
  {
    std::string value = pChar->getValue();
    if (value.length() > 0)
    {
      Serial.printf("%s\n", value.c_str());
      last_pong = millis();
      missed_pongs = 0;
    }
  }
};


class ServerCallbacks : public NimBLEServerCallbacks
{
  void onConnect(NimBLEServer* server, NimBLEConnInfo& connInfo)
  {
    connected = true;
    last_pong = millis();
    Serial.println("conectado");
  }

  void onDisconnect(NimBLEServer* pServer, NimBLEConnInfo& connInfo, int reason)
  {
    connected = false;
    Serial.println("desconectado");
    delay(500);
    NimBLEDevice::startAdvertising();
    Serial.println("advertising");
  }
};

#endif