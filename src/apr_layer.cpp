#include "apr_layer.h"
#include <Arduino.h>

static const int MARGIN_BOTTOM_TITLE = 5;
static const int HEADER_HEIGHT = 40;
static const uint32_t TOTAL_HEADER_LABELS = 3;
static const uint32_t CONTAINER_HEIGHT = (320 - HEADER_HEIGHT) / 2;


struct obj_subject {
    lv_obj_t* obj;
    lv_subject_t subj;
};


struct obj_subject_text {
    lv_obj_t* obj;
    lv_subject_t subj;
    char buf[32];
};


static obj_subject header_labels[TOTAL_HEADER_LABELS];
static obj_subject_text cards_label[MAX_CARDS];
static obj_subject_text esps_label[MAX_ESPS];


static const uint32_t BUF_SIZE = 320 * 240 / 10 * 2;


class LGFX : public lgfx::LGFX_Device
{
  lgfx::Panel_ST7789 _panel;
  lgfx::Bus_SPI _bus;

  public:
    LGFX(void)
    {
        {
            auto cfg = _bus.config();

            cfg.spi_host = HSPI_HOST; // o canal normal é vspi, mas ta diferente para nao conflitar com o rc522
            cfg.spi_mode = 0;
            cfg.freq_write = 40000000;
            cfg.freq_read = 16000000;

            cfg.pin_sclk = 14;
            cfg.pin_mosi = 13;
            cfg.pin_miso = -1;
            cfg.pin_dc = 27;

            _bus.config(cfg);
            _panel.setBus(&_bus);
        }

        {
            auto cfg = _panel.config();

            cfg.pin_cs = 25;
            cfg.pin_rst = 26;
            cfg.pin_busy = -1;

            cfg.panel_width = 240;
            cfg.panel_height = 320;

            cfg.offset_x = 0;
            cfg.offset_y = 0;

            cfg.invert = true;

            _panel.config(cfg);
        }

        setPanel(&_panel);
    }
};


static LGFX tft;
static lv_display_t* display;


#define SERIAL_LOG_ERROR(msg) Serial.printf("ERROR in (%s) - [%s: %d]\nmsg = %s\n", __FILE__, __func__, __LINE__, msg);


static void cb_color_update(lv_observer_t * observer, lv_subject_t * subject)
{
    lv_obj_t* label = lv_observer_get_target_obj(observer);
    lv_color_t color = lv_subject_get_color(subject);
    lv_obj_set_style_text_color(label, color, 0);
}


static void my_flush_callback(lv_display_t* display, const lv_area_t* area, uint8_t* px_map)
{
    uint32_t w = area->x2 - area->x1 + 1;
    uint32_t h = area->y2 - area->y1 + 1;

    tft.startWrite();
    tft.setAddrWindow(area->x1, area->y1, w, h);
    tft.writePixels((uint16_t*)px_map, w * h);
    tft.endWrite();

    lv_display_flush_ready(display);
}


static uint32_t my_get_millis(void)
{
    return millis();
}


