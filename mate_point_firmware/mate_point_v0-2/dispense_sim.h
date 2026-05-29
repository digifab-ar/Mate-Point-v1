#pragma once

#include <stdint.h>

enum DispenseEvent {
    DISPENSE_EVENT_NONE,
    DISPENSE_EVENT_IDLE,
};

const char *dispense_mqtt_state();
const char *dispense_active_order_id();

bool dispense_cycle_active();
bool dispense_on_command(const char *order_id, uint32_t duration_ms);
DispenseEvent dispense_tick();
