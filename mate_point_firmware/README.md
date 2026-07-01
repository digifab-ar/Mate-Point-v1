# Mate Point — Firmware ESP32-S3 (Waveshare 7B)

Firmware Arduino Fase 4: LVGL + Wi‑Fi + MQTT; POC UART Nobana en ESP32 aparte.

| Documento | Rol |
|-----------|-----|
| [`PLAN-IMPLEMENTACION.md`](PLAN-IMPLEMENTACION.md) | **Plan maestro** — MQTT, UI v0.2, roadmap §16 |
| [`PROTOCOLO-UART-NOBANA.md`](PROTOCOLO-UART-NOBANA.md) | **Diccionario UART** Nobana (tramas, flujos, telemetría) |
| [`PLAN-POC-NOBANA-UART.md`](PLAN-POC-NOBANA-UART.md) | POC Etapa 1 — replay ARMOR (`W`/`R`, v0-1) |
| [`PLAN-MATE-POINT-UART-v0-2.md`](PLAN-MATE-POINT-UART-v0-2.md) | POC kiosco — `W`/`S`/`R` (UART v0-2, ESP32 Dev) |
| [`PLAN-MATE-POINT-UART-v0-3.md`](PLAN-MATE-POINT-UART-v0-3.md) | POC Waveshare — ciclo auto `W→S→R` · roadmap v0-3-2 §12 |
| [`PLAN-MATE-POINT-v0-3.md`](PLAN-MATE-POINT-v0-3.md) | Producto Waveshare + Nobana — v0-3-1 (OK) · **v0-3-4** (E2E OK) · **v0-4** (UI Figma) |
| [`PLAN-MATE-POINT-v0-4-UI.md`](PLAN-MATE-POINT-v0-4-UI.md) | UI Figma v0-4 — plan + notas hardware Waveshare |
| [`PLAN-MATE-POINT-v0-5-2.md`](PLAN-MATE-POINT-v0-5-2.md) | Pausa / reanudar Cargar termo |
| [`PLAN-MATE-POINT-v0-6.md`](PLAN-MATE-POINT-v0-6.md) | **Wi-Fi SoftAP + portal (NVS)** — producto actual |
| [`fase-4-plan-4.1-4.3-TEMP.md`](../fase-4-plan-4.1-4.3-TEMP.md) | Plan temporal Fase 4 POC (cerrado 2026-05-29) |
| [`plan-de-implementacion.md`](../plan-de-implementacion.md) | Plan general fases 0–6 |

## Estado (2026-06-25)

| Versión | Carpeta | Estado |
|---------|---------|--------|
| **v0-6** | [`mate_point_v0-6/`](mate_point_v0-6/) | **E2E OK** hardware — Wi-Fi NVS + SoftAP + portal web ✅ |
| **v0-5-2** | [`mate_point_v0-5-2/`](mate_point_v0-5-2/) | **Implementado** — pausa/reanudar; UI OK hardware |
| **v0-5-1** | [`mate_point_v0-5-1/`](mate_point_v0-5-1/) | **E2E OK** banco — error agua UART |
| **v0-5** | [`mate_point_v0-5/`](mate_point_v0-5/) | **E2E OK** banco — sensor bandeja GPIO6 |
| **v0-4** | [`mate_point_v0-4/`](mate_point_v0-4/) | **OK hardware** — UI Figma + rotación 180° |
| **v0-3-4** | [`mate_point_v0-3-4/`](mate_point_v0-3-4/) | **E2E OK** banco — VL53L0X + Iniciar |
| **v0-3-1** | [`mate_point_v0-3-1/`](mate_point_v0-3-1/) | **E2E OK** banco — timer MQTT + Nobana |
| **v0.2** | [`mate_point_v0-2/`](mate_point_v0-2/) | **Operativo** — Comprar + QR + E2E hardware ✅ |
| **v0.1** | [`mate_point_v0-1/`](mate_point_v0-1/) | **Operativo** — MQTT + UI simulada |
| **UART v0-1** | [`mate_point_UART_v0-1/`](mate_point_UART_v0-1/) | **Etapa 1 cerrada** — replay ARMOR en banco (2026-06-04) |
| **UART v0-2** | [`mate_point_UART_v0-2/`](mate_point_UART_v0-2/) | **Implementado** — kiosco `W`/`S`/`R` (ESP32 Dev); banco pendiente |
| **UART v0-3** | [`mate_point_UART_v0-3/`](mate_point_UART_v0-3/) | **Etapa 2a-WS cerrada** — Waveshare ciclo auto 1×/boot ✅ banco (2026-06-04) |

> `mate_point_UART_v0-*` = POC UART Nobana. `mate_point_v0-2` = producto Waveshare (pantalla + MQTT).

## Qué abrir en Arduino IDE

```
mate_point_firmware/mate_point_v0-6/mate_point_v0-6.ino              ← producto actual: Wi-Fi NVS + portal
mate_point_firmware/mate_point_v0-5-2/mate_point_v0-5-2.ino        ← pausa / reanudar Cargar termo
mate_point_firmware/mate_point_v0-4/mate_point_v0-4.ino            ← UI Figma (sin provisioning Wi-Fi)
mate_point_firmware/mate_point_v0-3-4/mate_point_v0-3-4.ino      ← producto banco OK (placeholder UI)
```

Placa Waveshare: **ESP32-S3-Touch-LCD-7** · Flash 16 MB · PSRAM OPI · Serial 115200 · **v0-3 y v0-2 producto**.  
Placa POC UART (ESP32 Dev): **NodeMCU 38p** · v0-1 / v0-2 · ver [`PLAN-POC-NOBANA-UART.md`](PLAN-POC-NOBANA-UART.md) §3.

## Configuración

Editar `config.h` del sketch activo.

**v0-6:** Wi-Fi en **NVS** (sin `WIFI_SSID` en código). MQTT fijo `broker.hivemq.com:1883`.

| Constante | Versiones | Uso |
|-----------|-----------|-----|
| `WIFI_SSID` / `WIFI_PASSWORD` | v0-5-2 y anteriores | Red local hardcoded |
| `SERVER_HOST` | v0-2+ | Railway |
| `MQTT_HOST` / `MQTT_PORT` | todas | Broker HiveMQ POC |

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
