#include "display_ui.h"
#include "lvgl_port.h"
#include "lvgl.h"

static lv_obj_t *label_main;
static lv_obj_t *label_wifi;
static lv_obj_t *label_mqtt;

static void set_label_text(lv_obj_t *label, const char *text)
{
    if (lvgl_port_lock(-1)) {
        lv_label_set_text(label, text);
        lvgl_port_unlock();
    }
}

void display_ui_init()
{
    if (!lvgl_port_lock(-1)) {
        return;
    }

    label_main = lv_label_create(lv_scr_act());
    lv_label_set_text(label_main, "Listo");
    lv_obj_set_style_text_font(label_main, &lv_font_montserrat_44, 0);
    lv_obj_center(label_main);

    label_wifi = lv_label_create(lv_scr_act());
    lv_label_set_text(label_wifi, "Wifi: Error");
    lv_obj_align(label_wifi, LV_ALIGN_BOTTOM_LEFT, 16, -48);

    label_mqtt = lv_label_create(lv_scr_act());
    lv_label_set_text(label_mqtt, "MQTT: error");
    lv_obj_align(label_mqtt, LV_ALIGN_BOTTOM_LEFT, 16, -16);

    lvgl_port_unlock();
}

void display_ui_set_main(const char *message)
{
    set_label_text(label_main, message);
}

void display_ui_set_wifi(bool connected)
{
    set_label_text(label_wifi, connected ? "Wifi: Conectado" : "Wifi: Error");
}

void display_ui_set_mqtt(bool connected)
{
    set_label_text(label_mqtt, connected ? "MQTT: conectado" : "MQTT: error");
}
