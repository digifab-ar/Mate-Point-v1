#include "wifi_portal.h"

#include <Arduino.h>

#include "config.h"
#include "wifi_config.h"
#include "wifi_portal_html.h"
#include <ArduinoJson.h>
#include <DNSServer.h>
#include <WebServer.h>
#include <WiFi.h>

static WebServer server(80);
static DNSServer dns_server;
static bool portal_active = false;
static bool restart_pending = false;
static char ap_ssid[32] = "";
static uint32_t portal_started_ms = 0;

static bool try_sta_connect(const char *ssid, const char *pass)
{
    WiFi.begin(ssid, pass ? pass : "");
    const uint32_t deadline = millis() + WIFI_PORTAL_STA_TIMEOUT_MS;
    while (WiFi.status() != WL_CONNECTED && (int32_t)(deadline - millis()) > 0) {
        delay(100);
    }
    return WiFi.status() == WL_CONNECTED;
}

static void send_portal_page()
{
    server.send(200, "text/html", PORTAL_HTML);
}

static void handle_root()
{
    send_portal_page();
}

static void handle_scan()
{
    const int count = WiFi.scanNetworks(false, true);
    StaticJsonDocument<2048> doc;
    JsonArray arr = doc.to<JsonArray>();

    for (int i = 0; i < count; i++) {
        JsonObject item = arr.createNestedObject();
        item["ssid"] = WiFi.SSID(i);
        item["rssi"] = WiFi.RSSI(i);
        item["secure"] = WiFi.encryptionType(i) != WIFI_AUTH_OPEN;
    }

    String body;
    serializeJson(doc, body);
    server.send(200, "application/json", body);
}

static void handle_connect()
{
    if (!server.hasArg("ssid")) {
        server.send(400, "application/json", "{\"ok\":false,\"message\":\"Falta la red.\"}");
        return;
    }

    const String ssid = server.arg("ssid");
    const String pass = server.hasArg("password") ? server.arg("password") : "";

    if (ssid.length() == 0) {
        server.send(400, "application/json", "{\"ok\":false,\"message\":\"Elegi una red.\"}");
        return;
    }

    WiFi.disconnect(false, true);
    delay(100);

    if (!try_sta_connect(ssid.c_str(), pass.c_str())) {
        WiFi.disconnect(false, true);
        WiFi.mode(WIFI_AP_STA);
        WiFi.softAP(ap_ssid);
        server.send(200, "application/json",
                     "{\"ok\":false,\"message\":\"No se pudo conectar. Revisa la contrasena y que sea red 2.4 GHz.\"}");
        return;
    }

    if (!wifi_config_save(ssid.c_str(), pass.c_str())) {
        server.send(500, "application/json",
                     "{\"ok\":false,\"message\":\"Error al guardar la configuracion.\"}");
        return;
    }

    restart_pending = true;
    server.send(200, "application/json",
                 "{\"ok\":true,\"message\":\"Listo. El Mate Point se reiniciara.\"}");
}

static void handle_not_found()
{
    if (portal_active) {
        server.sendHeader("Location", "http://192.168.4.1/", true);
        server.send(302, "text/plain", "");
        return;
    }
    server.send(404, "text/plain", "Not found");
}

void wifi_portal_ap_ssid(char *buf, size_t len)
{
    if (!buf || len == 0) {
        return;
    }

    uint8_t mac[6] = {0};
    WiFi.macAddress(mac);
    snprintf(buf, len, "MatePoint-%02X%02X", mac[4], mac[5]);
}

const char *wifi_portal_ap_name()
{
    return ap_ssid;
}

bool wifi_portal_start()
{
    if (portal_active) {
        return true;
    }

    wifi_portal_ap_ssid(ap_ssid, sizeof(ap_ssid));
    WiFi.mode(WIFI_AP_STA);
    WiFi.softAP(ap_ssid);
    WiFi.softAPConfig(IPAddress(192, 168, 4, 1), IPAddress(192, 168, 4, 1), IPAddress(255, 255, 255, 0));

    dns_server.start(53, "*", WiFi.softAPIP());

    server.on("/", HTTP_GET, handle_root);
    server.on("/scan", HTTP_GET, handle_scan);
    server.on("/connect", HTTP_POST, handle_connect);
    server.onNotFound(handle_not_found);
    server.begin();

    portal_active = true;
    restart_pending = false;
    portal_started_ms = millis();
    WiFi.scanNetworks(true, true);
    return true;
}

void wifi_portal_stop()
{
    if (!portal_active) {
        return;
    }

    server.stop();
    dns_server.stop();
    WiFi.softAPdisconnect(true);
    portal_active = false;
    restart_pending = false;
    WiFi.mode(WIFI_STA);
    WiFi.disconnect(false, true);
}

bool wifi_portal_is_active()
{
    return portal_active;
}

PortalEvent wifi_portal_loop()
{
    if (!portal_active) {
        return PORTAL_EVENT_NONE;
    }

    dns_server.processNextRequest();
    server.handleClient();

    if (restart_pending) {
        delay(500);
        return PORTAL_EVENT_RESTART_PENDING;
    }

    if (millis() - portal_started_ms >= WIFI_PROVISIONING_TIMEOUT_MS) {
        wifi_portal_stop();
    }

    return PORTAL_EVENT_NONE;
}
