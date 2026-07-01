#include "dispense_controller.h"

#include "config.h"
#include "display_ui.h"
#include "nobana_uart.h"
#include "drip_tray_sensor.h"
#include "vl53l0x_sensor.h"

#include <Arduino.h>
#include <string.h>

enum Phase {
    LISTO,
    WAIT_TERMO,
    READY_START,
    DISPENSING,
    PAUSED,
    TERMINADO,
    LISTO_WAIT,
};

static Phase phase = LISTO;
static uint32_t phase_end_ms = 0;
static uint32_t last_dispense_ui_ms = 0;
static uint32_t last_termo_poll_ms = 0;
static uint32_t contract_duration_ms = 0;
static uint32_t dispensed_ms = 0;
static uint32_t segment_start_ms = 0;
static uint32_t pause_deadline_ms = 0;
static uint32_t pending_duration_ms = 0;
static uint32_t post_pay_deadline_ms = 0;
static uint8_t termo_streak = 0;
static char last_order_id[64] = "";
static char active_order_id[64] = "";
static char pending_order_id[64] = "";
static bool continuar_pending = false;

static bool same_order(const char *order_id)
{
    return order_id && order_id[0] && strcmp(order_id, last_order_id) == 0;
}

static bool read_termo_present()
{
    Vl53l0xSample sample;
    vl53l0x_sample(&sample);
    return sample.status == VL53L0X_STATUS_OK && sample.termo_present;
}

static void update_termo_debug_ui(const Vl53l0xSample *sample)
{
    char line1[72];
    char line2[72];

    snprintf(line1, sizeof(line1), "Sensor: %s", vl53l0x_init_status_text());

    if (sample->status == VL53L0X_STATUS_OK) {
        snprintf(line2, sizeof(line2), "Raw: %u | Corr: %u mm | %s | <%u mm",
                 (unsigned)sample->distance_mm,
                 (unsigned)sample->distance_corrected_mm,
                 sample->termo_present ? "TERMO OK" : "sin termo",
                 (unsigned)TERMO_PRESENT_MAX_MM);
    } else {
        snprintf(line2, sizeof(line2), "Dist: %u | %s",
                 (unsigned)sample->distance_mm,
                 vl53l0x_status_text(sample->status));
    }

    display_ui_set_termo_debug(line1, line2);
}

static void reset_termo_streak()
{
    termo_streak = 0;
}

static float ui_liters_from_dispensed_ms(uint32_t ms)
{
    const float sec = ms / 1000.0f;
    float liters = sec / (float)UI_LITERS_FILL_SEC * UI_PRODUCT_LITERS_DEFAULT;
    if (liters > UI_PRODUCT_LITERS_DEFAULT) {
        liters = UI_PRODUCT_LITERS_DEFAULT;
    }
    return liters;
}

static uint32_t current_dispensed_ms(uint32_t now)
{
    if (phase == DISPENSING && segment_start_ms != 0) {
        return dispensed_ms + (now - segment_start_ms);
    }
    return dispensed_ms;
}

static uint32_t contract_remaining_ms(uint32_t now)
{
    if (contract_duration_ms == 0) {
        return 0;
    }

    const uint32_t used = current_dispensed_ms(now);
    if (used >= contract_duration_ms) {
        return 0;
    }
    return contract_duration_ms - used;
}

static void accumulate_dispensed_ms(uint32_t now)
{
    if (phase == DISPENSING && segment_start_ms != 0) {
        dispensed_ms += (now - segment_start_ms);
        segment_start_ms = 0;
    }
}

static void refresh_temp_ui()
{
    uint8_t temp_c = 0;
    const bool temp_ok = nobana_live_temp_c(&temp_c);
    display_ui_set_dispense_temp_c(temp_c, temp_ok);
}

static void show_wait_termo_ui()
{
    display_ui_set_termo_debug_visible(true);
    display_ui_show_coloca_termo(true);

    Vl53l0xSample sample;
    vl53l0x_sample(&sample);
    update_termo_debug_ui(&sample);
}

static void show_ready_start_ui()
{
    display_ui_set_termo_debug_visible(false);
    display_ui_show_cargar_idle(true);
    display_ui_set_dispense_liters(0.0f);
    refresh_temp_ui();
}

static void update_dispense_ui(uint32_t now)
{
    refresh_temp_ui();
    display_ui_set_dispense_liters(ui_liters_from_dispensed_ms(current_dispensed_ms(now)));
}

static void enter_wait_termo(uint32_t now)
{
    phase = WAIT_TERMO;
    reset_termo_streak();
    last_termo_poll_ms = 0;
    show_wait_termo_ui();
    (void)now;
}

static void enter_ready_start(uint32_t now)
{
    phase = READY_START;
    reset_termo_streak();
    last_termo_poll_ms = 0;
    show_ready_start_ui();
    (void)now;
}

