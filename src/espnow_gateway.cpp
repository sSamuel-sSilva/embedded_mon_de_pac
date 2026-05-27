#include "espnow_gateway.h"
#include <unordered_map>

#define SERIAL_LOG_ERROR Serial.printf("ERROR in (%s) - [%s: %d]\n", __FILE__, __func__, __LINE__);


static const int PING_TIMEOUT = 3000;
static const int MAX_PEER_CON = 6; // por hora vai ser 6 porque só vou mostrar no maximo só 6 no monitor


enum package_type {
    VITAL_DATA = 0,
    PING_PONG = 1
};

struct package {
    package_type type;
    
    union
    {
        struct {
            uint32_t code;
        }ping;

        struct {
            uint32_t code;
            float hr;
            float spo;
            float temp;
        } vital;
    };
};


static emmiter emmiters[MAX_PEER_CON]; // os codes não estão pre-definidos, eles so chegam e conectam. 
                                // Futuramente configurar os peers pelo proprio esp via interface (ou outro meio)

static uint8_t emmiter_array_controller = 0; // impossivel passar de 255, mas se passar aumentar o tamanho
static espnow_status resume_status = NONE_PEER_CONNECTED;


static general_status status_manifest;


static void onDataReceive(const uint8_t* mac, const uint8_t* data_received, int len)
{
    struct package* data = (struct package*)data_received;

    switch (data->type)
    {
    case PING_PONG:
        {
            bool found = false;
            uint32_t code = data->ping.code;
            for (int i = 0; i < MAX_PEER_CON; i++)
                if (emmiters[i].code == code)
                {
                    emmiters[i].last_ping = millis();
                    found = true;
                }
            
            if (!found)
            {
                emmiters[emmiter_array_controller] = {code, millis(), ESPNOW_CONNECTED};
                emmiter_array_controller++;
            }

            if (emmiter_array_controller >= MAX_PEER_CON)
                emmiter_array_controller = MAX_PEER_CON;
        }
        break;
    
    case VITAL_DATA:
        {
            if (!send_data((uint8_t*)&data->vital, sizeof(data->vital), VITALS_CHAR)) SERIAL_LOG_ERROR
            break;
        }

    default:
        break;
    }
}


void espnow_check_ping()
{
    uint8_t peers_fault = 0;
    for(int i = 0; i < MAX_PEER_CON; i++)
    {
        if ((emmiters[i].code != 0) && (millis() - emmiters[i].last_ping > PING_TIMEOUT))
        {
            emmiters[i].status = ESPNOW_DISCONNECTED;
            peers_fault++;
        }
    }

    if (peers_fault == emmiter_array_controller) resume_status = NONE_PEER_CONNECTED;
    else if (peers_fault == 0) resume_status = FULL_PEERS_CONNECTED;
    else resume_status = PARTIAL_PEERS_CONNECTED;
}


bool init_espnow_gateway()
{
    WiFi.mode(WIFI_STA);
    WiFi.disconnect();

    if (esp_now_init() != ESP_OK)
    {
        SERIAL_LOG_ERROR
        return false;
    }
    
    if (esp_now_register_recv_cb(onDataReceive) != ESP_OK) return false;

    status_manifest.resume_status = &resume_status;
    status_manifest.peers = emmiters;

    return true;
}

general_status get_espnow_status()
{
    return status_manifest;
}