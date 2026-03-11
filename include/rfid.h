#ifndef RFID_H
#define RFID_H


#include <SPI.h>
#include <MFRC522.h>

const int SS_PIN = 5;
const int RST_PIN = 22;

extern MFRC522 mfrc;


void init_rfid();
bool read_card(char buf[16]);
// void get_card(char buf[16]);


#endif