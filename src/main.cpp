#include <SPI.h>
#include <MFRC522.h>
#include <NimBLEDevice.h>
#include <nvs_flash.h>


const int SS_PIN = 5;
const int RST_PIN = 22;


#define RFID_SERVICE_UUID "0000ffe0-0000-1000-8000-00805f9b34fb"
#define RFID_CHAR_UUID "0000ffe1-0000-1000-8000-00805f9b34fb"

#define PING_SERVICE_UUID "0000dead-0000-1000-8000-00805f9b34fb"
#define PING_CHAR_UUID "0000beef-0000-1000-8000-00805f9b34fb"





MFRC522 rfid(SS_PIN, RST_PIN);
char card[16];


NimBLEServer* server = NULL;
NimBLECharacteristic* rfid_char = NULL;
NimBLECharacteristic* ping_char = NULL;


bool connected = false;
unsigned long last_pong = 0;
int missed_pongs = 0;


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

  void onAuthenticationComplete(NimBLEConnInfo& connInfo)
  {
    Serial.println("auth ok");
  }
};


void check_ping()
{
  if (!connected) return;
  
  if (millis() - last_pong > 3000)
  {
    // last_pong = millis();
    ping_char->setValue("ping");
    ping_char->notify();
    Serial.println("ping");
  }

  Serial.println("miliss - last pong: " + String(millis() - last_pong));
  if (millis() - last_pong > 10000)
  {
    Serial.println("❌ 10s sem pong - desconectando");
    
    auto peers = server->getPeerDevices();
    if (!peers.empty()) {
      server->disconnect(peers[0]);
    }
    connected = false;
  }
}


int read_card(char* buf, MFRC522* mfrc)
{
  buf[0] = '\0';

  if (!mfrc->PICC_IsNewCardPresent()) return 0;
  if (!mfrc->PICC_ReadCardSerial()) return 0;

  int off = 0;
  for (byte i = 0; i < mfrc->uid.size && off < 31; i++)
  {
    off += snprintf(buf + off, 32 - off,
                    (i == mfrc->uid.size - 1) ? "%02X" : "%02X ",
                    mfrc->uid.uidByte[i]);
  }

  mfrc->PICC_HaltA();
  return 1;
}


void setup()
{
  Serial.begin(115200);
  delay(1000);

  Serial.println("\niniciano");

  
  SPI.begin();
  rfid.PCD_Init();
  Serial.println("rfid ok");

  
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

  
  NimBLEService* rfid_service = server->createService(RFID_SERVICE_UUID);
  rfid_char = rfid_service->createCharacteristic(
    RFID_CHAR_UUID,
    NIMBLE_PROPERTY::NOTIFY | NIMBLE_PROPERTY::READ
  );
  rfid_char->setValue("...");
  rfid_service->start();

  
  NimBLEService* ping_service = server->createService(PING_SERVICE_UUID);
  ping_char = ping_service->createCharacteristic(
    PING_CHAR_UUID,
    NIMBLE_PROPERTY::NOTIFY | NIMBLE_PROPERTY::WRITE
  );
  ping_char->setCallbacks(new PongCallbacks());
  ping_char->setValue("0");
  ping_service->start();


  NimBLEAdvertising* advertising = NimBLEDevice::getAdvertising();
  advertising->setName("esp32");
  advertising->addServiceUUID(RFID_SERVICE_UUID);
  advertising->addServiceUUID(PING_SERVICE_UUID);
  advertising->start();

  Serial.println("pronto");
}

void loop()
{
  if (read_card(card, &rfid))
  {
    Serial.print("cartao lido: ");
    Serial.println(card);

    if (connected)
    {
      rfid_char->setValue((uint8_t*)card, strlen(card));
      rfid_char->notify();
      Serial.println("enviado");
    } else {
      Serial.println("nao conectado, nao enviado");
    }
    delay(500);
  }

  check_ping();
  delay(100);
}