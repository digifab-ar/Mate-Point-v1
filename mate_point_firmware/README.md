# Mate Point — Firmware ESP32-S3 (Waveshare 7B)

Firmware Arduino Fase 4.3: LVGL + Wi‑Fi + MQTT + simulación de dispensado (sin UART Nobana).

| Documento | Contenido |
|-----------|-----------|
| [`PLAN-IMPLEMENTACION.md`](PLAN-IMPLEMENTACION.md) | Spec, contrato MQTT, criterios de aceptación |
| [`fase-4-plan-4.1-4.3-TEMP.md`](../fase-4-plan-4.1-4.3-TEMP.md) | Plan temporal Fase 4 POC (cerrado 2026-05-29) |
| [`plan-de-implementacion.md`](../plan-de-implementacion.md) | Plan general fases 0–6 |

## Estado: POC v0.1 completado (2026-05-29)

| Versión | Carpeta Arduino | Estado |
|---------|-----------------|--------|
| **v0.1** | [`mate_point_v0-1/`](mate_point_v0-1/) | **Operativo** — Wi‑Fi, MQTT, UI simulada, E2E Railway ✅ |
| **v0.2** | [`mate_point_v0-2/`](mate_point_v0-2/) | **En curso** — Comprar → QR placeholder → pago / timeout 2 min |

## Qué abrir en Arduino IDE

```
mate_point_firmware/mate_point_v0-2/mate_point_v0-2.ino   ← POC completa (Comprar + QR)
mate_point_firmware/mate_point_v0-1/mate_point_v0-1.ino   ← POC MQTT only
```

Placa: **Waveshare ESP32-S3-Touch-LCD-7** · Flash 16 MB · PSRAM OPI · Serial 115200.

## Configuración

Editar `mate_point_v0-1/config.h` (`WIFI_SSID`, `WIFI_PASSWORD`).

## Visibilidad MQTT

Broker: `broker.hivemq.com:1883` (ESP32) · topics: `mate/MATEPOINT001/command`, `mate/MATEPOINT001/status`

```bash
mosquitto_sub -h broker.hivemq.com -p 1883 -t 'mate/MATEPOINT001/#' -v
```

Ver [`PLAN-IMPLEMENTACION.md` §8](PLAN-IMPLEMENTACION.md) para pruebas CLI · **§14** para POC v0.2 (Comprar + QR).

## Estructura

```
mate_point_firmware/
├── README.md
├── PLAN-IMPLEMENTACION.md
├── reference/           ← backup demo 13 Waveshare
└── mate_point_v0-2/     ← sketch POC completa (Comprar + QR placeholder)
└── mate_point_v0-1/     ← sketch MQTT simulado
    ├── mate_point_v0-1.ino
    ├── config.h
    ├── display_ui.*
    ├── dispense_sim.*
    ├── mate_network.*
    └── [port Waveshare: lvgl_port, rgb_lcd, gt911, …]
```
