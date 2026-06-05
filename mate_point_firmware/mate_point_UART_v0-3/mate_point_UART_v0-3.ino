/*
 * Mate Point — UART Nobana POC v0-3 (Waveshare, ciclo automatico)
 * Placa: Waveshare ESP32-S3-Touch-LCD-7B + TXS0108E
 *
 * USB CDC On Boot = Disabled.
 * DIP UART2: UART0 (Serial) @ 9600 8N1 hacia Nobana — sin logs en Serial.
 *
 * Cableado (ARMOR desconectado):
 *   Nobana Tx → GPIO44 (RX)
 *   Nobana Rx ← GPIO43 (TX)
 *
 * Una pasada por encendido: boot -> F8 -> S (21) -> R (E2/22) -> silencio en TX.
 * Ver PROTOCOLO-UART-NOBANA.md
 */

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
#define NOBANA_NOB_B2_CLOSE 0x11

#define HANDSHAKE_T_RX_BOOT_MS 5000
#define HANDSHAKE_T_POST_F8_MS 3000
#define WAKE_T_POST_F8_DELAY_MS 170
#define DISPENSE_T_DISPENSE_MS 24000
#define DISPENSE_T_PRESTOP_MS 3900
#define DISPENSE_T_CLOSE_MIN_MS 2000
#define DISPENSE_T_CLOSE_MAX_MS 8000
#define DISPENSE_T_COOLDOWN_MS 15000
#define DISPENSE_PROGRESS_STOP 155

#define AUTO_T_BOOT_MS 3000
#define AUTO_T_AFTER_WAKE_MS 2000
#define AUTO_T_STANDBY_MS 3000
#define AUTO_WAKE_TIMEOUT_MS 12000

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

enum AutoPhase {
    AUTO_BOOT_WAIT,
    AUTO_WAKE_RUN,
    AUTO_DELAY_AFTER_WAKE,
    AUTO_DELAY_BEFORE_DISPENSE,
    AUTO_DISPENSE_RUN,
    AUTO_DONE,
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
    uint16_t progress;
};

static DispensePhase s_dispense = DISPENSE_OFF;
static HandshakePhase s_hs = HS_IDLE;
static AutoPhase s_auto = AUTO_BOOT_WAIT;
static uint8_t s_seq = 1;
static uint32_t s_phase_start_ms = 0;
static uint32_t s_next_poll_ms = 0;
static uint32_t s_auto_wake_started_ms = 0;
static bool s_wake_ready = false;
static bool s_standby_active = false;
static bool s_nob_close_seen = false;
static bool s_auto_dispense_started = false;

static LineState s_rx;
static NobanaTelemetry s_telem;

static void line_reset(LineState *st);
static void nobana_bus_init();

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

static void store_telemetry_from_frame(const uint8_t *data, size_t len)
{
    if (len != RX_FRAME_LEN) {
        return;
    }
    s_telem.valid = true;
    s_telem.b2 = data[2];
    s_telem.progress = ((uint16_t)data[8] << 8) | data[9];

    if (s_telem.b2 == NOBANA_NOB_B2_CLOSE) {
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

static void session_reset_flags()
{
    s_standby_active = false;
    s_dispense = DISPENSE_OFF;
    s_next_poll_ms = 0;
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
    session_reset_flags();
    s_phase_start_ms = millis();
    line_reset(&s_rx);
    memset(&s_telem, 0, sizeof(s_telem));
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
    }
}

static void dispense_finish()
{
    s_dispense = DISPENSE_OFF;
    s_next_poll_ms = 0;
    s_standby_active = false;
}

static void dispense_start()
{
    if (s_dispense != DISPENSE_OFF || !s_wake_ready || !s_standby_active) {
        return;
    }

    s_nob_close_seen = false;
    memset(&s_telem, 0, sizeof(s_telem));
    line_reset(&s_rx);
    dispense_enter_phase(DISPENSE_ACTIVE);
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
        if (s_telem.valid && s_telem.progress >= DISPENSE_PROGRESS_STOP) {
            dispense_enter_phase(DISPENSE_PRE_STOP);
        } else if (phase_elapsed(now) >= DISPENSE_T_DISPENSE_MS) {
            dispense_enter_phase(DISPENSE_PRE_STOP);
        }
        break;

    case DISPENSE_PRE_STOP:
        if (bus_poll_due(now)) {
            send_frame(NOBANA_CMD_HOT_UV, 0x00, 0x00, NOBANA_B5_PRESET, 0x00, NOBANA_D7_COFFEE);
            poll_rx(now);
        }
        if (phase_elapsed(now) >= DISPENSE_T_PRESTOP_MS) {
            dispense_enter_phase(DISPENSE_STOP_22_04);
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

static void auto_tick(uint32_t now)
{
    if (s_auto == AUTO_DONE) {
        return;
    }

    switch (s_auto) {
    case AUTO_BOOT_WAIT:
        if (phase_elapsed(now) >= AUTO_T_BOOT_MS) {
            s_auto_wake_started_ms = now;
            handshake_begin();
            s_auto = AUTO_WAKE_RUN;
        }
        break;

    case AUTO_WAKE_RUN:
        if (s_wake_ready) {
            s_phase_start_ms = now;
            s_auto = AUTO_DELAY_AFTER_WAKE;
            break;
        }
        if ((now - s_auto_wake_started_ms) >= AUTO_WAKE_TIMEOUT_MS) {
            s_auto = AUTO_DONE;
            s_standby_active = false;
        }
        break;

    case AUTO_DELAY_AFTER_WAKE:
        if (phase_elapsed(now) >= AUTO_T_AFTER_WAKE_MS) {
            standby_begin();
            s_phase_start_ms = now;
            s_auto = AUTO_DELAY_BEFORE_DISPENSE;
        }
        break;

    case AUTO_DELAY_BEFORE_DISPENSE:
        if (phase_elapsed(now) >= AUTO_T_STANDBY_MS) {
            s_auto_dispense_started = true;
            dispense_start();
            s_auto = AUTO_DISPENSE_RUN;
        }
        break;

    case AUTO_DISPENSE_RUN:
        if (s_auto_dispense_started && s_dispense == DISPENSE_OFF) {
            s_auto = AUTO_DONE;
        }
        break;

    default:
        break;
    }
}

void setup()
{
    memset(&s_rx, 0, sizeof(s_rx));
    memset(&s_telem, 0, sizeof(s_telem));

    delay(NOBANA_BUS_SETTLE_MS);
    nobana_bus_init();
    s_phase_start_ms = millis();
}

void loop()
{
    const uint32_t now = millis();
    auto_tick(now);
    handshake_tick(now);
    dispense_tick(now);
    standby_tick(now);
    delay(2);
}
