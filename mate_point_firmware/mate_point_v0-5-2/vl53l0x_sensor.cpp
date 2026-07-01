#include "vl53l0x_sensor.h"

#include "config.h"
#include "i2c.h"

#include <Arduino.h>
#include <string.h>

// Register map (Pololu VL53L0X / ST API)
enum Reg : uint8_t {
    SYSRANGE_START = 0x00,
    SYSTEM_SEQUENCE_CONFIG = 0x01,
    SYSTEM_INTERRUPT_CONFIG_GPIO = 0x0A,
    SYSTEM_INTERRUPT_CLEAR = 0x0B,
    RESULT_INTERRUPT_STATUS = 0x13,
    RESULT_RANGE_STATUS = 0x14,
    FINAL_RANGE_CONFIG_MIN_COUNT_RATE_RTN_LIMIT = 0x44,
    MSRC_CONFIG_CONTROL = 0x60,
    MSRC_CONFIG_TIMEOUT_MACROP = 0x46,
    PRE_RANGE_CONFIG_TIMEOUT_MACROP_HI = 0x51,
    FINAL_RANGE_CONFIG_TIMEOUT_MACROP_HI = 0x71,
    PRE_RANGE_CONFIG_VCSEL_PERIOD = 0x50,
    FINAL_RANGE_CONFIG_VCSEL_PERIOD = 0x70,
    GLOBAL_CONFIG_SPAD_ENABLES_REF_0 = 0xB0,
    GLOBAL_CONFIG_REF_EN_START_SELECT = 0xB6,
    DYNAMIC_SPAD_REF_EN_START_OFFSET = 0x4F,
    DYNAMIC_SPAD_NUM_REQUESTED_REF_SPAD = 0x4E,
    GPIO_HV_MUX_ACTIVE_HIGH = 0x84,
    IDENTIFICATION_MODEL_ID = 0xC0,
    VHV_CONFIG_PAD_SCL_SDA__EXTSUP_HV = 0x89,
};

static i2c_master_dev_handle_t s_dev = nullptr;
static bool s_initialized = false;
static bool s_i2c_registered = false;
static const char *s_init_status = "sin init";
static uint8_t s_stop_variable = 0;
static uint16_t s_io_timeout_ms = 500;

static bool write_reg(uint8_t reg, uint8_t value)
{
    const uint8_t data[2] = {reg, value};
    return i2c_master_transmit(s_dev, data, sizeof(data), 100) == ESP_OK;
}

static bool write_reg16(uint8_t reg, uint16_t value)
{
    const uint8_t data[3] = {reg, (uint8_t)(value >> 8), (uint8_t)value};
    return i2c_master_transmit(s_dev, data, sizeof(data), 100) == ESP_OK;
}

static bool write_multi(uint8_t reg, const uint8_t *src, uint8_t count)
{
    uint8_t buf[8];
    if (count > sizeof(buf) - 1) {
        return false;
    }
    buf[0] = reg;
    memcpy(buf + 1, src, count);
    return i2c_master_transmit(s_dev, buf, count + 1, 100) == ESP_OK;
}

static bool read_reg(uint8_t reg, uint8_t *value)
{
    return i2c_master_transmit_receive(s_dev, &reg, 1, value, 1, 100) == ESP_OK;
}

static uint8_t read_reg8(uint8_t reg)
{
    uint8_t value = 0;
    (void)read_reg(reg, &value);
    return value;
}

static uint16_t read_reg16(uint8_t reg)
{
    uint8_t data[2] = {0};
    if (i2c_master_transmit_receive(s_dev, &reg, 1, data, 2, 100) != ESP_OK) {
        return 0;
    }
    return ((uint16_t)data[0] << 8) | data[1];
}

static void read_multi(uint8_t reg, uint8_t *dst, uint8_t count)
{
    (void)i2c_master_transmit_receive(s_dev, &reg, 1, dst, count, 100);
}

static bool timeout_expired(uint32_t start_ms)
{
    return s_io_timeout_ms > 0 && (uint16_t)(millis() - start_ms) > s_io_timeout_ms;
}

static bool set_signal_rate_limit(float limit_mcps)
{
    if (limit_mcps < 0.0f || limit_mcps > 511.99f) {
        return false;
    }
    return write_reg16(FINAL_RANGE_CONFIG_MIN_COUNT_RATE_RTN_LIMIT,
                       (uint16_t)(limit_mcps * (1 << 7)));
}

