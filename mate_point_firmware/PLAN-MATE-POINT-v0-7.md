# Plan de implementación — Mate Point v0-7

**Proyecto:** Mate Point — OT-00268 Etapa 3  
**Carpeta:** [`mate_point_v0-7/`](mate_point_v0-7/) — fork de [`mate_point_v0-6/`](mate_point_v0-6/)  
**Base validada:** v0-6 (provisioning Wi-Fi SoftAP + portal NVS — E2E hardware 2026-06-25)  
**Plataforma:** Waveshare ESP32-S3-Touch-LCD-7B + Nobana UART + **VL6180** (I2C) + sensor bandeja (GPIO6)  
**Última actualización:** 2026-09-08  
**Estado:** **Cerrado — OK hardware** (2026-09-08)

| Documento | Uso |
|-----------|-----|
| [`PLAN-MATE-POINT-v0-6.md`](PLAN-MATE-POINT-v0-6.md) | Base inmediata — Wi-Fi NVS, portal, UI error Wi-Fi/MQTT |
| [`PLAN-MATE-POINT-v0-5-2.md`](PLAN-MATE-POINT-v0-5-2.md) | Pausa / reanudar Cargar termo |
| [`PLAN-MATE-POINT-v0-3-4.md`](PLAN-MATE-POINT-v0-3-4.md) | Gate Iniciar, poll termo, debounce, `TERMO_*` |
| [`arquitectura-hardware.md`](../arquitectura-hardware.md) | Bus I2C PH2.0 — VL6180 @ 0x29 |
| Captura calibración L0X | [`2026-06-17-Waveshare-Mate_point-v0-3-4_Test1.md`](../tools/nobana_uart_sniffer/capturas/2026-06-17-Waveshare-Mate_point-v0-3-4_Test1.md) |

---

## 1. Objetivo

Reemplazar el sensor de presencia de termo **VL53L0X** por un **VL6180** (módulo típico **TOF050C**) en el **mismo** conector I2C, sin cambiar el flujo de producto (Coloca termo / Iniciar / auto-Parar / pausa / Wi-Fi).

Motivo: el sensor está a **~1 cm** del termo. El VL53L0X opera ahí **dentro de su zona ciega** (~0–3 cm). En banco (v0-3-4) la lectura cruda era **real + ~85 mm**; `TERMO_OFFSET_MM = 85` es un **parche de calibración**, no la distancia de montaje. El VL6180 está pensado para **0–10 cm** (nativo 0–255 mm a escala 1×).

Comportamiento resumido:

1. Se **desconecta** el VL53L0X; entra el VL6180 en **SDA/SCL/3V3/GND**.
2. Dirección I2C sigue **`0x29`** — no conviven los dos chips.
3. `dispense_controller` sigue decidiendo presencia con distancia corregida + umbral + debounce.
4. Se **recalibra** offset/umbral en banco con termos pintados o cepillados (indoor).
5. GT911, CH422G, UART Nobana, GPIO6, MQTT y provisioning v0-6 **no cambian**.

> **Alcance v0-7:** driver VL6180 sobre el `i2c_master` existente, calibración de termo, docs hardware. **Sin** cambio de UI Figma, Nobana, MQTT, API servidor ni provisioning Wi-Fi.

---

## 2. Decisiones cerradas (2026-09-02)

