#ifndef ESPNOW_GATEWAY_H
#define ESPNOW_GATEWAY_H

#include <WiFi.h>
#include <esp_now.h>
#include "ble.h"



void init_espnow_gateway();
void onDataReceive(const uint8_t* mac, const uint8_t* data, int len);


#endif