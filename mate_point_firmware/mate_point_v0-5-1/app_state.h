#pragma once

#include <stdint.h>

void app_state_init();
void app_state_tick();
void app_state_on_comprar_pressed();
void app_state_on_iniciar_pressed();
void app_state_on_parar_pressed();
void app_state_on_post_pay_timeout();
void app_state_on_tray_full();
void app_state_on_tray_recovered();
void app_state_on_tank_empty();
void app_state_on_tank_recovered();
bool app_state_can_accept_dispense();
bool app_state_on_dispense_command(const char *order_id, uint32_t duration_ms);
