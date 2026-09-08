#pragma once

#include <stddef.h>

typedef enum {
    PORTAL_EVENT_NONE = 0,
    PORTAL_EVENT_RESTART_PENDING,
} PortalEvent;

void wifi_portal_ap_ssid(char *buf, size_t len);
const char *wifi_portal_ap_name();

bool wifi_portal_start();
void wifi_portal_stop();
bool wifi_portal_is_active();
PortalEvent wifi_portal_loop();
