#pragma once

void mate_network_init();
void mate_network_loop();
void mate_network_publish_status();
bool mate_network_wifi_ok();
bool mate_network_mqtt_ok();
bool mate_network_is_provisioning();
const char *mate_network_ap_ssid();

void mate_network_start_provisioning();
void mate_network_stop_provisioning();
