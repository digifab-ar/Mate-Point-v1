#include "display_ui.h"

#include "lvgl_port.h"
#include "lvgl.h"

static lv_obj_t *label_main;
static lv_obj_t *label_wifi;
static lv_obj_t *label_mqtt;
static lv_obj_t *btn_comprar;
static lv_obj_t *panel_qr;
static lv_obj_t *img_qr;
static lv_obj_t *label_qr_hint;
static lv_obj_t *label_qr_timer;
static display_ui_comprar_cb_t comprar_cb = nullptr;

static void set_label_text(lv_obj_t *label, const char *text)
{
    if (lvgl_port_lock(-1)) {
        lv_label_set_text(label, text);
        lvgl_port_unlock();
    }
}

static void on_comprar_clicked(lv_event_t *e)
{
    (void)e;
    if (comprar_cb) {
        comprar_cb();
    }
}

void display_ui_init()
{
    if (!lvgl_port_lock(-1)) {
        return;
    }

    label_main = lv_label_create(lv_scr_act());
    lv_label_set_text(label_main, "");
    lv_obj_set_style_text_font(label_main, &lv_font_montserrat_44, 0);
    lv_obj_center(label_main);
    lv_obj_add_flag(label_main, LV_OBJ_FLAG_HIDDEN);

    btn_comprar = lv_btn_create(lv_scr_act());
    lv_obj_set_size(btn_comprar, 280, 90);
    lv_obj_center(btn_comprar);
    lv_obj_add_event_cb(btn_comprar, on_comprar_clicked, LV_EVENT_CLICKED, nullptr);

    lv_obj_t *btn_label = lv_label_create(btn_comprar);
    lv_label_set_text(btn_label, "Comprar");
    lv_obj_set_style_text_font(btn_label, &lv_font_montserrat_44, 0);
    lv_obj_center(btn_label);

    panel_qr = lv_obj_create(lv_scr_act());
    lv_obj_set_size(panel_qr, 360, 420);
    lv_obj_center(panel_qr);
    lv_obj_set_style_bg_color(panel_qr, lv_color_white(), 0);
    lv_obj_set_style_border_width(panel_qr, 4, 0);
    lv_obj_set_style_border_color(panel_qr, lv_color_hex(0x333333), 0);
    lv_obj_add_flag(panel_qr, LV_OBJ_FLAG_HIDDEN);

    label_qr_hint = lv_label_create(panel_qr);
    lv_label_set_text(label_qr_hint,
                      "QR estatico\n\n(placeholder)\n\nAntes de flashear:\nconvertir PNG\ncon LVGL converter");
    lv_obj_set_style_text_align(label_qr_hint, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_align(label_qr_hint, LV_ALIGN_CENTER, 0, -20);

    label_qr_timer = lv_label_create(panel_qr);
    lv_label_set_text(label_qr_timer, "2:00");
    lv_obj_set_style_text_font(label_qr_timer, &lv_font_montserrat_44, 0);
    lv_obj_align(label_qr_timer, LV_ALIGN_BOTTOM_MID, 0, -12);

    label_wifi = lv_label_create(lv_scr_act());
    lv_label_set_text(label_wifi, "Wifi: Error");
    lv_obj_align(label_wifi, LV_ALIGN_BOTTOM_LEFT, 16, -48);

    label_mqtt = lv_label_create(lv_scr_act());
    lv_label_set_text(label_mqtt, "MQTT: error");
    lv_obj_align(label_mqtt, LV_ALIGN_BOTTOM_LEFT, 16, -16);

    lvgl_port_unlock();
}

void display_ui_set_comprar_callback(display_ui_comprar_cb_t cb)
{
    comprar_cb = cb;
}

void display_ui_show_comprar(bool visible)
{
    if (!lvgl_port_lock(-1)) {
        return;
    }

    if (visible) {
        lv_obj_clear_flag(btn_comprar, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(label_main, LV_OBJ_FLAG_HIDDEN);
    } else {
        lv_obj_add_flag(btn_comprar, LV_OBJ_FLAG_HIDDEN);
    }

    lvgl_port_unlock();
}

void display_ui_set_qr_image(const lv_img_dsc_t *img_dsc)
{
    if (!img_dsc || !lvgl_port_lock(-1)) {
        return;
    }

    if (img_qr == nullptr) {
        img_qr = lv_img_create(panel_qr);
        lv_obj_align(img_qr, LV_ALIGN_TOP_MID, 0, 8);
    }

    lv_img_set_src(img_qr, img_dsc);
    lv_obj_clear_flag(img_qr, LV_OBJ_FLAG_HIDDEN);
    if (label_qr_hint) {
        lv_obj_add_flag(label_qr_hint, LV_OBJ_FLAG_HIDDEN);
    }

    lvgl_port_unlock();
}

void display_ui_show_qr(bool visible)
{
    if (!lvgl_port_lock(-1)) {
        return;
    }

    if (visible) {
        lv_obj_clear_flag(panel_qr, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(btn_comprar, LV_OBJ_FLAG_HIDDEN);
    } else {
        lv_obj_add_flag(panel_qr, LV_OBJ_FLAG_HIDDEN);
    }

    lvgl_port_unlock();
}

void display_ui_set_qr_countdown_sec(int seconds)
{
    if (seconds < 0) {
        seconds = 0;
    }
    const int min = seconds / 60;
    const int sec = seconds % 60;
    char buf[16];
    snprintf(buf, sizeof(buf), "%d:%02d", min, sec);
    set_label_text(label_qr_timer, buf);
}

void display_ui_set_main(const char *message)
{
    set_label_text(label_main, message);
}

void display_ui_set_main_visible(bool visible)
{
    if (!lvgl_port_lock(-1)) {
        return;
    }

    if (visible) {
        lv_obj_clear_flag(label_main, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(btn_comprar, LV_OBJ_FLAG_HIDDEN);
    } else {
        lv_obj_add_flag(label_main, LV_OBJ_FLAG_HIDDEN);
    }

    lvgl_port_unlock();
}

void display_ui_set_wifi(bool connected)
{
    set_label_text(label_wifi, connected ? "Wifi: Conectado" : "Wifi: Error");
}

void display_ui_set_mqtt(bool connected)
{
    set_label_text(label_mqtt, connected ? "MQTT: conectado" : "MQTT: error");
}
