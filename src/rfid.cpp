#include "rfid.h"


MFRC522 mfrc(SS_PIN, RST_PIN);


void init_rfid()
{   
  SPI.begin();
  mfrc.PCD_Init();
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


// void get_card(char buf[16])
// {
//     strcpy(buf, card);
// }