/*
 * Mate Point — UART Nobana POC v0-2 (modo kiosco W/S/R)
 * Placa: ESP32 Dev Module (NodeMCU 38p) + TXS0108E
 *
 * Cableado (ARMOR desconectado):
 *   Nobana Tx → GPIO25 (Serial2 RX)
 *   Nobana Rx ← GPIO17 (Serial2 TX)
 *
 * Monitor USB: 115200 | Bus Nobana: 9600 8N1
 *
 * W = wake (F8) — 1x por sesion
 * S = standby (poll 21) — tras wake
 * R = dispensado E2 -> 22 (sin lock 23) — requiere S
 * Ver: PLAN-MATE-POINT-UART-v0-2.md, PROTOCOLO-UART-NOBANA.md
 */

#include <Arduino.h>
#include <string.h>

#define PIN_NOB_RX 25
#define PIN_NOB_TX 17

#define SERIAL_DEBUG_BAUD 115200
#define NOBANA_BAUD_DEFAULT 9600
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

#define SETUP_USB_SETTLE_MS 2000
#define HANDSHAKE_T_RX_BOOT_MS 5000
#define HANDSHAKE_T_POST_F8_MS 3000
#define WAKE_T_POST_F8_DELAY_MS 170
#define DISPENSE_T_DISPENSE_MS 24000
#define DISPENSE_T_PRESTOP_MS 3900
#define DISPENSE_T_CLOSE_MIN_MS 2000
#define DISPENSE_T_CLOSE_MAX_MS 8000
#define DISPENSE_T_COOLDOWN_MS 15000
#define DISPENSE_PROGRESS_STOP 155

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
    uint8_t t_live;
    uint8_t phase;
    uint16_t progress;
    uint32_t updated_ms;
};

static DispensePhase s_dispense = DISPENSE_OFF;
static HandshakePhase s_hs = HS_IDLE;
static uint8_t s_seq = 1;
static uint32_t s_bus_baud = NOBANA_BAUD_DEFAULT;
static uint32_t s_phase_start_ms = 0;
static uint32_t s_next_poll_ms = 0;
static bool s_verbose = false;
static bool s_handshake_rx_log = false;
static bool s_wake_ready = false;
static bool s_standby_active = false;
static bool s_nob_close_seen = false;
static uint32_t s_hs_rx_bytes = 0;
static uint32_t s_hs_valid_frames = 0;

static LineState s_rx;
static NobanaTelemetry s_telem;
static NobanaTelemetry s_last_logged;

static const char *dispense_phase_name(DispensePhase p)
{
    switch (p) {
    case DISPENSE_OFF:
        return "OFF";
    case DISPENSE_ACTIVE:
        return "DISPENSE";
    case DISPENSE_PRE_STOP:
        return "PRE_STOP";
    case DISPENSE_STOP_22_04:
        return "STOP_22_04";
    case DISPENSE_CLOSE_22_00:
        return "CLOSE_22_00";
    case DISPENSE_COOLDOWN_22:
        return "COOLDOWN_22";
    default:
        return "?";
    }
}

static const char *handshake_phase_name(HandshakePhase p)
{
    switch (p) {
    case HS_IDLE:
        return "IDLE";
    case HS_RX_BOOT:
        return "RX_BOOT";
    case HS_POST_F8:
        return "POST_F8";
    case HS_READY:
        return "READY";
    default:
        return "?";
    }
}

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

static void print_hex(const char *tag, const uint8_t *data, size_t len)
{
    Serial.printf("[%lu] %s len=%u HEX:", (unsigned long)millis(), tag, (unsigned)len);
    for (size_t i = 0; i < len; i++) {
        Serial.printf(" %02X", data[i]);
    }
    Serial.println();
}

static const char *phase_label(uint8_t phase)
{
    if (phase == 0x14) {
        return "activo/precalent";
    }
    if (phase == 0x15) {
        return "fin/cierre";
    }
    return "otro";
}

