#pragma once

#include <stddef.h>

bool wifi_config_has_credentials();
bool wifi_config_load(char *ssid, size_t ssid_len, char *pass, size_t pass_len);
bool wifi_config_save(const char *ssid, const char *pass);