static bool poll_termo_transition(uint32_t now, bool want_present)
{
    if (last_termo_poll_ms != 0
        && (int32_t)(now - last_termo_poll_ms) < (int32_t)TERMO_POLL_MS) {
        return false;
    }

    last_termo_poll_ms = now;

    Vl53l0xSample sample;
    vl53l0x_sample(&sample);

    if (phase == WAIT_TERMO) {
        update_termo_debug_ui(&sample);
    }

    const bool present = sample.status == VL53L0X_STATUS_OK && sample.termo_present;

    if (present == want_present) {
        if (termo_streak < TERMO_DEBOUNCE_COUNT) {
            termo_streak++;
        }
    } else {
        termo_streak = 0;
    }

    return termo_streak >= TERMO_DEBOUNCE_COUNT;
}

static void enter_finalize(uint32_t now);

static bool resume_dispensing(uint32_t now)
{
    const uint32_t remaining = contract_remaining_ms(now);
    if (remaining == 0) {
        enter_finalize(now);
        return false;
    }

    if (!nobana_dispense_busy() && nobana_wake_ready()) {
        nobana_standby_enable();
    }
    if (!nobana_dispense_start(remaining)) {
        return false;
    }

    segment_start_ms = now;
    continuar_pending = false;
    phase = DISPENSING;
    reset_termo_streak();
    last_termo_poll_ms = 0;
    display_ui_show_cargar_dispensing(true);
    display_ui_set_parar_enabled(true);
    update_dispense_ui(now);
    return true;
}

static void enter_paused(uint32_t now)
{
    accumulate_dispensed_ms(now);
    nobana_dispense_abort_pause();

    phase = PAUSED;
    pause_deadline_ms = now + PAUSE_DECISION_TIMEOUT_MS;
    continuar_pending = false;
    reset_termo_streak();
    last_termo_poll_ms = 0;

    display_ui_show_cargar_paused(true);
    display_ui_set_dispense_liters(ui_liters_from_dispensed_ms(dispensed_ms));
    refresh_temp_ui();
    display_ui_set_continuar_visible(false);
}

static bool start_dispensing(uint32_t now)
{
    if (!nobana_dispense_start(pending_duration_ms)) {
        return false;
    }

    strncpy(active_order_id, pending_order_id, sizeof(active_order_id) - 1);
    active_order_id[sizeof(active_order_id) - 1] = '\0';

    contract_duration_ms = pending_duration_ms;
    dispensed_ms = 0;
    segment_start_ms = now;
    pause_deadline_ms = 0;
    post_pay_deadline_ms = 0;
    continuar_pending = false;

    phase = DISPENSING;
    phase_end_ms = 0;
    last_dispense_ui_ms = 0;
    reset_termo_streak();
    last_termo_poll_ms = 0;
    display_ui_show_cargar_dispensing(true);
    display_ui_set_parar_enabled(true);
    update_dispense_ui(now);
    return true;
}

static void enter_finalize(uint32_t now)
{
    if (phase != DISPENSING && phase != PAUSED) {
        return;
    }

    if (phase == DISPENSING) {
        accumulate_dispensed_ms(now);
        nobana_dispense_abort();
    }

    pause_deadline_ms = 0;
    continuar_pending = false;
    phase = TERMINADO;
    phase_end_ms = now + TERMINADO_TO_LISTO_MS;
    contract_duration_ms = 0;
    display_ui_show_finish(true);
}

static void reset_to_listo()
{
    phase = LISTO;
    phase_end_ms = 0;
    last_dispense_ui_ms = 0;
    last_termo_poll_ms = 0;
    contract_duration_ms = 0;
    dispensed_ms = 0;
    segment_start_ms = 0;
    pause_deadline_ms = 0;
    pending_duration_ms = 0;
    post_pay_deadline_ms = 0;
    termo_streak = 0;
    continuar_pending = false;
    pending_order_id[0] = '\0';
    active_order_id[0] = '\0';
    display_ui_set_termo_debug_visible(false);
}

const char *dispense_mqtt_state()
{
    if (phase == DISPENSING || phase == PAUSED || phase == TERMINADO || phase == LISTO_WAIT) {
        return "dispensing";
    }
    return "idle";
}

const char *dispense_active_order_id()
{
    if (phase == DISPENSING || phase == PAUSED || phase == TERMINADO || phase == LISTO_WAIT) {
        return active_order_id[0] ? active_order_id : nullptr;
    }
    return nullptr;
}

bool dispense_cycle_active()
{
    return phase != LISTO;
}

bool dispense_can_accept_comprar()
{
    return phase == LISTO && !nobana_dispense_busy() && !drip_tray_is_full()
           && !nobana_tank_empty();
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

    strncpy(last_order_id, order_id, sizeof(last_order_id) - 1);
    strncpy(pending_order_id, order_id, sizeof(pending_order_id) - 1);
    pending_order_id[sizeof(pending_order_id) - 1] = '\0';
    active_order_id[0] = '\0';

    pending_duration_ms = duration_ms;
    contract_duration_ms = 0;
    dispensed_ms = 0;
    segment_start_ms = 0;
    post_pay_deadline_ms = millis() + POST_PAY_TIMEOUT_MS;

    if (read_termo_present()) {
        enter_ready_start(millis());
    } else {
        enter_wait_termo(millis());
    }

    return true;
}

