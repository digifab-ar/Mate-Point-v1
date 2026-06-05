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
};

static Phase phase = LISTO;
static uint32_t phase_end_ms = 0;
static uint32_t watchdog_end_ms = 0;
static char last_order_id[64] = "";
static char active_order_id[64] = "";
static bool watchdog_fired = false;

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
    if (!nobana_dispense_start()) {
        return false;
    }

    strncpy(last_order_id, order_id, sizeof(last_order_id) - 1);
    strncpy(active_order_id, order_id, sizeof(active_order_id) - 1);

    phase = DISPENSING;
    watchdog_end_ms = millis() + duration_ms;
    watchdog_fired = false;
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

    if (!watchdog_fired && (int32_t)(watchdog_end_ms - now) <= 0 && nobana_dispense_busy()) {
        watchdog_fired = true;
        display_ui_set_main_visible(true);
        display_ui_set_main("Error dispensado");
    }

    if (phase == DISPENSING) {
        if (nobana_dispense_consume_done()) {
            phase = TERMINADO;
            phase_end_ms = now + TERMINADO_TO_LISTO_MS;
            display_ui_set_main_visible(true);
            display_ui_set_main(watchdog_fired ? "Error dispensado" : "terminado");
        }
        return DISPENSE_EVENT_NONE;
    }

    if (phase == TERMINADO && (int32_t)(now - phase_end_ms) >= 0) {
        phase = LISTO;
        active_order_id[0] = '\0';
        watchdog_fired = false;
        display_ui_set_main_visible(true);
        display_ui_set_main("Listo");
        nobana_standby_enable();
        return DISPENSE_EVENT_IDLE;
    }

    return DISPENSE_EVENT_NONE;
}
