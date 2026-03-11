#include "rfid.h"
#include "ble.h"


char card[16];

unsigned long last_send = 0;

void setup()
{
  Serial.begin(115200);
  delay(1000);

  Serial.println("\niniciano");

  init_rfid();

  init_ble();
  init_rfid_service();
  init_vitals_service();
  init_pong_service();
  init_adversiting();
  
  
  Serial.println("pronto");
}


void loop()
{
  if (read_card(card))
  {
    if (send_card(card)) Serial.println("sucesso");
    else Serial.println("falha no envio");
    
    delay(500);
  }

  if (millis() - last_send > 2000)
  {
    last_send = millis();
    send_vitals();
  }

  check_ping();
  delay(100);
}