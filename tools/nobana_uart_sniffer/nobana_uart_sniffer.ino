/*
 * Nobana UART sniffer — ESP32 NodeMCU 38p + TXS0108E
 *
 * Hardware (relevamiento, ARMOR conectado en paralelo):
 *   Cable Tx (Nobana TX) → TXS0108E B1→A1 → GPIO25  Serial2  [NOB→ARM]
 *   Cable Rx (ARMOR TX)  → TXS0108E B2→A2 → GPIO17  Serial1  [ARM→NOB]
 *   Cable G              → GND común
 *   Cable 5V             → TXS0108E VCCB (no alimentar ESP32 desde aquí si usás USB)
 *   ESP32 3.3V           → TXS0108E VCCA + OE
 *
 * Monitor Serie (USB): 115200
 * Bus Nobana: NOBANA_BAUD_DEFAULT (default 9600 8N1)
 *
 * Comandos por Monitor Serie:
 *   ? o h     ayuda
 *   S         escanear baudrates (ambas direcciones; pulsar botones ARMOR ayuda)
 *   T         test 15 s: cuenta bytes NOB->ARM y ARM->NOB (sin cambiar baud)
 *   B9600     fijar baudrate (ej. B19200)
 *   M texto   marca manual (ej. al pulsar HOT en ARMOR: M HOT Coffee)
 *   V         alternar verbose (todas las FRAME vs solo cambios)
 */

#include <Arduino.h>

// --- Configuración (editar aquí) ---
#define PIN_NOB_RX 25
#define PIN_ARM_RX 17
#define PIN_SERIAL2_DUMMY_TX 4
#define PIN_SERIAL1_DUMMY_TX 5

#define SERIAL_DEBUG_BAUD 115200
#define NOBANA_BAUD_DEFAULT 9600
#define FRAME_GAP_MS 35
#define MAX_FRAME_BYTES 128
#define BAUD_SCAN_DWELL_MS 4000
#define TRAFFIC_TEST_MS 15000
#define DEDUP_SEMANTIC_DEFAULT 1

static const uint32_t BAUD_SCAN_LIST[] = {
    4800, 9600, 19200, 38400, 57600, 115200,
};
static const size_t BAUD_SCAN_COUNT =
    sizeof(BAUD_SCAN_LIST) / sizeof(BAUD_SCAN_LIST[0]);
// --- Fin configuración ---

enum Direction : uint8_t {
    DIR_NOB_ARM = 0,
    DIR_ARM_NOB = 1,
};

struct LineState {
    uint8_t buf[MAX_FRAME_BYTES];
    size_t len;
    uint32_t last_byte_ms;
    bool active;
};

struct LastFrame {
    uint8_t buf[MAX_FRAME_BYTES];
    size_t len;
    bool valid;
};

static LineState s_nob;
static LineState s_arm;
static LastFrame s_last_nob;
static LastFrame s_last_arm;
static uint32_t s_bus_baud = NOBANA_BAUD_DEFAULT;
static bool s_scan_mode = false;
static bool s_verbose_frames = !DEDUP_SEMANTIC_DEFAULT;

static void print_banner();
static void print_help();
static void uart_begin_bus(uint32_t baud);
static void line_reset(LineState *st);
static void line_push(LineState *st, Direction dir, uint8_t b, uint32_t now);
static void line_flush(LineState *st, Direction dir, uint32_t now);
static void poll_uart(HardwareSerial &port, LineState *st, Direction dir, uint32_t now);
static void handle_debug_serial();
static void run_baud_scan();
static void run_traffic_test();
static void last_frame_reset(LastFrame *lf);
static bool frame_semantic_equal(const LastFrame *prev, const uint8_t *data, size_t len);
static void last_frame_store(LastFrame *lf, const uint8_t *data, size_t len);
static bool should_print_frame(Direction dir, const uint8_t *data, size_t len);
static void note_frame_printed(Direction dir, const uint8_t *data, size_t len);
static void print_hex_line(const char *tag, Direction dir, const uint8_t *data, size_t len,
                           uint32_t t0);

