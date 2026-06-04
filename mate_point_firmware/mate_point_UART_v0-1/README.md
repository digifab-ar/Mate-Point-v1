# Mate Point UART v0-1 — POC ESP32 → Nobana

Firmware Etapa 1: el ESP32 **sustituye al ARMOR** y ejecuta el **replay** de la captura ref. Coffee 180 ml (timer).

Ver [`PLAN-POC-NOBANA-UART.md`](../PLAN-POC-NOBANA-UART.md) y [`PROTOCOLO-UART-NOBANA.md`](../PROTOCOLO-UART-NOBANA.md).

## Hardware

| Cable Nobana | TXS0108E | ESP32 |
|--------------|----------|-------|
| **Tx** | B1 → A1 | **GPIO25** (Serial2 RX) |
| **Rx** | B2 → A2 | **GPIO17** (Serial2 TX) |
| **G** | GND | GND |
| **5V** | VCCB | — |
| 3,3 V | VCCA + OE | ESP32 |

**ARMOR desconectado** del conector de 4 pines.

## Arduino IDE

1. Placa: **ESP32 Dev Module**
2. Abrir `mate_point_UART_v0-1.ino`
3. Monitor Serie: **115200**

## Comandos

| Comando | Acción |
|---------|--------|
| **`W`** | Wake: escucha RX → `F8` → log respuesta (repetir si hace falta) |
| **`R`** | Replay: `21` → `23` → Coffee `E2` → … (tras `[wake] LISTO`) |
| `X` | Abortar replay |
| `S` | Detener replay (fin anticipado) |
| `I` | Estado FSM + telemetría |
| `V` | Verbose (tramas HEX) |
| `B9600` | Baud del bus |
| `?` | Ayuda |

### Procedimiento banco

1. Nobana **OFF** → flashear/conectar ESP32 → abrir Monitor **115200**
2. **Nobana ON** durante los primeros 5 s (wake escucha RX; luego envía **F8** y loguea 3 s más)
3. Esperar **`[wake] LISTO`** — revisar líneas `NOB->ESP` — enviar **`R`**
4. Verificar: fases `START_21` … `LOCK_23`, `progress` ≥ 155, `b2=0x11`
5. Si Nobana se encendió tarde: **`W`** (repetir wake) y luego **`R`**

## Secuencia replay (resumen)

| Fase | UART maestro | ~duración |
|------|-------------|-----------|
| A′ | `F8` (1 byte) | 1× + 170 ms |
| B | `21` polling | 4 s |
| C | `23` polling | 3 s |
| D | `E2` + `d7=55` | hasta progress ≥ 155 o 24 s |
| E | `E2` + `b5=04` | 3,9 s |
| F | `22` + `b5=04` | 1 trama |
| G–H | `22` + `b5=00` | hasta `b2=11` + 15 s cooldown |
| I | `23` + `d7=55` | 3 s → fin |
