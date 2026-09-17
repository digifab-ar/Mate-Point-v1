#include "vl6180x_sensor.h"

#include "config.h"
#include "i2c.h"

#include <Arduino.h>

/* VL6180 / VL6180X — registros 16 bit (AN4545 + Pololu). Sin Wire. */
enum Reg : uint16_t {
    IDENTIFICATION__MODEL_ID = 0x000,
    SYSTEM__MODE_GPIO1 = 0x011,
    SYSTEM__INTERRUPT_CONFIG_GPIO = 0x014,
    SYSTEM__INTERRUPT_CLEAR = 0x015,
    SYSTEM__FRESH_OUT_OF_RESET = 0x016,
    SYSRANGE__START = 0x018,
    SYSRANGE__INTERMEASUREMENT_PERIOD = 0x01B,
    SYSRANGE__MAX_CONVERGENCE_TIME = 0x01C,
    SYSRANGE__VHV_RECALIBRATE = 0x02E,
    SYSRANGE__VHV_REPEAT_RATE = 0x031,
    SYSALS__INTERMEASUREMENT_PERIOD = 0x03E,
    SYSALS__ANALOGUE_GAIN = 0x03F,
    SYSALS__INTEGRATION_PERIOD = 0x040,
    RESULT__RANGE_STATUS = 0x04D,
    RESULT__INTERRUPT_STATUS_GPIO = 0x04F,
    RESULT__RANGE_VAL = 0x062,
    READOUT__AVERAGING_SAMPLE_PERIOD = 0x10A,
    INTERLEAVED_MODE__ENABLE = 0x2A3,
};

enum RangeErr : uint8_t {
    RANGE_ERR_NONE = 0,
    RANGE_ERR_ECEFAIL = 6,
    RANGE_ERR_NOCONVERGE = 7,
    RANGE_ERR_RANGEIGNORE = 8,
    RANGE_ERR_SNR = 11,
    RANGE_ERR_RAWUFLOW = 12,
    RANGE_ERR_RAWOFLOW = 13,
    RANGE_ERR_RANGEUFLOW = 14,
    RANGE_ERR_RANGEOFLOW = 15,
};

static constexpr uint8_t MODEL_ID_VL6180 = 0xB4;
static constexpr uint8_t MODEL_ID_VL53L0X = 0xEE;
static constexpr uint8_t RANGE_TIMEOUT_MM = 255;
static constexpr uint16_t IO_TIMEOUT_MS = 200;
static constexpr int I2C_XFER_MS = 200;
static constexpr uint8_t ID_RETRY_COUNT = 10;
static constexpr uint32_t ID_RETRY_MS = 20;

static i2c_master_dev_handle_t s_dev = nullptr;
static bool s_initialized = false;
static bool s_i2c_registered = false;
static const char *s_init_status = "sin init";

static bool write_index(uint16_t reg)
{
    const uint8_t addr[2] = {
        (uint8_t)(reg >> 8),
        (uint8_t)reg,
    };
    return i2c_master_transmit(s_dev, addr, sizeof(addr), I2C_XFER_MS) == ESP_OK;
}

static bool write_reg(uint16_t reg, uint8_t value)
{
    const uint8_t data[3] = {
        (uint8_t)(reg >> 8),
        (uint8_t)reg,
        value,
    };
    return i2c_master_transmit(s_dev, data, sizeof(data), I2C_XFER_MS) == ESP_OK;
}

static bool write_reg16(uint16_t reg, uint16_t value)
{
    const uint8_t data[4] = {
        (uint8_t)(reg >> 8),
        (uint8_t)reg,
        (uint8_t)(value >> 8),
        (uint8_t)value,
    };
    return i2c_master_transmit(s_dev, data, sizeof(data), I2C_XFER_MS) == ESP_OK;
}

