# Mate Point firmware v0-3-3

**Producto E2E validado banco** — fork de [`mate_point_v0-3-2`](../mate_point_v0-3-2/): Parar rápido + UI contrato desacoplado.

Plan: [`PLAN-MATE-POINT-v0-3.md`](../PLAN-MATE-POINT-v0-3.md) §16 · Test1: [`2026-06-05-Waveshare-Mate_point-v0-3-3_Test1.md`](../../tools/nobana_uart_sniffer/capturas/2026-06-05-Waveshare-Mate_point-v0-3-3_Test1.md)

## Flujo

Comprar → QR → pago → **`83°` + countdown + Parar** → (Parar o timer) → countdown **0:00** → **terminado** → **Listo** → **Comprar**

## Cambios vs v0-3-2

| Tema | v0-3-2 | v0-3-3 |
|------|--------|--------|
| Pre-stop manual | ~1800 ms | **200 ms** |
| Corte flujo (Parar) | ~2–3 s | **~300–500 ms** (banco OK) |
| Trigger **terminado** | Fin stop UART (`stop_done`) | **Countdown = 0** |
| Post-Parar sin flujo | UI cambia pronto | Countdown sigue (validado) |
| T_viva tras Parar | Hasta fin stop UI | **Viva** (validado) |

## Pantalla dispensado

| Elemento | Implementación |
|----------|----------------|
| Temperatura | `83°` — T_viva UART byte 3; Montserrat 44 |
| Countdown | `M:SS` — contrato `duration_ms` MQTT; Montserrat 14 |
| **Parar** | 280×90, rojo/blanco; disabled al tap |
| Refresh | 1 Hz |
| MQTT al Parar | Sin publish adicional |

## Stop manual UART

`nobana_dispense_abort()`: pre-stop `E2+04` **200 ms** → `22+00` directo (sin `22+04`).

**Banco 2026-06-05:** funcional en Nobana real; corte sensorial **~300–500 ms** (tap → agua para).

## Arduino IDE

| Tools | Valor |
|-------|--------|
| Board | ESP32S3 Dev Module |
| Flash | 16 MB |
| PSRAM | OPI 8 MB |
| **USB CDC On Boot** | **Disabled** |
| DIP | **UART2** → Nobana |

Abrir: `mate_point_v0-3-3/mate_point_v0-3-3.ino`

## Cableado Nobana

| Nobana | Waveshare |
|--------|-----------|
| Tx → | GPIO **44** (RX) |
| Rx ← | GPIO **43** (TX) |

## Contrato MQTT

Igual v0-3-1. `duration_ms` = tiempo total dispensado. Servidor: `DISPENSE_DURATION_MS=30000`.

## Validación banco (Test1 2026-06-05) — E2E OK

- [x] Parar: corte flujo ~**300–500 ms**; funcional con Nobana
- [x] UI countdown sin agua hasta **0:00** (tras Parar)
- [x] **terminado** al countdown 0 → **Listo** → **Comprar**
- [x] T_viva viva tras Parar
- [x] **Fin natural timer** — countdown 0 → terminado → Listo → Comprar
- [x] **Segunda compra** tras Parar — OK (esperar countdown a **0:00** antes de **Comprar**)
- [ ] Sniffer: `E2+04` ~200 ms → `22+00`
