#pragma once

#include "lvgl.h"

typedef void (*display_ui_comprar_cb_t)(void);
typedef void (*display_ui_parar_cb_t)(void);
typedef void (*display_ui_iniciar_cb_t)(void);

enum UiErrorType {
    UI_ERR_PAGO = 0,
    UI_ERR_WIFI,
    UI_ERR_AGUA,
    UI_ERR_BANDEJA,
};

void display_ui_init();
void display_ui_tick();

void display_ui_set_comprar_callback(display_ui_comprar_cb_t cb);
void display_ui_set_parar_callback(display_ui_parar_cb_t cb);
void display_ui_set_iniciar_callback(display_ui_iniciar_cb_t cb);

void display_ui_show_standby(bool visible);
void display_ui_show_loading(bool visible);
void display_ui_show_qr(bool visible);
void display_ui_set_qr_image(const lv_img_dsc_t *img_dsc);
void display_ui_set_product_info(const char *description, const char *price_display);
void display_ui_show_coloca_termo(bool visible);
void display_ui_show_cargar_idle(bool visible);
void display_ui_show_cargar_dispensing(bool visible);
void display_ui_set_dispense_liters(float liters);
void display_ui_set_dispense_temp_c(uint8_t temp_c, bool valid);
void display_ui_set_parar_enabled(bool enabled);
void display_ui_show_finish(bool visible);
void display_ui_show_error(UiErrorType type);

void display_ui_set_wifi(bool connected);
void display_ui_set_mqtt(bool connected);

#if defined(UI_DEBUG_TERMO) && UI_DEBUG_TERMO
void display_ui_set_termo_debug_visible(bool visible);
void display_ui_set_termo_debug(const char *line1, const char *line2);
#else
static inline void display_ui_set_termo_debug_visible(bool visible)
{
    (void)visible;
}
static inline void display_ui_set_termo_debug(const char *line1, const char *line2)
{
    (void)line1;
    (void)line2;
}
#endif

/* Compat v0-3-4 — no-op o alias */
void display_ui_show_comprar(bool visible);
void display_ui_show_iniciar(bool visible);
void display_ui_show_dispensing(bool visible);
void display_ui_set_qr_countdown_sec(int seconds);
void display_ui_set_dispense_countdown_sec(int seconds);
void display_ui_set_main(const char *message);
void display_ui_set_main_visible(bool visible);
