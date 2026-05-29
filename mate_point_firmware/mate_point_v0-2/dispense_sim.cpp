#include "dispense_sim.h"

#include "config.h"
#include "display_ui.h"

#include <Arduino.h>
#include <string.h>

enum Phase { LISTO, DISPENSING, TERMINADO };

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

bool dispense_on_command(const char *order_id, uint32_t duration_ms)
{
    if (!order_id || !order_id[0] || phase != LISTO || same_order(order_id)) {
        return false;
    }

    strncpy(last_order_id, order_id, sizeof(last_order_id) - 1);
    strncpy(active_order_id, order_id, sizeof(active_order_id) - 1);

    phase = DISPENSING;
    phase_end_ms = millis() + duration_ms;
    display_ui_set_main_visible(true);
    display_ui_set_main("Dispensado");
    return true;
}

DispenseEvent dispense_tick()
{
    if (phase == LISTO || (int32_t)(millis() - phase_end_ms) < 0) {
        return DISPENSE_EVENT_NONE;
    }

    if (phase == DISPENSING) {
    phase = TERMINADO;
    phase_end_ms = millis() + TERMINADO_TO_LISTO_MS;
    display_ui_set_main_visible(true);
    display_ui_set_main("terminado");
        return DISPENSE_EVENT_NONE;
    }

    phase = LISTO;
    active_order_id[0] = '\0';
    display_ui_set_main_visible(true);
    display_ui_set_main("Listo");
    return DISPENSE_EVENT_IDLE;
}
