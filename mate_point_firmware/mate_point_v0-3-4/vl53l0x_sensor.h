#pragma once

#include <stdbool.h>
#include <stdint.h>

typedef enum {
    VL53L0X_STATUS_OK,
    VL53L0X_STATUS_NOT_INIT,
    VL53L0X_STATUS_I2C_FAIL,
    VL53L0X_STATUS_TIMEOUT,
    VL53L0X_STATUS_OUT_OF_RANGE,
    VL53L0X_STATUS_READ_FAIL,
} Vl53l0xStatus;

typedef struct {
    Vl53l0xStatus status;
    uint16_t distance_mm;
    uint16_t distance_corrected_mm;
    bool termo_present;
    bool init_ok;
} Vl53l0xSample;

bool vl53l0x_init();
bool vl53l0x_is_ready();
const char *vl53l0x_init_status_text();
bool vl53l0x_read_mm(uint16_t *out_mm);
bool vl53l0x_termo_present();
void vl53l0x_sample(Vl53l0xSample *out);
const char *vl53l0x_status_text(Vl53l0xStatus status);