static void store_telemetry_from_frame(const uint8_t *data, size_t len)
{
    if (len != RX_FRAME_LEN) {
        return;
    }
    s_telem.valid = true;
    s_telem.b2 = data[2];
    s_telem.t_live = data[3];
    s_telem.phase = data[4];
    s_telem.progress = ((uint16_t)data[8] << 8) | data[9];
    s_telem.updated_ms = millis();

    if (s_telem.b2 == NOBANA_NOB_B2_CLOSE) {
        s_nob_close_seen = true;
    }
}

static void log_telemetry(bool force)
{
    if (!s_telem.valid) {
        return;
    }

    const bool changed = force || !s_last_logged.valid || s_telem.t_live != s_last_logged.t_live
        || s_telem.phase != s_last_logged.phase || s_telem.progress != s_last_logged.progress
        || s_telem.b2 != s_last_logged.b2;

    if (!changed && !s_verbose) {
        return;
    }

    s_last_logged = s_telem;

    Serial.printf("[%lu] telem disp=%s standby=%s T_obj=85 T_act=%u b2=0x%02X fase=0x%02X (%s) "
                  "progress=%u",
                  (unsigned long)millis(), dispense_phase_name(s_dispense),
                  s_standby_active ? "ON" : "OFF", s_telem.t_live, s_telem.b2, s_telem.phase,
                  phase_label(s_telem.phase), s_telem.progress);
    Serial.println();
}

static void log_valid_nob_frame(const uint8_t *data, size_t len, const char *tag)
{
    print_hex(tag, data, len);
    store_telemetry_from_frame(data, len);
    if (s_dispense != DISPENSE_OFF) {
        log_telemetry(false);
    }
}

static void parse_nob_frame(const uint8_t *data, size_t len)
{
    if (len != RX_FRAME_LEN || data[0] != NOBANA_HDR || !frame_checksum_ok(data, len)) {
        if (s_verbose && len > 0) {
            print_hex("NOB->ESP (ignorada)", data, len);
        }
        return;
    }

    store_telemetry_from_frame(data, len);

    if (s_verbose) {
        print_hex("NOB->ESP", data, len);
    }
    if (s_dispense != DISPENSE_OFF) {
        log_telemetry(false);
    }
}

static void uart_begin_bus(uint32_t baud)
{
    Serial2.end();
    delay(10);
    Serial2.begin(baud, SERIAL_8N1, PIN_NOB_RX, PIN_NOB_TX);
    s_bus_baud = baud;
    Serial.printf("[cfg] bus %lu 8N1 RX=GPIO%d TX=GPIO%d\n", (unsigned long)baud, PIN_NOB_RX,
                  PIN_NOB_TX);
}