bool dispense_on_iniciar_pressed()
{
    if (phase != READY_START || drip_tray_is_full() || nobana_tank_empty()) {
        return false;
    }

    if (!read_termo_present()) {
        enter_wait_termo(millis());
        return false;
    }

    return start_dispensing(millis());
}

void dispense_on_parar_pressed()
{
    if (phase != DISPENSING) {
        return;
    }

    enter_paused(millis());
}

bool dispense_on_continuar_pressed()
{
    if (phase != PAUSED || drip_tray_is_full() || nobana_tank_empty()) {
        return false;
    }

    if (!read_termo_present()) {
        return false;
    }

    const uint32_t now = millis();
    if (contract_remaining_ms(now) == 0) {
        enter_finalize(now);
        return true;
    }

    pause_deadline_ms = 0;

    if (nobana_dispense_busy()) {
        continuar_pending = true;
        display_ui_set_continuar_visible(false);
        return true;
    }

    return resume_dispensing(now);
}

void dispense_on_finalizar_pressed()
{
    if (phase != PAUSED) {
        return;
    }

    enter_finalize(millis());
}

void dispense_on_tray_full()
{
    if (phase == DISPENSING) {
        accumulate_dispensed_ms(millis());
        nobana_dispense_abort();
    }

    reset_to_listo();
}

void dispense_on_tank_empty()
{
    dispense_on_tray_full();
}

DispenseEvent dispense_tick()
{
    if (phase == LISTO) {
        return DISPENSE_EVENT_NONE;
    }

    const uint32_t now = millis();

    if (phase == WAIT_TERMO || phase == READY_START) {
        if (post_pay_deadline_ms != 0 && (int32_t)(now - post_pay_deadline_ms) >= 0) {
            reset_to_listo();
            return DISPENSE_EVENT_POST_PAY_TIMEOUT;
        }

        if (phase == WAIT_TERMO) {
            if (poll_termo_transition(now, true)) {
                enter_ready_start(now);
            }
        } else {
            if (poll_termo_transition(now, false)) {
                enter_wait_termo(now);
            } else if (last_dispense_ui_ms == 0
                       || (int32_t)(now - last_dispense_ui_ms)
                              >= (int32_t)DISPENSE_UI_REFRESH_MS) {
                last_dispense_ui_ms = now;
                refresh_temp_ui();
            }
        }

        return DISPENSE_EVENT_NONE;
    }

    if (phase == DISPENSING) {
        if (poll_termo_transition(now, false)) {
            enter_finalize(now);
            return DISPENSE_EVENT_NONE;
        }

        if (last_dispense_ui_ms == 0
            || (int32_t)(now - last_dispense_ui_ms) >= (int32_t)DISPENSE_UI_REFRESH_MS) {
            last_dispense_ui_ms = now;
            update_dispense_ui(now);
        }

        if (!nobana_dispense_busy() && nobana_wake_ready()) {
            nobana_standby_enable();
        }

        nobana_dispense_consume_stop_done();

        if (contract_remaining_ms(now) == 0) {
            accumulate_dispensed_ms(now);
            phase = TERMINADO;
            phase_end_ms = now + TERMINADO_TO_LISTO_MS;
            contract_duration_ms = 0;
            display_ui_show_finish(true);
        }
        return DISPENSE_EVENT_NONE;
    }

    if (phase == PAUSED) {
        if (pause_deadline_ms != 0 && (int32_t)(now - pause_deadline_ms) >= 0) {
            enter_finalize(now);
            return DISPENSE_EVENT_NONE;
        }

        if (poll_termo_transition(now, false)) {
            enter_finalize(now);
            return DISPENSE_EVENT_NONE;
        }

        if (last_dispense_ui_ms == 0
            || (int32_t)(now - last_dispense_ui_ms) >= (int32_t)DISPENSE_UI_REFRESH_MS) {
            last_dispense_ui_ms = now;
            refresh_temp_ui();
        }

        if (!nobana_dispense_busy() && nobana_wake_ready()) {
            nobana_standby_enable();
        }

        const bool can_resume = !nobana_dispense_busy() && nobana_standby_active()
                                && read_termo_present() && contract_remaining_ms(now) > 0;
        display_ui_set_continuar_visible(can_resume);

        if (continuar_pending && can_resume) {
            resume_dispensing(now);
        }

        return DISPENSE_EVENT_NONE;
    }

    if (phase == TERMINADO && (int32_t)(now - phase_end_ms) >= 0) {
        phase = LISTO_WAIT;
        return DISPENSE_EVENT_NONE;
    }

    if (phase == LISTO_WAIT
        && (nobana_dispense_consume_done() || !nobana_dispense_busy())) {
        reset_to_listo();
        nobana_standby_enable();
        return DISPENSE_EVENT_IDLE;
    }

    return DISPENSE_EVENT_NONE;
}
