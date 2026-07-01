#pragma once

#define DEVICE_ID "MATEPOINT001"

#define WIFI_SSID "DuoCasa"
#define WIFI_PASSWORD "01431931344"

#define MQTT_HOST "broker.hivemq.com"
#define MQTT_PORT 1883
#define MQTT_CLIENT_ID "mate-" DEVICE_ID "-esp32-v051"

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
#define QR_TIMEOUT_MS 120000
#define POST_PAY_TIMEOUT_MS 120000
#define DISPENSE_UI_REFRESH_MS 1000

#define UI_LITERS_FILL_SEC           120
#define UI_PRODUCT_LITERS_DEFAULT    1.0f
#define UI_ERROR_PAGO_MS             5000
#define UI_PRODUCT_DESC_PLACEHOLDER    "Recarga de 1 litro"
#define UI_PRODUCT_PRICE_PLACEHOLDER   "$500"

#define VL53L0X_I2C_ADDR 0x29
#define TERMO_OFFSET_MM 85
#define TERMO_PRESENT_MAX_MM 15
#define TERMO_POLL_MS 300
#define TERMO_DEBOUNCE_COUNT 2

#define WIFI_RECONNECT_MS 5000
#define MQTT_RECONNECT_MS 5000
