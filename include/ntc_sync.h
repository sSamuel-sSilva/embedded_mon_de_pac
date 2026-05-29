#ifndef NTC_SYNC_H
#define NTC_SYNC_H

#include <WiFi.h>
#include "time.h"


enum ntc_status {
    NTC_SUCCESS,
    NTC_ERROR,
};


bool ntc_init();
char* get_curr_time();
ntc_status get_curr_ntc_status();


#endif