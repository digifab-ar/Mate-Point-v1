#pragma once

#include "lvgl.h"

#define UI_PANEL_W          512
#define UI_PANEL_H          600
#define UI_SCREEN_W         1024
#define UI_SCREEN_H         600

/* RGB565-safe palette (R×8, G×4, B×8) — tuned for Waveshare 1024×600 */
#define UI_COLOR_GREEN_DARKEST    lv_color_hex(0x001000)
#define UI_COLOR_GREEN_DARK       lv_color_hex(0x001808)
#define UI_COLOR_GREEN_ACCENT     lv_color_hex(0xA0FC50)
#define UI_COLOR_GREEN_LIGHT      lv_color_hex(0xC8FCB8)
#define UI_COLOR_ORANGE_DARK      lv_color_hex(0x282008)
#define UI_COLOR_ORANGE_SECONDARY lv_color_hex(0xFF8028)
#define UI_COLOR_ORANGE_LIGHT     lv_color_hex(0xF8DCB0)

/* LVGL 8 built-in Montserrat is regular weight; CTA uses custom bold export. */
extern const lv_font_t ui_font_montserrat_bold_32;

#define UI_FONT_TITLE           (&lv_font_montserrat_36)
#define UI_FONT_PRODUCT_DESC    (&lv_font_montserrat_36)
#define UI_FONT_PRODUCT_PRICE   (&lv_font_montserrat_32)
#define UI_FONT_CTA             (&ui_font_montserrat_bold_32)
#define UI_FONT_CTA_ICON        (&lv_font_montserrat_32)
#define UI_FONT_CARD            (&lv_font_montserrat_20)
#define UI_FONT_VALUE           (&lv_font_montserrat_44)
#define UI_FONT_LABEL           (&lv_font_montserrat_14)

#define UI_CTA_W            420
#define UI_CTA_H            72

typedef enum {
    UI_TITLE_STYLE_GREEN,
    UI_TITLE_STYLE_ORANGE,
} UiTitleStyle;

typedef enum {
    UI_TITLE_LAYOUT_TOP,
    UI_TITLE_LAYOUT_CENTER_V,
} UiTitleLayout;
