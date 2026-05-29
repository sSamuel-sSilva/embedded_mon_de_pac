#ifndef APR_LAYER_H
#define APR_LAYER_H

#include <LovyanGFX.hpp>
#include <lvgl.h>

// SCK	14
// MOSI	13
// DC	27
// RST	26
// CS	25
// BLK	33


const int MAX_CARDS = 3;
const int MAX_ESPS = 3;


enum obj_label {
    BLE_LABEL = 0,
    ESP_NOW_LABEL = 1,
    NTP_LABEL = 2,
};


enum apr_layer_status
{
    UI_OK,
    UI_WARNING,
    UI_ERROR
};


LV_FONT_DECLARE(minecraftia_14_2bpp);


bool init_apr_layer();
void setup_draw_infos();
void set_new_color(obj_label label, apr_layer_status status);
void update_cards_list(const char* text, bool res);
void update_esps_list(const char* text[]);
void update_display();

#endif
