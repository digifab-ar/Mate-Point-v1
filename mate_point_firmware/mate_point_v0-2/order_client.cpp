#include "order_client.h"

#include "config.h"

#include <Arduino.h>
#include <ArduinoJson.h>
#include <HTTPClient.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>

static bool post_json(const char *path, const char *body, int *http_code_out, String *response_out)
{
    WiFiClientSecure client;
    client.setInsecure();

    HTTPClient http;
    const String url = String("https://") + SERVER_HOST + path;
    if (!http.begin(client, url)) {
        Serial.println("[http] begin failed");
        return false;
    }

    http.addHeader("Content-Type", "application/json");
    http.setTimeout(15000);

    const int code = http.POST(body ? body : "{}");
    const String response = http.getString();
    http.end();

    if (http_code_out) {
        *http_code_out = code;
    }
    if (response_out) {
        *response_out = response;
    }

    return code > 0;
}

bool order_create(char *order_id, size_t order_id_len,
                  char *external_ref, size_t external_ref_len)
{
    if (order_id && order_id_len) {
        order_id[0] = '\0';
    }
    if (external_ref && external_ref_len) {
        external_ref[0] = '\0';
    }

    StaticJsonDocument<128> req;
    req["device_id"] = DEVICE_ID;
    char body[128];
    serializeJson(req, body, sizeof(body));

    int code = 0;
    String response;
    if (!post_json("/orders/create", body, &code, &response)) {
        return false;
    }

    Serial.printf("[http] create code=%d body=%s\n", code, response.c_str());
    if (code != 201) {
        return false;
    }

    StaticJsonDocument<512> doc;
    if (deserializeJson(doc, response)) {
        return false;
    }

    const char *id = doc["order_id"] | "";
    const char *ext = doc["external_reference"] | "";
    if (order_id && order_id_len) {
        strncpy(order_id, id, order_id_len - 1);
        order_id[order_id_len - 1] = '\0';
    }
    if (external_ref && external_ref_len) {
        strncpy(external_ref, ext, external_ref_len - 1);
        external_ref[external_ref_len - 1] = '\0';
    }

    return id[0] != '\0';
}

bool order_cancel(const char *order_id)
{
    if (!order_id || !order_id[0]) {
        return false;
    }

    StaticJsonDocument<128> req;
    req["order_id"] = order_id;
    char body[128];
    serializeJson(req, body, sizeof(body));

    int code = 0;
    String response;
    if (!post_json("/orders/cancel", body, &code, &response)) {
        return false;
    }

    Serial.printf("[http] cancel code=%d body=%s\n", code, response.c_str());
    return code == 200;
}
