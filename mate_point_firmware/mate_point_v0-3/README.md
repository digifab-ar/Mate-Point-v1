# Mate Point firmware v0-3

> **Superseded** por [`mate_point_v0-3-1`](../mate_point_v0-3-1/) — producto vigente con timer MQTT.

Producto Waveshare ESP32-S3-Touch-LCD-7B: UI + MQTT + QR ([`mate_point_v0-2`](../mate_point_v0-2/)) y dispensado Nobana ([`mate_point_UART_v0-3`](../mate_point_UART_v0-3/)).

Plan: [`PLAN-MATE-POINT-v0-3.md`](../PLAN-MATE-POINT-v0-3.md)

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

## Bus serial

`Serial` @ 9600 en 44/43 — **solo** `F8` y tramas `0x68`. Sin logs de firmware.

Wake Nobana en `setup()` **antes** de LVGL.

## Módulos nuevos

| Archivo | Rol |
|---------|-----|
| `nobana_uart.cpp` | W / S / R |
| `dispense_controller.cpp` | UI dispensado + orquestación Nobana |

## Validación

Sniffer en TX + E2E Comprar → pago → dispensado físico. Dos compras en el mismo encendido.
