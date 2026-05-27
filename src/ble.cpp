#include <Arduino.h>
#include <stdbool.h>
#include "espnow_gateway.h"
#include "ble.h"


#define RFID_SERVICE_UUID "0000ffe0-0000-1000-8000-00805f9b34fb"
#define RFID_CHAR_UUID "0000ffe1-0000-1000-8000-00805f9b34fb"

#define VITALS_SERVICE_UUID "0000cafe-0000-1000-8000-00805f9b34fb"
#define VITALS_CHAR_UUID "0000c001-0000-1000-8000-00805f9b34fb"

#define PING_SERVICE_UUID "0000dead-0000-1000-8000-00805f9b34fb"
#define PING_CHAR_UUID "0000beef-0000-1000-8000-00805f9b34fb"


static NimBLEServer* server = NULL;
static NimBLECharacteristic* rfid_char = NULL;
static NimBLECharacteristic* ping_char = NULL;
static NimBLECharacteristic* vitals_char = NULL;

static NimBLECharacteristic* ble_ptr_chars[] = {rfid_char, vitals_char};

static const uint32_t PING_TIMEOUT = 10000;
static const uint32_t PING_INTERVAL = 3000;

static unsigned long last_pong = 0;
static uint32_t missed_pongs = 0;

ble_status bluetooth_status = BLE_DISCONNECTED; 

#define SERIAL_LOG_ERROR Serial.printf("ERROR in (%s) - [%s: %d]\n", __FILE__, __func__, __LINE__);


class ServerCallbacks : public NimBLEServerCallbacks
{
  void onConnect(NimBLEServer* server, NimBLEConnInfo& connInfo)
  {
    bluetooth_status = BLE_CONNECTED;
    last_pong = millis();
    Serial.println("conectado");
  }

  void onDisconnect(NimBLEServer* pServer, NimBLEConnInfo& connInfo, int reason)
  {
    NimBLEDevice::startAdvertising();
    Serial.println("advertising");
    bluetooth_status = BLE_ADVERSITING;
  }
};


class PingPongCallBacks : public NimBLECharacteristicCallbacks
{
    void onWrite(NimBLECharacteristic* pCharacteristc, NimBLEConnInfo &connInfo) override
    {
        last_pong = millis();
        missed_pongs = 0;
    }
};


static bool init_ble()
{
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        nvs_flash_erase();
        ret = nvs_flash_init();
    } else if (ret != ESP_OK) {
        SERIAL_LOG_ERROR
        return false;
    }
    
    NimBLEDevice::init("ESP32");

    server = NimBLEDevice::createServer();
    if (!server)
    {
        SERIAL_LOG_ERROR
        return false;
    }

    server->setCallbacks(new ServerCallbacks());
    server->start();
    return true;
}


static bool init_rfid_service()
{
    NimBLEService* rfid_service = server->createService(RFID_SERVICE_UUID);
    if (!rfid_service)
    {
        SERIAL_LOG_ERROR
        return false;
    }

    rfid_char = rfid_service->createCharacteristic(
        RFID_CHAR_UUID,
        NIMBLE_PROPERTY::NOTIFY | NIMBLE_PROPERTY::READ
    );

    if (!rfid_char)
    {
        SERIAL_LOG_ERROR
        return false;
    }    

    rfid_char->setValue("...");
    return true;
}


static bool init_vitals_service()
{
    NimBLEService* vitals_service = server->createService(VITALS_SERVICE_UUID);
    if (!vitals_service)
    {
        SERIAL_LOG_ERROR
        return false;
    }

    vitals_char = vitals_service->createCharacteristic(
        VITALS_CHAR_UUID,
        NIMBLE_PROPERTY::NOTIFY | NIMBLE_PROPERTY::READ,
        128
    );
    
    if (!vitals_char)
    {
        SERIAL_LOG_ERROR
        return false;
    }

    vitals_char->setValue("...");
    return true;
}


static bool init_pong_service()
{
    NimBLEService* ping_service = server->createService(PING_SERVICE_UUID);
    if (!ping_service)
    {
        SERIAL_LOG_ERROR
        return false;
    }

    ping_char = ping_service->createCharacteristic(
        PING_CHAR_UUID,
        NIMBLE_PROPERTY::NOTIFY | NIMBLE_PROPERTY::WRITE
    );
    
    if (!ping_char)
    {
        SERIAL_LOG_ERROR
        return false;
    }
    
    ping_char->setValue("0");
    return true;
}


static bool init_adversiting()
{
    NimBLEAdvertising* advertising = NimBLEDevice::getAdvertising();
    if (!advertising) return false;

    // existe alguma inconsistencia no retorno dessas funcoes
    // tudo esta sendo criado normalmente
    advertising->setName("ESP32");
    advertising->addServiceUUID(RFID_SERVICE_UUID);
    advertising->addServiceUUID(PING_SERVICE_UUID);
    advertising->addServiceUUID(VITALS_SERVICE_UUID);
    advertising->start();

    return true;
}


bool send_data(uint8_t* data, size_t size, ble_chars chr)
{
    if (bluetooth_status != BLE_CONNECTED)
    {
        SERIAL_LOG_ERROR
        return false;
    }
    
    NimBLECharacteristic* ch = ble_ptr_chars[chr];

    ch->setValue(data, size);
    if (!ch->notify())
    {
        SERIAL_LOG_ERROR
        return false;
    }

    return true;
}


void ble_check_ping()
{
  if (bluetooth_status == BLE_DISCONNECTED || bluetooth_status == BLE_ADVERSITING) return;
  
  if (millis() - last_pong > PING_INTERVAL)
  {
    ping_char->setValue("ping");
    ping_char->notify();
  }

  if (millis() - last_pong > PING_TIMEOUT)
  {    
    auto peers = server->getPeerDevices();
    if (!peers.empty())server->disconnect(peers[0]);
    
    bluetooth_status = BLE_DISCONNECTED;
  }
}


bool ble_init_all()
{
    if (!init_ble()) return false;
    if (!init_rfid_service()) return false;
    if (!init_pong_service()) return false;
    if (!init_vitals_service()) return false;
    if (!init_adversiting()) return false;

    return true;
}


ble_status get_ble_status()
{
    return bluetooth_status;
}