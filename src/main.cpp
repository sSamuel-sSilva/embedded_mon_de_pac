#include <LovyanGFX.hpp>
#include <lvgl.h>

LV_FONT_DECLARE(minecraftia);

const int MARGIN_BOTTOM_TITLE = 10;
const int HEADER_HEIGHT = 50;
const int MAX_CARDS = 6;
const int MAX_ESPS = 6;

// SCK	14
// MOSI	13
// DC	27
// RST	26
// CS	25
// BLK	33

class LGFX : public lgfx::LGFX_Device
{
  lgfx::Panel_ST7789 _panel;
  lgfx::Bus_SPI _bus;

  public:
    LGFX(void)
    {
        {
            auto cfg = _bus.config();

            cfg.spi_host = VSPI_HOST;
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
            // cfg.rgb_order = true;

            _panel.config(cfg);
        }

        setPanel(&_panel);
    }
};


LGFX tft;


void setup_draw_header()
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

  lv_obj_t* bluetooth_label = lv_label_create(header);
  lv_label_set_recolor(bluetooth_label, true);
  lv_label_set_text(bluetooth_label, "BLE");
  lv_obj_set_style_text_color(bluetooth_label, lv_palette_lighten(LV_PALETTE_RED, 1), 0);


  lv_obj_t* ntc_label = lv_label_create(header);
  lv_label_set_recolor(ntc_label, true);
  lv_label_set_text(ntc_label, "NTP");
  lv_obj_set_style_text_color(ntc_label, lv_palette_lighten(LV_PALETTE_GREEN, 1), 0);


  lv_obj_t* esp_now_label = lv_label_create(header);
  lv_label_set_recolor(esp_now_label, true);
  lv_label_set_text(esp_now_label, "ESPNOW");
  lv_obj_set_style_text_color(esp_now_label, lv_palette_lighten(LV_PALETTE_YELLOW, 1), 0);
}


void setup_draw_cards_log()
{
  lv_obj_t* container = lv_obj_create(lv_screen_active());
  lv_obj_set_size(container, 240, 320 - HEADER_HEIGHT);
  lv_obj_align(container, LV_ALIGN_BOTTOM_MID, 0, 0);
  lv_obj_set_layout(container, LV_LAYOUT_FLEX);
  lv_obj_set_flex_flow(container, LV_FLEX_FLOW_COLUMN);


  lv_obj_t* title = lv_label_create(container);
  lv_obj_align(title, LV_ALIGN_CENTER, 0, 5);
  lv_obj_set_style_margin_bottom(title, MARGIN_BOTTOM_TITLE, 0);
  lv_obj_set_style_text_color(title, lv_palette_lighten(LV_PALETTE_BLUE, 1), 0);
  lv_obj_set_style_align(title, LV_ALIGN_TOP_MID, 0);
  lv_label_set_text(title, "CARTOES LIDOS");

  const char* cards[MAX_CARDS] = {"Sem cartoes lidos", "", "", "", "", ""};
  lv_obj_t* labels[MAX_CARDS];
  
  for (int i = 00; i < MAX_CARDS; i++)
  {
    labels[i] = lv_label_create(container);
    lv_label_set_text(labels[i], cards[i]);
  }
}


void setup_draw_esp_connecteds()
{
  lv_obj_t* container = lv_obj_create(lv_screen_active());
  lv_obj_set_size(container, 240, 320 - HEADER_HEIGHT);
  lv_obj_align(container, LV_ALIGN_BOTTOM_MID, 0, 0);
  lv_obj_set_layout(container, LV_LAYOUT_FLEX);
  lv_obj_set_flex_flow(container, LV_FLEX_FLOW_COLUMN);

  lv_obj_t* title = lv_label_create(container);
  lv_obj_align(title, LV_ALIGN_CENTER, 0, 5);
  lv_obj_set_style_margin_bottom(title, MARGIN_BOTTOM_TITLE, 0);
  lv_obj_set_style_text_color(title, lv_palette_lighten(LV_PALETTE_BLUE, 1), 0);
  lv_obj_set_style_align(title, LV_ALIGN_TOP_MID, 0);
  lv_label_set_text(title, "ESPs CONECTADOS");

  const char* esps[MAX_ESPS] = {"Sem ESPs registrados", "", "", "", "",""};
  lv_obj_t* labels[MAX_ESPS];
  
  for (int i = 0; i < MAX_ESPS; i++)
  {
    labels[i] = lv_label_create(container);
    lv_label_set_text(labels[i], esps[i]);
  }
}


void my_flush_callback(lv_display_t* display, const lv_area_t* area, uint8_t* px_map)
{
  uint32_t w = area->x2 - area->x1 + 1;
  uint32_t h = area->y2 - area->y1 + 1;

  tft.startWrite();
  tft.setAddrWindow(area->x1, area->y1, w, h);
  tft.writePixels((uint16_t*)px_map, w * h);
  tft.endWrite();

  lv_display_flush_ready(display);
}


uint32_t my_get_millis(void)
{
  return millis();
}




void setup()
{
  tft.init();
  lv_init();

  lv_tick_set_cb(my_get_millis);

  lv_display_t* display = lv_display_create(240, 320);

  static uint8_t buf[320 * 240 / 10 * 2];
  lv_display_set_buffers(display, buf, NULL, sizeof(buf), LV_DISPLAY_RENDER_MODE_PARTIAL);
  lv_display_set_flush_cb(display, my_flush_callback);

  lv_theme_t* theme = lv_theme_default_init(
    display,
    lv_palette_main(LV_PALETTE_BLUE),
    lv_palette_main(LV_PALETTE_CYAN),
    true,
    &minecraftia
  );


  setup_draw_header();
  setup_draw_cards_log();
  // setup_draw_esp_connecteds();
}


void loop()
{
  lv_timer_handler();
  delay(5);
}