static bool get_spad_info(uint8_t *count, bool *type_is_aperture)
{
    write_reg(0x80, 0x01);
    write_reg(0xFF, 0x01);
    write_reg(0x00, 0x00);
    write_reg(0xFF, 0x06);
    write_reg(0x83, read_reg8(0x83) | 0x04);
    write_reg(0xFF, 0x07);
    write_reg(0x81, 0x01);
    write_reg(0x80, 0x01);
    write_reg(0x94, 0x6B);
    write_reg(0x83, 0x00);

    uint32_t start = millis();
    while (read_reg8(0x83) == 0x00) {
        if (timeout_expired(start)) {
            return false;
        }
    }

    write_reg(0x83, 0x01);
    const uint8_t tmp = read_reg8(0x92);
    *count = tmp & 0x7F;
    *type_is_aperture = (tmp >> 7) & 0x01;

    write_reg(0x81, 0x00);
    write_reg(0xFF, 0x06);
    write_reg(0x83, read_reg8(0x83) & ~0x04);
    write_reg(0xFF, 0x01);
    write_reg(0x00, 0x01);
    write_reg(0xFF, 0x00);
    write_reg(0x80, 0x00);
    return true;
}

static bool perform_single_ref_calibration(uint8_t vhv_init_byte)
{
    write_reg(SYSRANGE_START, 0x01 | vhv_init_byte);

    uint32_t start = millis();
    while ((read_reg8(RESULT_INTERRUPT_STATUS) & 0x07) == 0) {
        if (timeout_expired(start)) {
            return false;
        }
    }

    write_reg(SYSTEM_INTERRUPT_CLEAR, 0x01);
    write_reg(SYSRANGE_START, 0x00);
    return true;
}

static bool sensor_data_init()
{
    write_reg(VHV_CONFIG_PAD_SCL_SDA__EXTSUP_HV, read_reg8(VHV_CONFIG_PAD_SCL_SDA__EXTSUP_HV) | 0x01);
    write_reg(0x88, 0x00);
    write_reg(0x80, 0x01);
    write_reg(0xFF, 0x01);
    write_reg(0x00, 0x00);
    s_stop_variable = read_reg8(0x91);
    write_reg(0x00, 0x01);
    write_reg(0xFF, 0x00);
    write_reg(0x80, 0x00);

    write_reg(MSRC_CONFIG_CONTROL, read_reg8(MSRC_CONFIG_CONTROL) | 0x12);
    set_signal_rate_limit(0.25f);
    write_reg(SYSTEM_SEQUENCE_CONFIG, 0xFF);
    return true;
}

