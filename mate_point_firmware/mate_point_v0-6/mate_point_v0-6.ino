/*
 * Mate Point firmware v0-6 — Wi-Fi SoftAP + portal web (NVS)
 *
 * USB CDC On Boot = Disabled.
 * UART0 (Serial) @ 9600 GPIO44/43 — solo protocolo Nobana, sin logs.
 */

#include "app_state.h"
#include "config.h"
#include "display_ui.h"
#include "drip_tray_sensor.h"
#include "lvgl_port.h"
#include "mate_network.h"
#include "nobana_uart.h"
#include "vl53l0x_sensor.h"

#include <Arduino.h>

static void on_configurar_red_pressed()
{
    mate_network_start_provisioning();
    display_ui_set_provisioning(true, mate_network_ap_ssid());
}

static void on_provisioning_cancel_pressed()
{
    mate_network_stop_provisioning();
    display_ui_set_provisioning(false, nullptr);
    display_ui_set_wifi(mate_network_wifi_ok());
    display_ui_set_mqtt(mate_network_mqtt_ok());
}

static void wait_tank_stable_boot()
{
    const uint32_t deadline = millis() + WATER_TANK_BOOT_WAIT_MS;
    while (!nobana_tank_stable_ready() && (int32_t)(deadline - millis()) > 0) {
        nobana_tick();
        delay(5);
    }
}

static void check_hardware_errors_boot()
{
    if (drip_tray_is_full()) {
        app_state_on_tray_full();
    } else if (nobana_tank_empty()) {
        app_state_on_tank_empty();
    }
}

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
    display_ui_set_continuar_callback(app_state_on_continuar_pressed);
    display_ui_set_finalizar_callback(app_state_on_finalizar_pressed);
    display_ui_set_configurar_red_callback(on_configurar_red_pressed);
    display_ui_set_provisioning_cancel_callback(on_provisioning_cancel_pressed);
    app_state_init();

    (void)vl53l0x_init();
    (void)drip_tray_init();
    wait_tank_stable_boot();
    check_hardware_errors_boot();
    mate_network_init();

    display_ui_set_wifi(mate_network_wifi_ok());
    display_ui_set_mqtt(mate_network_mqtt_ok());
    display_ui_set_provisioning(mate_network_is_provisioning(), mate_network_ap_ssid());
}

void loop()
{
    static bool prev_wifi = false;
    static bool prev_mqtt = false;
    static bool prev_provisioning = false;

    nobana_tick();
    mate_network_loop();

    switch (nobana_tank_poll()) {
    case TANK_TRANSITION_BECAME_EMPTY:
        app_state_on_tank_empty();
        break;
    case TANK_TRANSITION_BECAME_OK:
        app_state_on_tank_recovered();
        break;
    default:
        break;
    }

    switch (drip_tray_poll()) {
    case DRIP_TRAY_TRANSITION_BECAME_FULL:
        app_state_on_tray_full();
        break;
    case DRIP_TRAY_TRANSITION_BECAME_OK:
        app_state_on_tray_recovered();
        break;
    default:
        break;
    }

    app_state_tick();
    display_ui_tick();

    const bool wifi_ok = mate_network_wifi_ok();
    const bool mqtt_ok = mate_network_mqtt_ok();
    const bool provisioning = mate_network_is_provisioning();

    if (wifi_ok != prev_wifi) {
        display_ui_set_wifi(wifi_ok);
        prev_wifi = wifi_ok;
    }
    if (mqtt_ok != prev_mqtt) {
        display_ui_set_mqtt(mqtt_ok);
        prev_mqtt = mqtt_ok;
    }
    if (provisioning != prev_provisioning) {
        display_ui_set_provisioning(provisioning, mate_network_ap_ssid());
        prev_provisioning = provisioning;
    }

    delay(5);
}
