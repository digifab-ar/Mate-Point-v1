#pragma once

#include <stdint.h>

void app_state_init();
void app_state_tick();
void app_state_on_comprar_pressed();
bool app_state_can_accept_dispense();
bool app_state_on_dispense_command(const char *order_id, uint32_t duration_ms);
