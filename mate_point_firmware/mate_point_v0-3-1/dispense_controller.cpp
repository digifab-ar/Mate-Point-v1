#include "dispense_controller.h"

#include "config.h"
#include "display_ui.h"
#include "nobana_uart.h"

#include <Arduino.h>
#include <string.h>

enum Phase {
    LISTO,
    DISPENSING,
    TERMINADO,
    LISTO_WAIT,
};

static Phase phase = LISTO;
static uint32_t phase_end_ms = 0;
static char last_order_id[64] = "";
static char active_order_id[64] = "";

static bool same_order(const char *order_id)
{
    return order_id && order_id[0] && strcmp(order_id, last_order_id) == 0;
}

const char *dispense_mqtt_state()
{
    return (phase == LISTO) ? "idle" : "dispensing";
}

const char *dispense_active_order_id()
{
    return active_order_id[0] ? active_order_id : nullptr;
}

bool dispense_cycle_active()
{
    return phase != LISTO;
}

bool dispense_can_accept_comprar()
{
    return phase == LISTO && !nobana_dispense_busy();
}

void dispense_reset_session()
{
    last_order_id[0] = '\0';
}

bool dispense_on_command(const char *order_id, uint32_t duration_ms)
{
    if (!order_id || !order_id[0] || phase != LISTO || same_order(order_id)) {
        return false;
    }
    if (!nobana_wake_ready()) {
        return false;
    }
    if (!nobana_standby_active()) {
        nobana_standby_enable();
    }
    if (!nobana_dispense_start(duration_ms)) {
        return false;
    }

    strncpy(last_order_id, order_id, sizeof(last_order_id) - 1);
    strncpy(active_order_id, order_id, sizeof(active_order_id) - 1);

    phase = DISPENSING;
    phase_end_ms = 0;

    display_ui_set_main_visible(true);
    display_ui_set_main("Dispensado");
    return true;
}

DispenseEvent dispense_tick()
{
    if (phase == LISTO) {
        return DISPENSE_EVENT_NONE;
    }

    const uint32_t now = millis();

    if (phase == DISPENSING) {
        if (nobana_dispense_consume_stop_done()) {
            phase = TERMINADO;
            phase_end_ms = now + TERMINADO_TO_LISTO_MS;
            display_ui_set_main_visible(true);
            display_ui_set_main("terminado");
        }
        return DISPENSE_EVENT_NONE;
    }

    if (phase == TERMINADO && (int32_t)(now - phase_end_ms) >= 0) {
        phase = LISTO_WAIT;
        display_ui_set_main_visible(true);
        display_ui_set_main("Listo");
        return DISPENSE_EVENT_NONE;
    }

    if (phase == LISTO_WAIT && nobana_dispense_consume_done()) {
        phase = LISTO;
        active_order_id[0] = '\0';
        nobana_standby_enable();
        return DISPENSE_EVENT_IDLE;
    }

    return DISPENSE_EVENT_NONE;
}