static const char *dir_label(Direction d)
{
    return (d == DIR_NOB_ARM) ? "NOB->ARM" : "ARM->NOB";
}

void setup()
{
    Serial.begin(SERIAL_DEBUG_BAUD);
    delay(300);

    uart_begin_bus(s_bus_baud);
    line_reset(&s_nob);
    line_reset(&s_arm);
    last_frame_reset(&s_last_nob);
    last_frame_reset(&s_last_arm);

    print_banner();
    print_help();
}

void loop()
{
    const uint32_t now = millis();

    if (!s_scan_mode) {
        poll_uart(Serial2, &s_nob, DIR_NOB_ARM, now);
        poll_uart(Serial1, &s_arm, DIR_ARM_NOB, now);

        if (s_nob.active && (now - s_nob.last_byte_ms) >= FRAME_GAP_MS) {
            line_flush(&s_nob, DIR_NOB_ARM, now);
        }
        if (s_arm.active && (now - s_arm.last_byte_ms) >= FRAME_GAP_MS) {
            line_flush(&s_arm, DIR_ARM_NOB, now);
        }
    }

    handle_debug_serial();
}

static void uart_begin_bus(uint32_t baud)
{
    Serial2.end();
    Serial1.end();
    delay(10);

    // RX-only hacia el bus; TX en pin dummy sin cable
    Serial2.begin(baud, SERIAL_8N1, PIN_NOB_RX, PIN_SERIAL2_DUMMY_TX);
    Serial1.begin(baud, SERIAL_8N1, PIN_ARM_RX, PIN_SERIAL1_DUMMY_TX);

    Serial.printf("[cfg] bus UART %lu 8N1 | NOB->ARM GPIO%d (Serial2) | ARM->NOB GPIO%d (Serial1)\n",
                  (unsigned long)baud, PIN_NOB_RX, PIN_ARM_RX);
    last_frame_reset(&s_last_nob);
    last_frame_reset(&s_last_arm);
}

static void last_frame_reset(LastFrame *lf)
{
    lf->len = 0;
    lf->valid = false;
}

// Igualdad semántica: ignora byte 1 (seq) y último byte (checksum).
static bool frame_semantic_equal(const LastFrame *prev, const uint8_t *data, size_t len)
{
    if (!prev->valid || prev->len != len || len < 3) {
        return false;
    }
    if (data[0] != prev->buf[0]) {
        return false;
    }
    for (size_t i = 2; i + 1 < len; i++) {
        if (data[i] != prev->buf[i]) {
            return false;
        }
    }
    return true;
}

static void last_frame_store(LastFrame *lf, const uint8_t *data, size_t len)
{
    if (len > MAX_FRAME_BYTES) {
        len = MAX_FRAME_BYTES;
    }
    memcpy(lf->buf, data, len);
    lf->len = len;
    lf->valid = true;
}

static bool should_print_frame(Direction dir, const uint8_t *data, size_t len)
{
    if (s_verbose_frames) {
        return true;
    }
    const LastFrame *prev = (dir == DIR_NOB_ARM) ? &s_last_nob : &s_last_arm;
    return !frame_semantic_equal(prev, data, len);
}

static void note_frame_printed(Direction dir, const uint8_t *data, size_t len)
{
    LastFrame *prev = (dir == DIR_NOB_ARM) ? &s_last_nob : &s_last_arm;
    last_frame_store(prev, data, len);
}

static void line_reset(LineState *st)
{
    st->len = 0;
    st->last_byte_ms = 0;
    st->active = false;
}

