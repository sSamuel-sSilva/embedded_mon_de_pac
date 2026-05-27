#ifndef ESPNOW_GATEWAY_H
#define ESPNOW_GATEWAY_H

#include <WiFi.h>
#include <esp_now.h>
#include "ble.h"
#include <stdbool.h>

enum espnow_status {
  FULL_PEERS_CONNECTED = 0,
  PARTIAL_PEERS_CONNECTED = 1,
  NONE_PEER_CONNECTED = 2
};

enum peer_status {
    ESPNOW_CONNECTED = 0,
    ESPNOW_DISCONNECTED = 1
};

struct emmiter {
    uint32_t code;
    unsigned long last_ping;
    peer_status status;
};

struct general_status {
    espnow_status* resume_status;
    emmiter* peers;
};

bool init_espnow_gateway();
void espnow_check_ping();
general_status get_espnow_status();

#endif