| # | Tema | Decisión |
|---|------|----------|
| D1 | Tipo de cambio | **Reemplazo 1:1** — el VL53L0X **sale** del bus; no hay dos ToF |
| D2 | Chip | **VL6180 / VL6180X** (p. ej. módulo TOF050C). No TOF10120 (mín. 10 cm). No TOF200C (sigue siendo L0X) |
| D3 | I2C esclavo | **`0x29`** (7 bits), mismo que el L0X. **Sin conflicto** con GT911 (`0x14`/`0x5D`) ni CH422G (`0x24`) |
| D4 | Conector | I2C PH2.0 Waveshare: **3V3, GND, SDA (GPIO8), SCL (GPIO9)** — 400 kHz |
| D5 | XSHUT / INT | **No usar GPIO extra** (GP6 = bandeja). SHUT del módulo atado a **3.3 V** si no se controla. INT sin conectar |
| D6 | Stack I2C | Solo **`driver/i2c_master.h`** vía `DEV_I2C_Register_Device(0x29)`. **Prohibido** `Wire` / Adafruit_BusIO en el mismo SDA/SCL que el táctil |
| D7 | Librerías Arduino | Pololu `VL6180X` y Adafruit `Adafruit_VL6180X` = **referencia** (init AN4545 + single-shot). **No** dependencias de producto |
| D8 | API de aplicación | Conservar contrato `init` / `sample` / `termo_present` / distancias mm. Renombrar archivos a `vl6180x_sensor.*` y actualizar call sites (~`.ino` + `dispense_controller`) |
| D9 | Identificación | Init OK solo si `IDENTIFICATION_MODEL_ID == 0xB4` (no `0xEE` del L0X) |
| D10 | Registros | Dirección de registro **16 bits** hacia `0x29` (`[reg_hi][reg_lo][data]`). El resto del bus (GT911/CH422G) **sin cambio** |
| D11 | Modo medición | **Single-shot** ranging, poll **300 ms** (`TERMO_POLL_MS`). Sin ALS (lux) en v0-7 |
| D12 | Escala | **1×** (1 LSB = 1 mm, 0–255 mm). Suficiente a 1 cm y para “sin termo” |
| D13 | Presencia | `termo_present` = lectura OK **y** distancia **corregida** `< TERMO_PRESENT_MAX_MM` |
| D14 | Offset | **No reutilizar 85 mm.** Partir de `TERMO_OFFSET_MM = 0` y fijar en banco (esperado **0–15 mm**, no ~85) |
| D15 | Umbral inicial | `TERMO_PRESENT_MAX_MM = 15` (misma semántica: termo a ~10 mm). Ajustar si el 6180 lo exige |
| D16 | Fallo I2C / timeout / out of range | Equivale a **sin termo** (hereda v0-3-4 D4) |
| D17 | Debounce | **2** lecturas consecutivas (`TERMO_DEBOUNCE_COUNT`) — sin cambio |
| D18 | Debug UI | `UI_DEBUG_TERMO=0` en producto. `=1` solo banco (raw / corr / TERMO OK) |
| D19 | Superficie / ambiente | Termos **pintados o cepillados**, **indoor**. No es requisito de firmware; condiciona QA |
| D20 | Herencia v0-6 | Pausa, bandeja, agua UART, UI Figma, NVS Wi-Fi, portal, partición 8 MB APP |
| D21 | MQTT / Nobana / servidor | **Sin cambios** |
| D22 | `MQTT_CLIENT_ID` | Sufijo **`v070`** |
| D23 | Docs | Actualizar `arquitectura-hardware.md` (VL53L0X → VL6180 @ 0x29) y `PLAN-IMPLEMENTACION.md` (hito v0-7) |

### 2.1 Cambio respecto a v0-6 / v0-3-4

| Aspecto | v0-3-4 … v0-6 | v0-7 |
|---------|----------------|------|
| Chip termo | VL53L0X | VL6180 |
| Distancia física | ~**10 mm** al termo | Igual |
| Zona ciega del chip | ~0–3 cm — **operando dentro** | Chip de corto alcance |
| `TERMO_OFFSET_MM` | **85** (sesgo empírico L0X) | Recalibrar; **no** copiar 85 |
| Driver | `vl53l0x_sensor.cpp` (regs 8 bit, ID `0xEE`) | `vl6180x_sensor.cpp` (regs 16 bit, ID `0xB4`) |
| Dirección I2C | 0x29 | 0x29 (mismo slot) |

### 2.2 Geometría y offset (no confundir)

```
dist_corregida = max(0, raw_mm − TERMO_OFFSET_MM)
termo_presente = (status OK) && (dist_corregida < TERMO_PRESENT_MAX_MM)
```

| Concepto | Valor |
|----------|--------|
| Distancia **física** sensor → tapa/hombro del termo | **~10 mm** |
| Offset **85 mm** (L0X) | Sesgo de lectura en zona ciega (`raw ≈ real + 85`) — ver captura 2026-06-17 |
| Offset v0-7 | Constante a **medir** con el 6180; no es altura de montaje |