static void line_reset(LineState *st)
{
    st->len = 0;
    st->last_byte_ms = 0;
    st->active = false;
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

static void line_flush(LineState *st, uint32_t now)
{
    (void)now;
    if (!st->active || st->len == 0) {
        line_reset(st);
        return;
    }

    if (s_handshake_rx_log) {
        s_hs_rx_bytes += st->len;
        if (st->len == RX_FRAME_LEN && st->buf[0] == NOBANA_HDR && frame_checksum_ok(st->buf, st->len)) {
            s_hs_valid_frames++;
            log_valid_nob_frame(st->buf, st->len, "NOB->ESP (wake)");
        } else {
            print_hex("NOB->ESP (raw)", st->buf, st->len);
        }
        line_reset(st);
        return;
    }

    parse_nob_frame(st->buf, st->len);
    line_reset(st);
}

static void poll_rx(uint32_t now)
{
    while (Serial2.available() > 0) {
        line_push(&s_rx, (uint8_t)Serial2.read(), now);
    }
    if (s_rx.active && (now - s_rx.last_byte_ms) >= FRAME_GAP_MS) {
        line_flush(&s_rx, now);
    }
}

static void send_frame(uint8_t cmd, uint8_t b3, uint8_t b4, uint8_t b5, uint8_t b6, uint8_t d7)
{
    uint8_t frame[TX_FRAME_LEN];
    build_tx(frame, cmd, b3, b4, b5, b6, d7);
    Serial2.write(frame, TX_FRAME_LEN);
    if (s_verbose) {
        print_hex("ESP->NOB", frame, TX_FRAME_LEN);
    }
}

static void send_wake_f8(bool log_tx)
{
    Serial2.write(NOBANA_WAKE_BYTE);
    if (log_tx || s_verbose) {
        Serial.printf("[%lu] ESP->NOB len=1 HEX: %02X\n", (unsigned long)millis(),
                      NOBANA_WAKE_BYTE);
    }
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
    s_handshake_rx_log = false;
    s_wake_ready = true;
    Serial.println();
    Serial.printf("[wake] LISTO — bytes_rx=%lu tramas_68_ok=%lu\n",
                  (unsigned long)s_hs_rx_bytes, (unsigned long)s_hs_valid_frames);
    if (s_hs_valid_frames == 0) {
        Serial.println("[wake] Sin trama 0x68 valida tras F8.");
        Serial.println("[wake] Repetir W con Nobana ON o revisar cableado/baud.");
    } else {
        Serial.println("[wake] Revisar lineas NOB->ESP. Luego S (standby) -> R (dispensar).");
    }
    Serial.println("[wake] S = standby (21 poll) | R = Coffee 180 ml (sin lock 23)");
    Serial.println();
}

static void handshake_begin()
{
    if (s_dispense != DISPENSE_OFF) {
        Serial.println("[err] Abortar dispensado (X) antes de wake (W)");
        return;
    }
    if (s_hs == HS_RX_BOOT || s_hs == HS_POST_F8) {
        Serial.println("[err] Wake en curso, espere [wake] LISTO");
        return;
    }

    s_hs = HS_RX_BOOT;
    s_wake_ready = false;
    session_reset_flags();
    s_handshake_rx_log = true;
    s_hs_rx_bytes = 0;
    s_hs_valid_frames = 0;
    s_phase_start_ms = millis();
    line_reset(&s_rx);
    memset(&s_telem, 0, sizeof(s_telem));
    memset(&s_last_logged, 0, sizeof(s_last_logged));

    Serial.println();
    Serial.println("[wake] INICIO — escuchando bus antes de F8");
    Serial.printf("[wake] fase=%s (%u ms)\n", handshake_phase_name(s_hs),
                  (unsigned)HANDSHAKE_T_RX_BOOT_MS);
    Serial.println("[wake] Encender Nobana si aun OFF; ver NOB->ESP en log");
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
        Serial.printf("[wake] fase=POST_F8 — enviando F8, escucha %u ms\n",
                      (unsigned)HANDSHAKE_T_POST_F8_MS);
        send_wake_f8(true);
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

static void standby_resume_log()
{
    s_seq = 1;
    s_next_poll_ms = 0;
    Serial.println("[standby] ON — poll 21 (auto tras fin o comando S)");
}

static void standby_begin()
{
    if (s_dispense != DISPENSE_OFF) {
        Serial.println("[standby] ignorado — dispensado en curso (use X para abortar)");
        return;
    }
    if (!s_wake_ready) {
        Serial.println("[err] Wake no listo. Nobana ON -> W -> [wake] LISTO -> S");
        return;
    }
    if (s_standby_active) {
        Serial.println("[standby] ya activo (poll 21)");
        return;
    }

    s_standby_active = true;
    standby_resume_log();
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
    Serial.printf("[dispense] fase=%s seq=%u\n", dispense_phase_name(phase), s_seq);

    if (phase == DISPENSE_STOP_22_04) {
        send_frame(NOBANA_CMD_NATURAL_UV, 0x00, 0x00, NOBANA_B5_PRESET, 0x00, NOBANA_D7_COFFEE);
        poll_rx(millis());
        dispense_enter_phase(DISPENSE_CLOSE_22_00);
    }
}

static void dispense_finish(const char *reason, bool resume_standby)
{
    log_telemetry(true);
    s_dispense = DISPENSE_OFF;
    s_next_poll_ms = 0;
    Serial.printf("[dispense] FIN (%s)\n", reason);

    if (resume_standby && s_standby_active) {
        standby_resume_log();
    }
}

static void dispense_abort(const char *reason)
{
    dispense_finish(reason, true);
}

static void dispense_start()
{
    if (s_dispense != DISPENSE_OFF) {
        Serial.println("[err] dispensado en curso. X = abortar");
        return;
    }
    if (!s_wake_ready) {
        Serial.println("[err] Wake no listo. Nobana ON -> W -> [wake] LISTO -> S -> R");
        return;
    }
    if (!s_standby_active) {
        Serial.println("[err] Standby no activo. Pulse S antes de R");
        return;
    }

    s_nob_close_seen = false;
    memset(&s_telem, 0, sizeof(s_telem));
    memset(&s_last_logged, 0, sizeof(s_last_logged));
    line_reset(&s_rx);

    Serial.println();
    Serial.println("[dispense] INICIO — Coffee 180 ml (E2 -> 22, sin lock 23)");

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
            Serial.printf("[dispense] progress>=%u -> PRE_STOP\n", DISPENSE_PROGRESS_STOP);
            dispense_enter_phase(DISPENSE_PRE_STOP);
        } else if (phase_elapsed(now) >= DISPENSE_T_DISPENSE_MS) {
            Serial.println("[dispense] T_DISPENSE -> PRE_STOP");
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
            if (s_nob_close_seen) {
                Serial.println("[dispense] NOB b2=0x11 -> COOLDOWN");
            } else {
                Serial.println("[dispense] CLOSE timeout -> COOLDOWN");
            }
            dispense_enter_phase(DISPENSE_COOLDOWN_22);
        }
        break;

    case DISPENSE_COOLDOWN_22:
        if (bus_poll_due(now)) {
            send_frame(NOBANA_CMD_NATURAL_UV, 0x00, 0x00, 0x00, 0x00, NOBANA_D7_COFFEE);
            poll_rx(now);
        }
        if (phase_elapsed(now) >= DISPENSE_T_COOLDOWN_MS) {
            dispense_finish("ciclo_sin_lock", true);
        }
        break;

    default:
        dispense_abort("fase_invalida");
        break;
    }
}

