#include "display_ui.h"

#include "config.h"
#include "lvgl_port.h"
#include "ui/ui_assets.h"
#include "ui/ui_strings.h"
#include "ui/ui_theme.h"

#include <stdio.h>
#include <string.h>

enum ScreenId {
    SCR_NONE = 0,
    SCR_STANDBY,
    SCR_LOADING,
    SCR_QR,
    SCR_COLOCA,
    SCR_CARGAR,
    SCR_FINISH,
    SCR_ERROR,
};

static display_ui_comprar_cb_t comprar_cb = nullptr;
static display_ui_parar_cb_t parar_cb = nullptr;
static display_ui_iniciar_cb_t iniciar_cb = nullptr;
static display_ui_continuar_cb_t continuar_cb = nullptr;
static display_ui_finalizar_cb_t finalizar_cb = nullptr;
static display_ui_configurar_red_cb_t configurar_red_cb = nullptr;
static display_ui_provisioning_cancel_cb_t provisioning_cancel_cb = nullptr;

enum CargarUiMode {
    CARGAR_UI_IDLE,
    CARGAR_UI_DISPENSING,
    CARGAR_UI_PAUSED,
};

static CargarUiMode cargar_mode = CARGAR_UI_IDLE;
static bool cargar_continuar_visible = false;

static ScreenId current_screen = SCR_NONE;
static ScreenId saved_screen = SCR_NONE;
static UiErrorType current_error = UI_ERR_PAGO;
static bool connectivity_blocked = false;
static bool provisioning_active = false;
static bool wifi_ok = false;
static bool mqtt_ok = false;

static lv_obj_t *scr_standby;
static lv_obj_t *scr_loading;
static lv_obj_t *scr_qr;
static lv_obj_t *scr_coloca;
static lv_obj_t *scr_cargar;
static lv_obj_t *scr_finish;
static lv_obj_t *scr_error;
static lv_obj_t *scr_error_wifi;
static lv_obj_t *scr_error_mqtt;
static lv_obj_t *scr_provisioning;

static lv_obj_t *qr_img;
static lv_obj_t *qr_lbl_desc;
static lv_obj_t *qr_lbl_price;
static lv_obj_t *cargar_lbl_estado;
static lv_obj_t *cargar_lbl_litros;
static lv_obj_t *cargar_lbl_temp_val;
static lv_obj_t *cargar_btn_action;
static lv_obj_t *cargar_btn_continuar;
static lv_obj_t *cargar_btn_finalizar;
static lv_obj_t *error_left_img;
static lv_obj_t *error_lbl_title;
static lv_obj_t *error_title_bar;
static lv_obj_t *error_lbl_card;
static lv_obj_t *prov_lbl_wifi;
static char provisioning_ap_ssid[32] = "";

#if defined(UI_DEBUG_TERMO) && UI_DEBUG_TERMO
static lv_obj_t *label_termo_debug_1;
static lv_obj_t *label_termo_debug_2;
#endif

static bool ui_lock()
{
    return lvgl_port_lock(-1);
}

static void ui_unlock()
{
    lvgl_port_unlock();
}

