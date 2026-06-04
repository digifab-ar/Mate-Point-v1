# Mate Point UART v0-2 — modo kiosco (W / S / R)

Firmware POC ESP32 maestro Nobana: **sin replay ARMOR** (sin `23` idle previo ni lock final).  
Base: v0-1 · Plan: [`PLAN-MATE-POINT-UART-v0-2.md`](../PLAN-MATE-POINT-UART-v0-2.md)

> No confundir con [`mate_point_v0-2/`](../mate_point_v0-2/) (Waveshare + MQTT).

## Hardware

Igual que v0-1 — ver [`mate_point_UART_v0-1/README.md`](../mate_point_UART_v0-1/README.md).

| Cable Nobana | ESP32 |
|--------------|-------|
| Tx → | GPIO25 (Serial2 RX) |
| Rx ← | GPIO17 (Serial2 TX) |

**ARMOR desconectado.**

## Comandos

| Comando | Acción |
|---------|--------|
| **`W`** | Wake: `F8` + escucha → `[wake] LISTO` (1× por sesión) |
| **`S`** | Standby: polling **`21`** cada ~100 ms |
| **`R`** | Dispensar Coffee 180 ml (`E2` → pre-stop → `22` + cooldown). **Requiere `S` activo** |
| **`X`** | Abortar dispensado **o** salir standby |
| `I` | Estado |
| `V` | Verbose HEX |
| `B9600` | Baud bus |
| `?` | Ayuda |

- Durante dispensado: **`S` se ignora** (mensaje en Serial); solo **`X`** aborta.
- Tras `R` OK: vuelve a **standby automático** (`21` poll) sin pulsar `S` otra vez.
- **No** se envía `23` lock al final del ciclo.

## Procedimiento banco

1. Nobana OFF → flashear → Monitor **115200**
2. Nobana ON → **`W`** → `[wake] LISTO`
3. **`S`** → `[standby] ON`
4. (Espera la que quieras medir — sin tope en firmware)
5. **`R`** → ciclo completo → `[dispense] FIN (ciclo_sin_lock)` → standby auto
6. Segundo **`R`** (misma sesión, sin nuevo `S`) — validar segundo pedido
7. Archivar log en `tools/nobana_uart_sniffer/capturas/`

## Secuencia UART (resumen)

| Paso | UART |
|------|------|
| Wake | `F8` |
| Standby | `68 01 21 …` poll |
| Dispensado | `68 02 E2 … 55` → … → `22` cierre + cooldown ~15 s |
| Fin | **sin** `68 … 23 … 55` |