---

## 3. Hardware e I2C

### 3.1 Mapa de bus (tras el swap)

| Dirección | Dispositivo | Origen |
|-----------|-------------|--------|
| 0x14 / 0x5D | GT911 (táctil) | Integrado |
| 0x24 | CH422G (IO expander) | Integrado |
| **0x29** | **VL6180** (termo) | Externo — reemplaza VL53L0X |

No hay colisión. El mapa de registros de 16 bits del 6180 **no** es una dirección de bus: solo cambia el payload hacia el esclavo `0x29`.

### 3.2 Cableado módulo (TOF050C típico)

| Pin módulo | Waveshare I2C PH2.0 |
|------------|---------------------|
| VIN | 3V3 |
| GND | GND |
| SDA | SDA |
| SCL | SCL |
| SHUT | 3V3 (enable) o NC si el módulo ya lo lleva high |
| INT | NC |

Pull-ups: los del módulo Waveshare (táctil). Cable corto (&lt;30 cm).

> **Banco 2026-09-08:** el pinout del módulo **no** copia el del VL53L0X. Un cable L0X reutilizado dejó **SDA/SCL invertidos** → `ID no responde`. Mapear por silkscreen del 6180, no por color/orden del loom anterior.

### 3.3 Lo que no se toca

UART2 + TXS0108E (Nobana), GPIO6 reed bandeja, RS-485/CAN reserva.

---

## 4. Firmware — driver

### 4.1 Enfoque

Portar el flujo **Pololu / Adafruit** (AN4545 SR03 + `configureDefault` + single-shot) a transacciones `i2c_master_transmit` / `transmit_receive`, igual que el L0X actual.

Init mínimo:

1. `DEV_I2C_Register_Device(0x29, &s_dev)`
2. Leer MODEL_ID → debe ser **`0xB4`**
3. Si `SYSTEM__FRESH_OUT_OF_RESET`: writes privados AN4545 + públicos recomendados
4. Single-shot: `SYSRANGE__START` → poll interrupt range ready → `RESULT__RANGE_VAL` (uint8 mm) → clear interrupt

Rango 255 en single-shot Pololu suele significar timeout: mapear a `STATUS_TIMEOUT` / sin termo.

### 4.2 API objetivo (`vl6180x_sensor.h`)

Misma forma que `vl53l0x_sensor.h` (renombrada):

```c
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
    uint16_t distance_mm;              /* raw, 0–255 típico */
    uint16_t distance_corrected_mm;    /* raw − TERMO_OFFSET_MM */
    bool termo_present;
    bool init_ok;
} Vl6180xSample;

bool vl6180x_init();
void vl6180x_sample(Vl6180xSample *out);
/* is_ready, init_status_text, read_mm, termo_present, status_text — análogos */
```

`config.h`:

```c
#define VL6180X_I2C_ADDR 0x29
#define TERMO_OFFSET_MM 0          /* recalibrar en banco — D14 */
#define TERMO_PRESENT_MAX_MM 15
#define TERMO_POLL_MS 300
#define TERMO_DEBOUNCE_COUNT 2
```

Quitar `VL53L0X_I2C_ADDR`.

### 4.3 Call sites (cambio de nombre, no de lógica)

| Archivo | Cambio |
|---------|--------|
| `mate_point_v0-7.ino` | `#include "vl6180x_sensor.h"`; `vl6180x_init()` |
| `dispense_controller.cpp` | Tipos `Vl6180xSample` / `vl6180x_sample` / textos de status |
| `vl53l0x_sensor.cpp` / `.h` | **Eliminar** del fork (reemplazados) |

Estimación: **~350–450 líneas** en el driver nuevo; **~20–40** en `.ino` + controller; **2–6** en `config.h`. Máquina de estados, UI, Nobana, MQTT: **0** líneas funcionales.

### 4.4 `i2c.cpp` / `i2c.h`

