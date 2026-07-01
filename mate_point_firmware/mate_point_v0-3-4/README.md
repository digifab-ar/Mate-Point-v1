# Mate Point firmware v0-3-4

**Producto E2E validado banco** — fork de [`mate_point_v0-3-3`](../mate_point_v0-3-3/): VL53L0X + gate **Iniciar** post-pago.

Plan: [`PLAN-MATE-POINT-v0-3-4.md`](../PLAN-MATE-POINT-v0-3-4.md) · Test1: [`2026-06-17-Waveshare-Mate_point-v0-3-4_Test1.md`](../../tools/nobana_uart_sniffer/capturas/2026-06-17-Waveshare-Mate_point-v0-3-4_Test1.md)

## Flujo

Comprar → QR → pago → **Coloque el termo** / **Iniciar** → **`83°` + countdown + Parar** → terminado → Listo → Comprar

## Cambios vs v0-3-3

| Tema | v0-3-3 | v0-3-4 |
|------|--------|--------|
| Post-pago | Dispensado inmediato | Espera termo + botón **Iniciar** |
| Sensor | — | VL53L0X @ 0x29; offset **85 mm**; termo si corregido **< 15 mm** |
| MQTT `dispensing` | Al pago | Solo al pulsar **Iniciar** |
| Termo retirado | — | Pre-Iniciar: vuelve a mensaje; dispensando: **auto-Parar** |
| Timeout post-pago | — | 2 min silencioso → cancel → Comprar |
| Countdown / `duration_ms` | Al pago | Al pulsar **Iniciar** |
| Dispensado activo | Igual | Hereda v0-3-3 (Parar 200 ms, terminado al countdown 0) |

## Calibración VL53L0X

```c
#define TERMO_OFFSET_MM 85
#define TERMO_PRESENT_MAX_MM 15
```

`dist_corregida = max(0, raw − 85)` · termo presente si `dist_corregida < 15`.

## Arduino IDE

Abrir: `mate_point_v0-3-4/mate_point_v0-3-4.ino`

| Tools | Valor |
|-------|--------|
| Board | ESP32S3 Dev Module |
| **USB CDC On Boot** | **Disabled** |
| DIP | **UART2** → Nobana |

## Hardware VL53L0X

Bus I2C PH2.0 (GPIO8/9), dirección **0x29**. Ver [`arquitectura-hardware.md`](../../arquitectura-hardware.md) §3.

## Validación banco (Test1 2026-06-17) — E2E OK

- [x] Post-pago: Coloque el termo / Iniciar; MQTT idle hasta Iniciar
- [x] Polling termo + debounce; retiro → mensaje / auto-Parar
- [x] Iniciar → dispensado + countdown desde tap
- [x] Regresión v0-3-3: Parar, fin natural, 2.ª compra
- [~] Timeout 2 min post-pago — no probado Test1
- [~] Fallo sensor desconectado — no probado Test1
