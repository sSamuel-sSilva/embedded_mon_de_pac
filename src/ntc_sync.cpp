#include "ntc_sync.h"
#include <Arduino.h>

static const char* SSID = "David";
static const char* PASSWORD = "47555193";

// static const char* SSID = "IFPI_Visitante";
// static const char* PASSWORD = "bwUXNdpft0m6R9f";
static const char* ntpServer = "pool.ntp.org";

static const uint32_t GMT_OFFSET_SEC = -10800;
static const uint32_t DAYLIGHT_OFFSET_SEC = 0;

static const uint32_t TIMEOUT = 15000;
static char current_time[10];

static ntc_status curr_status = NTC_ERROR; 
static struct tm timeinfo;

#define SERIAL_LOG_ERROR Serial.printf("ERROR in (%s) - [%s: %d]\n", __FILE__, __func__, __LINE__);


bool ntc_init()
{
    WiFi.begin(SSID, PASSWORD);

    uint32_t time_init = millis();
    while((millis() - time_init < TIMEOUT) && (WiFi.status() != WL_CONNECTED)) { delay(10); }

    if (WiFi.status() != WL_CONNECTED)
    {
        curr_status = NTC_ERROR;
        SERIAL_LOG_ERROR;
        return false;
    }

    configTime(GMT_OFFSET_SEC, DAYLIGHT_OFFSET_SEC, ntpServer);


    if (!getLocalTime(&timeinfo))
    {
        curr_status = NTC_ERROR;
        SERIAL_LOG_ERROR
        
        WiFi.disconnect(true);
        WiFi.mode(WIFI_OFF);
        
        return false;
    }
    
    curr_status = NTC_SUCCESS;
    
    WiFi.disconnect(true);
    WiFi.mode(WIFI_OFF);
    return true;
}


char* get_curr_time()
{

    if (!getLocalTime(&timeinfo))
    {
        curr_status = NTC_ERROR;
        SERIAL_LOG_ERROR
        return NULL;
    }
    
    snprintf(current_time, sizeof(current_time), "%.2d:%.2d:%.2d", timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);
    curr_status = NTC_SUCCESS;
    return current_time;
}


ntc_status get_curr_ntc_status()
{
    return curr_status;
}