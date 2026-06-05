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

/** Inicia ciclo R (Coffee). Requiere wake listo y standby activo. */
bool nobana_dispense_start();

bool nobana_dispense_busy();

/** true una sola vez tras fin de cooldown R. */
bool nobana_dispense_consume_done();
