# Mate Point — Firmware ESP32-S3 (Waveshare 7B)

Firmware Arduino Fase 4: LVGL + Wi‑Fi + MQTT + simulación de dispensado (sin UART Nobana).

| Documento | Contenido |
|-----------|-----------|
| [`PLAN-IMPLEMENTACION.md`](PLAN-IMPLEMENTACION.md) | Spec, contrato MQTT, POC v0.2 §14–§15 |
| [`fase-4-plan-4.1-4.3-TEMP.md`](../fase-4-plan-4.1-4.3-TEMP.md) | Plan temporal Fase 4 POC (cerrado 2026-05-29) |
| [`plan-de-implementacion.md`](../plan-de-implementacion.md) | Plan general fases 0–6 |

## Estado (2026-05-29)

| Versión | Carpeta Arduino | Estado |
|---------|-----------------|--------|
| **v0.1** | [`mate_point_v0-1/`](mate_point_v0-1/) | **Operativo** — MQTT + UI simulada, E2E Railway ✅ |
| **v0.2** | [`mate_point_v0-2/`](mate_point_v0-2/) | **Implementado** — Comprar → QR PROGMEM → pago/timeout 2 min · **E2E hardware pendiente** |

## Qué abrir en Arduino IDE

```
mate_point_firmware/mate_point_v0-2/mate_point_v0-2.ino   ← POC completa (Comprar + QR)
mate_point_firmware/mate_point_v0-1/mate_point_v0-1.ino   ← POC MQTT only
```

Placa: **Waveshare ESP32-S3-Touch-LCD-7** · Flash 16 MB · PSRAM OPI · Serial 115200.

## Configuración

Editar `config.h` del sketch activo:

| Constante | v0.2 | Uso |
|-----------|------|-----|
| `WIFI_SSID` / `WIFI_PASSWORD` | ambos | Red local |
| `SERVER_HOST` | v0.2 | Railway (`mate-point-v1-production.up.railway.app`) |
| `QR_TIMEOUT_MS` | v0.2 | 120000 (2 min, alineado `MP_ORDER_EXPIRATION=PT2M`) |

## Visibilidad MQTT

Broker: `broker.hivemq.com:1883` · topics: `mate/MATEPOINT001/command`, `mate/MATEPOINT001/status`

```bash
mosquitto_sub -h broker.hivemq.com -p 1883 -t 'mate/MATEPOINT001/#' -v
```

Pruebas CLI: [`PLAN-IMPLEMENTACION.md` §8](PLAN-IMPLEMENTACION.md) · POC v0.2: **§14–§15**.

## Estructura

```
mate_point_firmware/
├── README.md
├── PLAN-IMPLEMENTACION.md
├── reference/              ← backup demo 13 Waveshare
├── mate_point_v0-2/          ← POC Comprar + QR + HTTP órdenes
│   ├── app_state.*
│   ├── order_client.*
│   ├── qr_static_img.c       ← QR LVGL 8 (320×320)
│   ├── qr_image.h
│   └── display_ui.*
└── mate_point_v0-1/          ← POC MQTT simulado
    ├── mate_point_v0-1.ino
    ├── config.h
    └── …
```