static void standby_end(const char *reason)
{
    s_standby_active = false;
    s_seq = 1;
    s_next_poll_ms = 0;
    Serial.printf("[standby] OFF (%s)\n", reason);
}

static void print_status()
{
    Serial.println();
    Serial.printf("  wake=%s (%s) standby=%s dispense=%s baud=%lu seq=%u verbose=%s\n",
                  s_wake_ready ? "OK" : "pendiente", handshake_phase_name(s_hs),
                  s_standby_active ? "ON" : "OFF", dispense_phase_name(s_dispense),
                  (unsigned long)s_bus_baud, s_seq, s_verbose ? "ON" : "OFF");
    if (s_dispense != DISPENSE_OFF) {
        Serial.printf("  fase_ms=%lu nob_close=%s\n", (unsigned long)phase_elapsed(millis()),
                      s_nob_close_seen ? "si" : "no");
    }
    if (s_telem.valid) {
        Serial.printf("  T_obj=85 T_act=%u b2=0x%02X fase=0x%02X progress=%u (hace %lu ms)\n",
                      s_telem.t_live, s_telem.b2, s_telem.phase, s_telem.progress,
                      (unsigned long)(millis() - s_telem.updated_ms));
    } else {
        Serial.println("  telemetria: sin trama NOB valida aun");
    }
    Serial.println();
}

static void print_banner()
{
    Serial.println();
    Serial.println("=== Mate Point UART v0-2 — kiosco W/S/R ===");
    Serial.printf("USB %d | Bus %lu 8N1 | Poll %d ms | ARMOR OFF\n", SERIAL_DEBUG_BAUD,
                  (unsigned long)NOBANA_BAUD_DEFAULT, NOBANA_POLL_MS);
    Serial.println("W wake | S standby (21) | R dispensar | X abortar/salir standby");
}

