#include <SPI.h>
#include <MFRC522.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <BLE2902.h>

const int SS_PIN = 5;
const int RST_PIN = 22;

// UUIDs 16-bit - SIMPLES E FUNCIONAL
#define SERVICE_UUID        "ABCD"      // 0xABCD (4 caracteres)
#define CHARACTERISTIC_UUID "1234"      // 0x1234 (4 caracteres)

MFRC522 rfid(SS_PIN, RST_PIN);
char card[32];

// Variáveis globais
BLEServer* pServer = NULL;
BLECharacteristic* pCharacteristic = NULL;
bool deviceConnected = false;

// ==================== CALLBACKS ====================
class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
      deviceConnected = true;
      Serial.println("✅ Dispositivo conectado");
    }

    void onDisconnect(BLEServer* pServer) {
      deviceConnected = false;
      Serial.println("📵 Dispositivo desconectado");
      // Reinicia advertising após desconexão
      delay(500);
      pServer->getAdvertising()->start();
      Serial.println("🔄 Advertising reiniciado");
    }
};

// ==================== SETUP ====================
void setup() {
  Serial.begin(115200);
  delay(1000);  // Aguarda Serial
  
  Serial.println("\n═══════════════════════════════════════");
  Serial.println("       ESP32 RFID + BLE - SIMPLES      ");
  Serial.println("═══════════════════════════════════════");
  
  // 1. INICIALIZA RFID
  Serial.println("1. Inicializando RFID...");
  SPI.begin();
  rfid.PCD_Init();
  delay(200);
  
  // 2. INICIALIZA BLE (ORDEM CORRETA!)
  Serial.println("2. Inicializando BLE...");
  
  // A. Cria dispositivo BLE
  BLEDevice::init("ESP32-RFID-Reader");
  
  // B. Cria servidor BLE
  pServer = BLEDevice::createServer();
  
  // C. Configura callbacks do servidor
  pServer->setCallbacks(new MyServerCallbacks());
  
  // D. Cria serviço BLE
  BLEService *pService = pServer->createService(SERVICE_UUID);
  
  // E. Cria característica BLE
  pCharacteristic = pService->createCharacteristic(
                      CHARACTERISTIC_UUID,
                      BLECharacteristic::PROPERTY_READ   |
                      BLECharacteristic::PROPERTY_WRITE  |
                      BLECharacteristic::PROPERTY_NOTIFY
                    );
  
  // F. Adiciona descriptor OBRIGATÓRIO para notificações
  pCharacteristic->addDescriptor(new BLE2902());
  
  // G. Define valor inicial
  pCharacteristic->setValue("Aguardando cartão RFID...");
  
  // H. Inicia o serviço
  pService->start();
  
  // I. Configura advertising
  BLEAdvertising *pAdvertising = pServer->getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(false);
  pAdvertising->setMinPreferred(0x06);   // Parâmetros para conexão
  pAdvertising->setMaxPreferred(0x12);
  
  // J. Inicia advertising
  pAdvertising->start();
  
  Serial.println("3. Sistema pronto!");
  Serial.println("   Nome BLE: 'ESP32-RFID-Reader'");
  Serial.println("   UUID Serviço: 0xABCD");
  Serial.println("   UUID Característica: 0x1234");
  Serial.println("═══════════════════════════════════════\n");
}

// ==================== LEITURA RFID ====================
int read_card(char* card_buffer, MFRC522* mfrc) {
  // Limpa buffer
  card_buffer[0] = '\0';

  // Verifica se tem cartão
  if (!mfrc->PICC_IsNewCardPresent()) {
    return 0;
  }
  
  // Tenta ler o cartão
  if (!mfrc->PICC_ReadCardSerial()) {
    return 0;
  }

  // Converte UID para string
  int off = 0;
  for (byte i = 0; i < mfrc->uid.size && off < 31; i++) {
    off += snprintf(card_buffer + off, 32 - off,
                    (i == mfrc->uid.size - 1) ? "%02X" : "%02X ",
                    mfrc->uid.uidByte[i]);
  }

  // Para a leitura
  mfrc->PICC_HaltA();
  mfrc->PCD_StopCrypto1();
  
  return 1;
}

// ==================== LOOP ====================
void loop() {
  // Tenta ler cartão RFID
  if (read_card(card, &rfid)) {
    Serial.print("🎫 Cartão detectado: ");
    Serial.println(card);
    
    // Envia via BLE se estiver conectado
    if (deviceConnected) {
      pCharacteristic->setValue(card);
      pCharacteristic->notify();
      Serial.println("   📤 Enviado via BLE");
    } else {
      Serial.println("   ⚠️  Nenhum dispositivo BLE conectado");
    }
    
    delay(1000);  // Espera 1 segundo entre leituras
  }
  
  delay(100);  // Polling a cada 100ms
}