#include "app_state.h"

#include "config.h"
#include "dispense_sim.h"
#include "display_ui.h"
#include "order_client.h"
#include "qr_image.h"

#include <Arduino.h>
#include <string.h>

enum AppState {
    APP_COMPRAR,
    APP_CREATING,
    APP_QR_SHOW,
    APP_DISPENSE,
    APP_POST_LISTO,
};

static AppState state = APP_COMPRAR;
static char active_order_id[64] = "";
static uint32_t qr_deadline_ms = 0;
static uint32_t listo_until_ms = 0;
static bool create_pending = false;

static void enter_comprar()
{
    state = APP_COMPRAR;
    active_order_id[0] = '\0';
    qr_deadline_ms = 0;
    display_ui_show_qr(false);
    display_ui_show_comprar(true);
}

void app_state_init()
{
    enter_comprar();
}

void app_state_on_comprar_pressed()
{
    if (state != APP_COMPRAR || create_pending) {
        return;
    }

    state = APP_CREATING;
    display_ui_show_comprar(false);
    display_ui_set_main("Creando orden...");
    create_pending = true;
    Serial.println("[app] comprar pressed");
}

bool app_state_can_accept_dispense()
{
    return state == APP_QR_SHOW;
}

bool app_state_on_dispense_command(const char *order_id, uint32_t duration_ms)
{
    if (state != APP_QR_SHOW) {
        Serial.println("[app] dispense ignored (not in QR_SHOW)");
        return false;
    }

    if (active_order_id[0] && order_id && order_id[0]
        && strcmp(order_id, active_order_id) != 0) {
        Serial.printf("[app] dispense ignored order mismatch active=%s got=%s\n",
                      active_order_id, order_id);
        return false;
    }

    display_ui_show_qr(false);
    display_ui_set_main_visible(true);
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
            display_ui_set_main_visible(false);
            display_ui_set_qr_image(&qr_static_img);
            display_ui_show_qr(true);
            display_ui_set_qr_countdown_sec(QR_TIMEOUT_MS / 1000);
            Serial.printf("[app] order created id=%s ref=%s\n", order_id, external_ref);
        } else {
            Serial.println("[app] order create failed");
            display_ui_set_main("Error orden");
            display_ui_set_main_visible(true);
            enter_comprar();
        }
    }

    if (state == APP_QR_SHOW && qr_deadline_ms != 0) {
        const int32_t remaining = (int32_t)(qr_deadline_ms - millis());
        if (remaining <= 0) {
            Serial.printf("[app] QR timeout order_id=%s\n", active_order_id);
            if (active_order_id[0]) {
                order_cancel(active_order_id);
            }
            enter_comprar();
        } else {
            display_ui_set_qr_countdown_sec((remaining + 999) / 1000);
        }
    }

    if (state == APP_DISPENSE && !dispense_cycle_active()) {
        state = APP_POST_LISTO;
        listo_until_ms = millis() + 2000;
        display_ui_set_main_visible(true);
        display_ui_set_main("Listo");
        Serial.println("[app] dispense cycle complete -> Listo");
    }

    if (state == APP_POST_LISTO && (int32_t)(millis() - listo_until_ms) >= 0) {
        enter_comprar();
    }
}
