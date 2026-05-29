#include <Arduino.h>
#include "rfid.h"
#include "ntc_sync.h"
#include "apr_layer.h"
#include "espnow_gateway.h"
#include "ble.h"


const uint32_t READ_CARD_DELAY = 100;
const uint32_t UPDATE_INFO_DELAY = 50;
uint32_t last_read = 0;
uint32_t last_update = 0;
uint32_t last_pong = 0;

ntc_status ntc_init_status;

char* curr_card;
char* curr_time;
char new_record[25];
char* esp_records[MAX_PEER_CON];
const uint32_t ESP_RECORD_SIZE = 25;


void assmeble_and_send_new_message()
{
  curr_card = get_card();
  
  if (curr_card)
  {
    Serial.println(curr_card);
    if (ntc_init_status == NTC_SUCCESS)
    {
      curr_time = get_curr_time();
      snprintf(new_record, sizeof(new_record), "%s  -  %s\n", curr_card, curr_time);
    } else {
      snprintf(new_record, sizeof(new_record), "%s - X\n", curr_card);
    }
    
    bool res = send_data((uint8_t*)curr_card, strlen(curr_card), RFID_CHAR);
    update_cards_list(new_record, res);
    update_display();
  }
}


void translate_get_esp_now_status()
{
  general_status s = espnow_check_ping();
  uint32_t status = (uint32_t)s.resume_status;

  switch (status)
  {
    case FULL_PEERS_CONNECTED:
      set_new_color(ESP_NOW_LABEL, UI_OK);
      break;

    case PARTIAL_PEERS_CONNECTED:
      set_new_color(ESP_NOW_LABEL, UI_WARNING);
      break;

    case NONE_PEER_CONNECTED:
      set_new_color(ESP_NOW_LABEL, UI_ERROR);
      break;
    
    default:
      break;
  }


  for (int i = 0; i < MAX_PEER_CON; i++)
  {
    if (s.peers[i].code != 0)
    {
      const char* status_string = s.peers[i].status == ESPNOW_CONNECTED ? "Conectado" : "Desconectado"; 
      sniprintf(esp_records[i], ESP_RECORD_SIZE, "#%d - %s", s.peers[i].code, status_string);
      continue;
    }
    
    strncpy(esp_records[i], "", 1);
  }
  
  update_esps_list((const char**)esp_records);
  update_display();
}


void translate_and_update_get_ble_status()
{
  ble_status status = get_ble_status();

  switch (status)
  {
    case BLE_CONNECTED:
      set_new_color(BLE_LABEL, UI_OK);
      break;

    case BLE_DISCONNECTED:
      set_new_color(BLE_LABEL, UI_ERROR);
      break;

    case BLE_ADVERSITING:
      set_new_color(BLE_LABEL, UI_WARNING);
      break;
    
    default:
      break;
  }
}


void setup()
{
  Serial.begin(115200);
  
  ntc_init();
  ntc_status ntc_init_status = get_curr_ntc_status();
  
  init_espnow_gateway();
  ble_init_all();
  init_rfid();
  init_apr_layer();
  setup_draw_infos();
  

  if (get_curr_ntc_status() == NTC_SUCCESS) set_new_color(NTP_LABEL, UI_OK);
  else set_new_color(NTP_LABEL, UI_ERROR);  


  for (int i = 0; i < MAX_PEER_CON; i++)
    esp_records[i] = (char*)malloc(sizeof(char) * ESP_RECORD_SIZE);
  
}


void loop()
{
  lv_timer_handler();

  if (millis() - last_read > READ_CARD_DELAY) 
  {
    assmeble_and_send_new_message();
    last_read = millis();
  }
  

  if (millis() - last_update > UPDATE_INFO_DELAY)
  {
    translate_get_esp_now_status();
    translate_and_update_get_ble_status();
    last_update = millis();
  }

  if (millis() - last_pong > PING_INTERVAL)
  {
    ble_check_ping();
    last_pong = millis();
  }
  delay(15);




  // curr_time = get_curr_time();
  // Serial.println(curr_time);
  // delay(1000);




  // update_list((char*)&test);
  // update_display();
  // delay(1000);
  // test++;




  // set_new_color(NTC_LABEL, UI_ERROR);
  // set_new_color(ESP_NOW_LABEL, UI_WARNING);
  // update_display();
  // delay(300);

  // set_new_color(BLE_LABEL, UI_WARNING);
  // set_new_color(NTC_LABEL, UI_OK);
  // set_new_color(ESP_NOW_LABEL, UI_ERROR);
  // update_display();
  // delay(300);

  // set_new_color(BLE_LABEL, UI_ERROR);
  // set_new_color(NTC_LABEL, UI_WARNING);
  // set_new_color(ESP_NOW_LABEL, UI_OK);
  // update_display();
  // delay(300);
}