static bool read_reg(uint16_t reg, uint8_t *value)
{
    if (!value) {
        return false;
    }
    /* Pololu/Wire: STOP after 16-bit index, then a new START+read.
     * Repeated-start (transmit_receive) fails on some VL6180 modules. */
    if (write_index(reg) && i2c_master_receive(s_dev, value, 1, I2C_XFER_MS) == ESP_OK) {
        return true;
    }
    const uint8_t addr[2] = {
        (uint8_t)(reg >> 8),
        (uint8_t)reg,
    };
    return i2c_master_transmit_receive(s_dev, addr, sizeof(addr), value, 1, I2C_XFER_MS)
           == ESP_OK;
}

static bool read_l0x_model_id(uint8_t *value)
{
    const uint8_t reg = 0xC0;
    return i2c_master_transmit_receive(s_dev, &reg, 1, value, 1, I2C_XFER_MS) == ESP_OK;
}

static uint8_t read_reg8(uint16_t reg)
{
    uint8_t value = 0;
    (void)read_reg(reg, &value);
    return value;
}

static bool timeout_expired(uint32_t start_ms)
{
    return IO_TIMEOUT_MS > 0 && (uint16_t)(millis() - start_ms) > IO_TIMEOUT_MS;
}

/* AN4545 §9 SR03 — private registers (mandatory after power-up). */
static bool load_sr03_private()
{
    static const uint16_t k_regs[] = {
        0x0207, 0x0208, 0x0096, 0x0097, 0x00E3, 0x00E4, 0x00E5, 0x00E6, 0x00E7,
        0x00F5, 0x00D9, 0x00DB, 0x00DC, 0x00DD, 0x009F, 0x00A3, 0x00B7, 0x00BB,
        0x00B2, 0x00CA, 0x0198, 0x01B0, 0x01AD, 0x00FF, 0x0100, 0x0199, 0x01A6,
        0x01AC, 0x01A7, 0x0030,
    };
    static const uint8_t k_vals[] = {
        0x01, 0x01, 0x00, 0xFD, 0x00, 0x04, 0x02, 0x01, 0x03,
        0x02, 0x05, 0xCE, 0x03, 0xF8, 0x00, 0x3C, 0x00, 0x3C,
        0x09, 0x09, 0x01, 0x17, 0x00, 0x05, 0x05, 0x05, 0x1B,
        0x3E, 0x1F, 0x00,
    };
    static_assert(sizeof(k_regs) / sizeof(k_regs[0]) == sizeof(k_vals) / sizeof(k_vals[0]),
                  "SR03 table size");

    for (uint8_t i = 0; i < (uint8_t)sizeof(k_vals); i++) {
        if (!write_reg(k_regs[i], k_vals[i])) {
            return false;
        }
    }
    return true;
}

/* AN4545 recommended public + Pololu configureDefault (escala 1×, sin ALS). */
static bool configure_default()
{
    if (!write_reg(SYSTEM__MODE_GPIO1, 0x10)) {
        return false;
    }
    if (!write_reg(READOUT__AVERAGING_SAMPLE_PERIOD, 0x30)) {
        return false;
    }
    if (!write_reg(SYSALS__ANALOGUE_GAIN, 0x46)) {
        return false;
    }
    if (!write_reg(SYSRANGE__VHV_REPEAT_RATE, 0xFF)) {
        return false;
    }
    if (!write_reg16(SYSALS__INTEGRATION_PERIOD, 0x0063)) {
        return false;
    }
    if (!write_reg(SYSRANGE__VHV_RECALIBRATE, 0x01)) {
        return false;
    }
    if (!write_reg(SYSRANGE__INTERMEASUREMENT_PERIOD, 0x09)) {
        return false;
    }
    if (!write_reg(SYSALS__INTERMEASUREMENT_PERIOD, 0x31)) {
        return false;
    }
    if (!write_reg(SYSTEM__INTERRUPT_CONFIG_GPIO, 0x24)) {
        return false;
    }
    if (!write_reg(SYSRANGE__MAX_CONVERGENCE_TIME, 0x31)) {
        return false;
    }
    if (!write_reg(INTERLEAVED_MODE__ENABLE, 0x00)) {
        return false;
    }
    return true;
}