static void line_push(LineState *st, Direction dir, uint8_t b, uint32_t now)
{
    if (st->len == 0) {
        st->active = true;
    }

    if (st->len < MAX_FRAME_BYTES) {
        st->buf[st->len++] = b;
    } else {
        // Trama demasiado larga: volcar y seguir
        print_hex_line("WARN trunc", dir, st->buf, st->len, now);
        st->len = 0;
        st->buf[st->len++] = b;
    }
    st->last_byte_ms = now;
}

static void line_flush(LineState *st, Direction dir, uint32_t now)
{
    if (!st->active || st->len == 0) {
        line_reset(st);
        return;
    }
    if (should_print_frame(dir, st->buf, st->len)) {
        print_hex_line("FRAME", dir, st->buf, st->len, now);
        note_frame_printed(dir, st->buf, st->len);
    }
    line_reset(st);
}

static void poll_uart(HardwareSerial &port, LineState *st, Direction dir, uint32_t now)
{
    while (port.available() > 0) {
        const int raw = port.read();
        if (raw < 0) {
            break;
        }
        line_push(st, dir, (uint8_t)raw, now);
    }
}

static bool is_mostly_printable(const uint8_t *data, size_t len)
{
    if (len == 0) {
        return false;
    }
    size_t ok = 0;
    for (size_t i = 0; i < len; i++) {
        const uint8_t c = data[i];
        if (c == '\r' || c == '\n' || c == '\t' || (c >= 0x20 && c < 0x7F)) {
            ok++;
        }
    }
    return (ok * 100U) / len >= 60U;
}

static void print_hex_line(const char *tag, Direction dir, const uint8_t *data, size_t len,
                           uint32_t t0)
{
    Serial.printf("[%lu] %s %s len=%u HEX:",
                  (unsigned long)t0, tag, dir_label(dir), (unsigned)len);

    for (size_t i = 0; i < len; i++) {
        Serial.printf(" %02X", data[i]);
    }

    if (is_mostly_printable(data, len)) {
        Serial.print(" | ASCII:\"");
        for (size_t i = 0; i < len; i++) {
            const uint8_t c = data[i];
            if (c == '\r') {
                Serial.print("\\r");
            } else if (c == '\n') {
                Serial.print("\\n");
            } else if (c >= 0x20 && c < 0x7F) {
                Serial.print((char)c);
            } else {
                Serial.print('.');
            }
        }
        Serial.print('"');
    }

    Serial.println();
}

static void print_banner()
{
    Serial.println();
    Serial.println("=== Nobana UART sniffer (ESP32 NodeMCU 38p) ===");
    Serial.println("NO es el firmware Waveshare Mate Point.");
    Serial.printf("Debug USB: %d | Frame gap: %d ms\n", SERIAL_DEBUG_BAUD, FRAME_GAP_MS);
    Serial.println(s_verbose_frames
                       ? "FRAME: verbose (todas las tramas)"
                       : "FRAME: dedup ON (solo cambios; ignora seq/checksum)");
}

static void print_help()
{
    Serial.println();
    Serial.println("Comandos (terminar con Enter):");
    Serial.println("  ? / h       esta ayuda");
    Serial.println("  S           escanear baudrates (NOB->ARM + ARM->NOB)");
    Serial.println("  T           test 15 s de bytes por direccion (pulsar botones ARMOR)");
    Serial.println("  B<baud>     fijar baudrate, ej: B9600");
    Serial.println("  V           alternar verbose / solo cambios en FRAME");
    Serial.println();
    Serial.println("  FRAME dedup: imprime solo si cmd/datos cambian (no repite seq).");
    Serial.println("  Botones ARMOR = trafico ARM->NOB (GPIO17).");
    Serial.println("  M <texto>   marca manual, ej: M HOT Coffee 85C");
    Serial.println();
    Serial.printf("Baud actual del bus: %lu\n", (unsigned long)s_bus_baud);
}

