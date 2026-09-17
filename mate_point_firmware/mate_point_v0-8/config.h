#pragma once

/* Identidad de fábrica — una por ESP32 (flota v0-8):
 *   MATEPOINT001 ↔ MATEPOINT001POS001
 *   MATEPOINT002 ↔ MATEPOINT002POS001
 *   MATEPOINT003 ↔ MATEPOINT003POS001
 *   MATEPOINT004 ↔ MATEPOINT004POS001
 * Antes de flashear cada unidad: este DEVICE_ID + qr_static_img.c de ESA caja.
 */
#define DEVICE_ID "MATEPOINT001"

#define MQTT_HOST "broker.hivemq.com"
#define MQTT_PORT 1883
#define MQTT_CLIENT_ID "mate-" DEVICE_ID "-esp32-v080"

#define WATER_TANK_EMPTY_B2        0x10
#define WATER_TANK_EMPTY_B7        0x01
#define WATER_TANK_DEBOUNCE_FRAMES 2
#define WATER_TANK_BOOT_WAIT_MS    1000

#define DRIP_TRAY_GPIO       6
#define DRIP_TRAY_POLL_MS    5000
/** LOW (0) = reed cerrado → bandeja llena. */
#define DRIP_TRAY_FULL_LEVEL 0

#define TOPIC_COMMAND "mate/" DEVICE_ID "/command"
#define TOPIC_STATUS "mate/" DEVICE_ID "/status"

#define SERVER_HOST "mate-point-v1-production.up.railway.app"
#define SERVER_PORT 443

#define STATUS_INTERVAL_MS 30000
#define TERMINADO_TO_LISTO_MS 3000
#define PAUSE_DECISION_TIMEOUT_MS 20000
/** Cooldown UART tras pausa (Parar): ciclo corto + ventana para Continuar. */
#define PAUSE_COOLDOWN_MS            5000
#define QR_TIMEOUT_MS 120000
#define POST_PAY_TIMEOUT_MS 120000
#define DISPENSE_UI_REFRESH_MS 1000

#define UI_MS_PER_LITER              120000u
#define UI_TEMP_DISPLAY_MIN_C        80
#define UI_ERROR_PAGO_MS             5000
#define UI_PRODUCT_DESC_PLACEHOLDER    "Recarga de 1 litro"
#define UI_PRODUCT_PRICE_PLACEHOLDER   "$500"

#define VL6180X_I2C_ADDR 0x29
#define TERMO_OFFSET_MM 0          /* recalibrar en banco — no copiar 85 mm del L0X */
#define TERMO_PRESENT_MAX_MM 15
#define TERMO_POLL_MS 300
#define TERMO_DEBOUNCE_COUNT 2
#define UI_DEBUG_TERMO 0           /* 1 = raw/corr en Coloca termo (banco) */

#define WIFI_RECONNECT_MS 5000
#define MQTT_RECONNECT_MS 5000
#define WIFI_BOOT_ATTEMPT_MS 15000
#define WIFI_BOOT_MAX_ATTEMPTS 5
#define WIFI_PORTAL_STA_TIMEOUT_MS 20000
#define WIFI_PROVISIONING_TIMEOUT_MS 600000
