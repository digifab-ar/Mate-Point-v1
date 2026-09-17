# Mate Point firmware v0-8

Flota: un `device_id` y un QR estático por máquina. Fork de [`mate_point_v0-9`](../mate_point_v0-9/).

| Documento | Uso |
|-----------|-----|
| [`PLAN-MATE-POINT-v0-8.md`](../PLAN-MATE-POINT-v0-8.md) | Plan normativo — sucursal `MATEPOINT`, 4 cajas |
| [`PLAN-MATE-POINT-v0-9.md`](../PLAN-MATE-POINT-v0-9.md) | Herencia: oferta, litros, retiro termo |
| [`PLAN-MATE-POINT-v0-7.md`](../PLAN-MATE-POINT-v0-7.md) | Herencia: VL6180 |
| [`PLAN-MATE-POINT-v0-6.md`](../PLAN-MATE-POINT-v0-6.md) | Herencia: SoftAP + portal NVS |

**Estado:** **Implementado** (2026-09-17) — pendiente alta MP (script) + PNG de cada caja + QA E2E.

---

## Resumen

El producto (UI, Nobana, ToF, Wi-Fi) es el de v0-9. v0-8 solo cambia **identidad por unidad**:

1. `DEVICE_ID` único (`MATEPOINT001` … `MATEPOINT004`).
2. `MQTT_CLIENT_ID` sufijo **`v080`**.
3. `qr_static_img.c` = PNG de **esa** caja (no el QR del POC v0-9).

El servidor resuelve `device_id` → `external_pos_id` y publica MQTT en `mate/{device_id}/command`.

---

## Flota (sandbox DigiFAB, opción B)

Sucursal nueva `external_id=MATEPOINT` (Santamarina 1352, San Fernando). El store/POS del POC **no** se reutiliza.

| `DEVICE_ID` | `external_pos_id` | Nombre de caja |
|-------------|-------------------|----------------|
| `MATEPOINT001` | `MATEPOINT001POS001` | Mate point - Dispensador 1 |
| `MATEPOINT002` | `MATEPOINT002POS001` | Mate point - Dispensador 2 |
| `MATEPOINT003` | `MATEPOINT003POS001` | Mate point - Dispensador 3 |
| `MATEPOINT004` | `MATEPOINT004POS001` | Mate point - Dispensador 4 |

Alta: `cd servidor && npm run provision:mp-v08` (token en `.env`, no en git). PNGs en `servidor/ops/mp-v08/`.

**Corte con v0-9:** el QR que está hoy en `qr_static_img.c` es el del POC. No flashear v0-8 ni redesplegar el servidor ruteado hasta reemplazar el PNG de la unidad y convertir a LVGL.

### Convertir PNG → `qr_static_img.c`

1. PNG: `servidor/ops/mp-v08/MATEPOINT00n.png` (el de `qr.image`, no la plantilla).
2. [LVGL Image Converter](https://lvgl.io/tools/imageconverter) pestaña **LVGL v8**: `CF_TRUE_COLOR`, C array, 320×320, fondo blanco.
3. Reemplazar `qr_static_img.c` (símbolos `qr_static_map[]` / `qr_static_img`).
4. En `config.h`: `#define DEVICE_ID "MATEPOINT00n"` coincidente.
5. Compilar y flashear **esa** placa.

---

## `config.h` relevante

```c
#define DEVICE_ID "MATEPOINT001"   /* cambiar por unidad */
#define MQTT_CLIENT_ID "mate-" DEVICE_ID "-esp32-v080"
```

Oferta / litros / pausa: igual v0-9 (`Recarga de 1 litro` / `$500` como fallback).

---

## Arduino IDE

Abrir: `mate_point_v0-8/mate_point_v0-8.ino`

| Parámetro | Valor |
|-----------|-------|
| Placa | ESP32-S3-Touch-LCD-7 |
| Flash | 16 MB |
| Partición | Custom → `partitions.csv` (8 MB APP) |
| PSRAM | OPI |
| USB CDC On Boot | **Disabled** |
| UART0 | GPIO44/43 @ 9600 — solo Nobana |

Provisioning Wi-Fi: ver [`mate_point_v0-6/README.md`](../mate_point_v0-6/README.md).
