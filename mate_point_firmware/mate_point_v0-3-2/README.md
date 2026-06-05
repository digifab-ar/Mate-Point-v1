# Mate Point firmware v0-3-2

**Producto en desarrollo** — fork de [`mate_point_v0-3-1`](../mate_point_v0-3-1/): UI dispensado en vivo + stop manual **Parar**.

Plan: [`PLAN-MATE-POINT-v0-3.md`](../PLAN-MATE-POINT-v0-3.md) §15

## Flujo

Comprar → QR → pago → **`83°` + countdown + Parar** → (timer o Parar) → **terminado** → **Listo** → **Comprar**

## Pantalla dispensado

| Elemento | Implementación |
|----------|----------------|
| Temperatura | `83°` — T_viva UART byte 3; Montserrat 44 |
| Countdown | `M:SS` — `duration_ms` MQTT; Montserrat 14 |
| **Parar** | 280×90, rojo/blanco; disabled al tap |
| Refresh | 1 Hz (`DISPENSE_UI_REFRESH_MS`) |
| MQTT al Parar | Sin publish adicional |

## Stop manual UART (§7.4)

`nobana_dispense_abort()`: pre-stop `E2+04` ~1,8 s → `22+00` directo (sin `22+04`). Ref. [`2026-06-03-inicio-dispense-coffee-fin_manual.md`](../../tools/nobana_uart_sniffer/capturas/2026-06-03-inicio-dispense-coffee-fin_manual.md).

**No** reutiliza `dispense_abort()` del POC UART v0-2 (abort blando sin tramas).

## Arduino IDE

| Tools | Valor |
|-------|--------|
| Board | ESP32S3 Dev Module |
| Flash | 16 MB |
| PSRAM | OPI 8 MB |
| **USB CDC On Boot** | **Disabled** |
| DIP | **UART2** → Nobana |

Abrir: `mate_point_v0-3-2/mate_point_v0-3-2.ino`

## Cableado Nobana

| Nobana | Waveshare |
|--------|-----------|
| Tx → | GPIO **44** (RX) |
| Rx ← | GPIO **43** (TX) |

## Contrato MQTT

Igual v0-3-1. `duration_ms` = tiempo total dispensado. Servidor: `DISPENSE_DURATION_MS=30000`.

## Validación banco pendiente

- [ ] E2E timer 30 s con temp + countdown
- [ ] Parar manual + sniffer (sin `22+04`)
- [ ] Segunda compra tras parada
