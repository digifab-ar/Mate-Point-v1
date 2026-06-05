#pragma once

#include "lvgl.h"

typedef void (*display_ui_comprar_cb_t)(void);

void display_ui_init();
void display_ui_set_comprar_callback(display_ui_comprar_cb_t cb);
void display_ui_show_comprar(bool visible);
void display_ui_show_qr(bool visible);
void display_ui_set_qr_image(const lv_img_dsc_t *img_dsc);
void display_ui_set_qr_countdown_sec(int seconds);
void display_ui_set_main(const char *message);
void display_ui_set_main_visible(bool visible);
void display_ui_set_wifi(bool connected);
void display_ui_set_mqtt(bool connected);
