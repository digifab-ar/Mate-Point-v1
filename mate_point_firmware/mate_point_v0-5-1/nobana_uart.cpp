#include "nobana_uart.h"

#include "config.h"

#include <Arduino.h>
#include <string.h>

#define PIN_NOB_RX 44
#define PIN_NOB_TX 43

#define NOBANA_BUS Serial

#define NOBANA_BUS_SETTLE_MS 300
#define NOBANA_BAUD 9600
#define FRAME_GAP_MS 35
#define MAX_FRAME_BYTES 128
#define NOBANA_POLL_MS 100
#define TX_FRAME_LEN 9
#define RX_FRAME_LEN 11

#define NOBANA_HDR 0x68
#define NOBANA_WAKE_BYTE 0xF8
#define NOBANA_CMD_START_UV 0x21
#define NOBANA_CMD_HOT_UV 0xE2
#define NOBANA_CMD_NATURAL_UV 0x22
#define NOBANA_D7_COFFEE 0x55
#define NOBANA_B5_PRESET 0x04
#define NOBANA_NOB_B2_CYCLE_END 0x11

#define HANDSHAKE_T_RX_BOOT_MS 5000
#define HANDSHAKE_T_POST_F8_MS 3000
#define WAKE_T_POST_F8_DELAY_MS 170
#define DISPENSE_T_PRESTOP_MS 3900
#define DISPENSE_T_PRESTOP_FAST_MS 200
#define DISPENSE_T_CLOSE_MIN_MS 2000
#define DISPENSE_T_CLOSE_MAX_MS 8000
#define DISPENSE_T_COOLDOWN_MS 15000
#define DISPENSE_CLOSE_BUDGET_MS DISPENSE_T_CLOSE_MIN_MS
#define DISPENSE_ACTIVE_MIN_MS 1000
#define DISPENSE_DURATION_DEFAULT_MS 30000

#define NOBANA_T_BOOT_MS 3000
#define NOBANA_WAKE_TIMEOUT_MS 12000

enum HandshakePhase {
    HS_IDLE,
    HS_RX_BOOT,
    HS_POST_F8,
    HS_READY,
};

enum DispensePhase {
    DISPENSE_OFF,
    DISPENSE_ACTIVE,
    DISPENSE_PRE_STOP,
    DISPENSE_STOP_22_04,
    DISPENSE_CLOSE_22_00,
    DISPENSE_COOLDOWN_22,
};

struct LineState {
    uint8_t buf[MAX_FRAME_BYTES];
    size_t len;
    uint32_t last_byte_ms;
    bool active;
};

struct NobanaTelemetry {
    bool valid;
    uint8_t b2;
    uint8_t b7;
    uint8_t t_live;
};

static bool s_tank_empty_stable = false;
static bool s_tank_debounce_active = false;
static uint8_t s_tank_debounce_streak = 0;
static bool s_tank_debounce_target = false;
static bool s_tank_stable_ready = false;
static TankTransition s_tank_pending_transition = TANK_TRANSITION_NONE;

static DispensePhase s_dispense = DISPENSE_OFF;
static HandshakePhase s_hs = HS_IDLE;
static uint8_t s_seq = 1;
static uint32_t s_phase_start_ms = 0;
static uint32_t s_active_deadline_ms = 0;
static uint32_t s_dispense_start_ms = 0;
static uint32_t s_duration_ms = 0;
static uint32_t s_next_poll_ms = 0;
static bool s_wake_ready = false;
static bool s_standby_active = false;
static bool s_nob_close_seen = false;
static bool s_manual_abort = false;
static bool s_dispense_stop_done_pulse = false;
static bool s_dispense_done_pulse = false;

static LineState s_rx;
static NobanaTelemetry s_telem;

static void line_reset(LineState *st);
static void nobana_bus_init();
static void tank_reset_state();
static bool tank_empty_raw(const uint8_t *data);
static void tank_update_debounce(const uint8_t *data);

