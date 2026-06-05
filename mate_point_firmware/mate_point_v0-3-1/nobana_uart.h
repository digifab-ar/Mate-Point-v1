#pragma once

#include <stdint.h>

/** Bus + wake bloqueante — llamar en setup() antes de LVGL. */
void nobana_product_init();

/** Handshake residual, standby y dispensado — llamar cada loop(). */
void nobana_tick();

bool nobana_wake_ready();
bool nobana_standby_active();

void nobana_standby_enable();
void nobana_standby_disable();

/**
 * Inicia ciclo R (Coffee).
 * duration_ms: tiempo total contratado (activo + pre-stop + cierre), sin cooldown.
 */
bool nobana_dispense_start(uint32_t duration_ms);

bool nobana_dispense_busy();

/** true una sola vez al fin del stop/cierre (antes del cooldown). */
bool nobana_dispense_consume_stop_done();

/** true una sola vez tras fin de cooldown R. */
bool nobana_dispense_consume_done();