static void run_baud_scan()
{
    s_scan_mode = true;
    line_reset(&s_nob);
    line_reset(&s_arm);

    Serial.println();
    Serial.println("[scan] Escaneo en AMBAS direcciones.");
    Serial.println("[scan] Durante cada baud: esperar 2 s, pulsar un boton ARMOR (HOT), esperar.");
    Serial.println();

    uint32_t best_score = 0;
    uint32_t best_baud = s_bus_baud;

    for (size_t i = 0; i < BAUD_SCAN_COUNT; i++) {
        const uint32_t baud = BAUD_SCAN_LIST[i];
        Serial.printf("[scan] --- probando %lu bps (%u s) ---\n", (unsigned long)baud,
                      (unsigned)(BAUD_SCAN_DWELL_MS / 1000U));

        uart_begin_bus(baud);
        line_reset(&s_nob);
        line_reset(&s_arm);

        const uint32_t t_end = millis() + BAUD_SCAN_DWELL_MS;
        uint32_t nob_bytes = 0;
        uint32_t arm_bytes = 0;
        uint32_t frame_count = 0;

        while ((int32_t)(millis() - t_end) < 0) {
            const uint32_t now = millis();
            while (Serial2.available() > 0) {
                const uint8_t b = (uint8_t)Serial2.read();
                nob_bytes++;
                line_push(&s_nob, DIR_NOB_ARM, b, now);
            }
            while (Serial1.available() > 0) {
                const uint8_t b = (uint8_t)Serial1.read();
                arm_bytes++;
                line_push(&s_arm, DIR_ARM_NOB, b, now);
            }
            if (s_nob.active && (now - s_nob.last_byte_ms) >= FRAME_GAP_MS) {
                print_hex_line("scan", DIR_NOB_ARM, s_nob.buf, s_nob.len, now);
                frame_count++;
                line_reset(&s_nob);
            }
            if (s_arm.active && (now - s_arm.last_byte_ms) >= FRAME_GAP_MS) {
                print_hex_line("scan", DIR_ARM_NOB, s_arm.buf, s_arm.len, now);
                frame_count++;
                line_reset(&s_arm);
            }
            handle_debug_serial();
            delay(1);
        }

        const uint32_t score = nob_bytes + arm_bytes + frame_count * 10U;
        Serial.printf("[scan] resumen %lu bps: NOB->ARM bytes=%lu | ARM->NOB bytes=%lu | frames=%lu | score=%lu\n",
                      (unsigned long)baud, (unsigned long)nob_bytes, (unsigned long)arm_bytes,
                      (unsigned long)frame_count, (unsigned long)score);

        if (score > best_score) {
            best_score = score;
            best_baud = baud;
        }
    }

    Serial.printf("\n[scan] Mejor candidato: %lu bps (score=%lu). Enviar B%lu para fijar.\n",
                  (unsigned long)best_baud, (unsigned long)best_score,
                  (unsigned long)best_baud);
    if (best_score == 0) {
        Serial.println("[scan] score=0: revisar cable Tx/Rx -> B1/B2, OE, GND, o probar T con B9600.");
    }

    s_bus_baud = best_baud;
    uart_begin_bus(s_bus_baud);
    s_scan_mode = false;
    line_reset(&s_nob);
    line_reset(&s_arm);
}

