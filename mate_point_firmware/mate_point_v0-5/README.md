# Mate Point firmware v0-5

**Sensor bandeja de goteo** — fork de [`mate_point_v0-4`](../mate_point_v0-4/) (UI Figma + flujo v0-3-4).

Plan: [`PLAN-MATE-POINT-v0-5.md`](../PLAN-MATE-POINT-v0-5.md) · Hardware: [`arquitectura-hardware.md`](../../arquitectura-hardware.md) §3.4 · Esquema: [`docs/hardware/sensor-bandeja-reed-gpio6.png`](../../docs/hardware/sensor-bandeja-reed-gpio6.png)

**Estado (2026-06-24):** **E2E OK** — Test1 banco · [`2026-06-24-Waveshare-Mate_point-v0-5_Test1.md`](../../tools/nobana_uart_sniffer/capturas/2026-06-24-Waveshare-Mate_point-v0-5_Test1.md)

## Flujo UI

Igual v0-4, con monitoreo de bandeja en todo momento:

```
Iniciar → QR → pago → Coloca termo → Cargar termo (Iniciar) → dispensado (Parar) → Listo el mate → Iniciar
```

Si la bandeja se llena → pantalla **BANDEJA GOTEO LLENA** (bloqueante). Al vaciar → Standby.

## Cambios vs v0-4

| Tema | v0-4 | v0-5 |
|------|------|------|
| Sensor bandeja | UI `UI_ERR_BANDEJA` sin trigger | Reed NO @ **GPIO6**, poll **5 s** |
| Cableado | — | Pull-up **10 kΩ** 3V3→GPIO6, reed→GND |
| Bandeja llena en standby | — | Error bloqueante; no Iniciar |
| Bandeja llena dispensando | — | Auto-Parar + `order_cancel` + error (sin Finish) |
| Recuperación | — | Bandeja vacía → Standby automático |
| Módulo nuevo | — | `drip_tray_sensor.cpp/.h` |

## Historial

| Fecha | Cambio |
|-------|--------|
| 2026-06-24 | **Test1 E2E OK** — sensor bandeja + regresión v0-4 |
| 2026-06-24 | Implementación inicial v0-5 |

## Arduino IDE

Abrir: `mate_point_v0-5/mate_point_v0-5.ino`

| Tools | Valor |
|-------|--------|
| Board | ESP32S3 Dev Module |
| **Flash Size** | **16 MB (128 Mb)** |
| **Partition Scheme** | **Custom Partition Table** |
| **Custom partition CSV** | `partitions.csv` |
| **PSRAM** | **Enabled** |
| **USB CDC On Boot** | **Disabled** |
| DIP | **UART2** → Nobana |

Conexión sensor: conector PH2.0 **GPIO** — **3V3**, **GND**, **GP6** (ver esquema).

## `config.h` — constantes bandeja

```c
#define DRIP_TRAY_GPIO       6
#define DRIP_TRAY_POLL_MS    5000
#define DRIP_TRAY_FULL_LEVEL 0    // LOW = reed cerrado = bandeja llena
```

## Criterios banco (Test1 — todos OK)

Ver [`PLAN-MATE-POINT-v0-5.md`](../PLAN-MATE-POINT-v0-5.md) §12 y captura Test1.
