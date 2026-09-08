#include "mate_network.h"

#include "app_state.h"
#include "config.h"
#include "dispense_controller.h"
#include "wifi_config.h"
#include "wifi_portal.h"

#include <Arduino.h>
#include <ArduinoJson.h>
#include <ESP.h>
#include <WiFi.h>

#define MQTT_MAX_PACKET_SIZE 512
#include <PubSubClient.h>

static WiFiClient wifi_client;
static PubSubClient mqtt(wifi_client);

static char stored_ssid[33] = "";
static char stored_pass[65] = "";
static bool has_stored_credentials = false;

static uint32_t last_wifi_attempt_ms = 0;
static uint32_t last_mqtt_attempt_ms = 0;
static uint32_t last_status_ms = 0;
static uint8_t boot_wifi_attempts = 0;
static bool boot_wifi_exhausted = false;

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
    if (mate_network_wifi_ok()) {
        doc["wifi_rssi"] = WiFi.RSSI();
    }
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
    (void)topic;
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

    if (!app_state_can_accept_dispense()) {
        return;
    }

    const char *order_id = doc["order_id"] | "";
    const uint32_t duration_ms = doc["duration_ms"] | 30000U;

    strncpy(pending_order_id, order_id, sizeof(pending_order_id) - 1);
    pending_order_id[sizeof(pending_order_id) - 1] = '\0';
    pending_duration_ms = duration_ms;
    pending_command = true;
}

static void reload_stored_credentials()
{
    has_stored_credentials =
        wifi_config_load(stored_ssid, sizeof(stored_ssid), stored_pass, sizeof(stored_pass));
}

static void connect_wifi_sta()
{
    if (!has_stored_credentials || wifi_portal_is_active()) {
        return;
    }

    if (WiFi.status() == WL_CONNECTED) {
        return;
    }

    WiFi.mode(WIFI_STA);
    WiFi.begin(stored_ssid, stored_pass);
    last_wifi_attempt_ms = millis();
}

static void connect_mqtt()
{
    if (!mate_network_wifi_ok() || mqtt.connected() || wifi_portal_is_active()) {
        return;
    }

    mqtt.setServer(MQTT_HOST, MQTT_PORT);
    mqtt.setCallback(on_mqtt_message);
    mqtt.setBufferSize(MQTT_MAX_PACKET_SIZE);

    if (mqtt.connect(MQTT_CLIENT_ID)) {
        mqtt.subscribe(TOPIC_COMMAND);
        publish_status_payload();
    }
    last_mqtt_attempt_ms = millis();
}

static void service_normal_network()
{
    if (!has_stored_credentials) {
        return;
    }

    if (WiFi.status() != WL_CONNECTED) {
        if (boot_wifi_exhausted) {
            if (millis() - last_wifi_attempt_ms >= WIFI_RECONNECT_MS) {
                connect_wifi_sta();
            }
            return;
        }

        if (millis() - last_wifi_attempt_ms >= WIFI_BOOT_ATTEMPT_MS) {
            if (boot_wifi_attempts + 1 >= WIFI_BOOT_MAX_ATTEMPTS) {
                boot_wifi_exhausted = true;
            } else {
                boot_wifi_attempts++;
                WiFi.disconnect(false, true);
                connect_wifi_sta();
            }
        }
        return;
    }

    boot_wifi_attempts = 0;
    boot_wifi_exhausted = false;

    if (!mqtt.connected()) {
        if (millis() - last_mqtt_attempt_ms >= MQTT_RECONNECT_MS) {
            connect_mqtt();
        }
    } else {
        mqtt.loop();
    }
}

void mate_network_init()
{
    WiFi.mode(WIFI_STA);
    WiFi.setAutoReconnect(false);

    reload_stored_credentials();
    mqtt.setServer(MQTT_HOST, MQTT_PORT);

    if (has_stored_credentials) {
        connect_wifi_sta();
    }
}

void mate_network_loop()
{
    if (wifi_portal_is_active()) {
        const PortalEvent event = wifi_portal_loop();
        if (event == PORTAL_EVENT_RESTART_PENDING) {
            ESP.restart();
        }
        return;
    }

    service_normal_network();

    if (pending_command) {
        pending_command = false;
        app_state_on_dispense_command(pending_order_id, pending_duration_ms);
    }

    const DispenseEvent event = dispense_tick();
    if (event == DISPENSE_EVENT_POST_PAY_TIMEOUT) {
        app_state_on_post_pay_timeout();
    }
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
    if (wifi_portal_is_active()) {
        return false;
    }
    return has_stored_credentials && WiFi.status() == WL_CONNECTED;
}

bool mate_network_mqtt_ok()
{
    return mate_network_wifi_ok() && mqtt.connected();
}

bool mate_network_is_provisioning()
{
    return wifi_portal_is_active();
}

const char *mate_network_ap_ssid()
{
    return wifi_portal_ap_name();
}

void mate_network_start_provisioning()
{
    if (wifi_portal_is_active()) {
        return;
    }

    WiFi.disconnect(false, true);
    mqtt.disconnect();
    wifi_portal_start();
}

void mate_network_stop_provisioning()
{
    wifi_portal_stop();
    reload_stored_credentials();
    boot_wifi_attempts = 0;
    boot_wifi_exhausted = false;
    connect_wifi_sta();
}

void mate_network_publish_status()
{
    if (mate_network_mqtt_ok()) {
        publish_status_payload();
    }
}
