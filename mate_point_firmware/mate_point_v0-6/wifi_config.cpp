#include "wifi_config.h"

#include <Preferences.h>
#include <string.h>

static const char *NVS_NS = "mate_cfg";
static const char *KEY_SSID = "wifi_ssid";
static const char *KEY_PASS = "wifi_pass";

bool wifi_config_has_credentials()
{
    Preferences prefs;
    prefs.begin(NVS_NS, true);
    const String ssid = prefs.getString(KEY_SSID, "");
    prefs.end();
    return ssid.length() > 0;
}

bool wifi_config_load(char *ssid, size_t ssid_len, char *pass, size_t pass_len)
{
    if (!ssid || ssid_len == 0 || !pass || pass_len == 0) {
        return false;
    }

    Preferences prefs;
    prefs.begin(NVS_NS, true);
    const String stored_ssid = prefs.getString(KEY_SSID, "");
    const String stored_pass = prefs.getString(KEY_PASS, "");
    prefs.end();

    if (stored_ssid.length() == 0) {
        return false;
    }

    strncpy(ssid, stored_ssid.c_str(), ssid_len - 1);
    ssid[ssid_len - 1] = '\0';
    strncpy(pass, stored_pass.c_str(), pass_len - 1);
    pass[pass_len - 1] = '\0';
    return true;
}

bool wifi_config_save(const char *ssid, const char *pass)
{
    if (!ssid || ssid[0] == '\0') {
        return false;
    }

    Preferences prefs;
    prefs.begin(NVS_NS, false);
    prefs.putString(KEY_SSID, ssid);
    prefs.putString(KEY_PASS, pass ? pass : "");
    prefs.end();
    return true;
}