static void print_help()
{
    Serial.println();
    Serial.println("Comandos (Enter):");
    Serial.println("  W           wake: escucha 5s -> F8 -> log 3s");
    Serial.println("  S           standby: poll 21 (tras [wake] LISTO)");
    Serial.println("  R           dispensar Coffee 180 ml (requiere S; sin lock 23)");
    Serial.println("  X           abortar dispensado O salir standby");
    Serial.println("  ? / h       ayuda");
    Serial.println("  B9600       baud bus Nobana");
    Serial.println("  V           verbose HEX");
    Serial.println("  I           estado / telemetria");
    Serial.println();
    Serial.printf("Wake: %s | Standby: %s | Dispense: %s | Baud: %lu\n",
                  handshake_phase_name(s_hs), s_standby_active ? "ON" : "OFF",
                  dispense_phase_name(s_dispense), (unsigned long)s_bus_baud);
}

static void handle_serial_line(String line)
{
    line.trim();
    if (line.length() == 0) {
        return;
    }

    const char c0 = line.charAt(0);

    if (c0 == '?' || c0 == 'h' || c0 == 'H') {
        print_help();
        return;
    }

    if (c0 == 'B' || c0 == 'b') {
        const unsigned long baud = line.substring(1).toInt();
        if (baud < 300 || baud > 2000000) {
            Serial.println("[err] Baud invalido. Ej: B9600");
            return;
        }
        if (s_dispense != DISPENSE_OFF) {
            Serial.println("[err] No cambiar baud durante dispensado (X primero)");
            return;
        }
        uart_begin_bus((uint32_t)baud);
        line_reset(&s_rx);
        s_hs = HS_IDLE;
        s_wake_ready = false;
        session_reset_flags();
        s_handshake_rx_log = false;
        Serial.println("[cfg] Baud cambiado. Pulse W (wake) -> S -> R.");
        return;
    }

    if (c0 == 'V' || c0 == 'v') {
        s_verbose = !s_verbose;
        memset(&s_last_logged, 0, sizeof(s_last_logged));
        Serial.println(s_verbose ? "[cfg] verbose ON" : "[cfg] verbose OFF");
        return;
    }

    if (c0 == 'I' || c0 == 'i') {
        print_status();
        return;
    }

    if (c0 == 'W' || c0 == 'w') {
        if (line.length() > 1) {
            Serial.println("[err] W = wake (sin argumentos)");
            return;
        }
        handshake_begin();
        return;
    }

    if (c0 == 'S' || c0 == 's') {
        if (line.length() > 1) {
            Serial.println("[err] S = standby (sin argumentos)");
            return;
        }
        standby_begin();
        return;
    }

    if (c0 == 'R' || c0 == 'r') {
        if (line.length() > 1) {
            Serial.println("[err] R = dispensar (sin argumentos)");
            return;
        }
        dispense_start();
        return;
    }

    if (c0 == 'X' || c0 == 'x') {
        if (line.length() > 1) {
            Serial.println("[err] X = abort (sin argumentos)");
            return;
        }
        if (s_dispense != DISPENSE_OFF) {
            dispense_abort("usuario_X");
            return;
        }
        if (s_standby_active) {
            standby_end("usuario_X");
            return;
        }
        Serial.println("[standby] ya OFF");
        return;
    }

    Serial.println("[err] Comando desconocido. ? = ayuda");
}

static void poll_usb_serial()
{
    if (!Serial.available()) {
        return;
    }
    String line = Serial.readStringUntil('\n');
    handle_serial_line(line);
}

void setup()
{
    Serial.begin(SERIAL_DEBUG_BAUD);
    delay(SETUP_USB_SETTLE_MS);

    memset(&s_rx, 0, sizeof(s_rx));
    memset(&s_telem, 0, sizeof(s_telem));
    memset(&s_last_logged, 0, sizeof(s_last_logged));

    uart_begin_bus(NOBANA_BAUD_DEFAULT);
    print_banner();
    print_help();
    Serial.println("[ready] Nobana ON -> W -> S -> R (ver PLAN-MATE-POINT-UART-v0-2.md)");
}

void loop()
{
    const uint32_t now = millis();
    poll_usb_serial();
    handshake_tick(now);
    dispense_tick(now);
    standby_tick(now);
    delay(2);
}