static lv_obj_t *make_panel(lv_obj_t *parent, lv_coord_t x, lv_coord_t w, lv_color_t bg)
{
    lv_obj_t *panel = lv_obj_create(parent);
    lv_obj_set_size(panel, w, UI_PANEL_H);
    lv_obj_set_pos(panel, x, 0);
    lv_obj_set_style_bg_color(panel, bg, 0);
    lv_obj_set_style_bg_opa(panel, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(panel, 0, 0);
    lv_obj_set_style_radius(panel, 0, 0);
    lv_obj_set_style_pad_all(panel, 0, 0);
    lv_obj_clear_flag(panel, LV_OBJ_FLAG_SCROLLABLE);
    return panel;
}

static lv_obj_t *add_left_image(lv_obj_t *scr, const lv_img_dsc_t *img)
{
    lv_obj_t *left = lv_img_create(scr);
    lv_img_set_src(left, img);
    lv_obj_set_pos(left, 0, 0);
    return left;
}

static void apply_title_style(lv_obj_t *lbl, lv_obj_t *bar, UiTitleStyle style)
{
    if (!lbl || !bar) {
        return;
    }
    if (style == UI_TITLE_STYLE_ORANGE) {
        lv_obj_set_style_text_color(lbl, UI_COLOR_ORANGE_LIGHT, 0);
        lv_obj_set_style_bg_color(bar, UI_COLOR_ORANGE_SECONDARY, 0);
    } else {
        lv_obj_set_style_text_color(lbl, UI_COLOR_GREEN_LIGHT, 0);
        lv_obj_set_style_bg_color(bar, UI_COLOR_GREEN_ACCENT, 0);
    }
}

static lv_obj_t *add_title_block(lv_obj_t *panel, const char *title, UiTitleStyle style,
                                 UiTitleLayout layout)
{
    lv_obj_t *block = lv_obj_create(panel);
    lv_obj_set_width(block, UI_PANEL_W);
    lv_obj_set_height(block, LV_SIZE_CONTENT);
    lv_obj_set_style_bg_opa(block, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(block, 0, 0);
    lv_obj_set_style_pad_all(block, 0, 0);
    lv_obj_clear_flag(block, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t *lbl = lv_label_create(block);
    lv_label_set_text(lbl, title);
    lv_obj_set_style_text_font(lbl, UI_FONT_TITLE, 0);
    lv_obj_set_style_text_align(lbl, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_width(lbl, UI_PANEL_W - 48);
    lv_obj_align(lbl, LV_ALIGN_TOP_MID, 0, 0);

    lv_obj_t *bar = lv_obj_create(block);
    lv_obj_set_size(bar, 44, 4);
    lv_obj_set_style_border_width(bar, 0, 0);
    lv_obj_set_style_radius(bar, 2, 0);
    lv_obj_align_to(bar, lbl, LV_ALIGN_OUT_BOTTOM_MID, 0, 12);

    apply_title_style(lbl, bar, style);

    if (layout == UI_TITLE_LAYOUT_CENTER_V) {
        lv_obj_align(block, LV_ALIGN_CENTER, 0, 0);
    } else {
        lv_obj_align(block, LV_ALIGN_TOP_MID, 0, 42);
    }

    return lbl;
}

static lv_obj_t *get_title_bar(lv_obj_t *title_lbl)
{
    if (!title_lbl) {
        return nullptr;
    }
    lv_obj_t *block = lv_obj_get_parent(title_lbl);
    if (!block) {
        return nullptr;
    }
    return lv_obj_get_child(block, 1);
}

static lv_obj_t *add_bottom_card(lv_obj_t *panel, const char *text, lv_color_t bg, lv_color_t fg)
{
    lv_obj_t *card = lv_obj_create(panel);
    lv_obj_set_size(card, 440, 88);
    lv_obj_align(card, LV_ALIGN_BOTTOM_MID, 0, -36);
    lv_obj_set_style_bg_color(card, bg, 0);
    lv_obj_set_style_border_width(card, 0, 0);
    lv_obj_set_style_radius(card, 16, 0);
    lv_obj_clear_flag(card, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t *lbl = lv_label_create(card);
    lv_label_set_text(lbl, text);
    lv_obj_set_style_text_font(lbl, UI_FONT_CARD, 0);
    lv_obj_set_style_text_color(lbl, fg, 0);
    lv_obj_set_style_text_align(lbl, LV_TEXT_ALIGN_CENTER, 0);
    lv_label_set_long_mode(lbl, LV_LABEL_LONG_WRAP);
    lv_obj_set_width(lbl, 400);
    lv_obj_center(lbl);
    return card;
}

static void set_cta_arrow_icon(lv_obj_t *btn, lv_color_t fg)
{
    lv_obj_t *icon = lv_obj_get_child(btn, 1);
    if (!icon) {
        return;
    }
    lv_obj_clean(icon);
    lv_obj_set_size(icon, 32, 32);
    lv_obj_set_style_bg_opa(icon, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(icon, 0, 0);
    lv_obj_clear_flag(icon, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_clear_flag(icon, LV_OBJ_FLAG_CLICKABLE);

    lv_obj_t *arrow = lv_label_create(icon);
    lv_label_set_text(arrow, LV_SYMBOL_RIGHT);
    lv_obj_set_style_text_font(arrow, UI_FONT_CTA_ICON, 0);
    lv_obj_set_style_text_color(arrow, fg, 0);
    lv_obj_center(arrow);
}

static void set_cta_stop_icon(lv_obj_t *btn, lv_color_t fg)
{
    lv_obj_t *icon = lv_obj_get_child(btn, 1);
    if (!icon) {
        return;
    }
    lv_obj_clean(icon);
    lv_obj_set_size(icon, 32, 32);
    lv_obj_set_style_bg_opa(icon, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(icon, 0, 0);
    lv_obj_clear_flag(icon, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_clear_flag(icon, LV_OBJ_FLAG_CLICKABLE);

    lv_obj_t *ring = lv_obj_create(icon);
    lv_obj_set_size(ring, 28, 28);
    lv_obj_set_style_radius(ring, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_bg_opa(ring, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(ring, 2, 0);
    lv_obj_set_style_border_color(ring, fg, 0);
    lv_obj_clear_flag(ring, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_center(ring);

    lv_obj_t *square = lv_obj_create(ring);
    lv_obj_set_size(square, 10, 10);
    lv_obj_set_style_bg_color(square, fg, 0);
    lv_obj_set_style_border_width(square, 0, 0);
    lv_obj_set_style_radius(square, 0, 0);
    lv_obj_clear_flag(square, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_center(square);
}

static lv_obj_t *add_cta_button_at(lv_obj_t *panel, const char *text, lv_color_t bg, lv_color_t fg,
                                   lv_coord_t bottom_y, lv_event_cb_t cb)
{
    lv_obj_t *btn = lv_btn_create(panel);
    lv_obj_set_size(btn, UI_CTA_W, UI_CTA_H);
    lv_obj_align(btn, LV_ALIGN_BOTTOM_MID, 0, bottom_y);
    lv_obj_set_style_bg_color(btn, bg, 0);
    lv_obj_set_style_bg_color(btn, bg, LV_STATE_DISABLED);
    lv_obj_set_style_radius(btn, 12, 0);
    if (cb) {
        lv_obj_add_event_cb(btn, cb, LV_EVENT_CLICKED, nullptr);
    }

    lv_obj_t *lbl = lv_label_create(btn);
    lv_label_set_text(lbl, text);
    lv_obj_set_style_text_font(lbl, UI_FONT_CTA, 0);
    lv_obj_set_style_text_color(lbl, fg, 0);
    lv_obj_align(lbl, LV_ALIGN_LEFT_MID, 24, 0);

    lv_obj_t *icon = lv_obj_create(btn);
    lv_obj_align(icon, LV_ALIGN_RIGHT_MID, -20, 0);
    lv_obj_clear_flag(icon, LV_OBJ_FLAG_CLICKABLE);
    set_cta_arrow_icon(btn, fg);
    return btn;
}

static lv_obj_t *add_cta_button_plain(lv_obj_t *panel, const char *text, lv_color_t bg, lv_color_t fg,
                                      lv_coord_t bottom_y, lv_event_cb_t cb)
{
    lv_obj_t *btn = lv_btn_create(panel);
    lv_obj_set_size(btn, UI_CTA_W, UI_CTA_H);
    lv_obj_align(btn, LV_ALIGN_BOTTOM_MID, 0, bottom_y);
    lv_obj_set_style_bg_color(btn, bg, 0);
    lv_obj_set_style_bg_color(btn, bg, LV_STATE_DISABLED);
    lv_obj_set_style_radius(btn, 12, 0);
    if (cb) {
        lv_obj_add_event_cb(btn, cb, LV_EVENT_CLICKED, nullptr);
    }

    lv_obj_t *lbl = lv_label_create(btn);
    lv_label_set_text(lbl, text);
    lv_obj_set_style_text_font(lbl, UI_FONT_CTA, 0);
    lv_obj_set_style_text_color(lbl, fg, 0);
    lv_obj_center(lbl);
    return btn;
}

static lv_obj_t *add_cta_button(lv_obj_t *panel, const char *text, lv_color_t bg, lv_color_t fg,
                                lv_event_cb_t cb)
{
    return add_cta_button_at(panel, text, bg, fg, -48, cb);
}

static void load_screen(lv_obj_t *scr, ScreenId id)
{
    if (!scr) {
        return;
    }
    current_screen = id;
    if (!connectivity_blocked && !provisioning_active) {
        saved_screen = id;
    }
    lv_scr_load(scr);
}

static void on_standby_clicked(lv_event_t *e)
{
    (void)e;
    if (comprar_cb) {
        comprar_cb();
    }
}

static void on_cargar_action_clicked(lv_event_t *e)
{
    (void)e;
    if (cargar_mode == CARGAR_UI_DISPENSING) {
        if (parar_cb) {
            parar_cb();
        }
    } else if (cargar_mode == CARGAR_UI_IDLE && iniciar_cb) {
        iniciar_cb();
    }
}

static void on_cargar_continuar_clicked(lv_event_t *e)
{
    (void)e;
    if (continuar_cb) {
        continuar_cb();
    }
}

static void on_cargar_finalizar_clicked(lv_event_t *e)
{
    (void)e;
    if (finalizar_cb) {
        finalizar_cb();
    }
}

static void on_configurar_red_clicked(lv_event_t *e)
{
    (void)e;
    if (configurar_red_cb) {
        configurar_red_cb();
    }
}

static void on_provisioning_cancel_clicked(lv_event_t *e)
{
    (void)e;
    if (provisioning_cancel_cb) {
        provisioning_cancel_cb();
    }
}

static void build_standby()
{
    scr_standby = lv_obj_create(nullptr);
    lv_obj_set_size(scr_standby, UI_SCREEN_W, UI_SCREEN_H);
    lv_obj_clear_flag(scr_standby, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t *bg = lv_img_create(scr_standby);
    lv_img_set_src(bg, &img_full_image_ad1);
    lv_obj_set_pos(bg, 0, 0);

    add_cta_button(scr_standby, UI_STR_INICIAR, UI_COLOR_GREEN_ACCENT, UI_COLOR_GREEN_DARKEST,
                   on_standby_clicked);
}

static void build_loading()
{
    scr_loading = lv_obj_create(nullptr);
    lv_obj_set_size(scr_loading, UI_SCREEN_W, UI_SCREEN_H);
    lv_obj_set_style_bg_color(scr_loading, UI_COLOR_GREEN_DARKEST, 0);
    lv_obj_clear_flag(scr_loading, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t *lbl = lv_label_create(scr_loading);
    lv_label_set_text(lbl, UI_STR_LOADING);
    lv_obj_set_style_text_font(lbl, UI_FONT_VALUE, 0);
    lv_obj_set_style_text_color(lbl, UI_COLOR_GREEN_ACCENT, 0);
    lv_obj_center(lbl);
}

static void build_qr()
{
    scr_qr = lv_obj_create(nullptr);
    lv_obj_set_size(scr_qr, UI_SCREEN_W, UI_SCREEN_H);
    lv_obj_clear_flag(scr_qr, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t *left = make_panel(scr_qr, 0, UI_PANEL_W, UI_COLOR_GREEN_DARK);
    qr_lbl_desc = lv_label_create(left);
    lv_label_set_text(qr_lbl_desc, UI_PRODUCT_DESC_PLACEHOLDER);
    lv_obj_set_style_text_font(qr_lbl_desc, UI_FONT_PRODUCT_DESC, 0);
    lv_obj_set_style_text_color(qr_lbl_desc, UI_COLOR_GREEN_LIGHT, 0);
    lv_obj_set_style_text_align(qr_lbl_desc, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_width(qr_lbl_desc, 440);
    lv_obj_align(qr_lbl_desc, LV_ALIGN_TOP_MID, 0, 40);

    qr_lbl_price = lv_label_create(left);
    lv_label_set_text(qr_lbl_price, UI_PRODUCT_PRICE_PLACEHOLDER);
    lv_obj_set_style_text_font(qr_lbl_price, UI_FONT_PRODUCT_PRICE, 0);
    lv_obj_set_style_text_color(qr_lbl_price, UI_COLOR_GREEN_ACCENT, 0);
    lv_obj_align_to(qr_lbl_price, qr_lbl_desc, LV_ALIGN_OUT_BOTTOM_MID, 0, 16);

    qr_img = lv_img_create(left);
    lv_obj_align(qr_img, LV_ALIGN_CENTER, 0, 24);

    lv_obj_t *right = make_panel(scr_qr, UI_PANEL_W, UI_PANEL_W, UI_COLOR_GREEN_DARKEST);
    add_title_block(right, UI_STR_PAGA_QR, UI_TITLE_STYLE_GREEN, UI_TITLE_LAYOUT_CENTER_V);
    add_bottom_card(right, UI_STR_QR_CARD, UI_COLOR_GREEN_DARK, UI_COLOR_GREEN_LIGHT);
}

static void build_coloca()
{
    scr_coloca = lv_obj_create(nullptr);
    lv_obj_set_size(scr_coloca, UI_SCREEN_W, UI_SCREEN_H);
    lv_obj_clear_flag(scr_coloca, LV_OBJ_FLAG_SCROLLABLE);

    add_left_image(scr_coloca, &img_split_image_coloca_termo);
    lv_obj_t *right = make_panel(scr_coloca, UI_PANEL_W, UI_PANEL_W, UI_COLOR_GREEN_DARKEST);
    add_title_block(right, UI_STR_COLOCA_TERMO, UI_TITLE_STYLE_ORANGE, UI_TITLE_LAYOUT_CENTER_V);
    add_bottom_card(right, UI_STR_COLOCA_CARD, UI_COLOR_ORANGE_DARK, UI_COLOR_ORANGE_LIGHT);
}

static void build_cargar()
{
    scr_cargar = lv_obj_create(nullptr);
    lv_obj_set_size(scr_cargar, UI_SCREEN_W, UI_SCREEN_H);
    lv_obj_clear_flag(scr_cargar, LV_OBJ_FLAG_SCROLLABLE);

    add_left_image(scr_cargar, &img_split_image_ad1);
    lv_obj_t *right = make_panel(scr_cargar, UI_PANEL_W, UI_PANEL_W, UI_COLOR_GREEN_DARKEST);
    add_title_block(right, UI_STR_CARGAR_TERMO, UI_TITLE_STYLE_GREEN, UI_TITLE_LAYOUT_TOP);

    lv_obj_t *lbl_est = lv_label_create(right);
    lv_label_set_text(lbl_est, UI_STR_ESTADO_ESPERA);
    lv_obj_set_style_text_font(lbl_est, UI_FONT_LABEL, 0);
    lv_obj_set_style_text_color(lbl_est, UI_COLOR_GREEN_LIGHT, 0);
    lv_obj_align(lbl_est, LV_ALIGN_LEFT_MID, 48, -40);
    cargar_lbl_estado = lbl_est;

    cargar_lbl_litros = lv_label_create(right);
    lv_label_set_text(cargar_lbl_litros, "0.0 litros");
    lv_obj_set_style_text_font(cargar_lbl_litros, UI_FONT_VALUE, 0);
    lv_obj_set_style_text_color(cargar_lbl_litros, UI_COLOR_GREEN_ACCENT, 0);
    lv_obj_align(cargar_lbl_litros, LV_ALIGN_LEFT_MID, 48, 8);

    lv_obj_t *lbl_temp = lv_label_create(right);
    lv_label_set_text(lbl_temp, UI_STR_TEMPERATURA);
    lv_obj_set_style_text_font(lbl_temp, UI_FONT_LABEL, 0);
    lv_obj_set_style_text_color(lbl_temp, UI_COLOR_GREEN_LIGHT, 0);
    lv_obj_align(lbl_temp, LV_ALIGN_RIGHT_MID, -48, -40);

    cargar_lbl_temp_val = lv_label_create(right);
    lv_label_set_text(cargar_lbl_temp_val, UI_STR_TEMP_INVALID);
    lv_obj_set_style_text_font(cargar_lbl_temp_val, UI_FONT_VALUE, 0);
    lv_obj_set_style_text_color(cargar_lbl_temp_val, UI_COLOR_GREEN_LIGHT, 0);
    lv_obj_align(cargar_lbl_temp_val, LV_ALIGN_RIGHT_MID, -48, 8);

    cargar_btn_action = add_cta_button(right, UI_STR_INICIAR, UI_COLOR_GREEN_ACCENT,
                                       UI_COLOR_GREEN_DARKEST, on_cargar_action_clicked);

    cargar_btn_continuar = add_cta_button_at(right, UI_STR_CONTINUAR, UI_COLOR_GREEN_ACCENT,
                                             UI_COLOR_GREEN_DARKEST, -136,
                                             on_cargar_continuar_clicked);
    cargar_btn_finalizar = add_cta_button_at(right, UI_STR_FINALIZAR, UI_COLOR_ORANGE_SECONDARY,
                                           UI_COLOR_GREEN_DARKEST, -48,
                                           on_cargar_finalizar_clicked);
    set_cta_stop_icon(cargar_btn_finalizar, UI_COLOR_GREEN_DARKEST);
    lv_obj_add_flag(cargar_btn_continuar, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(cargar_btn_finalizar, LV_OBJ_FLAG_HIDDEN);
}

static void build_finish()
{
    scr_finish = lv_obj_create(nullptr);
    lv_obj_set_size(scr_finish, UI_SCREEN_W, UI_SCREEN_H);
    lv_obj_clear_flag(scr_finish, LV_OBJ_FLAG_SCROLLABLE);

    add_left_image(scr_finish, &img_split_image_retira_termo);
    lv_obj_t *right = make_panel(scr_finish, UI_PANEL_W, UI_PANEL_W, UI_COLOR_GREEN_DARKEST);
    add_title_block(right, UI_STR_FINISH, UI_TITLE_STYLE_GREEN, UI_TITLE_LAYOUT_CENTER_V);
}

static void build_error()
{
    scr_error = lv_obj_create(nullptr);
    lv_obj_set_size(scr_error, UI_SCREEN_W, UI_SCREEN_H);
    lv_obj_clear_flag(scr_error, LV_OBJ_FLAG_SCROLLABLE);

    error_left_img = lv_img_create(scr_error);
    lv_img_set_src(error_left_img, &img_split_image_error_pago);
    lv_obj_set_pos(error_left_img, 0, 0);

    lv_obj_t *right = make_panel(scr_error, UI_PANEL_W, UI_PANEL_W, UI_COLOR_GREEN_DARKEST);
    error_lbl_title = add_title_block(right, UI_STR_ERROR_PAGO, UI_TITLE_STYLE_GREEN,
                                      UI_TITLE_LAYOUT_CENTER_V);
    error_title_bar = get_title_bar(error_lbl_title);
    lv_obj_t *card = add_bottom_card(right, UI_STR_ERROR_PAGO_CARD, UI_COLOR_ORANGE_DARK,
                                     UI_COLOR_ORANGE_LIGHT);
    error_lbl_card = lv_obj_get_child(card, 0);
}

static void build_error_wifi()
{
    scr_error_wifi = lv_obj_create(nullptr);
    lv_obj_set_size(scr_error_wifi, UI_SCREEN_W, UI_SCREEN_H);
    lv_obj_clear_flag(scr_error_wifi, LV_OBJ_FLAG_SCROLLABLE);

    add_left_image(scr_error_wifi, &img_split_image_error_wifi_mqtt);
    lv_obj_t *right = make_panel(scr_error_wifi, UI_PANEL_W, UI_PANEL_W, UI_COLOR_GREEN_DARKEST);
    add_title_block(right, UI_STR_ERROR_WIFI, UI_TITLE_STYLE_ORANGE, UI_TITLE_LAYOUT_CENTER_V);
    add_cta_button(right, UI_STR_CONFIGURAR_RED, UI_COLOR_ORANGE_SECONDARY, UI_COLOR_GREEN_DARKEST,
                   on_configurar_red_clicked);
}

static void build_error_mqtt()
{
    scr_error_mqtt = lv_obj_create(nullptr);
    lv_obj_set_size(scr_error_mqtt, UI_SCREEN_W, UI_SCREEN_H);
    lv_obj_clear_flag(scr_error_mqtt, LV_OBJ_FLAG_SCROLLABLE);

    add_left_image(scr_error_mqtt, &img_split_image_error_wifi_mqtt);
    lv_obj_t *right = make_panel(scr_error_mqtt, UI_PANEL_W, UI_PANEL_W, UI_COLOR_GREEN_DARKEST);
    add_title_block(right, UI_STR_ERROR_SERVIDOR, UI_TITLE_STYLE_ORANGE, UI_TITLE_LAYOUT_CENTER_V);
    add_bottom_card(right, UI_STR_ERROR_PROVIDER, UI_COLOR_ORANGE_DARK, UI_COLOR_ORANGE_LIGHT);
}

static lv_obj_t *add_provisioning_card_line(lv_obj_t *card, const char *text, lv_coord_t y)
{
    lv_obj_t *lbl = lv_label_create(card);
    lv_label_set_text(lbl, text);
    lv_obj_set_style_text_font(lbl, UI_FONT_CARD, 0);
    lv_obj_set_style_text_color(lbl, UI_COLOR_ORANGE_LIGHT, 0);
    lv_obj_set_style_text_align(lbl, LV_TEXT_ALIGN_CENTER, 0);
    lv_label_set_long_mode(lbl, LV_LABEL_LONG_WRAP);
    lv_obj_set_width(lbl, 400);
    lv_obj_align(lbl, LV_ALIGN_TOP_MID, 0, y);
    return lbl;
}

static void build_provisioning()
{
    scr_provisioning = lv_obj_create(nullptr);
    lv_obj_set_size(scr_provisioning, UI_SCREEN_W, UI_SCREEN_H);
    lv_obj_clear_flag(scr_provisioning, LV_OBJ_FLAG_SCROLLABLE);

    add_left_image(scr_provisioning, &img_split_image_error_wifi_mqtt);
    lv_obj_t *right = make_panel(scr_provisioning, UI_PANEL_W, UI_PANEL_W, UI_COLOR_GREEN_DARKEST);
    add_title_block(right, UI_STR_PROV_TITLE, UI_TITLE_STYLE_ORANGE, UI_TITLE_LAYOUT_TOP);

    lv_obj_t *card = lv_obj_create(right);
    lv_obj_set_size(card, 440, 132);
    lv_obj_align(card, LV_ALIGN_CENTER, 0, 36);
    lv_obj_set_style_bg_color(card, UI_COLOR_ORANGE_DARK, 0);
    lv_obj_set_style_border_width(card, 0, 0);
    lv_obj_set_style_radius(card, 16, 0);
    lv_obj_set_style_pad_top(card, 20, 0);
    lv_obj_set_style_pad_bottom(card, 20, 0);
    lv_obj_clear_flag(card, LV_OBJ_FLAG_SCROLLABLE);

    add_provisioning_card_line(card, UI_STR_PROV_CARD_HEADER, 0);
    prov_lbl_wifi = add_provisioning_card_line(card, "", 36);
    add_provisioning_card_line(card, UI_STR_PROV_CARD_CONFIGURE, 72);

    add_cta_button_plain(right, UI_STR_PROV_CANCEL, UI_COLOR_ORANGE_SECONDARY, UI_COLOR_GREEN_DARKEST,
                         -48, on_provisioning_cancel_clicked);
}

static void update_provisioning_card_wifi(const char *ap_ssid)
{
    if (!prov_lbl_wifi) {
        return;
    }

    const char *ssid = ap_ssid;
    if (!ssid || ssid[0] == '\0') {
        ssid = provisioning_ap_ssid;
    }
    if (!ssid || ssid[0] == '\0') {
        ssid = "MatePoint-????";
    }

    char line[80];
    snprintf(line, sizeof(line), UI_STR_PROV_CARD_WIFI_FMT, ssid);
    lv_label_set_text(prov_lbl_wifi, line);
}

static void update_connectivity_overlay()
{
    if (current_screen == SCR_ERROR &&
        (current_error == UI_ERR_AGUA || current_error == UI_ERR_BANDEJA)) {
        return;
    }

    if (provisioning_active) {
        connectivity_blocked = true;
        update_provisioning_card_wifi(provisioning_ap_ssid);
        lv_scr_load(scr_provisioning);
        return;
    }

    if (!wifi_ok) {
        if (!connectivity_blocked || lv_scr_act() != scr_error_wifi) {
            connectivity_blocked = true;
            lv_scr_load(scr_error_wifi);
        }
        return;
    }

    if (!mqtt_ok) {
        if (!connectivity_blocked || lv_scr_act() != scr_error_mqtt) {
            connectivity_blocked = true;
            lv_scr_load(scr_error_mqtt);
        }
        return;
    }

    if (connectivity_blocked) {
        connectivity_blocked = false;
        switch (saved_screen) {
        case SCR_STANDBY:
            lv_scr_load(scr_standby);
            break;
        case SCR_LOADING:
            lv_scr_load(scr_loading);
            break;
        case SCR_QR:
            lv_scr_load(scr_qr);
            break;
        case SCR_COLOCA:
            lv_scr_load(scr_coloca);
            break;
        case SCR_CARGAR:
            lv_scr_load(scr_cargar);
            break;
        case SCR_FINISH:
            lv_scr_load(scr_finish);
            break;
        case SCR_ERROR:
            lv_scr_load(scr_error);
            break;
        default:
            lv_scr_load(scr_standby);
            break;
        }
        current_screen = saved_screen;
    }
}

static void apply_cargar_visibility()
{
    if (!cargar_btn_action || !cargar_btn_continuar || !cargar_btn_finalizar) {
        return;
    }

    if (cargar_mode == CARGAR_UI_PAUSED) {
        lv_obj_add_flag(cargar_btn_action, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(cargar_btn_finalizar, LV_OBJ_FLAG_HIDDEN);
        if (cargar_continuar_visible) {
            lv_obj_clear_flag(cargar_btn_continuar, LV_OBJ_FLAG_HIDDEN);
        } else {
            lv_obj_add_flag(cargar_btn_continuar, LV_OBJ_FLAG_HIDDEN);
        }
    } else {
        lv_obj_clear_flag(cargar_btn_action, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(cargar_btn_continuar, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(cargar_btn_finalizar, LV_OBJ_FLAG_HIDDEN);
    }
}

static void set_cargar_mode(CargarUiMode mode)
{
    cargar_mode = mode;
    if (!cargar_btn_action || !cargar_lbl_estado) {
        return;
    }

    switch (mode) {
    case CARGAR_UI_DISPENSING:
        lv_label_set_text(cargar_lbl_estado, UI_STR_ESTADO_CARGANDO);
        lv_label_set_text(lv_obj_get_child(cargar_btn_action, 0), UI_STR_PARAR);
        lv_obj_set_style_bg_color(cargar_btn_action, UI_COLOR_ORANGE_SECONDARY, 0);
        lv_obj_set_style_bg_color(cargar_btn_action, UI_COLOR_ORANGE_SECONDARY, LV_STATE_DISABLED);
        lv_obj_set_style_text_color(lv_obj_get_child(cargar_btn_action, 0), UI_COLOR_GREEN_DARKEST, 0);
        set_cta_stop_icon(cargar_btn_action, UI_COLOR_GREEN_DARKEST);
        break;
    case CARGAR_UI_PAUSED:
        cargar_continuar_visible = false;
        lv_label_set_text(cargar_lbl_estado, UI_STR_ESTADO_ESPERA);
        break;
    case CARGAR_UI_IDLE:
    default:
        lv_label_set_text(cargar_lbl_estado, UI_STR_ESTADO_ESPERA);
        lv_label_set_text(lv_obj_get_child(cargar_btn_action, 0), UI_STR_INICIAR);
        lv_obj_set_style_bg_color(cargar_btn_action, UI_COLOR_GREEN_ACCENT, 0);
        lv_obj_set_style_bg_color(cargar_btn_action, UI_COLOR_GREEN_ACCENT, LV_STATE_DISABLED);
        lv_obj_set_style_text_color(lv_obj_get_child(cargar_btn_action, 0), UI_COLOR_GREEN_DARKEST, 0);
        set_cta_arrow_icon(cargar_btn_action, UI_COLOR_GREEN_DARKEST);
        break;
    }

    apply_cargar_visibility();
}

static const lv_img_dsc_t *error_left_image(UiErrorType type)
{
    switch (type) {
    case UI_ERR_PAGO:
        return &img_split_image_error_pago;
    case UI_ERR_WIFI:
        return &img_split_image_error_wifi_mqtt;
    case UI_ERR_AGUA:
    case UI_ERR_BANDEJA:
        return &img_split_image_agua;
    default:
        return &img_split_image_error_pago;
    }
}

static const char *error_title(UiErrorType type)
{
    switch (type) {
    case UI_ERR_PAGO:
        return UI_STR_ERROR_PAGO;
    case UI_ERR_WIFI:
        return UI_STR_ERROR_WIFI;
    case UI_ERR_AGUA:
        return UI_STR_ERROR_AGUA;
    case UI_ERR_BANDEJA:
        return UI_STR_ERROR_BANDEJA;
    default:
        return UI_STR_ERROR_PAGO;
    }
}

static const char *error_card(UiErrorType type)
{
    switch (type) {
    case UI_ERR_PAGO:
        return UI_STR_ERROR_PAGO_CARD;
    default:
        return UI_STR_ERROR_PROVIDER;
    }
}

void display_ui_init()
{
    if (!ui_lock()) {
        return;
    }

    build_standby();
    build_loading();
    build_qr();
    build_coloca();
    build_cargar();
    build_finish();
    build_error();
    build_error_wifi();
    build_error_mqtt();
    build_provisioning();

#if defined(UI_DEBUG_TERMO) && UI_DEBUG_TERMO
    label_termo_debug_1 = lv_label_create(scr_coloca);
    label_termo_debug_2 = lv_label_create(scr_coloca);
    lv_obj_add_flag(label_termo_debug_1, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(label_termo_debug_2, LV_OBJ_FLAG_HIDDEN);
#endif

    load_screen(scr_standby, SCR_STANDBY);
    ui_unlock();
}

void display_ui_tick()
{
    (void)0;
}

void display_ui_set_comprar_callback(display_ui_comprar_cb_t cb)
{
    comprar_cb = cb;
}

void display_ui_set_parar_callback(display_ui_parar_cb_t cb)
{
    parar_cb = cb;
}

void display_ui_set_iniciar_callback(display_ui_iniciar_cb_t cb)
{
    iniciar_cb = cb;
}

void display_ui_set_continuar_callback(display_ui_continuar_cb_t cb)
{
    continuar_cb = cb;
}

void display_ui_set_finalizar_callback(display_ui_finalizar_cb_t cb)
{
    finalizar_cb = cb;
}

void display_ui_set_configurar_red_callback(display_ui_configurar_red_cb_t cb)
{
    configurar_red_cb = cb;
}

void display_ui_set_provisioning_cancel_callback(display_ui_provisioning_cancel_cb_t cb)
{
    provisioning_cancel_cb = cb;
}

void display_ui_set_provisioning(bool active, const char *ap_ssid)
{
    provisioning_active = active;
    if (ap_ssid) {
        strncpy(provisioning_ap_ssid, ap_ssid, sizeof(provisioning_ap_ssid) - 1);
        provisioning_ap_ssid[sizeof(provisioning_ap_ssid) - 1] = '\0';
    } else if (!active) {
        provisioning_ap_ssid[0] = '\0';
    }

    if (!ui_lock()) {
        return;
    }
    update_connectivity_overlay();
    ui_unlock();
}

void display_ui_show_standby(bool visible)
{
    if (!visible || !ui_lock()) {
        return;
    }
    load_screen(scr_standby, SCR_STANDBY);
    update_connectivity_overlay();
    ui_unlock();
}

void display_ui_show_loading(bool visible)
{
    if (!visible || !ui_lock()) {
        return;
    }
    load_screen(scr_loading, SCR_LOADING);
    update_connectivity_overlay();
    ui_unlock();
}

void display_ui_show_qr(bool visible)
{
    if (!ui_lock()) {
        return;
    }
    if (visible) {
        load_screen(scr_qr, SCR_QR);
    }
    update_connectivity_overlay();
    ui_unlock();
}

void display_ui_set_qr_image(const lv_img_dsc_t *img_dsc)
{
    if (!img_dsc || !qr_img || !ui_lock()) {
        return;
    }
    lv_img_set_src(qr_img, img_dsc);
    lv_obj_clear_flag(qr_img, LV_OBJ_FLAG_HIDDEN);
    ui_unlock();
}

void display_ui_set_product_info(const char *description, const char *price_display)
{
    if (!ui_lock()) {
        return;
    }
    if (qr_lbl_desc && description) {
        lv_label_set_text(qr_lbl_desc, description);
    }
    if (qr_lbl_price && price_display) {
        lv_label_set_text(qr_lbl_price, price_display);
    }
    ui_unlock();
}

void display_ui_show_coloca_termo(bool visible)
{
    if (!visible || !ui_lock()) {
        return;
    }
    load_screen(scr_coloca, SCR_COLOCA);
    update_connectivity_overlay();
    ui_unlock();
}

void display_ui_show_cargar_idle(bool visible)
{
    if (!visible || !ui_lock()) {
        return;
    }
    set_cargar_mode(CARGAR_UI_IDLE);
    char buf[24];
    snprintf(buf, sizeof(buf), UI_STR_LITROS_FMT, 0.0f);
    lv_label_set_text(cargar_lbl_litros, buf);
    load_screen(scr_cargar, SCR_CARGAR);
    update_connectivity_overlay();
    ui_unlock();
}

void display_ui_show_cargar_dispensing(bool visible)
{
    if (!visible || !ui_lock()) {
        return;
    }
    set_cargar_mode(CARGAR_UI_DISPENSING);
    load_screen(scr_cargar, SCR_CARGAR);
    update_connectivity_overlay();
    ui_unlock();
}

void display_ui_show_cargar_paused(bool visible)
{
    if (!visible || !ui_lock()) {
        return;
    }
    set_cargar_mode(CARGAR_UI_PAUSED);
    load_screen(scr_cargar, SCR_CARGAR);
    update_connectivity_overlay();
    ui_unlock();
}

void display_ui_set_dispense_liters(float liters)
{
    if (!cargar_lbl_litros || !ui_lock()) {
        return;
    }
    char buf[24];
    snprintf(buf, sizeof(buf), UI_STR_LITROS_FMT, (double)liters);
    lv_label_set_text(cargar_lbl_litros, buf);
    ui_unlock();
}

void display_ui_set_dispense_temp_c(uint8_t temp_c, bool valid)
{
    if (!cargar_lbl_temp_val || !ui_lock()) {
        return;
    }
    char buf[12];
    if (valid) {
        snprintf(buf, sizeof(buf), UI_STR_TEMP_FMT, (unsigned)temp_c);
    } else {
        snprintf(buf, sizeof(buf), "%s", UI_STR_TEMP_INVALID);
    }
    lv_label_set_text(cargar_lbl_temp_val, buf);
    ui_unlock();
}

void display_ui_set_parar_enabled(bool enabled)
{
    if (!cargar_btn_action || !ui_lock()) {
        return;
    }
    if (enabled) {
        lv_obj_clear_state(cargar_btn_action, LV_STATE_DISABLED);
    } else {
        lv_obj_add_state(cargar_btn_action, LV_STATE_DISABLED);
    }
    ui_unlock();
}

void display_ui_set_continuar_visible(bool visible)
{
    if (!cargar_btn_continuar || !ui_lock()) {
        return;
    }
    cargar_continuar_visible = visible;
    if (cargar_mode != CARGAR_UI_PAUSED) {
        ui_unlock();
        return;
    }
    if (visible) {
        lv_obj_clear_state(cargar_btn_continuar, LV_STATE_DISABLED);
        lv_obj_clear_flag(cargar_btn_continuar, LV_OBJ_FLAG_HIDDEN);
    } else {
        lv_obj_add_flag(cargar_btn_continuar, LV_OBJ_FLAG_HIDDEN);
    }
    ui_unlock();
}

void display_ui_show_finish(bool visible)
{
    if (!visible || !ui_lock()) {
        return;
    }
    load_screen(scr_finish, SCR_FINISH);
    update_connectivity_overlay();
    ui_unlock();
}

void display_ui_show_error(UiErrorType type)
{
    if (!ui_lock()) {
        return;
    }
    current_error = type;
    if (error_left_img) {
        lv_img_set_src(error_left_img, error_left_image(type));
    }
    if (error_lbl_title) {
        lv_label_set_text(error_lbl_title, error_title(type));
        const UiTitleStyle style =
            (type == UI_ERR_PAGO) ? UI_TITLE_STYLE_GREEN : UI_TITLE_STYLE_ORANGE;
        apply_title_style(error_lbl_title, error_title_bar, style);
    }
    if (error_lbl_card) {
        lv_label_set_text(error_lbl_card, error_card(type));
    }

    load_screen(scr_error, SCR_ERROR);
    update_connectivity_overlay();
    ui_unlock();
}

void display_ui_set_wifi(bool connected)
{
    wifi_ok = connected;
    if (!ui_lock()) {
        return;
    }
    update_connectivity_overlay();
    ui_unlock();
}

void display_ui_set_mqtt(bool connected)
{
    mqtt_ok = connected;
    if (!ui_lock()) {
        return;
    }
    update_connectivity_overlay();
    ui_unlock();
}

#if defined(UI_DEBUG_TERMO) && UI_DEBUG_TERMO
void display_ui_set_termo_debug_visible(bool visible)
{
    if (!ui_lock()) {
        return;
    }
    if (visible) {
        lv_obj_clear_flag(label_termo_debug_1, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(label_termo_debug_2, LV_OBJ_FLAG_HIDDEN);
    } else {
        lv_obj_add_flag(label_termo_debug_1, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(label_termo_debug_2, LV_OBJ_FLAG_HIDDEN);
    }
    ui_unlock();
}

void display_ui_set_termo_debug(const char *line1, const char *line2)
{
    if (!ui_lock()) {
        return;
    }
    if (line1 && label_termo_debug_1) {
        lv_label_set_text(label_termo_debug_1, line1);
    }
    if (line2 && label_termo_debug_2) {
        lv_label_set_text(label_termo_debug_2, line2);
    }
    ui_unlock();
}
#endif

void display_ui_show_comprar(bool visible)
{
    display_ui_show_standby(visible);
}

void display_ui_show_iniciar(bool visible)
{
    display_ui_show_cargar_idle(visible);
}

void display_ui_show_dispensing(bool visible)
{
    display_ui_show_cargar_dispensing(visible);
}

void display_ui_set_qr_countdown_sec(int seconds)
{
    (void)seconds;
}

void display_ui_set_dispense_countdown_sec(int seconds)
{
    (void)seconds;
}

void display_ui_set_main(const char *message)
{
    (void)message;
}

void display_ui_set_main_visible(bool visible)
{
    (void)visible;
}