static uint8_t checksum(const uint8_t *data, size_t len)
{
    uint8_t sum = 0;
    for (size_t i = 0; i < len; i++) {
        sum += data[i];
    }
    return sum;
}

static void build_tx(uint8_t *out9, uint8_t cmd, uint8_t b3, uint8_t b4, uint8_t b5, uint8_t b6,
                     uint8_t d7)
{
    out9[0] = NOBANA_HDR;
    out9[1] = s_seq;
    out9[2] = cmd;
    out9[3] = b3;
    out9[4] = b4;
    out9[5] = b5;
    out9[6] = b6;
    out9[7] = d7;
    out9[8] = checksum(out9, 8);
}

static bool frame_checksum_ok(const uint8_t *data, size_t len)
{
    if (len < 2) {
        return false;
    }
    return checksum(data, len - 1) == data[len - 1];
}

static bool tank_empty_raw(const uint8_t *data)
{
    return data[2] == WATER_TANK_EMPTY_B2 && data[7] == WATER_TANK_EMPTY_B7;
}

static void tank_reset_state()
{
    s_tank_empty_stable = false;
    s_tank_debounce_active = false;
    s_tank_debounce_streak = 0;
    s_tank_debounce_target = false;
    s_tank_stable_ready = false;
    s_tank_pending_transition = TANK_TRANSITION_NONE;
}

static void tank_update_debounce(const uint8_t *data)
{
    const bool raw = tank_empty_raw(data);

    if (!s_tank_debounce_active || raw != s_tank_debounce_target) {
        s_tank_debounce_active = true;
        s_tank_debounce_target = raw;
        s_tank_debounce_streak = 1;
        return;
    }

    if (s_tank_debounce_streak < 255) {
        s_tank_debounce_streak++;
    }

    if (s_tank_debounce_streak < WATER_TANK_DEBOUNCE_FRAMES) {
        return;
    }

    s_tank_stable_ready = true;

    if (raw == s_tank_empty_stable) {
        return;
    }

    s_tank_empty_stable = raw;
    s_tank_pending_transition = raw ? TANK_TRANSITION_BECAME_EMPTY : TANK_TRANSITION_BECAME_OK;
}

static void store_telemetry_from_frame(const uint8_t *data, size_t len)
{
    if (len != RX_FRAME_LEN) {
        return;
    }
    s_telem.valid = true;
    s_telem.b2 = data[2];
    s_telem.b7 = data[7];
    s_telem.t_live = data[3];

    tank_update_debounce(data);

    if (s_telem.b2 == NOBANA_NOB_B2_CYCLE_END) {
        s_nob_close_seen = true;
    }
}

static void parse_nob_frame(const uint8_t *data, size_t len)
{
    if (len != RX_FRAME_LEN || data[0] != NOBANA_HDR || !frame_checksum_ok(data, len)) {
        return;
    }
    store_telemetry_from_frame(data, len);
}

static void line_reset(LineState *st)
{
    st->len = 0;
    st->last_byte_ms = 0;
    st->active = false;
}

static void nobana_bus_init()
{
    NOBANA_BUS.begin(NOBANA_BAUD, SERIAL_8N1, PIN_NOB_RX, PIN_NOB_TX);
    line_reset(&s_rx);
}

static void line_push(LineState *st, uint8_t b, uint32_t now)
{
    if (st->len == 0) {
        st->active = true;
    }
    if (st->len < MAX_FRAME_BYTES) {
        st->buf[st->len++] = b;
    } else {
        line_reset(st);
        st->buf[st->len++] = b;
        st->active = true;
    }
    st->last_byte_ms = now;
}

static void line_flush(LineState *st)
{
    if (!st->active || st->len == 0) {
        line_reset(st);
        return;
    }
    parse_nob_frame(st->buf, st->len);
    line_reset(st);
}

