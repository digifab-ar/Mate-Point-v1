#include "config.h"
#include "display_ui.h"
#include "lvgl_port.h"
#include "mate_network.h"

#include <Arduino.h>

void setup()
{
    Serial.begin(115200);
    delay(500);
    Serial.println("[boot] Mate Point firmware");

    esp_lcd_touch_handle_t touch = touch_gt911_init();
    esp_lcd_panel_handle_t panel = waveshare_esp32_s3_rgb_lcd_init();
    wavesahre_rgb_lcd_bl_on();

    ESP_ERROR_CHECK(lvgl_port_init(panel, touch));
    display_ui_init();

    mate_network_init();
    display_ui_set_main("Listo");
}

void loop()
{
    static bool prev_wifi = false;
    static bool prev_mqtt = false;

    mate_network_loop();

    const bool wifi_ok = mate_network_wifi_ok();
    const bool mqtt_ok = mate_network_mqtt_ok();
    if (wifi_ok != prev_wifi) {
        display_ui_set_wifi(wifi_ok);
        prev_wifi = wifi_ok;
    }
    if (mqtt_ok != prev_mqtt) {
        display_ui_set_mqtt(mqtt_ok);
        prev_mqtt = mqtt_ok;
    }

    delay(5);
}
