#include <stdbool.h>
#include "rfid.h"


static const int SDA_PIN  = 5;
static const int SCK_PIN  = 18;
static const int MISO_PIN = 19;
static const int MOSI_PIN = 21;
static const int RST_PIN  = 22;

static MFRC522 mfrc(SDA_PIN, RST_PIN);
static char current_card[16];

#define SERIAL_LOG_ERROR Serial.printf("ERROR in (%s) - [%s: %d]\n", __FILE__, __func__, __LINE__);


bool init_rfid()
{   
  SPI.begin(SCK_PIN, MISO_PIN, MOSI_PIN, SDA_PIN);
  mfrc.PCD_Init();

  byte v = mfrc.PCD_ReadRegister(mfrc.VersionReg);
  if (v == 0x00 || v == 0xFF)
  {
    SERIAL_LOG_ERROR
    return false;
  }

  return true;
}


static bool read_card()
{
  current_card[0] = '\0';

  if (!(&mfrc)->PICC_IsNewCardPresent()) return false;
  if (!(&mfrc)->PICC_ReadCardSerial()) return false;

  int off = 0;
  for (byte i = 0; i < (&mfrc)->uid.size && off < 31; i++)
  {
    off += snprintf(current_card + off, 32 - off,
                    (i == (&mfrc)->uid.size - 1) ? "%02X" : "%02X ",
                    (&mfrc)->uid.uidByte[i]);
  }

  (&mfrc)->PICC_HaltA();
  return true;
}


char* get_card()
{
  if (read_card()) return current_card;
  return NULL;
}
