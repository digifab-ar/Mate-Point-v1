# Mate Point — Firmware ESP32-S3 (Waveshare 7B)

Firmware Arduino Fase 4: LVGL + Wi‑Fi + MQTT; POC UART Nobana en ESP32 aparte.

| Documento | Rol |
|-----------|-----|
| [`PLAN-IMPLEMENTACION.md`](PLAN-IMPLEMENTACION.md) | **Plan maestro** — MQTT, UI v0.2, roadmap §16 |
| [`PROTOCOLO-UART-NOBANA.md`](PROTOCOLO-UART-NOBANA.md) | **Diccionario UART** Nobana (tramas, flujos, telemetría) |
| [`PLAN-POC-NOBANA-UART.md`](PLAN-POC-NOBANA-UART.md) | POC Etapa 1 — replay ARMOR (`W`/`R`, v0-1) |
| [`PLAN-MATE-POINT-UART-v0-2.md`](PLAN-MATE-POINT-UART-v0-2.md) | POC kiosco — `W`/`S`/`R` (UART v0-2, ESP32 Dev) |
| [`PLAN-MATE-POINT-UART-v0-3.md`](PLAN-MATE-POINT-UART-v0-3.md) | POC Waveshare — ciclo auto `W→S→R` (UART v0-3) |
| [`fase-4-plan-4.1-4.3-TEMP.md`](../fase-4-plan-4.1-4.3-TEMP.md) | Plan temporal Fase 4 POC (cerrado 2026-05-29) |
| [`plan-de-implementacion.md`](../plan-de-implementacion.md) | Plan general fases 0–6 |

## Estado (2026-06-04)

| Versión | Carpeta | Estado |
|---------|---------|--------|
| **v0.1** | [`mate_point_v0-1/`](mate_point_v0-1/) | **Operativo** — MQTT + UI simulada, E2E Railway ✅ |
| **v0.2** | [`mate_point_v0-2/`](mate_point_v0-2/) | **Operativo** — Comprar + QR + E2E hardware ✅ (2026-06-03); dispensado **simulado** |
| **UART v0-1** | [`mate_point_UART_v0-1/`](mate_point_UART_v0-1/) | **Etapa 1 cerrada** — replay ARMOR en banco (2026-06-04) |
| **UART v0-2** | [`mate_point_UART_v0-2/`](mate_point_UART_v0-2/) | **Implementado** — kiosco `W`/`S`/`R` (ESP32 Dev); banco pendiente |
| **UART v0-3** | [`mate_point_UART_v0-3/`](mate_point_UART_v0-3/) | **Etapa 2a-WS cerrada** — Waveshare ciclo auto 1×/boot ✅ banco (2026-06-04) |

> `mate_point_UART_v0-*` = POC UART Nobana. `mate_point_v0-2` = producto Waveshare (pantalla + MQTT).

## Qué abrir en Arduino IDE

```
mate_point_firmware/mate_point_v0-2/mate_point_v0-2.ino      ← producto (Comprar + QR)
mate_point_firmware/mate_point_UART_v0-1/mate_point_UART_v0-1.ino  ← POC UART replay
mate_point_firmware/mate_point_UART_v0-2/mate_point_UART_v0-2.ino  ← POC UART kiosco (ESP32 Dev)
mate_point_firmware/mate_point_UART_v0-3/mate_point_UART_v0-3.ino  ← POC UART Waveshare auto
```

Placa Waveshare: **ESP32-S3-Touch-LCD-7** · Flash 16 MB · PSRAM OPI · Serial 115200 · **v0-3 y v0-2 producto**.  
Placa POC UART (ESP32 Dev): **NodeMCU 38p** · v0-1 / v0-2 · ver [`PLAN-POC-NOBANA-UART.md`](PLAN-POC-NOBANA-UART.md) §3.

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
├── PROTOCOLO-UART-NOBANA.md
├── PLAN-POC-NOBANA-UART.md
├── PLAN-MATE-POINT-UART-v0-2.md
├── PLAN-MATE-POINT-UART-v0-3.md
├── reference/
├── mate_point_v0-1/
├── mate_point_v0-2/
├── mate_point_UART_v0-1/
├── mate_point_UART_v0-2/
└── mate_point_UART_v0-3/
```
