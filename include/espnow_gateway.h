#ifndef ESPNOW_GATEWAY_H
#define ESPNOW_GATEWAY_H

#include <WiFi.h>
#include <esp_now.h>
#include "ble.h"
#include <stdbool.h>


const uint32_t MAX_PEER_CON = 3; // por hora vai ser 3 porque só vou mostrar no maximo só 3 no monitor


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
    espnow_status resume_status;
    emmiter* peers;
};

bool init_espnow_gateway();
general_status espnow_check_ping();

#endif