static bool sensor_init_sequence()
{
    uint8_t fresh = 0;
    if (!read_reg(SYSTEM__FRESH_OUT_OF_RESET, &fresh)) {
        return false;
    }
    if (fresh == 0x01) {
        if (!load_sr03_private()) {
            return false;
        }
        if (!write_reg(SYSTEM__FRESH_OUT_OF_RESET, 0x00)) {
            return false;
        }
    }
    return configure_default();
}

static bool wait_device_ready()
{
    uint32_t start = millis();
    uint8_t status = 0;
    while (true) {
        if (!read_reg(RESULT__RANGE_STATUS, &status)) {
            return false;
        }
        if (status & 0x01) {
            return true;
        }
        if (timeout_expired(start)) {
            return false;
        }
    }
}

static bool wait_range_ready()
{
    uint32_t start = millis();
    uint8_t irq = 0;
    while (true) {
        if (!read_reg(RESULT__INTERRUPT_STATUS_GPIO, &irq)) {
            return false;
        }
        if (irq & 0x04) {
            return true;
        }
        if (timeout_expired(start)) {
            return false;
        }
    }
}

typedef struct {
    bool i2c_ok;
    bool timed_out;
    uint8_t range_mm;
    uint8_t range_err;
} RangeRaw;

static RangeRaw read_range_single()
{
    RangeRaw out = {};
    out.i2c_ok = true;

    if (!wait_device_ready()) {
        out.i2c_ok = false;
        out.timed_out = true;
        out.range_mm = RANGE_TIMEOUT_MM;
        return out;
    }

    if (!write_reg(SYSRANGE__START, 0x01)) {
        out.i2c_ok = false;
        out.range_mm = RANGE_TIMEOUT_MM;
        return out;
    }

    if (!wait_range_ready()) {
        out.timed_out = true;
        out.range_mm = RANGE_TIMEOUT_MM;
        (void)write_reg(SYSTEM__INTERRUPT_CLEAR, 0x07);
        return out;
    }

    uint8_t range = 0;
    uint8_t status = 0;
    if (!read_reg(RESULT__RANGE_VAL, &range) || !read_reg(RESULT__RANGE_STATUS, &status)) {
        out.i2c_ok = false;
        out.range_mm = RANGE_TIMEOUT_MM;
        (void)write_reg(SYSTEM__INTERRUPT_CLEAR, 0x07);
        return out;
    }

    (void)write_reg(SYSTEM__INTERRUPT_CLEAR, 0x07);
    out.range_mm = range;
    out.range_err = (uint8_t)(status >> 4);
    return out;
}

static Vl6180xStatus range_to_status(const RangeRaw *raw)
{
    if (!raw->i2c_ok && !raw->timed_out) {
        return VL6180X_STATUS_I2C_FAIL;
    }
    if (raw->timed_out || raw->range_mm == RANGE_TIMEOUT_MM) {
        return VL6180X_STATUS_TIMEOUT;
    }

    switch (raw->range_err) {
    case RANGE_ERR_NONE:
    case RANGE_ERR_RAWUFLOW:
    case RANGE_ERR_RANGEUFLOW:
        return VL6180X_STATUS_OK;
    case RANGE_ERR_ECEFAIL:
    case RANGE_ERR_NOCONVERGE:
    case RANGE_ERR_RANGEIGNORE:
    case RANGE_ERR_SNR:
    case RANGE_ERR_RAWOFLOW:
    case RANGE_ERR_RANGEOFLOW:
        return VL6180X_STATUS_OUT_OF_RANGE;
    default:
        if (raw->range_err >= 1 && raw->range_err <= 5) {
            return VL6180X_STATUS_READ_FAIL;
        }
        return VL6180X_STATUS_OK;
    }
}

static uint16_t apply_range_offset(uint16_t raw_mm)
{
    if (raw_mm <= TERMO_OFFSET_MM) {
        return 0;
    }
    return (uint16_t)(raw_mm - TERMO_OFFSET_MM);
}

