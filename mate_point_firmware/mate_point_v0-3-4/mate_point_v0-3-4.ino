/*
 * Mate Point firmware v0-3-4 — Waveshare + Nobana UART + VL53L0X
 * Base: mate_point_v0-3-3
 *
 * Post-pago: validar termo (VL53L0X) → Iniciar → dispensado.
 * MQTT dispensing solo desde Iniciar. Auto-Parar si retiran termo.
 *
 * USB CDC On Boot = Disabled.
 * UART0 (Serial) @ 9600 GPIO44/43 — solo protocolo Nobana, sin logs.
 */

#include "app_state.h"
#include "config.h"
#include "display_ui.h"
#include "lvgl_port.h"
#include "mate_network.h"
#include "nobana_uart.h"
#include "vl53l0x_sensor.h"

#include <Arduino.h>

void setup()
{
    nobana_product_init();

    esp_lcd_touch_handle_t touch = touch_gt911_init();
    esp_lcd_panel_handle_t panel = waveshare_esp32_s3_rgb_lcd_init();
    wavesahre_rgb_lcd_bl_on();

    ESP_ERROR_CHECK(lvgl_port_init(panel, touch));
    display_ui_init();
    display_ui_set_comprar_callback(app_state_on_comprar_pressed);
    display_ui_set_iniciar_callback(app_state_on_iniciar_pressed);
    display_ui_set_parar_callback(app_state_on_parar_pressed);
    app_state_init();

    (void)vl53l0x_init();
    mate_network_init();
}

void loop()
{
    static bool prev_wifi = false;
    static bool prev_mqtt = false;

    nobana_tick();
    mate_network_loop();
    app_state_tick();

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