No es obligatorio extender la API pública si el driver 6180 hace `i2c_master_transmit` a pelo (como el L0X). Opcional: helpers `write_reg16addr` — solo si se reutilizan. **No** mezclar `Wire.begin()` en GPIO 8/9.

---

## 5. Calibración en banco

Usar debug UI (raw, corr, TERMO OK) y un termo **pintado o cepillado** a la distancia de producto (~1 cm).

| Paso | Acción |
|------|--------|
| 1 | `TERMO_OFFSET_MM = 0`, umbral 15 |
| 2 | Con termo colocado: anotar `raw` (esperado ~8–20 mm, no ~95) |
| 3 | Sin termo: `raw` alto, timeout u out-of-range |
| 4 | Si hay sesgo constante `b`: `TERMO_OFFSET_MM = b` |
| 5 | Ajustar `TERMO_PRESENT_MAX_MM` para histéresis (no falsos positivos de vapor/rejilla) |
| 6 | Documentar tabla real/raw/corr en captura Test1 v0-7 |

Si `raw` con termo sigue ~90 mm, revisar óptica (cover, vapor, SHUT en low, no es 6180).

---

## 6. Tareas de implementación

| ID | Tarea | Criterio |
|----|-------|----------|
| T1 | Fork `mate_point_v0-7/` desde v0-6 | **Hecho** — carpeta + `MQTT_CLIENT_ID` v070 |
| T2 | `vl6180x_sensor.cpp/.h` — I2C 16 bit + init AN4545 + ID `0xB4` | **Hecho** — init OK en banco (2026-09-08) |
| T3 | Single-shot + mapeo de status | **Hecho** — timeout ≠ OK |
| T4 | Offset/umbral en `config.h` (D14–D15) | **Hecho** — `TERMO_OFFSET_MM = 0`; umbral 15 mm |
| T5 | Cablear call sites; borrar `vl53l0x_sensor.*` | **Hecho** |
| T6 | Debug UI Coloca termo | **Hecho** — usado en banco; producto `UI_DEBUG_TERMO=0` |
| T7 | Calibración banco termo @ ~1 cm | **Hecho** — gate termo OK; offset 0 / umbral 15 |
| T8 | Regresión v0-6: gate Iniciar, flujo heredado | **Hecho** — sin cambio de máquina de estados vs v0-6 |
| T9 | `arquitectura-hardware.md` — VL6180 @ 0x29 | **Hecho** |
| T10 | README `mate_point_v0-7/` + este plan | **Hecho** |
| T11 | `PLAN-IMPLEMENTACION.md` + [`plan-de-implementacion.md`](../plan-de-implementacion.md) — hito v0-7 | **Hecho** |

---

## 7. Criterios de aceptación (banco)

| ID | Criterio | Ref | Banco 2026-09-08 |
|----|----------|-----|------------------|
| S1 | Boot: init VL6180 OK (ID `0xB4`); táctil GT911 sigue operativo | D3, D9 | **OK** |
| S2 | CH422G / display sin regresión | D3 | **OK** |
| S3 | Con termo a ~1 cm: `termo_present` estable (debounce 2) | D13, D17 | **OK** |
| S4 | Sin termo: Coloca termo; no dispara Iniciar | D16 | **OK** |
| S5 | Offset **no** es 85 salvo medición que lo justifique | D14 | **OK** (`TERMO_OFFSET_MM = 0`) |
| S6 | Post-pago: Coloca termo → Iniciar → dispensado (hereda v0-5-2 / v0-6) | D20 | **OK** — flujo heredado |
| S7 | Retiro de termo en dispensado: cierre de sesión según v0-5-2 (no pausa) | D20 | Hereda v0-6 (sin cambio de código) |
| S8 | Pausa / Continuar / Finalizar | D20 | Hereda v0-6 (sin cambio de código) |
| S9 | Bandeja GPIO6 y tanque UART | D20 | Hereda v0-6 (sin cambio de código) |
| S10 | Provisioning Wi-Fi v0-6 intacto | D20 | Hereda v0-6 (sin cambio de código) |
| S11 | Indoor + termo pintado **y** uno cepillado | D19 | Condición de QA; umbral 15 mm en banco |
| S12 | Vapor: sin falsos “sin termo” sostenidos durante carga (anotar si falla) | Riesgo | No observado en este cierre |

