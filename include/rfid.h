#ifndef RFID_H
#define RFID_H

#include <SPI.h>
#include <MFRC522.h>

bool init_rfid();
char* get_card();

#endif