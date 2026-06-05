# Mate Point UART v0-3 — Waveshare, ciclo automático

Firmware POC **Waveshare ESP32-S3-Touch-LCD-7B** como maestro Nobana: **una pasada por encendido**, **solo tráfico protocolo** en UART0 (sin logs Serial).

**Estado banco (2026-06-04):** **validado OK** — ciclo W→S→R completo en Nobana. Evidencia: [`capturas/2026-06-04-Waveshare-UART-v0-3_banco-validacion-OK.md`](../../tools/nobana_uart_sniffer/capturas/2026-06-04-Waveshare-UART-v0-3_banco-validacion-OK.md).

Plan: [`PLAN-MATE-POINT-UART-v0-3.md`](../PLAN-MATE-POINT-UART-v0-3.md)  
Lógica UART: [`mate_point_UART_v0-2/`](../mate_point_UART_v0-2/) (kiosco, sin lock `23`)

> No confundir con [`mate_point_v0-2/`](../mate_point_v0-2/) (producto pantalla + MQTT).

## Hardware

| Item | Valor |
|------|--------|
| Placa | Waveshare ESP32-S3-Touch-LCD-7 |
| Flash / PSRAM | 16 MB / OPI 8 MB (Arduino IDE) |
| DIP switch | **UART2** → PH2.0 hacia Nobana |
| Level shifter | TXS0108E 3.3 V ↔ 5 V |
| ARMOR | **Desconectado** |

| Cable Nobana | Waveshare (UART2 / U0) |
|--------------|-------------------------|
| Nobana **Tx** → | **GPIO44** (RX) |
| Nobana **Rx** ← | **GPIO43** (TX) |
| GND | GND |

## Ciclo automático (1× por power-on)

1. Espera **3 s** — Nobana puede encender ~1–2 s después del ESP  
2. **W** — escucha 5 s → **`F8`** → escucha 3 s  
3. Espera **2 s**  
4. **S** — polling `68 01 21 …` cada 100 ms  
5. Espera **3 s**  
6. **R** — Coffee 180 ml: `E2` → pre-stop → `22` → cooldown (~15 s); **sin** `23` lock  
7. **Fin** — sin más TX (standby apagado). Reset / power-cycle para repetir.

## Arduino IDE

| Tools | Valor |
|-------|--------|
| Board | **ESP32S3 Dev Module** (wiki 7B) |
| Flash Size | **16 MB** |
| PSRAM | **OPI 8 MB** |
| **USB CDC On Boot** | **Disabled** |

### UART0 = Nobana únicamente

El header PH2.0 usa **UART0 (`Serial`)** a **9600 8N1** desde el reset. No hay `Serial.begin(115200)`, banner ni comandos `I`/`V`/`?`.

- **Flashear:** cualquier DIP que deje acceso al programador (según wiki Waveshare); tras flashear, **DIP UART2** para banco Nobana.  
- **Validar:** sniffer en TX Waveshare (`tools/nobana_uart_sniffer/`) — solo bytes `F8` y tramas `68 …`.

## Procedimiento banco

1. Cablear Nobana + TXS0108E; ARMOR off.  
2. Flashear `mate_point_UART_v0-3.ino`.  
3. **DIP UART2** → Nobana ON → **RESET** Waveshare.  
4. Observar chimes / dispensado; archivar captura sniffer si aplica.

## Siguiente fase

UI LVGL mínima (estado + botones opcionales) → integración en `mate_point_v0-2`.
