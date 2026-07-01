#include "app_state.h"

#include "config.h"
#include "dispense_controller.h"
#include "display_ui.h"
#include "nobana_uart.h"
#include "order_client.h"
#include "mate_network.h"
#include "qr_image.h"

#include <Arduino.h>
#include <string.h>

enum AppState {
    APP_COMPRAR,
    APP_CREATING,
    APP_QR_SHOW,
    APP_DISPENSE,
    APP_ERROR_PAGO,
};

static AppState state = APP_COMPRAR;
static char active_order_id[64] = "";
static uint32_t qr_deadline_ms = 0;
static uint32_t error_pago_deadline_ms = 0;
static bool create_pending = false;

static void enter_error_pago()
{
    state = APP_ERROR_PAGO;
    error_pago_deadline_ms = millis() + UI_ERROR_PAGO_MS;
    display_ui_show_error(UI_ERR_PAGO);
}

static void enter_comprar()
{
    state = APP_COMPRAR;
    active_order_id[0] = '\0';
    qr_deadline_ms = 0;
    error_pago_deadline_ms = 0;
    dispense_reset_session();
    nobana_standby_enable();
    display_ui_set_termo_debug_visible(false);
    display_ui_show_standby(true);
}

void app_state_init()
{
    display_ui_set_product_info(UI_PRODUCT_DESC_PLACEHOLDER, UI_PRODUCT_PRICE_PLACEHOLDER);
    enter_comprar();
}

void app_state_on_comprar_pressed()
{
    if (state != APP_COMPRAR || create_pending || !dispense_can_accept_comprar()) {
        return;
    }

    state = APP_CREATING;
    display_ui_show_loading(true);
    create_pending = true;
}

void app_state_on_parar_pressed()
{
    if (state != APP_DISPENSE) {
        return;
    }
    dispense_on_parar_pressed();
}

void app_state_on_iniciar_pressed()
{
    if (state != APP_DISPENSE) {
        return;
    }
    if (dispense_on_iniciar_pressed()) {
        mate_network_publish_status();
    }
}

void app_state_on_post_pay_timeout()
{
    if (active_order_id[0]) {
        order_cancel(active_order_id);
    }
    enter_comprar();
}

bool app_state_can_accept_dispense()
{
    return state == APP_QR_SHOW && !dispense_cycle_active();
}

bool app_state_on_dispense_command(const char *order_id, uint32_t duration_ms)
{
    if (state != APP_QR_SHOW) {
        return false;
    }

    if (active_order_id[0] && order_id && order_id[0]
        && strcmp(order_id, active_order_id) != 0) {
        return false;
    }

    state = APP_DISPENSE;

    if (!dispense_on_command(order_id, duration_ms)) {
        enter_comprar();
        return false;
    }

    return true;
}

void app_state_tick()
{
    if (create_pending) {
        create_pending = false;

        char order_id[64] = "";
        char external_ref[64] = "";
        if (order_create(order_id, sizeof(order_id), external_ref, sizeof(external_ref))) {
            strncpy(active_order_id, order_id, sizeof(active_order_id) - 1);
            state = APP_QR_SHOW;
            qr_deadline_ms = millis() + QR_TIMEOUT_MS;
            nobana_standby_enable();
            display_ui_set_product_info(UI_PRODUCT_DESC_PLACEHOLDER, UI_PRODUCT_PRICE_PLACEHOLDER);
            display_ui_set_qr_image(&qr_static_img);
            display_ui_show_qr(true);
        } else {
            enter_error_pago();
        }
    }

    if (state == APP_ERROR_PAGO && error_pago_deadline_ms != 0
        && (int32_t)(millis() - error_pago_deadline_ms) >= 0) {
        enter_comprar();
    }

    if (state == APP_QR_SHOW && qr_deadline_ms != 0) {
        const int32_t remaining = (int32_t)(qr_deadline_ms - millis());
        if (remaining <= 0) {
            if (active_order_id[0]) {
                order_cancel(active_order_id);
            }
            qr_deadline_ms = 0;
            enter_error_pago();
        }
    }

    if (state == APP_DISPENSE && !dispense_cycle_active()) {
        enter_comprar();
    }
}
