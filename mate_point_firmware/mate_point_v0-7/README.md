# Mate Point firmware v0-7

**VL6180 (reemplazo VL53L0X)** — fork de [`mate_point_v0-6`](../mate_point_v0-6/) (Wi-Fi NVS + portal + pausa/reanudar + bandeja + agua UART + UI Figma).

| Documento | Uso |
|-----------|-----|
| [`PLAN-MATE-POINT-v0-7.md`](../PLAN-MATE-POINT-v0-7.md) | Plan normativo + criterios de aceptación |
| [`arquitectura-hardware.md`](../../arquitectura-hardware.md) | Bus I2C PH2.0 — VL6180 @ 0x29 |
| [`PLAN-MATE-POINT-v0-6.md`](../PLAN-MATE-POINT-v0-6.md) | Herencia: SoftAP + portal NVS |

**Estado:** **OK hardware** (2026-09-08) — VL6180 @ 0x29; gate termo validado. `TERMO_OFFSET_MM = 0`, umbral 15 mm. Overlay debug off.

---

## Resumen

El sensor de presencia de termo pasa de **VL53L0X** a **VL6180** (módulo típico TOF050C) en el **mismo** conector I2C. El L0X operaba a ~1 cm **dentro de su zona ciega**; el offset 85 mm era un parche de lectura, no la altura de montaje. El 6180 cubre 0–255 mm a escala 1×.

El flujo de producto no cambia: Coloca termo / Iniciar / auto-Parar / pausa / Wi-Fi NVS.

---

## Hardware

| Pin módulo (TOF050C típico) | Waveshare I2C PH2.0 |
|-----------------------------|---------------------|
| VIN | 3V3 |
| GND | GND |
| SDA | SDA (GPIO8) |
| SCL | SCL (GPIO9) |
| SHUT | 3V3 (enable) o NC si el módulo ya lo lleva high |
| INT | NC |

Dirección I2C **`0x29`**. **No** conviven L0X y 6180. Init OK solo si `IDENTIFICATION_MODEL_ID == 0xB4` (si responde `0xEE`, sigue conectado el L0X).

Mapear **SDA/SCL por silkscreen del 6180**: reutilizar el loom del L0X dejó los hilos invertidos (`ID no responde`) hasta cruzarlos (banco 2026-09-08).

GT911 (`0x14`/`0x5D`), CH422G (`0x24`), UART Nobana y GPIO6 bandeja **sin cambio**.

---

## Firmware — sensor

| Archivo | Rol |
|---------|-----|
| `vl6180x_sensor.cpp` / `.h` | Driver `i2c_master` — regs 16 bit, AN4545 SR03, single-shot |
| `config.h` | `VL6180X_I2C_ADDR`, `TERMO_*`, `UI_DEBUG_TERMO` |
| `dispense_controller.cpp` | Poll 300 ms + debounce 2 (misma lógica que v0-6) |

**Prohibido** `Wire` / Adafruit en GPIO 8/9 (táctil GT911).

### `config.h` relevante

```c
#define MQTT_CLIENT_ID "mate-" DEVICE_ID "-esp32-v070"
#define VL6180X_I2C_ADDR 0x29
#define TERMO_OFFSET_MM 0          /* recalibrar en banco — no copiar 85 */
#define TERMO_PRESENT_MAX_MM 15
#define TERMO_POLL_MS 300
#define TERMO_DEBOUNCE_COUNT 2
#define UI_DEBUG_TERMO 0           /* 1 = raw/corr en Coloca termo (banco) */
```

Presencia:

```
dist_corregida = max(0, raw_mm − TERMO_OFFSET_MM)
termo_presente = (status OK) && (dist_corregida < TERMO_PRESENT_MAX_MM)
```

Timeout / I2C fail / out of range = **sin termo**.

---

## Calibración en banco

Debug UI en **Coloca termo** (opt-in): poner `UI_DEBUG_TERMO 1` y recompilar. Líneas `Sensor: …`, `Raw: … | Corr: …`. El build de producto lleva `UI_DEBUG_TERMO 0`.

Termo **pintado o cepillado**, indoor, a ~1 cm.

| Paso | Acción |
|------|--------|
| 1 | Offset 0, umbral 15 (este build) |
| 2 | Con termo: anotar `raw` (esperado ~8–20 mm, **no** ~95) |
| 3 | Sin termo: `raw` alto, timeout u out-of-range |
| 4 | Si hay sesgo constante `b`: `TERMO_OFFSET_MM = b` |
| 5 | Ajustar `TERMO_PRESENT_MAX_MM` si vapor/rejilla da falsos positivos |
| 6 | Documentar tabla real/raw/corr en captura Test1 v0-7 |

Si `raw` con termo sigue ~90 mm: óptica, SHUT en low, SDA/SCL invertidos, o no es un 6180.

El build de producto usa `UI_DEBUG_TERMO 0` (sin leyendas en Coloca termo).

---

## Arduino IDE

Abrir: `mate_point_v0-7/mate_point_v0-7.ino`

| Parámetro | Valor |
|-----------|-------|
| Placa | ESP32-S3-Touch-LCD-7 |
| Flash | 16 MB |
| Partición | Custom → `partitions.csv` (8 MB APP) |
| PSRAM | OPI |
| USB CDC On Boot | **Disabled** |
| UART0 | GPIO44/43 @ 9600 — solo Nobana |

---

## Herencia v0-6

Sin cambios funcionales: pausa/reanudar Cargar termo, bandeja GPIO6, error agua UART, UI Figma, SoftAP + portal NVS, flujo E2E compra.

Provisioning Wi-Fi: ver [`mate_point_v0-6/README.md`](../mate_point_v0-6/README.md).
