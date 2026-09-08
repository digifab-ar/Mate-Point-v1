# Mate Point firmware v0-9

Oferta y sesión de carga según el servidor — fork de [`mate_point_v0-7`](../mate_point_v0-7/) (VL6180 + Wi-Fi NVS).

| Documento | Uso |
|-----------|-----|
| [`PLAN-MATE-POINT-v0-9.md`](../PLAN-MATE-POINT-v0-9.md) | Plan normativo + criterios de aceptación |
| [`PLAN-MATE-POINT-v0-7.md`](../PLAN-MATE-POINT-v0-7.md) | Herencia: VL6180 |
| [`PLAN-MATE-POINT-v0-6.md`](../PLAN-MATE-POINT-v0-6.md) | Herencia: SoftAP + portal NVS |

**Estado:** **Implementado** (2026-09-08) — QA banco pendiente (A1–A16, P-B1, P-B2).

---

## Resumen

Cuatro ajustes de producto **sin reflash** en cada cambio comercial (salvo el piso 80 °C, que queda en firmware):

1. **Temperatura en Cargar termo:** si `T_viva < 80` °C, la UI muestra **80 °C**; si no, el valor real. Solo `DISPENSING` / `PAUSED`. Serial y MQTT siguen con la telemetría cruda.
2. **Litros:** `litros = dispensed_ms / 120000`; tope = `duration_ms / 120000`. Railway ya publica `DISPENSE_DURATION_MS=120000` (1 L).
3. **Oferta en Paga con QR:** `product_description` y `price_display` vienen de `POST /orders/create`. Fallback local: `Recarga de 1 litro` / `$500`.
4. **Retiro de termo en carga:** misma pausa UART que **Parar** → **COLOCA EL TERMO**; al reponer, UI pausa; hay que tocar **Continuar**. Timer X (`pause_timeout_ms` MQTT, fallback 20 s) corre en Coloca termo y en pausa.

v0-8 (flota) queda para el final; este sketch no lo incluye.

---

## `config.h` relevante

```c
#define MQTT_CLIENT_ID "mate-" DEVICE_ID "-esp32-v090"
#define UI_MS_PER_LITER              120000u
#define UI_TEMP_DISPLAY_MIN_C        80
#define PAUSE_DECISION_TIMEOUT_MS    20000   /* fallback si MQTT no trae pause_timeout_ms */
#define UI_PRODUCT_DESC_PLACEHOLDER    "Recarga de 1 litro"
#define UI_PRODUCT_PRICE_PLACEHOLDER   "$500"
```

---

## Servidor (mismo deploy)

Railway / `.env`:

```
PRODUCT_DESCRIPTION=Recarga de 1 litro
MP_SALE_AMOUNT=500.00
DISPENSE_DURATION_MS=120000
PAUSE_TIMEOUT_MS=20000
```

`POST /orders/create` responde `product_description` + `price_display`. MQTT `command` incluye `pause_timeout_ms`. Firmware v0-7 ignora los campos extra.

---

## Arduino IDE

Abrir: `mate_point_v0-9/mate_point_v0-9.ino`

| Parámetro | Valor |
|-----------|-------|
| Placa | ESP32-S3-Touch-LCD-7 |
| Flash | 16 MB |
| Partición | Custom → `partitions.csv` (8 MB APP) |
| PSRAM | OPI |
| USB CDC On Boot | **Disabled** |
| UART0 | GPIO44/43 @ 9600 — solo Nobana |

Captura Test1 objetivo: `tools/nobana_uart_sniffer/capturas/2026-XX-XX-Waveshare-Mate_point-v0-9_Test1.md`

Provisioning Wi-Fi: ver [`mate_point_v0-6/README.md`](../mate_point_v0-6/README.md).
