#include "rfid.h"


MFRC522 mfrc(SDA_PIN, RST_PIN);


void init_rfid()
{   
  SPI.begin(SCK_PIN, MISO_PIN, MOSI_PIN, SDA_PIN);
  mfrc.PCD_Init();

  byte v = mfrc.PCD_ReadRegister(mfrc.VersionReg);
  if (v == 0x00 || v == 0xFF)
  {
    Serial.println("rfid erro");
    return;
  }

  Serial.println("rfid ok");
}


bool read_card(char buf[16])
{
  buf[0] = '\0';

  if (!(&mfrc)->PICC_IsNewCardPresent()) return false;
  if (!(&mfrc)->PICC_ReadCardSerial()) return false;

  int off = 0;
  for (byte i = 0; i < (&mfrc)->uid.size && off < 31; i++)
  {
    off += snprintf(buf + off, 32 - off,
                    (i == (&mfrc)->uid.size - 1) ? "%02X" : "%02X ",
                    (&mfrc)->uid.uidByte[i]);
  }

  (&mfrc)->PICC_HaltA();
  return true;
}