static void run_traffic_test()
{
    s_scan_mode = true;
    line_reset(&s_nob);
    line_reset(&s_arm);

    Serial.println();
    Serial.printf("[test] %u s @ %lu bps — NO tocar cables. Pulsar botones ARMOR.\n",
                  (unsigned)(TRAFFIC_TEST_MS / 1000U), (unsigned long)s_bus_baud);

    const uint32_t t_end = millis() + TRAFFIC_TEST_MS;
    uint32_t nob_total = 0;
    uint32_t arm_total = 0;
    uint32_t last_report = millis();

    while ((int32_t)(millis() - t_end) < 0) {
        const uint32_t now = millis();
        uint32_t nob_tick = 0;
        uint32_t arm_tick = 0;

        while (Serial2.available() > 0) {
            const uint8_t b = (uint8_t)Serial2.read();
            nob_total++;
            nob_tick++;
            line_push(&s_nob, DIR_NOB_ARM, b, now);
        }
        while (Serial1.available() > 0) {
            const uint8_t b = (uint8_t)Serial1.read();
            arm_total++;
            arm_tick++;
            line_push(&s_arm, DIR_ARM_NOB, b, now);
        }
        if (s_nob.active && (now - s_nob.last_byte_ms) >= FRAME_GAP_MS) {
            print_hex_line("test", DIR_NOB_ARM, s_nob.buf, s_nob.len, now);
            line_reset(&s_nob);
        }
        if (s_arm.active && (now - s_arm.last_byte_ms) >= FRAME_GAP_MS) {
            print_hex_line("test", DIR_ARM_NOB, s_arm.buf, s_arm.len, now);
            line_reset(&s_arm);
        }

        if (now - last_report >= 1000) {
            Serial.printf("[test] +1s  NOB->ARM=%lu  ARM->NOB=%lu  (total %lu / %lu)\n",
                          (unsigned long)nob_tick, (unsigned long)arm_tick,
                          (unsigned long)nob_total, (unsigned long)arm_total);
            last_report = now;
        }

        handle_debug_serial();
        delay(1);
    }

    Serial.printf("[test] FIN total NOB->ARM=%lu  ARM->NOB=%lu\n",
                  (unsigned long)nob_total, (unsigned long)arm_total);
    if (arm_total == 0) {
        Serial.println("[test] ARM->NOB=0: cable Rx del Nobana -> B2->A2->GPIO17, o baud mal, o B1/B2 invertidos.");
    }
    if (nob_total == 0 && arm_total == 0) {
        Serial.println("[test] Sin bytes: revisar OE, GND, VCCB 5V, taps en Tx y Rx del conector.");
    }

    s_scan_mode = false;
    line_reset(&s_nob);
    line_reset(&s_arm);
}

static void handle_debug_serial()
{
    if (!Serial.available()) {
        return;
    }

    String line = Serial.readStringUntil('\n');
    line.trim();
    if (line.length() == 0) {
        return;
    }

    const char c0 = line.charAt(0);

    if (c0 == '?' || c0 == 'h' || c0 == 'H') {
        print_help();
        return;
    }

    if (c0 == 'S' || c0 == 's') {
        run_baud_scan();
        return;
    }

    if (c0 == 'T' || c0 == 't') {
        run_traffic_test();
        return;
    }

    if (c0 == 'B' || c0 == 'b') {
        const unsigned long baud = line.substring(1).toInt();
        if (baud < 300 || baud > 2000000) {
            Serial.println("[err] Baud invalido. Ejemplo: B9600");
            return;
        }
        s_bus_baud = (uint32_t)baud;
        uart_begin_bus(s_bus_baud);
        line_reset(&s_nob);
        line_reset(&s_arm);
        Serial.printf("[cfg] Baud fijado a %lu\n", baud);
        return;
    }

    if (c0 == 'V' || c0 == 'v') {
        s_verbose_frames = !s_verbose_frames;
        last_frame_reset(&s_last_nob);
        last_frame_reset(&s_last_arm);
        Serial.println(s_verbose_frames
                           ? "[cfg] FRAME verbose ON (todas las tramas)"
                           : "[cfg] FRAME dedup ON (solo cambios)");
        return;
    }

    if (c0 == 'M' || c0 == 'm') {
        String note = line.substring(1);
        note.trim();
        if (note.length() == 0) {
            Serial.println("[err] Uso: M texto (ej. M HOT pressed)");
            return;
        }
        Serial.printf("[%lu] MARK | %s\n", (unsigned long)millis(), note.c_str());
        last_frame_reset(&s_last_nob);
        last_frame_reset(&s_last_arm);
        return;
    }

    Serial.println("[err] Comando desconocido. Enviar ? para ayuda.");
}