static bool sensor_static_init()
{
    uint8_t spad_count = 0;
    bool spad_type_is_aperture = false;
    if (!get_spad_info(&spad_count, &spad_type_is_aperture)) {
        return false;
    }

    uint8_t ref_spad_map[6];
    read_multi(GLOBAL_CONFIG_SPAD_ENABLES_REF_0, ref_spad_map, 6);

    write_reg(0xFF, 0x01);
    write_reg(DYNAMIC_SPAD_REF_EN_START_OFFSET, 0x00);
    write_reg(DYNAMIC_SPAD_NUM_REQUESTED_REF_SPAD, 0x2C);
    write_reg(0xFF, 0x00);
    write_reg(GLOBAL_CONFIG_REF_EN_START_SELECT, 0xB4);

    const uint8_t first_spad = spad_type_is_aperture ? 12 : 0;
    uint8_t spads_enabled = 0;
    for (uint8_t i = 0; i < 48; i++) {
        if (i < first_spad || spads_enabled == spad_count) {
            ref_spad_map[i / 8] &= (uint8_t)~(1 << (i % 8));
        } else if ((ref_spad_map[i / 8] >> (i % 8)) & 0x01) {
            spads_enabled++;
        }
    }
    write_multi(GLOBAL_CONFIG_SPAD_ENABLES_REF_0, ref_spad_map, 6);

    // Default tuning settings (Pololu VL53L0X.cpp)
    write_reg(0xFF, 0x01);
    write_reg(0x00, 0x00);
    write_reg(0xFF, 0x00);
    write_reg(0x09, 0x00);
    write_reg(0x10, 0x00);
    write_reg(0x11, 0x00);
    write_reg(0x24, 0x01);
    write_reg(0x25, 0xFF);
    write_reg(0x75, 0x00);
    write_reg(0xFF, 0x01);
    write_reg(0x4E, 0x2C);
    write_reg(0x48, 0x00);
    write_reg(0x30, 0x20);
    write_reg(0xFF, 0x00);
    write_reg(0x30, 0x09);
    write_reg(0x54, 0x00);
    write_reg(0x31, 0x04);
    write_reg(0x32, 0x03);
    write_reg(0x40, 0x83);
    write_reg(0x46, 0x25);
    write_reg(0x60, 0x00);
    write_reg(0x27, 0x00);
    write_reg(0x50, 0x06);
    write_reg(0x51, 0x00);
    write_reg(0x52, 0x96);
    write_reg(0x56, 0x08);
    write_reg(0x57, 0x30);
    write_reg(0x61, 0x00);
    write_reg(0x62, 0x00);
    write_reg(0x64, 0x00);
    write_reg(0x65, 0x00);
    write_reg(0x66, 0xA0);
    write_reg(0xFF, 0x01);
    write_reg(0x22, 0x32);
    write_reg(0x47, 0x14);
    write_reg(0x49, 0xFF);
    write_reg(0x4A, 0x00);
    write_reg(0xFF, 0x00);
    write_reg(0x7A, 0x0A);
    write_reg(0x7B, 0x00);
    write_reg(0x78, 0x21);
    write_reg(0xFF, 0x01);
    write_reg(0x23, 0x34);
    write_reg(0x42, 0x00);
    write_reg(0x44, 0xFF);
    write_reg(0x45, 0x26);
    write_reg(0x46, 0x05);
    write_reg(0x40, 0x40);
    write_reg(0x0E, 0x06);
    write_reg(0x20, 0x1A);
    write_reg(0x43, 0x40);
    write_reg(0xFF, 0x00);
    write_reg(0x34, 0x03);
    write_reg(0x35, 0x44);
    write_reg(0xFF, 0x01);
    write_reg(0x31, 0x04);
    write_reg(0x4B, 0x09);
    write_reg(0x4C, 0x05);
    write_reg(0x4D, 0x04);
    write_reg(0xFF, 0x00);
    write_reg(0x44, 0x00);
    write_reg(0x45, 0x20);
    write_reg(0x47, 0x08);
    write_reg(0x48, 0x28);
    write_reg(0x67, 0x00);
    write_reg(0x70, 0x04);
    write_reg(0x71, 0x01);
    write_reg(0x72, 0xFE);
    write_reg(0x76, 0x00);
    write_reg(0x77, 0x00);
    write_reg(0xFF, 0x01);
    write_reg(0x0D, 0x01);
    write_reg(0xFF, 0x00);
    write_reg(0x80, 0x01);
    write_reg(0x01, 0xF8);
    write_reg(0xFF, 0x01);
    write_reg(0x8E, 0x01);
    write_reg(0x00, 0x01);
    write_reg(0xFF, 0x00);
    write_reg(0x80, 0x00);

    write_reg(SYSTEM_INTERRUPT_CONFIG_GPIO, 0x04);
    write_reg(GPIO_HV_MUX_ACTIVE_HIGH, read_reg8(GPIO_HV_MUX_ACTIVE_HIGH) & ~0x10);
    write_reg(SYSTEM_INTERRUPT_CLEAR, 0x01);
    write_reg(SYSTEM_SEQUENCE_CONFIG, 0xE8);
    return true;
}

static bool sensor_ref_calibration()
{
    write_reg(SYSTEM_SEQUENCE_CONFIG, 0x01);
    if (!perform_single_ref_calibration(0x40)) {
        return false;
    }
    write_reg(SYSTEM_SEQUENCE_CONFIG, 0x02);
    if (!perform_single_ref_calibration(0x00)) {
        return false;
    }
    write_reg(SYSTEM_SEQUENCE_CONFIG, 0xE8);
    return true;
}

static bool sensor_init_sequence()
{
    if (read_reg8(IDENTIFICATION_MODEL_ID) != 0xEE) {
        return false;
    }
    if (!sensor_data_init()) {
        return false;
    }
    if (!sensor_static_init()) {
        return false;
    }
    return sensor_ref_calibration();
}