static void poll_rx(uint32_t now)
{
    while (NOBANA_BUS.available() > 0) {
        line_push(&s_rx, (uint8_t)NOBANA_BUS.read(), now);
    }
    if (s_rx.active && (now - s_rx.last_byte_ms) >= FRAME_GAP_MS) {
        line_flush(&s_rx);
    }
}

static void send_frame(uint8_t cmd, uint8_t b3, uint8_t b4, uint8_t b5, uint8_t b6, uint8_t d7)
{
    uint8_t frame[TX_FRAME_LEN];
    build_tx(frame, cmd, b3, b4, b5, b6, d7);
    NOBANA_BUS.write(frame, TX_FRAME_LEN);
}

static void send_wake_f8()
{
    NOBANA_BUS.write(NOBANA_WAKE_BYTE);
}

static void handshake_mark_ready()
{
    s_hs = HS_READY;
    s_wake_ready = true;
}

static void handshake_begin()
{
    s_hs = HS_RX_BOOT;
    s_wake_ready = false;
    s_dispense = DISPENSE_OFF;
    s_standby_active = false;
    s_next_poll_ms = 0;
    s_phase_start_ms = millis();
    line_reset(&s_rx);
    memset(&s_telem, 0, sizeof(s_telem));
    tank_reset_state();
}

static void handshake_tick(uint32_t now)
{
    if (s_hs == HS_IDLE || s_hs == HS_READY) {
        return;
    }

    poll_rx(now);

    const uint32_t elapsed = now - s_phase_start_ms;

    if (s_hs == HS_RX_BOOT) {
        if (elapsed < HANDSHAKE_T_RX_BOOT_MS) {
            return;
        }
        send_wake_f8();
        delay(WAKE_T_POST_F8_DELAY_MS);
        poll_rx(millis());
        s_hs = HS_POST_F8;
        s_phase_start_ms = millis();
        return;
    }

    if (s_hs == HS_POST_F8 && elapsed >= HANDSHAKE_T_POST_F8_MS) {
        handshake_mark_ready();
    }
}

static bool bus_poll_due(uint32_t now)
{
    if ((int32_t)(now - s_next_poll_ms) < 0) {
        return false;
    }
    s_next_poll_ms = now + NOBANA_POLL_MS;
    return true;
}

static uint32_t phase_elapsed(uint32_t now)
{
    return now - s_phase_start_ms;
}

static uint32_t prestop_duration_ms()
{
    return s_manual_abort ? DISPENSE_T_PRESTOP_FAST_MS : DISPENSE_T_PRESTOP_MS;
}

static uint32_t compute_active_ms(uint32_t duration_ms)
{
    const uint32_t overhead = DISPENSE_T_PRESTOP_MS + DISPENSE_CLOSE_BUDGET_MS;
    if (duration_ms <= overhead + DISPENSE_ACTIVE_MIN_MS) {
        return DISPENSE_ACTIVE_MIN_MS;
    }
    return duration_ms - overhead;
}

static void standby_begin()
{
    if (s_dispense != DISPENSE_OFF || !s_wake_ready) {
        return;
    }
    s_standby_active = true;
    s_seq = 1;
    s_next_poll_ms = 0;
}

static void standby_tick(uint32_t now)
{
    if (!s_standby_active || s_dispense != DISPENSE_OFF) {
        return;
    }

    poll_rx(now);

    if (bus_poll_due(now)) {
        send_frame(NOBANA_CMD_START_UV, 0x00, 0x00, 0x00, 0x00, 0x00);
        poll_rx(now);
    }
}

static void dispense_set_seq_for_phase(DispensePhase phase)
{
    switch (phase) {
    case DISPENSE_ACTIVE:
    case DISPENSE_PRE_STOP:
    case DISPENSE_STOP_22_04:
    case DISPENSE_CLOSE_22_00:
        s_seq = 2;
        break;
    case DISPENSE_COOLDOWN_22:
        s_seq = 3;
        break;
    default:
        break;
    }
}

