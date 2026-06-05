# Mate Point firmware v0-3-1

**Producto vigente en banco** — [`mate_point_v0-3-1`](../mate_point_v0-3-1/)

> **Desarrollo activo:** [`mate_point_v0-3-2`](../mate_point_v0-3-2/) — pantalla dispensado (T_viva, countdown, Parar).

Plan: [`PLAN-MATE-POINT-v0-3.md`](../PLAN-MATE-POINT-v0-3.md) §14  
Validación: [`2026-06-05-Waveshare-Mate_point-v0-3-1_Test1.md`](../../tools/nobana_uart_sniffer/capturas/2026-06-05-Waveshare-Mate_point-v0-3-1_Test1.md) — **E2E OK**

## Flujo validado en banco (2026-06-05)

Comprar → QR → pago → **Dispensado** → (tiempo MQTT) → **terminado** → (~3 s) → **Listo** → **Comprar**

- Dispensado físico alineado a `duration_ms` del servidor (`DISPENSE_DURATION_MS=30000`).
- Sin pantalla **Error dispensado** (watchdog eliminado).
- Cooldown Nobana en background entre **Listo** y nuevo **Comprar**.

## Cambios respecto a v0-3.0

| Tema | v0-3.0 | v0-3-1 |
|------|--------|--------|
| Timer UART | Replay fijo 180 ml | `duration_ms` MQTT |
| Watchdog | Paralelo a UART | Eliminado |
| UI **terminado** | Al fin cooldown / error | Al **fin del stop** |
| **Listo** | Tras cooldown largo | ~3 s tras **terminado** |

## Arduino IDE

| Tools | Valor |
|-------|--------|
| Board | ESP32S3 Dev Module |
| Flash | 16 MB |
| PSRAM | OPI 8 MB |
| **USB CDC On Boot** | **Disabled** |
| DIP | **UART2** → Nobana |

## Cableado Nobana

| Nobana | Waveshare |
|--------|-----------|
| Tx → | GPIO **44** (RX) |
| Rx ← | GPIO **43** (TX) |

## Contrato MQTT

`duration_ms` = tiempo total de dispensado (activo + pre-stop + cierre). Default firmware: **30000** ms. Cooldown (~15 s) fuera del contrato.

Servidor: `DISPENSE_DURATION_MS=30000`.

## Pendiente opcional

- Sniffer TX en E2E (bus limpio formal).
- Medición ml vs `duration_ms` para calibración de producto.
