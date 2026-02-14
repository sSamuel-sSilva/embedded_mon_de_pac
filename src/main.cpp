#include <SPI.h>
#include <MFRC522.h>
// #include <BLEDevice.h>
// #include <BLEUtils.h>
// #include <BLEServer.h>
// #include <BLE2902.h>

#include <NimBLEDevice.h>
#include <nvs.h>
#include <nvs_flash.h>

const int SS_PIN = 5;
const int RST_PIN = 22;


#define SERVICE_UUID        "ABCD"      // 0xABCD (4 caracteres)
#define CHARACTERISTIC_UUID "1234"      // 0x1234 (4 caracteres)


MFRC522 rfid(SS_PIN, RST_PIN);
char card[16];


NimBLEService* service = NULL;
NimBLECharacteristic* characteristic = NULL;
NimBLEServer* server = NULL;

bool device_connected = false;


class server_callbacks: public NimBLEServerCallbacks
{
  void onConnect(NimBLEServer *pServer, NimBLEConnInfo &connInfo)
  {
    device_connected = true;
    Serial.println("disp.  conectado");
  }
  
  void onDisconnect(NimBLEServer *pServer, NimBLEConnInfo &connInfo, int reason)
  {
    device_connected = false;
    Serial.println("disp. desconectado");
    
    delay(500);
    NimBLEDevice::startAdvertising();
    Serial.println("reiniciando conexão");
  }
  
  void onAuthenticationComplete(NimBLEConnInfo& connInfo)
  {
    Serial.println("authentication complete");
  }
};

server_callbacks callbacks;

void setup()
{
  Serial.begin(115200);
  delay(1000);
  

  Serial.println("inicializando rfid");
  SPI.begin();
  rfid.PCD_Init();
  delay(200);
  

  Serial.println("inicializando BLE");
  

  esp_err_t ret = nvs_flash_init();
  if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
    nvs_flash_erase();
    ret = nvs_flash_init();
  }


  NimBLEDevice::init("esp32");

  NimBLEDevice::setSecurityAuth(true, true, false);
  NimBLEDevice::setSecurityPasskey(123456);
  NimBLEDevice::setSecurityIOCap(BLE_HS_IO_DISPLAY_ONLY);

  server = NimBLEDevice::createServer();
  server->setCallbacks(&callbacks);
  service = server->createService(SERVICE_UUID);

  characteristic = service->createCharacteristic(
                      CHARACTERISTIC_UUID,
                      NIMBLE_PROPERTY::NOTIFY | 
                      NIMBLE_PROPERTY::READ |
                      NIMBLE_PROPERTY::READ_AUTHEN
                    );
  

  characteristic->setValue("...");
  
  service->start();
  
  NimBLEAdvertising* advertising = NimBLEDevice::getAdvertising();
  advertising->setName("esp32");
  advertising->addServiceUUID(SERVICE_UUID);
  advertising->start();
  
  Serial.println("pronto");
}


int read_card(char* card_buffer, MFRC522* mfrc)
{
  
  card_buffer[0] = '\0';

  if (!mfrc->PICC_IsNewCardPresent()) return 0;
  if (!mfrc->PICC_ReadCardSerial()) return 0;


  int off = 0;
  for (byte i = 0; i < mfrc->uid.size && off < 31; i++)
  {
    off += snprintf(card_buffer + off, 32 - off,
                    (i == mfrc->uid.size - 1) ? "%02X" : "%02X ",
                    mfrc->uid.uidByte[i]);
  }


  mfrc->PICC_HaltA();
  mfrc->PCD_StopCrypto1();
  
  return 1;
}


void loop()
{
  if (read_card(card, &rfid)) {
    Serial.print("cartão detectado: ");
    Serial.println(card);
    
    if (device_connected)
    {
      characteristic->setValue((uint8_t*)card, strlen(card));
      characteristic->notify();
      Serial.println("enviado via BLE");
    } else {
      Serial.println("nenhum dispositivo BLE conectado");
    } 
    
    delay(700);
  }
  
  delay(100);
}