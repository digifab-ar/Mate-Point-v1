#pragma once

#include <stdbool.h>
#include <stdint.h>

typedef enum {
    TANK_TRANSITION_NONE,
    TANK_TRANSITION_BECAME_EMPTY,
    TANK_TRANSITION_BECAME_OK,
} TankTransition;

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

/** Stop manual — pre-stop 200 ms luego 22+00 directo (sin 22+04). */
bool nobana_dispense_abort();

bool nobana_dispense_busy();

/** T_viva (byte 3 RX) en °C; false si aun no hay telemetria valida. */
bool nobana_live_temp_c(uint8_t *out_c);

/** ms restantes del contrato duration_ms (activo + pre-stop + cierre). */
uint32_t nobana_dispense_remaining_ms();

/** true una sola vez al fin del stop/cierre (antes del cooldown). */
bool nobana_dispense_consume_stop_done();

/** true una sola vez tras fin de cooldown R. */
bool nobana_dispense_consume_done();

/** Tanque vacío debounced (b2=0x10, byte7=0x01). */
bool nobana_tank_empty();

/** true tras al menos un ciclo de debounce completo. */
bool nobana_tank_stable_ready();

/** Consumir transición estable; llamar después de nobana_tick(). */
TankTransition nobana_tank_poll();
