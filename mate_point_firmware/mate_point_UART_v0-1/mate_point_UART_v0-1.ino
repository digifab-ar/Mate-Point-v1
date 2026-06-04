/*
 * Mate Point — UART Nobana POC (Etapa 1)
 * Placa: ESP32 Dev Module (NodeMCU 38p) + TXS0108E
 *
 * Cableado (ARMOR desconectado):
 *   Nobana Tx → GPIO25 (Serial2 RX)
 *   Nobana Rx ← GPIO17 (Serial2 TX)
 *
 * Monitor USB: 115200 | Bus Nobana: 9600 8N1
 *
 * Arranque: wake automático (F8 + log RX) → luego R = replay 21→23→E2…
 * Ver: PLAN-POC-NOBANA-UART.md, PROTOCOLO-UART-NOBANA.md
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
#define NOBANA_CMD_LOCK_UV 0x23
#define NOBANA_CMD_HOT_UV 0xE2
#define NOBANA_CMD_NATURAL_UV 0x22
#define NOBANA_D7_COFFEE 0x55
#define NOBANA_B5_PRESET 0x04
#define NOBANA_NOB_B2_ACTIVE 0x12
#define NOBANA_NOB_B2_CLOSE 0x11

#define HANDSHAKE_T_RX_BOOT_MS 5000
#define HANDSHAKE_T_POST_F8_MS 3000
#define REPLAY_T_WAKE_DELAY_MS 170
#define REPLAY_T_START_21_MS 4000
#define REPLAY_T_IDLE_23_MS 3000
#define REPLAY_T_DISPENSE_MS 24000
#define REPLAY_T_PRESTOP_MS 3900
#define REPLAY_T_CLOSE_MIN_MS 2000
#define REPLAY_T_CLOSE_MAX_MS 8000
#define REPLAY_T_COOLDOWN_MS 15000
#define REPLAY_T_LOCK_MS 3000
#define REPLAY_PROGRESS_STOP 155

enum HandshakePhase {
    HS_RX_BOOT,
    HS_POST_F8,
    HS_READY,
};

enum ReplayPhase {
    REPLAY_OFF,
    REPLAY_START_21,
    REPLAY_IDLE_23,
    REPLAY_DISPENSE,
    REPLAY_PRE_STOP,
    REPLAY_STOP_22_04,
    REPLAY_CLOSE_22_00,
    REPLAY_COOLDOWN_22,
    REPLAY_LOCK_23,
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

static ReplayPhase s_replay = REPLAY_OFF;
static HandshakePhase s_hs = HS_RX_BOOT;
static uint8_t s_seq = 1;
static uint32_t s_bus_baud = NOBANA_BAUD_DEFAULT;
static uint32_t s_phase_start_ms = 0;
static uint32_t s_next_poll_ms = 0;
static bool s_verbose = false;
static bool s_handshake_rx_log = false;
static bool s_wake_ready = false;
static bool s_nob_close_seen = false;
static uint32_t s_hs_rx_bytes = 0;
static uint32_t s_hs_valid_frames = 0;

static LineState s_rx;
static NobanaTelemetry s_telem;
static NobanaTelemetry s_last_logged;

static const char *replay_phase_name(ReplayPhase p)
{
    switch (p) {
    case REPLAY_OFF:
        return "OFF";
    case REPLAY_START_21:
        return "START_21";
    case REPLAY_IDLE_23:
        return "IDLE_23";
    case REPLAY_DISPENSE:
        return "DISPENSE";
    case REPLAY_PRE_STOP:
        return "PRE_STOP";
    case REPLAY_STOP_22_04:
        return "STOP_22_04";
    case REPLAY_CLOSE_22_00:
        return "CLOSE_22_00";
    case REPLAY_COOLDOWN_22:
        return "COOLDOWN_22";
    case REPLAY_LOCK_23:
        return "LOCK_23";
    default:
        return "?";
    }
}

static const char *handshake_phase_name(HandshakePhase p)
{
    switch (p) {
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

    Serial.printf("[%lu] telem replay=%s T_obj=85 T_act=%u b2=0x%02X fase=0x%02X (%s) progress=%u",
                  (unsigned long)millis(), replay_phase_name(s_replay), s_telem.t_live, s_telem.b2,
                  s_telem.phase, phase_label(s_telem.phase), s_telem.progress);
    Serial.println();
}

static void log_valid_nob_frame(const uint8_t *data, size_t len, const char *tag)
{
    print_hex(tag, data, len);
    store_telemetry_from_frame(data, len);
    if (s_replay != REPLAY_OFF) {
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
    log_telemetry(false);
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
        Serial.println("[wake] Si Nobana estaba OFF: encenderlo, pulsar W, revisar log, luego R.");
    } else {
        Serial.println("[wake] Revisar lineas NOB->ESP arriba. Cuando OK, enviar R en Monitor.");
    }
    Serial.println("[wake] R = replay 21 -> 23 -> E2 (sin repetir F8)");
    Serial.println();
}

static void handshake_begin()
{
    if (s_replay != REPLAY_OFF) {
        Serial.println("[err] Abortar replay (X) antes de repetir wake");
        return;
    }

    s_hs = HS_RX_BOOT;
    s_wake_ready = false;
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
    Serial.println("[wake] En estos 5 s: encender Nobana (ver NOB->ESP raw/ok en log)");
}

static void handshake_tick(uint32_t now)
{
    if (s_hs == HS_READY) {
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
        delay(REPLAY_T_WAKE_DELAY_MS);
        poll_rx(millis());
        s_hs = HS_POST_F8;
        s_phase_start_ms = millis();
        return;
    }

    if (s_hs == HS_POST_F8 && elapsed >= HANDSHAKE_T_POST_F8_MS) {
        handshake_mark_ready();
    }
}

static void replay_set_seq_for_phase(ReplayPhase phase)
{
    switch (phase) {
    case REPLAY_START_21:
        s_seq = 1;
        break;
    case REPLAY_IDLE_23:
        s_seq = 2;
        break;
    case REPLAY_DISPENSE:
    case REPLAY_PRE_STOP:
    case REPLAY_STOP_22_04:
    case REPLAY_CLOSE_22_00:
        s_seq = 3;
        break;
    case REPLAY_COOLDOWN_22:
        s_seq = 4;
        break;
    case REPLAY_LOCK_23:
        s_seq = 5;
        break;
    default:
        break;
    }
}

static void replay_enter_phase(ReplayPhase phase)
{
    s_replay = phase;
    s_phase_start_ms = millis();
    s_next_poll_ms = 0;
    replay_set_seq_for_phase(phase);
    Serial.printf("[replay] fase=%s seq=%u\n", replay_phase_name(phase), s_seq);

    if (phase == REPLAY_STOP_22_04) {
        send_frame(NOBANA_CMD_NATURAL_UV, 0x00, 0x00, NOBANA_B5_PRESET, 0x00, NOBANA_D7_COFFEE);
        poll_rx(millis());
        replay_enter_phase(REPLAY_CLOSE_22_00);
    }
}

static void replay_finish(const char *reason)
{
    log_telemetry(true);
    s_replay = REPLAY_OFF;
    s_next_poll_ms = 0;
    Serial.printf("[replay] FIN (%s)\n", reason);
}

static void replay_abort(const char *reason)
{
    replay_finish(reason);
}

static void replay_start()
{
    if (s_replay != REPLAY_OFF) {
        Serial.println("[err] replay en curso. X = abortar");
        return;
    }
    if (!s_wake_ready) {
        Serial.println("[err] Wake no listo. Espere [wake] LISTO o pulse W tras Nobana ON.");
        return;
    }

    s_nob_close_seen = false;
    memset(&s_telem, 0, sizeof(s_telem));
    memset(&s_last_logged, 0, sizeof(s_last_logged));
    line_reset(&s_rx);

    Serial.println();
    Serial.println("[replay] INICIO — Coffee 180 ml (captura ref.)");
    Serial.println("[replay] 21 -> 23 -> E2 -> pre-stop -> 22 -> lock");

    replay_enter_phase(REPLAY_START_21);
}

static bool replay_poll_due(uint32_t now)
{
    if ((int32_t)(now - s_next_poll_ms) < 0) {
        return false;
    }
    s_next_poll_ms = now + NOBANA_POLL_MS;
    return true;
}

static uint32_t replay_phase_elapsed(uint32_t now)
{
    return now - s_phase_start_ms;
}

static void replay_tick(uint32_t now)
{
    if (s_replay == REPLAY_OFF) {
        return;
    }

    poll_rx(now);

    switch (s_replay) {
    case REPLAY_START_21:
        if (replay_poll_due(now)) {
            send_frame(NOBANA_CMD_START_UV, 0x00, 0x00, 0x00, 0x00, 0x00);
            poll_rx(now);
        }
        if (replay_phase_elapsed(now) >= REPLAY_T_START_21_MS) {
            replay_enter_phase(REPLAY_IDLE_23);
        }
        break;

    case REPLAY_IDLE_23:
        if (replay_poll_due(now)) {
            send_frame(NOBANA_CMD_LOCK_UV, 0x00, 0x00, 0x00, 0x00, 0x00);
            poll_rx(now);
        }
        if (replay_phase_elapsed(now) >= REPLAY_T_IDLE_23_MS) {
            replay_enter_phase(REPLAY_DISPENSE);
        }
        break;

    case REPLAY_DISPENSE:
        if (replay_poll_due(now)) {
            send_frame(NOBANA_CMD_HOT_UV, 0x00, 0x00, 0x00, 0x00, NOBANA_D7_COFFEE);
            poll_rx(now);
        }
        if (s_telem.valid && s_telem.progress >= REPLAY_PROGRESS_STOP) {
            Serial.printf("[replay] progress>=%u -> PRE_STOP\n", REPLAY_PROGRESS_STOP);
            replay_enter_phase(REPLAY_PRE_STOP);
        } else if (replay_phase_elapsed(now) >= REPLAY_T_DISPENSE_MS) {
            Serial.println("[replay] T_DISPENSE -> PRE_STOP");
            replay_enter_phase(REPLAY_PRE_STOP);
        }
        break;

    case REPLAY_PRE_STOP:
        if (replay_poll_due(now)) {
            send_frame(NOBANA_CMD_HOT_UV, 0x00, 0x00, NOBANA_B5_PRESET, 0x00, NOBANA_D7_COFFEE);
            poll_rx(now);
        }
        if (replay_phase_elapsed(now) >= REPLAY_T_PRESTOP_MS) {
            replay_enter_phase(REPLAY_STOP_22_04);
        }
        break;

    case REPLAY_STOP_22_04:
        break;

    case REPLAY_CLOSE_22_00:
        if (replay_poll_due(now)) {
            send_frame(NOBANA_CMD_NATURAL_UV, 0x00, 0x00, 0x00, 0x00, NOBANA_D7_COFFEE);
            poll_rx(now);
        }
        if (replay_phase_elapsed(now) >= REPLAY_T_CLOSE_MIN_MS
            && (s_nob_close_seen || replay_phase_elapsed(now) >= REPLAY_T_CLOSE_MAX_MS)) {
            if (s_nob_close_seen) {
                Serial.println("[replay] NOB b2=0x11 -> COOLDOWN");
            } else {
                Serial.println("[replay] CLOSE timeout -> COOLDOWN");
            }
            replay_enter_phase(REPLAY_COOLDOWN_22);
        }
        break;

    case REPLAY_COOLDOWN_22:
        if (replay_poll_due(now)) {
            send_frame(NOBANA_CMD_NATURAL_UV, 0x00, 0x00, 0x00, 0x00, NOBANA_D7_COFFEE);
            poll_rx(now);
        }
        if (replay_phase_elapsed(now) >= REPLAY_T_COOLDOWN_MS) {
            replay_enter_phase(REPLAY_LOCK_23);
        }
        break;

    case REPLAY_LOCK_23:
        if (replay_poll_due(now)) {
            send_frame(NOBANA_CMD_LOCK_UV, 0x00, 0x00, 0x00, 0x00, NOBANA_D7_COFFEE);
            poll_rx(now);
        }
        if (replay_phase_elapsed(now) >= REPLAY_T_LOCK_MS) {
            replay_finish("ciclo_completo");
        }
        break;

    default:
        replay_abort("fase_invalida");
        break;
    }
}

static void print_status()
{
    Serial.println();
    Serial.printf("  wake=%s (%s) replay=%s baud=%lu seq=%u verbose=%s\n",
                  s_wake_ready ? "OK" : "pendiente", handshake_phase_name(s_hs),
                  replay_phase_name(s_replay), (unsigned long)s_bus_baud, s_seq,
                  s_verbose ? "ON" : "OFF");
    if (s_replay != REPLAY_OFF) {
        Serial.printf("  fase_ms=%lu nob_close=%s\n", (unsigned long)replay_phase_elapsed(millis()),
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
    Serial.println("=== Mate Point UART v0-1 — wake + replay ===");
    Serial.printf("USB %d | Bus %lu 8N1 | Poll %d ms | ARMOR OFF\n", SERIAL_DEBUG_BAUD,
                  (unsigned long)NOBANA_BAUD_DEFAULT, NOBANA_POLL_MS);
    Serial.println("Al arrancar: wake auto (F8 + log RX). Luego R = replay.");
}

static void print_help()
{
    Serial.println();
    Serial.println("Comandos (Enter):");
    Serial.println("  W           repetir wake (F8 + log RX, 2s+3s)");
    Serial.println("  R           replay 21->23->E2... (tras [wake] LISTO)");
    Serial.println("  X           abortar replay");
    Serial.println("  S           detener replay (fin anticipado)");
    Serial.println("  ? / h       ayuda");
    Serial.println("  B9600       baud bus Nobana");
    Serial.println("  V           verbose HEX (replay)");
    Serial.println("  I           estado / telemetria");
    Serial.println();
    Serial.printf("Wake: %s | Replay: %s | Baud: %lu\n", handshake_phase_name(s_hs),
                  replay_phase_name(s_replay), (unsigned long)s_bus_baud);
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
        if (s_replay != REPLAY_OFF) {
            Serial.println("[err] No cambiar baud durante replay (X primero)");
            return;
        }
        uart_begin_bus((uint32_t)baud);
        line_reset(&s_rx);
        handshake_begin();
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

    if (c0 == 'R' || c0 == 'r') {
        if (line.length() > 1) {
            Serial.println("[err] R = replay completo (sin argumentos)");
            return;
        }
        replay_start();
        return;
    }

    if (c0 == 'X' || c0 == 'x') {
        if (s_replay == REPLAY_OFF) {
            Serial.println("[replay] ya OFF");
            return;
        }
        replay_abort("usuario_X");
        return;
    }

    if (c0 == 'S' || c0 == 's') {
        if (line.length() > 1) {
            Serial.println("[err] S = stop replay (sin argumentos)");
            return;
        }
        if (s_replay == REPLAY_OFF) {
            Serial.println("[replay] ya OFF");
            return;
        }
        replay_abort("usuario_S");
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
    delay(300);

    memset(&s_rx, 0, sizeof(s_rx));
    memset(&s_telem, 0, sizeof(s_telem));
    memset(&s_last_logged, 0, sizeof(s_last_logged));

    uart_begin_bus(NOBANA_BAUD_DEFAULT);
    print_banner();
    print_help();
    handshake_begin();
}

void loop()
{
    const uint32_t now = millis();
    poll_usb_serial();
    handshake_tick(now);
    replay_tick(now);
    delay(2);
}