bool vl6180x_init()
{
    if (!DEV_I2C_Register_Device(VL6180X_I2C_ADDR, &s_dev)) {
        s_initialized = false;
        s_i2c_registered = false;
        s_init_status = "I2C 0x29 fallo";
        return false;
    }

    s_i2c_registered = true;
    delay(20);

    uint8_t model_id = 0;
    bool id_ok = false;
    for (uint8_t i = 0; i < ID_RETRY_COUNT; i++) {
        if (read_reg(IDENTIFICATION__MODEL_ID, &model_id)) {
            id_ok = true;
            break;
        }
        delay(ID_RETRY_MS);
    }

    if (!id_ok) {
        s_initialized = false;
        uint8_t l0x_id = 0;
        if (read_l0x_model_id(&l0x_id) && l0x_id == MODEL_ID_VL53L0X) {
            s_init_status = "ID 0xEE (L0X)";
        } else {
            s_init_status = "ID no responde";
        }
        return false;
    }
    if (model_id == MODEL_ID_VL53L0X) {
        s_initialized = false;
        s_init_status = "ID 0xEE (L0X)";
        return false;
    }
    if (model_id != MODEL_ID_VL6180) {
        s_initialized = false;
        s_init_status = "ID != 0xB4";
        return false;
    }

    s_initialized = sensor_init_sequence();
    s_init_status = s_initialized ? "init OK" : "secuencia fallo";
    return s_initialized;
}

bool vl6180x_is_ready()
{
    return s_initialized && s_dev != nullptr;
}

const char *vl6180x_init_status_text()
{
    if (!s_i2c_registered) {
        return "I2C no registrado";
    }
    return s_init_status;
}

const char *vl6180x_status_text(Vl6180xStatus status)
{
    switch (status) {
    case VL6180X_STATUS_OK:
        return "lectura OK";
    case VL6180X_STATUS_NOT_INIT:
        return "sin init";
    case VL6180X_STATUS_I2C_FAIL:
        return "I2C fallo";
    case VL6180X_STATUS_TIMEOUT:
        return "timeout";
    case VL6180X_STATUS_OUT_OF_RANGE:
        return "fuera de rango";
    case VL6180X_STATUS_READ_FAIL:
        return "lectura fallo";
    default:
        return "?";
    }
}

void vl6180x_sample(Vl6180xSample *out)
{
    if (!out) {
        return;
    }

    out->distance_mm = 0;
    out->distance_corrected_mm = 0;
    out->termo_present = false;
    out->init_ok = vl6180x_is_ready();

    if (!s_i2c_registered) {
        out->status = VL6180X_STATUS_I2C_FAIL;
        return;
    }

    if (!s_initialized || s_dev == nullptr) {
        out->status = VL6180X_STATUS_NOT_INIT;
        return;
    }

    const RangeRaw raw = read_range_single();
    out->status = range_to_status(&raw);
    out->distance_mm = raw.range_mm;

    if (out->status == VL6180X_STATUS_OK) {
        uint16_t mm = raw.range_mm;
        if (raw.range_err == RANGE_ERR_RAWUFLOW || raw.range_err == RANGE_ERR_RANGEUFLOW) {
            mm = 0;
            out->distance_mm = 0;
        }
        out->distance_corrected_mm = apply_range_offset(mm);
        out->termo_present = out->distance_corrected_mm < TERMO_PRESENT_MAX_MM;
    }
}

bool vl6180x_read_mm(uint16_t *out_mm)
{
    Vl6180xSample sample;
    vl6180x_sample(&sample);
    if (sample.status != VL6180X_STATUS_OK || !out_mm) {
        return false;
    }

    *out_mm = sample.distance_corrected_mm;
    return true;
}

bool vl6180x_termo_present()
{
    Vl6180xSample sample;
    vl6180x_sample(&sample);
    return sample.status == VL6180X_STATUS_OK && sample.termo_present;
}
