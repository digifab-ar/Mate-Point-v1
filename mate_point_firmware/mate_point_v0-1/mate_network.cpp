#include "mate_network.h"

#include "config.h"
#include "dispense_sim.h"

#include <Arduino.h>
#include <ArduinoJson.h>
#include <WiFi.h>

#define MQTT_MAX_PACKET_SIZE 512
#include <PubSubClient.h>

static WiFiClient wifi_client;
static PubSubClient mqtt(wifi_client);

static uint32_t last_wifi_attempt_ms = 0;
static uint32_t last_mqtt_attempt_ms = 0;
static uint32_t last_status_ms = 0;

static char pending_order_id[64] = "";
static uint32_t pending_duration_ms = 0;
static volatile bool pending_command = false;

static void publish_status_payload()
{
    StaticJsonDocument<256> doc;
    doc["device_id"] = DEVICE_ID;
    doc["state"] = dispense_mqtt_state();
    doc["ts"] = (uint64_t)millis();
    doc["uptime_ms"] = millis();
    doc["wifi_rssi"] = WiFi.RSSI();
    doc["mqtt_connected"] = mqtt.connected();

    const char *order_id = dispense_active_order_id();
    if (order_id) {
        doc["order_id"] = order_id;
    }

    char payload[256];
    serializeJson(doc, payload, sizeof(payload));
    mqtt.publish(TOPIC_STATUS, payload);
}

static void on_mqtt_message(char *topic, byte *payload, unsigned int length)
{
    if (length >= 512) {
        return;
    }

    char message[512];
    memcpy(message, payload, length);
    message[length] = '\0';

    StaticJsonDocument<384> doc;
    if (deserializeJson(doc, message)) {
        return;
    }

    const char *cmd = doc["cmd"] | "";
    if (strcmp(cmd, "dispense") != 0) {
        return;
    }

    const char *order_id = doc["order_id"] | "";
    uint32_t duration_ms = doc["duration_ms"] | 120000U;

    strncpy(pending_order_id, order_id, sizeof(pending_order_id) - 1);
    pending_duration_ms = duration_ms;
    pending_command = true;

    Serial.printf("[mqtt] command order_id=%s duration_ms=%lu\n", pending_order_id, pending_duration_ms);
}

static void connect_wifi()
{
    if (WiFi.status() == WL_CONNECTED) {
        return;
    }

    Serial.printf("[wifi] connecting to %s\n", WIFI_SSID);
    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    last_wifi_attempt_ms = millis();
}

static void connect_mqtt()
{
    if (!mate_network_wifi_ok() || mqtt.connected()) {
        return;
    }

    mqtt.setServer(MQTT_HOST, MQTT_PORT);
    mqtt.setCallback(on_mqtt_message);
    mqtt.setBufferSize(MQTT_MAX_PACKET_SIZE);

    Serial.printf("[mqtt] connecting to %s:%d\n", MQTT_HOST, MQTT_PORT);
    if (mqtt.connect(MQTT_CLIENT_ID)) {
        mqtt.subscribe(TOPIC_COMMAND);
        Serial.println("[mqtt] connected");
        publish_status_payload();
    } else {
        Serial.printf("[mqtt] failed rc=%d\n", mqtt.state());
    }
    last_mqtt_attempt_ms = millis();
}

void mate_network_init()
{
    connect_wifi();
}

void mate_network_loop()
{
    if (WiFi.status() != WL_CONNECTED) {
        if (millis() - last_wifi_attempt_ms >= WIFI_RECONNECT_MS) {
            connect_wifi();
        }
    } else if (!mqtt.connected()) {
        if (millis() - last_mqtt_attempt_ms >= MQTT_RECONNECT_MS) {
            connect_mqtt();
        }
    } else {
        mqtt.loop();
    }

    if (pending_command) {
        pending_command = false;
        if (dispense_on_command(pending_order_id, pending_duration_ms)) {
            publish_status_payload();
        }
    }

    DispenseEvent event = dispense_tick();
    if (event == DISPENSE_EVENT_IDLE) {
        publish_status_payload();
    }

    if (mate_network_mqtt_ok() && millis() - last_status_ms >= STATUS_INTERVAL_MS) {
        publish_status_payload();
        last_status_ms = millis();
    }
}

bool mate_network_wifi_ok()
{
    return WiFi.status() == WL_CONNECTED;
}

bool mate_network_mqtt_ok()
{
    return mate_network_wifi_ok() && mqtt.connected();
}