static void setup_draw_header()
{
    lv_obj_t* header = lv_obj_create(lv_screen_active());

    lv_obj_set_size(header, 240, HEADER_HEIGHT);
    lv_obj_set_style_bg_color(header, lv_palette_main(LV_PALETTE_GREY), 0);
    lv_obj_set_style_bg_opa(header, 64, 0);
    lv_obj_align(header, LV_ALIGN_TOP_MID, 0, 0);
    lv_obj_set_layout(header, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(header, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align( header,
        LV_FLEX_ALIGN_SPACE_EVENLY,
        LV_FLEX_ALIGN_CENTER,
        LV_FLEX_ALIGN_CENTER
    );

    lv_obj_set_style_bg_color(header, lv_palette_darken(LV_PALETTE_GREY, 3), 0);
    lv_obj_set_style_bg_opa(header, LV_OPA_50, 0);
    lv_obj_set_style_border_width(header, 2, 0);
    lv_obj_set_style_border_color(header, lv_palette_main(LV_PALETTE_BLUE_GREY), 0);
    lv_obj_set_style_radius(header, 0, 0);



    header_labels[0].obj = lv_label_create(header);
    lv_label_set_recolor(header_labels[0].obj, true);
    lv_label_set_text(header_labels[0].obj, "BLE");
    lv_obj_set_style_text_font(header_labels[0].obj, &minecraftia_14_2bpp, 0);
    lv_obj_set_style_text_color(header_labels[0].obj, lv_palette_lighten(LV_PALETTE_GREY, 1), 0);
        
    
    header_labels[1].obj = lv_label_create(header);
    lv_label_set_recolor(header_labels[1].obj, true);
    lv_label_set_text(header_labels[1].obj, "ESPNOW");
    lv_obj_set_style_text_font(header_labels[1].obj, &minecraftia_14_2bpp, 0);
    lv_obj_set_style_text_color(header_labels[1].obj, lv_palette_lighten(LV_PALETTE_GREY, 1), 0);


    header_labels[2].obj = lv_label_create(header);
    lv_label_set_recolor(header_labels[2].obj, true);
    lv_label_set_text(header_labels[2].obj, "NTP");
    lv_obj_set_style_text_font(header_labels[2].obj, &minecraftia_14_2bpp, 0);
    lv_obj_set_style_text_color(header_labels[2].obj, lv_palette_lighten(LV_PALETTE_GREY, 1), 0);
}


void setup_draw_infos()
{
    // cards
    lv_obj_t* cards_container = lv_obj_create(lv_screen_active());
    lv_obj_set_size(cards_container, 240, CONTAINER_HEIGHT);
    lv_obj_align(cards_container, LV_ALIGN_TOP_MID, 0, HEADER_HEIGHT);
    lv_obj_set_layout(cards_container, LV_LAYOUT_FLEX);
    lv_obj_set_style_radius(cards_container, 0, 0);
    lv_obj_set_flex_flow(cards_container, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_bg_color(cards_container, lv_palette_darken(LV_PALETTE_BLUE, 3), 0);
    lv_obj_set_style_bg_opa(cards_container, LV_OPA_30, 0);
    lv_obj_set_style_border_width(cards_container, 2, 0);
    lv_obj_set_style_border_color(cards_container, lv_palette_main(LV_PALETTE_BLUE), 0);
    lv_obj_set_style_pad_all(cards_container, 8, 0);



    lv_obj_t* cards_title = lv_label_create(cards_container);
    lv_obj_align(cards_title, LV_ALIGN_CENTER, 0, 5);
    lv_obj_set_style_margin_bottom(cards_title, MARGIN_BOTTOM_TITLE, 0);
    lv_obj_set_style_text_color(cards_title, lv_palette_lighten(LV_PALETTE_BLUE, 1), 0);
    lv_obj_set_style_align(cards_title, LV_ALIGN_TOP_MID, 0);
    lv_label_set_text(cards_title, "ULTIMOS CARTOES LIDOS");
    lv_obj_set_style_text_color(cards_title, lv_palette_lighten(LV_PALETTE_BLUE, 1), 0);
    lv_obj_set_style_text_font(cards_title, &minecraftia_14_2bpp, 0);
    

    for (int i = 0; i < MAX_CARDS; i++)
    {
        cards_label[i].obj = lv_label_create(cards_container);
        lv_label_set_recolor(cards_label[i].obj, true);
        lv_label_set_text(cards_label[i].obj, " ");
        lv_label_bind_text(cards_label[i].obj, &cards_label[i].subj, NULL);
        lv_obj_set_style_text_font(cards_label[i].obj, &minecraftia_14_2bpp, 0);
        lv_obj_set_style_text_color(cards_label[i].obj, lv_color_white(), 0);
        lv_label_bind_text(cards_label[i].obj, &cards_label[i].subj, NULL);

    }
    lv_subject_copy_string(&cards_label[0].subj, "Sem cartoes lidos");



    // esps
    lv_obj_t* esp_container = lv_obj_create(lv_screen_active());
    lv_obj_set_size(esp_container, 240, CONTAINER_HEIGHT);
    lv_obj_align(esp_container, LV_ALIGN_BOTTOM_MID, 0, 0);
    lv_obj_set_layout(esp_container, LV_LAYOUT_FLEX);
    lv_obj_set_style_radius(esp_container, 0, 0);
    lv_obj_set_flex_flow(esp_container, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_flow(esp_container, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_bg_color(esp_container, lv_palette_darken(LV_PALETTE_PURPLE, 4), 0);
    lv_obj_set_style_bg_opa(esp_container, LV_OPA_30, 0);
    lv_obj_set_style_border_width(esp_container, 2, 0);
    lv_obj_set_style_border_color(esp_container, lv_palette_main(LV_PALETTE_PURPLE), 0);
    lv_obj_set_style_pad_all(esp_container, 8, 0);
    
    lv_obj_t* esp_title = lv_label_create(esp_container);
    lv_obj_align(esp_title, LV_ALIGN_CENTER, 0, 5);
    lv_obj_set_style_margin_bottom(esp_title, MARGIN_BOTTOM_TITLE, 0);
    lv_obj_set_style_align(esp_title, LV_ALIGN_TOP_MID, 0);
    lv_label_set_text(esp_title, "ESPs CONECTADOS");
    lv_obj_set_style_text_color(esp_title, lv_palette_lighten(LV_PALETTE_PURPLE, 1), 0);
    lv_obj_set_style_text_font(esp_title, &minecraftia_14_2bpp, 0);
    // lv_obj_set_style_margin_bottom(esp_title, MARGIN_BOTTOM_TITLE, 0);


    for (int i = 0; i < MAX_ESPS; i++)
    {
        esps_label[i].obj = lv_label_create(esp_container);
        lv_label_set_text(esps_label[i].obj, " ");
        lv_label_bind_text(esps_label[i].obj, &esps_label[i].subj, NULL);
        lv_obj_set_style_text_font(esps_label[i].obj, &minecraftia_14_2bpp, 0);
        lv_obj_set_style_text_color(esps_label[i].obj, lv_color_white(), 0);
        lv_label_bind_text(esps_label[i].obj, &esps_label[i].subj, NULL);

    }
    lv_subject_copy_string(&esps_label[0].subj, "Sem ESPs conectados");
}


bool init_apr_layer()
{
    tft.init();
    lv_init();

    lv_tick_set_cb(my_get_millis);

    display = lv_display_create(240, 320);
    if (!display)
    {
        SERIAL_LOG_ERROR("none");
        return false;
    }


    static uint8_t* buf = (uint8_t*)heap_caps_malloc(BUF_SIZE, MALLOC_CAP_DMA);
    if (!buf)
    {
        SERIAL_LOG_ERROR("none");
        return false;
    }


    lv_display_set_buffers(display, buf, NULL, BUF_SIZE, LV_DISPLAY_RENDER_MODE_PARTIAL);
    lv_display_set_flush_cb(display, my_flush_callback);

    lv_theme_t* theme = lv_theme_default_init(display,
    lv_palette_main(LV_PALETTE_BLUE), lv_palette_main(LV_PALETTE_CYAN),
    true, &minecraftia_14_2bpp);

    if (!theme)
    {
        SERIAL_LOG_ERROR("none");
        return false;
    }

    setup_draw_header();

    for (int i = 0; i < TOTAL_HEADER_LABELS; i++)
    {
        lv_subject_init_color(&header_labels[i].subj, lv_palette_lighten(LV_PALETTE_GREY, 1));
        if (!lv_subject_add_observer_obj(&header_labels[i].subj, cb_color_update, header_labels[i].obj, NULL))
        {
            SERIAL_LOG_ERROR(i);
            return false;
        }
    }


    for (int i = 0; i < MAX_CARDS; i++)
        lv_subject_init_string(&cards_label[i].subj, cards_label[i].buf, NULL, sizeof(cards_label[i].buf), " ");
    

    for (int i = 0; i < MAX_ESPS; i++)
        lv_subject_init_string(&esps_label[i].subj, esps_label[i].buf, NULL, sizeof(esps_label[i].buf), " ");


    return true;
}


void set_new_color(obj_label label, apr_layer_status status)
{
    lv_color_t color;

    switch (status)
    {
        case UI_OK:
            color = lv_palette_lighten(LV_PALETTE_GREEN, 1);
            break;

        case UI_WARNING:
            color = lv_palette_lighten(LV_PALETTE_YELLOW, 1);
            break;

        case UI_ERROR:
            color = lv_palette_lighten(LV_PALETTE_RED, 1);
            break;
        
        default:
            break;
    }

    lv_subject_set_color(&header_labels[label].subj, color);
}


void update_cards_list(const char* text, bool res)
{
    if (!strcmp("Sem cartoes lidos", lv_subject_get_string(&cards_label[0].subj)))
    {
        lv_subject_copy_string(&cards_label[0].subj, text);
        lv_color_t color = res ? lv_palette_lighten(LV_PALETTE_GREEN, 1) : lv_palette_lighten(LV_PALETTE_RED, 1); 
        lv_obj_set_style_text_color(cards_label[0].obj, color, 0);

    } else {
        for (int i = MAX_CARDS-1; i > 0; i--)
        {
            lv_subject_copy_string(&cards_label[i].subj, lv_subject_get_string(&cards_label[i-1].subj));
            lv_obj_set_style_text_color(cards_label[i].obj, lv_obj_get_style_text_color(cards_label[i-1].obj, LV_PART_MAIN), 0);
        }

        lv_subject_copy_string(&cards_label[0].subj, text);
        lv_color_t color = res ? lv_palette_lighten(LV_PALETTE_GREEN, 1) : lv_palette_lighten(LV_PALETTE_RED, 1); 
        lv_obj_set_style_text_color(cards_label[0].obj, color, 0);
    }
}


void update_esps_list(const char* text[])
{
    for (int i = 0; i < MAX_ESPS; i++)
    {
        if (strlen(text[i]) == 0) continue;
        lv_subject_copy_string(&esps_label[i].subj, text[i]);
    }
}


void update_display()
{
    lv_refr_now(display);
}