**Cierre banco (2026-09-08):**

1. VL53L0X fuera; VL6180 en I2C PH2.0.
2. Blocker init (`ID no responde`): **SDA/SCL invertidos** respecto al loom del L0X. Tras cruzar los hilos según silkscreen del 6180 → init OK y gate termo OK.
3. `TERMO_OFFSET_MM = 0`, `TERMO_PRESENT_MAX_MM = 15`. Overlay debug apagado (`UI_DEBUG_TERMO=0`).
4. Máquina de estados, Nobana, bandeja y Wi-Fi NVS sin cambio vs v0-6.

---

## 8. Riesgos y mitigaciones

| Riesgo | Mitigación |
|--------|------------|
| `Wire` + `i2c_master` en GPIO 8/9 tumba el táctil | D6 — driver nativo solamente |
| Copiar `TERMO_OFFSET_MM = 85` | D14 — calibrar de cero |
| Vapor / gota en la óptica (agua caliente) | Orientación, cover, QA S12; no es luz ambiente |
| Tapa muy inclinada o canto del termo | Apuntar al hombro/tapa de frente; umbral en banco |
| Módulo no es 6180 (ID ≠ `0xB4`) | Fallar init; no operar con driver L0X |
| SHUT flotante → chip en shutdown | Atar a 3V3 (D5) |
| SDA/SCL invertidos vs loom L0X | Mapear silkscreen del 6180 (banco 2026-09-08) |
| Alcance 255 mm | Irrelevante a 1 cm; “sin termo” puede ser timeout — ya cuenta como ausente |

---

## 9. Fuera de alcance v0-7

- TOF10120, VL53L1X, inductivo
- ALS (lux) del 6180
- Multi-sensor / cambio de dirección I2C en runtime
- GPIO para XSHUT o INT
- Cambios MQTT, Nobana, Figma, portal Wi-Fi, `order_complete`
- Librerías Adafruit/Pololu como dependencia de compilación

---

## 10. Estimación

| Fase | Días |
|------|------|
| Fork + driver I2C 16 bit + init + single-shot | 1.0 |
| Integración call sites + debug UI | 0.5 |
| Calibración banco + ajuste umbral | 0.5 |
| QA E2E + regresión v0-6 | 1.0 |
| Docs hardware / README / índice | 0.5 |
| **Total** | **~3.5 días** |

---

## 11. Estructura de carpeta objetivo

```
mate_point_firmware/mate_point_v0-7/
├── mate_point_v0-7.ino              ← vl6180x_init(); MQTT_CLIENT_ID v070
├── config.h                         ← VL6180X_I2C_ADDR; TERMO_* recalibrados
├── vl6180x_sensor.cpp / .h          ← nuevo (reemplaza vl53l0x_*)
├── dispense_controller.cpp / .h     ← call sites ToF
├── i2c.cpp / .h                     ← sin Wire; bus compartido
├── … (resto igual v0-6: app_state, display_ui, nobana, wifi_*, bandeja)
└── README.md
```

Carpeta creada 2026-09-08. Hito **cerrado** en banco el mismo día.

---

## Changelog

| Fecha | Cambio |
|-------|--------|
| 2026-09-08 | **Cerrado OK hardware** — init + gate termo; causa `ID no responde`: SDA/SCL invertidos; `UI_DEBUG_TERMO=0`; offset 0 / umbral 15 mm |
| 2026-09-08 | **Implementado** — [`mate_point_v0-7/`](mate_point_v0-7/); driver VL6180 `i2c_master` |
| 2026-09-02 | Hito enganchado en [`plan-de-implementacion.md`](../plan-de-implementacion.md) (paso **4.11**) y [`PLAN-IMPLEMENTACION.md`](PLAN-IMPLEMENTACION.md) |
| 2026-09-02 | Plan inicial v0-7 — reemplazo VL53L0X → VL6180 @ 0x29; offset 85 mm reinterpretado como sesgo L0X a ~1 cm físico; driver `i2c_master` sin Wire |