static void dispense_enter_phase(DispensePhase phase)
{
    s_dispense = phase;
    s_phase_start_ms = millis();
    s_next_poll_ms = 0;
    dispense_set_seq_for_phase(phase);

    if (phase == DISPENSE_STOP_22_04) {
        send_frame(NOBANA_CMD_NATURAL_UV, 0x00, 0x00, NOBANA_B5_PRESET, 0x00, NOBANA_D7_COFFEE);
        poll_rx(millis());
        dispense_enter_phase(DISPENSE_CLOSE_22_00);
    } else if (phase == DISPENSE_COOLDOWN_22) {
        s_dispense_stop_done_pulse = true;
    }
}

static void dispense_finish()
{
    s_dispense = DISPENSE_OFF;
    s_manual_abort = false;
    s_duration_ms = 0;
    s_dispense_start_ms = 0;
    s_next_poll_ms = 0;
    s_standby_active = false;
    s_dispense_done_pulse = true;
}

static void dispense_start_internal(uint32_t duration_ms)
{
    if (s_dispense != DISPENSE_OFF || !s_wake_ready || !s_standby_active) {
        return;
    }

    s_nob_close_seen = false;
    s_manual_abort = false;
    memset(&s_telem, 0, sizeof(s_telem));
    line_reset(&s_rx);

    s_duration_ms = duration_ms;
    s_dispense_start_ms = millis();

    const uint32_t active_ms = compute_active_ms(duration_ms);
    s_active_deadline_ms = s_dispense_start_ms + active_ms;
    dispense_enter_phase(DISPENSE_ACTIVE);
}

static void dispense_enter_manual_close()
{
    send_frame(NOBANA_CMD_NATURAL_UV, 0x00, 0x00, 0x00, 0x00, NOBANA_D7_COFFEE);
    poll_rx(millis());
    dispense_enter_phase(DISPENSE_CLOSE_22_00);
}

static void dispense_tick(uint32_t now)
{
    if (s_dispense == DISPENSE_OFF) {
        return;
    }

    poll_rx(now);

    switch (s_dispense) {
    case DISPENSE_ACTIVE:
        if (bus_poll_due(now)) {
            send_frame(NOBANA_CMD_HOT_UV, 0x00, 0x00, 0x00, 0x00, NOBANA_D7_COFFEE);
            poll_rx(now);
        }
        if ((int32_t)(now - s_active_deadline_ms) >= 0) {
            dispense_enter_phase(DISPENSE_PRE_STOP);
        }
        break;

    case DISPENSE_PRE_STOP:
        if (bus_poll_due(now)) {
            send_frame(NOBANA_CMD_HOT_UV, 0x00, 0x00, NOBANA_B5_PRESET, 0x00, NOBANA_D7_COFFEE);
            poll_rx(now);
        }
        if (phase_elapsed(now) >= prestop_duration_ms()) {
            if (s_manual_abort) {
                dispense_enter_manual_close();
            } else {
                dispense_enter_phase(DISPENSE_STOP_22_04);
            }
        }
        break;

    case DISPENSE_STOP_22_04:
        break;

    case DISPENSE_CLOSE_22_00:
        if (bus_poll_due(now)) {
            send_frame(NOBANA_CMD_NATURAL_UV, 0x00, 0x00, 0x00, 0x00, NOBANA_D7_COFFEE);
            poll_rx(now);
        }
        if (phase_elapsed(now) >= DISPENSE_T_CLOSE_MIN_MS
            && (s_nob_close_seen || phase_elapsed(now) >= DISPENSE_T_CLOSE_MAX_MS)) {
            dispense_enter_phase(DISPENSE_COOLDOWN_22);
        }
        break;

    case DISPENSE_COOLDOWN_22:
        if (bus_poll_due(now)) {
            send_frame(NOBANA_CMD_NATURAL_UV, 0x00, 0x00, 0x00, 0x00, NOBANA_D7_COFFEE);
            poll_rx(now);
        }
        if (phase_elapsed(now) >= DISPENSE_T_COOLDOWN_MS) {
            dispense_finish();
        }
        break;

    default:
        dispense_finish();
        break;
    }
}

