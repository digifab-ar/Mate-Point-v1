# Mate Point firmware v0-5-1

**Error agua desconectada (UART)** — fork de [`mate_point_v0-5`](../mate_point_v0-5/) (bandeja GPIO6 + UI Figma).

Plan: [`PLAN-MATE-POINT-v0-5-1.md`](../PLAN-MATE-POINT-v0-5-1.md) · Protocolo tanque: [`PROTOCOLO-UART-NOBANA.md`](../PROTOCOLO-UART-NOBANA.md) §4.4

**Estado (2026-06-24):** **E2E OK** — vacío en dispensado + recuperación tras relleno validado en banco

## Flujo UI

Igual v0-5, con monitoreo de tanque vía UART Nobana:

```
Iniciar → QR → pago → Coloca termo → Cargar termo (Iniciar) → dispensado (Parar) → Listo el mate → Iniciar
```

Si el tanque queda vacío (`b2=0x10`, byte 7=`0x01`) → pantalla **AGUA DESCONECTADA** (bloqueante). Al restablecer agua → Standby.

## Cambios vs v0-5

| Tema | v0-5 | v0-5-1 |
|------|------|--------|
| Tanque vacío | UI `UI_ERR_AGUA` sin trigger | UART Nobana, debounce **2 tramas** (~200 ms) |
| Nivel bajo (`b2=0x11`) | — | Sin UI; flujo normal |
| Vacío en standby | — | Error bloqueante; no Iniciar |
| Vacío dispensando | — | Auto-Parar + `order_cancel` + error (sin Finish) |
| Recuperación agua | — | Tanque OK → Standby (si bandeja también OK) |
| Poll en error agua | — | `ensure_tank_monitor_poll()` — standby UART activo en `APP_ERROR_AGUA` |
| Convivencia bandeja | — | Primera alarma gana; recuperar solo con ambas OK |
| `nobana_uart` | Solo `b2`, `T_viva` | + byte 7, `nobana_tank_empty()`, `nobana_tank_poll()` |

## Fix recuperación post-dispensado

Si el tanque se vacía **durante** un dispensado, tras el abort UART el firmware debe **seguir haciendo poll** al Nobana mientras muestra el error. Sin eso, un relleno lento (minutos) no actualiza la telemetría y la pantalla queda bloqueada.

Implementación: `ensure_tank_monitor_poll()` al inicio de `app_state_tick()` — ver plan §17.

## Historial

| Fecha | Cambio |
|-------|--------|
| 2026-06-24 | **Recuperación agua OK** — dispensado → vacío → relleno → Standby |
| 2026-06-24 | Fix `ensure_tank_monitor_poll()` en `APP_ERROR_AGUA` |
| 2026-06-24 | Implementación inicial error agua UART |

## Arduino IDE

Abrir: `mate_point_v0-5-1/mate_point_v0-5-1.ino`

| Tools | Valor |
|-------|--------|
| Board | ESP32S3 Dev Module |
| **Flash Size** | **16 MB (128 Mb)** |
| **Partition Scheme** | **Custom Partition Table** |
| **Custom partition CSV** | `partitions.csv` |
| **PSRAM** | **Enabled** |
| **USB CDC On Boot** | **Disabled** |
| DIP | **UART2** → Nobana |

## `config.h` — constantes agua

```c
#define WATER_TANK_EMPTY_B2        0x10
#define WATER_TANK_EMPTY_B7        0x01
#define WATER_TANK_DEBOUNCE_FRAMES 2
#define WATER_TANK_BOOT_WAIT_MS    1000
```

## Criterios banco

Ver [`PLAN-MATE-POINT-v0-5-1.md`](../PLAN-MATE-POINT-v0-5-1.md) §12 (V1–V13). Validados en banco: **V6**, **V7** (§17).
