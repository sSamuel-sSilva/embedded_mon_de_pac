#include "rfid.h"
#include "ble.h"
#include "espnow_gateway.h"


char card[16];

// unsigned long last_send = 0;

void setup()
{
  Serial.begin(115200);
  delay(1000);

  Serial.println("\niniciano");

  init_rfid();

  init_espnow_gateway();

  init_ble();
  init_rfid_service();
  init_vitals_service();
  init_pong_service();
  init_adversiting();

  WiFi.mode(WIFI_MODE_STA);
  Serial.println(WiFi.macAddress());
  
  
  Serial.println("pronto");
}


void loop()
{
  if (read_card(card))
  {
    if (send_card(card)) Serial.println("sucesso");
    else Serial.println("falha no envio - card");
    Serial.println(card);
    delay(500);
  }

  // if (millis() - last_send > 2000)
  // {
  //   last_send = millis();
  //   send_vitals(message_t packet);
  // }

  check_ping();
  delay(100);
}