static uint16_t read_range_single_mm()
{
    write_reg(0x80, 0x01);
    write_reg(0xFF, 0x01);
    write_reg(0x00, 0x00);
    write_reg(0x91, s_stop_variable);
    write_reg(0x00, 0x01);
    write_reg(0xFF, 0x00);
    write_reg(0x80, 0x00);
    write_reg(SYSRANGE_START, 0x01);

    uint32_t start = millis();
    while (read_reg8(SYSRANGE_START) & 0x01) {
        if (timeout_expired(start)) {
            return 65535;
        }
    }

    start = millis();
    while ((read_reg8(RESULT_INTERRUPT_STATUS) & 0x07) == 0) {
        if (timeout_expired(start)) {
            return 65535;
        }
    }

    const uint16_t range = read_reg16(RESULT_RANGE_STATUS + 10);
    write_reg(SYSTEM_INTERRUPT_CLEAR, 0x01);
    return range;
}

bool vl53l0x_init()
{
    if (!DEV_I2C_Register_Device(VL53L0X_I2C_ADDR, &s_dev)) {
        s_initialized = false;
        s_i2c_registered = false;
        s_init_status = "I2C 0x29 fallo";
        return false;
    }

    s_i2c_registered = true;

    if (read_reg8(IDENTIFICATION_MODEL_ID) != 0xEE) {
        s_initialized = false;
        s_init_status = "ID != 0xEE";
        return false;
    }

    s_initialized = sensor_init_sequence();
    s_init_status = s_initialized ? "init OK" : "secuencia fallo";
    return s_initialized;
}

bool vl53l0x_is_ready()
{
    return s_initialized && s_dev != nullptr;
}

const char *vl53l0x_init_status_text()
{
    if (!s_i2c_registered) {
        return "I2C no registrado";
    }
    return s_init_status;
}

static Vl53l0xStatus range_to_status(uint16_t range)
{
    if (range == 65535) {
        return VL53L0X_STATUS_TIMEOUT;
    }
    if (range == 8190) {
        return VL53L0X_STATUS_OUT_OF_RANGE;
    }
    return VL53L0X_STATUS_OK;
}

static uint16_t apply_range_offset(uint16_t raw_mm)
{
    if (raw_mm <= TERMO_OFFSET_MM) {
        return 0;
    }
    return (uint16_t)(raw_mm - TERMO_OFFSET_MM);
}

const char *vl53l0x_status_text(Vl53l0xStatus status)
{
    switch (status) {
    case VL53L0X_STATUS_OK:
        return "lectura OK";
    case VL53L0X_STATUS_NOT_INIT:
        return "sin init";
    case VL53L0X_STATUS_I2C_FAIL:
        return "I2C fallo";
    case VL53L0X_STATUS_TIMEOUT:
        return "timeout";
    case VL53L0X_STATUS_OUT_OF_RANGE:
        return "fuera de rango";
    case VL53L0X_STATUS_READ_FAIL:
        return "lectura fallo";
    default:
        return "?";
    }
}

void vl53l0x_sample(Vl53l0xSample *out)
{
    if (!out) {
        return;
    }

    out->distance_mm = 0;
    out->distance_corrected_mm = 0;
    out->termo_present = false;
    out->init_ok = vl53l0x_is_ready();

    if (!s_i2c_registered) {
        out->status = VL53L0X_STATUS_I2C_FAIL;
        return;
    }

    if (!s_initialized || s_dev == nullptr) {
        out->status = VL53L0X_STATUS_NOT_INIT;
        return;
    }

    const uint16_t range = read_range_single_mm();
    out->status = range_to_status(range);
    out->distance_mm = range;

    if (out->status == VL53L0X_STATUS_OK) {
        out->distance_corrected_mm = apply_range_offset(range);
        out->termo_present = out->distance_corrected_mm < TERMO_PRESENT_MAX_MM;
    }
}

bool vl53l0x_read_mm(uint16_t *out_mm)
{
    Vl53l0xSample sample;
    vl53l0x_sample(&sample);
    if (sample.status != VL53L0X_STATUS_OK || !out_mm) {
        return false;
    }

    *out_mm = sample.distance_corrected_mm;
    return true;
}

bool vl53l0x_termo_present()
{
    Vl53l0xSample sample;
    vl53l0x_sample(&sample);
    return sample.status == VL53L0X_STATUS_OK && sample.termo_present;
}
