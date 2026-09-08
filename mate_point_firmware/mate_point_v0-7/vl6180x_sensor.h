#pragma once

#include <stdbool.h>
#include <stdint.h>

typedef enum {
    VL6180X_STATUS_OK,
    VL6180X_STATUS_NOT_INIT,
    VL6180X_STATUS_I2C_FAIL,
    VL6180X_STATUS_TIMEOUT,
    VL6180X_STATUS_OUT_OF_RANGE,
    VL6180X_STATUS_READ_FAIL,
} Vl6180xStatus;

typedef struct {
    Vl6180xStatus status;
    uint16_t distance_mm;           /* raw, 0–255 típico */
    uint16_t distance_corrected_mm; /* raw − TERMO_OFFSET_MM */
    bool termo_present;
    bool init_ok;
} Vl6180xSample;

bool vl6180x_init();
bool vl6180x_is_ready();
const char *vl6180x_init_status_text();
bool vl6180x_read_mm(uint16_t *out_mm);
bool vl6180x_termo_present();
void vl6180x_sample(Vl6180xSample *out);
const char *vl6180x_status_text(Vl6180xStatus status);
