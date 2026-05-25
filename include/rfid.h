#ifndef RFID_H
#define RFID_H


#include <SPI.h>
#include <MFRC522.h>

const int SDA_PIN  = 5;
const int SCK_PIN  = 18;
const int MISO_PIN = 19;
const int MOSI_PIN = 21;
const int RST_PIN  = 22;

extern MFRC522 mfrc;

void init_rfid();
bool read_card(char buf[16]);
// void get_card(char buf[16]);


#endif