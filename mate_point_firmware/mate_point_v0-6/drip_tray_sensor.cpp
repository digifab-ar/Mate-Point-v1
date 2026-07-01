#include "drip_tray_sensor.h"

#include <Arduino.h>

#include "config.h"

static bool s_init_ok = false;
static bool s_tray_full = false;
static bool s_has_sample = false;
static uint32_t s_last_poll_ms = 0;

static bool read_tray_full_raw()
{
    const int level = digitalRead(DRIP_TRAY_GPIO);
#if DRIP_TRAY_FULL_LEVEL == 0
    return level == LOW;
#else
    return level == HIGH;
#endif
}

bool drip_tray_init()
{
    pinMode(DRIP_TRAY_GPIO, INPUT);
    s_init_ok = true;
    s_tray_full = read_tray_full_raw();
    s_has_sample = true;
    s_last_poll_ms = millis();
    return s_init_ok;
}

void drip_tray_sample(DripTraySample *out)
{
    if (!out) {
        return;
    }
    out->init_ok = s_init_ok;
    out->tray_full = s_tray_full;
}

bool drip_tray_is_full()
{
    return s_init_ok && s_tray_full;
}

DripTrayTransition drip_tray_poll()
{
    if (!s_init_ok) {
        return DRIP_TRAY_TRANSITION_NONE;
    }

    const uint32_t now = millis();
    if (s_last_poll_ms != 0 && (int32_t)(now - s_last_poll_ms) < (int32_t)DRIP_TRAY_POLL_MS) {
        return DRIP_TRAY_TRANSITION_NONE;
    }

    s_last_poll_ms = now;

    const bool full = read_tray_full_raw();
    DripTrayTransition transition = DRIP_TRAY_TRANSITION_NONE;

    if (!s_has_sample) {
        s_has_sample = true;
        s_tray_full = full;
        return DRIP_TRAY_TRANSITION_NONE;
    }

    if (full != s_tray_full) {
        if (full) {
            transition = DRIP_TRAY_TRANSITION_BECAME_FULL;
        } else {
            transition = DRIP_TRAY_TRANSITION_BECAME_OK;
        }
        s_tray_full = full;
    }

    return transition;
}
