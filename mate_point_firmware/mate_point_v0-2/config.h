#pragma once

#define DEVICE_ID "MATEPOINT001"

#define WIFI_SSID "DuoCasa"
#define WIFI_PASSWORD "01431931344"

#define MQTT_HOST "broker.hivemq.com"
#define MQTT_PORT 1883
#define MQTT_CLIENT_ID "mate-" DEVICE_ID "-esp32-v02"

#define TOPIC_COMMAND "mate/" DEVICE_ID "/command"
#define TOPIC_STATUS "mate/" DEVICE_ID "/status"

#define SERVER_HOST "mate-point-v1-production.up.railway.app"
#define SERVER_PORT 443

#define STATUS_INTERVAL_MS 30000
#define TERMINADO_TO_LISTO_MS 3000
#define QR_TIMEOUT_MS 120000

#define WIFI_RECONNECT_MS 5000
#define MQTT_RECONNECT_MS 5000