void nobana_product_init()
{
    memset(&s_rx, 0, sizeof(s_rx));
    memset(&s_telem, 0, sizeof(s_telem));
    tank_reset_state();
    s_dispense_stop_done_pulse = false;
    s_dispense_done_pulse = false;

    delay(NOBANA_BUS_SETTLE_MS);
    nobana_bus_init();
    delay(NOBANA_T_BOOT_MS);

    handshake_begin();
    const uint32_t wake_deadline = millis() + NOBANA_WAKE_TIMEOUT_MS;
    while (!s_wake_ready && (int32_t)(wake_deadline - millis()) > 0) {
        handshake_tick(millis());
        delay(2);
    }

    if (s_wake_ready) {
        standby_begin();
    }
}

void nobana_tick()
{
    const uint32_t now = millis();
    handshake_tick(now);
    dispense_tick(now);
    standby_tick(now);
}

bool nobana_wake_ready()
{
    return s_wake_ready;
}

bool nobana_standby_active()
{
    return s_standby_active;
}

void nobana_standby_enable()
{
    if (s_wake_ready && s_dispense == DISPENSE_OFF) {
        standby_begin();
    }
}

void nobana_standby_disable()
{
    s_standby_active = false;
    s_next_poll_ms = 0;
}

bool nobana_dispense_start(uint32_t duration_ms)
{
    if (s_dispense != DISPENSE_OFF || !s_wake_ready || !s_standby_active) {
        return false;
    }
    if (duration_ms == 0) {
        duration_ms = DISPENSE_DURATION_DEFAULT_MS;
    }
    s_dispense_stop_done_pulse = false;
    s_dispense_done_pulse = false;
    dispense_start_internal(duration_ms);
    return s_dispense != DISPENSE_OFF;
}

bool nobana_dispense_abort()
{
    if (s_dispense == DISPENSE_OFF) {
        return false;
    }
    if (s_dispense == DISPENSE_CLOSE_22_00 || s_dispense == DISPENSE_COOLDOWN_22
        || s_dispense == DISPENSE_STOP_22_04) {
        return false;
    }

    s_manual_abort = true;

    if (s_dispense == DISPENSE_ACTIVE) {
        dispense_enter_phase(DISPENSE_PRE_STOP);
    }

    return true;
}

bool nobana_dispense_busy()
{
    return s_dispense != DISPENSE_OFF;
}

bool nobana_live_temp_c(uint8_t *out_c)
{
    if (!out_c || !s_telem.valid) {
        return false;
    }
    *out_c = s_telem.t_live;
    return true;
}

uint32_t nobana_dispense_remaining_ms()
{
    if (s_dispense == DISPENSE_OFF || s_duration_ms == 0) {
        return 0;
    }

    const uint32_t now = millis();
    const uint32_t end_ms = s_dispense_start_ms + s_duration_ms;
    if ((int32_t)(end_ms - now) <= 0) {
        return 0;
    }
    return end_ms - now;
}

bool nobana_dispense_consume_stop_done()
{
    if (!s_dispense_stop_done_pulse) {
        return false;
    }
    s_dispense_stop_done_pulse = false;
    return true;
}

bool nobana_dispense_consume_done()
{
    if (!s_dispense_done_pulse) {
        return false;
    }
    s_dispense_done_pulse = false;
    return true;
}

bool nobana_tank_empty()
{
    return s_tank_stable_ready && s_tank_empty_stable;
}

bool nobana_tank_stable_ready()
{
    return s_tank_stable_ready;
}

TankTransition nobana_tank_poll()
{
    const TankTransition transition = s_tank_pending_transition;
    s_tank_pending_transition = TANK_TRANSITION_NONE;
    return transition;
}
