#include "espnow_gateway.h"
// #include "ble.h"


message_t received_package;


void init_espnow_gateway()
{
    WiFi.mode(WIFI_STA);
    WiFi.disconnect();

    Serial.print("WiFi channel: ");
    Serial.println(WiFi.channel());
    Serial.println(WiFi.macAddress());

    if (esp_now_init() != ESP_OK) {
        Serial.println("Error initializing ESP-NOW");
        return;
    }
    
    esp_now_register_recv_cb(onDataReceive);
    Serial.println("esp_now_getaway - iniciado");
}


void onDataReceive(const uint8_t* mac, const uint8_t* data, int len)
{
    memcpy(&received_package, data, sizeof(received_package));
    // Serial.print("Bytes received: ");
    // Serial.println(len);
    // Serial.print("code: ");
    // Serial.println(received_package.code);
    // received_package.code = 473197;
    // Serial.print("heartrate: ");
    // Serial.println(received_package.hr);
    // Serial.print("spo: ");
    // Serial.println(received_package.spo);
    // Serial.print("temp: ");
    // Serial.println(received_package.temp);
    // Serial.println();

    if (send_vitals(received_package))
    {
        Serial.println("enviado com succes");
        return;
    }
    Serial.println("falha no envio - package");
}