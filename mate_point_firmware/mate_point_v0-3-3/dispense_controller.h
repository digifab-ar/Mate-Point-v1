#pragma once

#include <stdint.h>

enum DispenseEvent {
    DISPENSE_EVENT_NONE,
    DISPENSE_EVENT_IDLE,
};

const char *dispense_mqtt_state();
const char *dispense_active_order_id();

bool dispense_cycle_active();
bool dispense_can_accept_comprar();
bool dispense_on_command(const char *order_id, uint32_t duration_ms);
void dispense_on_parar_pressed();
DispenseEvent dispense_tick();

/** Nueva compra: permite otro order_id (llamar al volver a Comprar). */
void dispense_reset_session();
