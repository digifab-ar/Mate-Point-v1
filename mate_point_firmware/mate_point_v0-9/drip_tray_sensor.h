#pragma once

#include <stdbool.h>
#include <stdint.h>

typedef struct {
    bool init_ok;
    bool tray_full;
} DripTraySample;

typedef enum {
    DRIP_TRAY_TRANSITION_NONE,
    DRIP_TRAY_TRANSITION_BECAME_FULL,
    DRIP_TRAY_TRANSITION_BECAME_OK,
} DripTrayTransition;

bool drip_tray_init();
DripTrayTransition drip_tray_poll();
bool drip_tray_is_full();
void drip_tray_sample(DripTraySample *out);
