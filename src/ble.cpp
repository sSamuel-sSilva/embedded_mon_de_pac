#include "ble.h"
#include "espnow_gateway.h"


NimBLEServer* server = NULL;
NimBLECharacteristic* rfid_char = NULL;
NimBLECharacteristic* ping_char = NULL;
NimBLECharacteristic* vitals_char = NULL;


bool connected = false;
unsigned long last_pong = 0;
int missed_pongs = 0;



void init_ble()
{
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        nvs_flash_erase();
        ret = nvs_flash_init();
    }
    
    NimBLEDevice::init("esp32");
    NimBLEDevice::setSecurityAuth(true, true, true);
    NimBLEDevice::setSecurityPasskey(123456);
    NimBLEDevice::setSecurityIOCap(BLE_HS_IO_DISPLAY_ONLY);

    server = NimBLEDevice::createServer();
    server->setCallbacks(new ServerCallbacks());
}


void init_rfid_service()
{
    NimBLEService* rfid_service = server->createService(RFID_SERVICE_UUID);
    rfid_char = rfid_service->createCharacteristic(
        RFID_CHAR_UUID,
        NIMBLE_PROPERTY::NOTIFY | NIMBLE_PROPERTY::READ
    );
    rfid_char->setValue("...");
    rfid_service->start();
    Serial.println("init_rfid_service - iniciado");
}


void init_vitals_service()
{
    NimBLEService* vitals_service = server->createService(VITALS_SERVICE_UUID);
    vitals_char = vitals_service->createCharacteristic(
        VITALS_CHAR_UUID,
        NIMBLE_PROPERTY::NOTIFY | NIMBLE_PROPERTY::READ,
        128
    );
    vitals_char->setValue("...");
    vitals_service->start();
    Serial.println("init_vitals_service - iniciado");
}


void init_pong_service()
{
    NimBLEService* ping_service = server->createService(PING_SERVICE_UUID);
    ping_char = ping_service->createCharacteristic(
        PING_CHAR_UUID,
        NIMBLE_PROPERTY::NOTIFY | NIMBLE_PROPERTY::WRITE
    );
    ping_char->setCallbacks(new PongCallbacks());
    ping_char->setValue("0");
    ping_service->start();
    Serial.println("init_pong_service - iniciado");
}


void init_adversiting()
{
    NimBLEAdvertising* advertising = NimBLEDevice::getAdvertising();
    advertising->setName("esp32");
    advertising->addServiceUUID(RFID_SERVICE_UUID);
    advertising->addServiceUUID(PING_SERVICE_UUID);
    advertising->addServiceUUID(VITALS_SERVICE_UUID);
    advertising->start();
    Serial.println("adversiting - iniciado");
}


bool send_card(const char* card)
{
    if (connected)
    {
      rfid_char->setValue((uint8_t*)card, strlen(card));
      rfid_char->notify();
      return true;
    }

    return false;
}


bool send_vitals(message_t packet)
{
    if (connected)
    {
        vitals_char->setValue((uint8_t*)&packet, sizeof(packet));
        vitals_char->notify();
    
        return true;
    }

    return false;
}


void check_ping()
{
  if (!connected) return;
  
  if (millis() - last_pong > 3000)
  {
    ping_char->setValue("ping");
    ping_char->notify();
    Serial.println("ping");
  }

  Serial.println("miliss - last pong: " + String(millis() - last_pong));
  if (millis() - last_pong > 10000)
  {
    Serial.println("10s sem pong - desconectando");
    
    auto peers = server->getPeerDevices();
    if (!peers.empty()) {
      server->disconnect(peers[0]);
    }
    connected = false;
  }
}