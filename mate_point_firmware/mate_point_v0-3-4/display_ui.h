#pragma once

#include "lvgl.h"

typedef void (*display_ui_comprar_cb_t)(void);
typedef void (*display_ui_parar_cb_t)(void);
typedef void (*display_ui_iniciar_cb_t)(void);

void display_ui_init();
void display_ui_set_comprar_callback(display_ui_comprar_cb_t cb);
void display_ui_set_parar_callback(display_ui_parar_cb_t cb);
void display_ui_set_iniciar_callback(display_ui_iniciar_cb_t cb);
void display_ui_show_comprar(bool visible);
void display_ui_show_qr(bool visible);
void display_ui_set_qr_image(const lv_img_dsc_t *img_dsc);
void display_ui_set_qr_countdown_sec(int seconds);
void display_ui_show_iniciar(bool visible);
void display_ui_show_dispensing(bool visible);
void display_ui_set_dispense_temp_c(uint8_t temp_c, bool valid);
void display_ui_set_dispense_countdown_sec(int seconds);
void display_ui_set_parar_enabled(bool enabled);
void display_ui_set_main(const char *message);
void display_ui_set_main_visible(bool visible);
void display_ui_set_termo_debug_visible(bool visible);
void display_ui_set_termo_debug(const char *line1, const char *line2);
void display_ui_set_wifi(bool connected);
void display_ui_set_mqtt(bool connected);
