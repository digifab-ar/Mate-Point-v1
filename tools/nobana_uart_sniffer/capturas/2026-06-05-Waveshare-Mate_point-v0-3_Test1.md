# Captura banco — Mate Point producto v0-3 + Nobana (Test1 E2E)

## Metadatos

| Campo | Valor |
|-------|--------|
| **Fecha** | 2026-06-05 |
| **Firmware** | [`mate_point_v0-3.ino`](../../../mate_point_firmware/mate_point_v0-3/mate_point_v0-3.ino) |
| **Placa** | Waveshare ESP32-S3-Touch-LCD-7B |
| **Nobana** | PCB con agua, ARMOR desconectado |
| **Level shifter** | TXS0108E 3.3 V ↔ 5 V |
| **DIP** | **UART2** (PH2.0 → GPIO43/44 @ 9600) |
| **Cableado** | Nobana Tx → GPIO44 · Nobana Rx ← GPIO43 · GND común |
| **Sniffer** | No usado en esta sesión |
| **Pago** | Mercado Pago sandbox (QR real) |
| **Resultado** | **E2E parcial** — integración UI+MQTT+Nobana OK; UI fin de ciclo con **Error dispensado**; sniffer y cierre formal §11 pendientes |

## Objetivo

Validar [`mate_point_v0-3`](../../../mate_point_firmware/mate_point_v0-3/) en Waveshare como **producto**: wake inicial al boot (antes de LVGL), standby **`S`** (poll `21`), dispensado **`R`** Coffee (`E2`/`22`, sin lock `23`) disparado por MQTT tras pago, y **segunda compra** en el mismo encendido. Ver [`PLAN-MATE-POINT-v0-3.md`](../../../mate_point_firmware/PLAN-MATE-POINT-v0-3.md) §11.

## Resultado observable (banco)

- Tras encender Nobana y Waveshare, ~**14 s**: chime Nobana y pantalla **Comprar**; Wi‑Fi y MQTT aún sin conexión (coherente con wake bloqueante pre-LVGL).
- **1–2 s** después: Wi‑Fi y MQTT conectados (footer).
- **Comprar** → **Creando orden…** → QR.
- Pago vía app Mercado Pago (QR escaneado).
- Tras el pago: pantalla **Dispensado** e **inicio de dispensado físico** en Nobana (chimes/flujo normales).
- ~**30 s** desde inicio dispensado: chime y **corte del flujo** en Nobana (coherente con fin fase caliente + pre-stop del driver, ~24 s + ~3,9 s).
- Pantalla **Error dispensado** (~30 s; posible watchdog `duration_ms` — ver Notas).
- ~**20 s** después: chime, pantalla **Listo**, luego **Comprar**.
- **Segunda compra**: se repite el mismo flujo (Comprar → QR → pago → dispensado → error → listo).

## Evidencia UART (sniffer)

**No registrada en Test1.** Validación Nobana/UART en esta sesión fue **sensorial** (chimes, panel, flujo) y por comportamiento de UI.

| Fase | TX esperado (producto v0-3) | Test1 |
|------|-----------------------------|-------|
| Boot (pre-UI) | `F8` ×1 tras ventanas RX | [ ] sin sniffer — chime ~14 s sugiere wake OK |
| Standby (`COMPRAR` / QR) | `68 01 21 00 00 00 00 00 8A` ~100 ms | [ ] pendiente |
| Dispense (MQTT `dispense`) | `68 02 E2 … 55` → pre-stop → `22` → cooldown `22` | [ ] pendiente — dispensado físico observado |
| Post-ciclo | Poll `21` de nuevo en **Comprar** | [ ] pendiente |

**Referencia POC (bus limpio, 2026-06-04):** [`2026-06-04-Waveshare-UART-v0-3_banco-validacion-OK.md`](2026-06-04-Waveshare-UART-v0-3_banco-validacion-OK.md).

**Próxima captura:** repetir Test1 con `nobana_uart_sniffer.ino` en TX Waveshare; pegar log en esta sección o en `Test1-sniffer.md`.

## Criterios de aceptación (PLAN v0-3 §11)

### UI / red

| ID | Criterio | Test1 |
|----|----------|-------|
| U1 | Comprar, QR, countdown | [x] |
| U2 | E2E pago → **Dispensado** → **terminado** → Listo → Comprar | [~] Dispensado y Listo OK; **terminado** no visto — aparece **Error dispensado** |
| U3 | Wi‑Fi/MQTT en footer (sin Serial) | [x] |
| U4 | Dos ciclos Comprar→pago→dispensado en un encendido | [x] segundo ciclo iniciado con mismo patrón |

### Nobana

| ID | Criterio | Test1 |
|----|----------|-------|
| N1 | Wake antes de LVGL; **`S`** en inicio y durante QR | [~] wake indirecto (~14 s, sin “Error Nobana”); poll `21` sin sniffer |
| N2 | MQTT `dispense` → **`R`** Coffee | [x] dispensado físico tras pago |
| N3 | Bus solo protocolo (sniffer) | [ ] no medido |
| N4 | Tras ciclo: **`S`** de nuevo en Comprar | [ ] no medido |

### Bus limpio

| ID | Criterio | Test1 |
|----|----------|-------|
| B1 | Sin logs firmware en USB / UART Nobana | [ ] no medido (USB CDC off; sniffer pendiente) |

### Fuera de alcance v0-3.0

Cancelar en UI (P1), `duration_ms` como volumen UART, lock `23` — no evaluados.

## Secuencia inferida

Tiempos aproximados desde power-on o desde MQTT `dispense` según contexto.

| Paso | Tiempo (aprox.) | Evento |
|------|-----------------|--------|
| 1 | 0 s | Power-on Nobana + Waveshare (DIP UART2) |
| 2 | 0–3 s | `nobana_product_init`: settle + boot wait |
| 3 | 3–11 s | Handshake wake: RX 5 s → `F8` → post-F8 3 s (bloqueante, sin LVGL) |
| 4 | ~11–14 s | LVGL + UI **Comprar**; standby **`S`** en `loop` |
| 5 | +1–2 s | Wi‑Fi + MQTT |
| 6 | — | **Comprar** → HTTP orden → **QR_SHOW** (`S` activo) |
| 7 | — | Pago MP → servidor publica `dispense` |
| 8 | t=0 disp. | UI **Dispensado** + **`R`**: `E2` café (~24 s o progress≥155) |
| 9 | ~24–28 s | Pre-stop `E2`+`b5=04` → `22` cierre → cooldown (~15 s) |
| 10 | ~30 s | Corte físico + chime; UI **Error dispensado** si watchdog `duration_ms` venció con Nobana aún busy |
| 11 | ~45–50 s | Fin cooldown Nobana → UI **Listo** → **Comprar** |
| 12 | — | Segundo ciclo: pasos 6–11 repetidos (mismo patrón) |

**Ciclo Nobana esperado (constantes firmware):** ~45 s totales (`DISPENSE_T_DISPENSE_MS` 24 s + pre-stop 3,9 s + cierre 2–8 s + cooldown 15 s). Ver [`nobana_uart.cpp`](../../../mate_point_firmware/mate_point_v0-3/nobana_uart.cpp).

## Notas

- **Error dispensado:** generado por `dispense_controller` cuando `millis() >= watchdog_end_ms` y Nobana sigue en ciclo **`R`** (`duration_ms` del payload MQTT). El watchdog **no aborta** UART; Nobana completa cooldown y luego UI pasa a **Listo** (comportamiento §8 del plan).
- **Hipótesis Test1:** disparo del watchdog ~30 s sugiere revisar `duration_ms` en `mate/{device}/command` al momento del pago (servidor default **120000** en `DISPENSE_DURATION_MS`; si el payload fuera ~30000, el error a ~30 s es esperado).
- **Éxito parcial:** primera validación **producto** con pago real + dispensado físico + **doble compra** en un boot; distinto del POC UART (2026-06-04) que no tenía UI/MQTT.
- **Pendiente Test2:** sniffer TX + confirmar solo `F8`/`0x68`; USB silencio; payload MQTT archivado; comprobar poll `21` tras volver a **Comprar**; camino feliz sin **Error dispensado** (confirmar `duration_ms` ≥ ~60000 o 120000).

## Conclusión

**Test1 (2026-06-05) cierra la integración E2E básica del producto v0-3 en banco:** arranque con Nobana antes de UI, flujo Comprar→QR→pago→dispensado físico, recuperación UI hasta **Listo**/**Comprar**, y repetición de compra en el mismo encendido.

**No cierra** la validación formal del plan: bus limpio por sniffer, standby **`S`** verificable, fin de ciclo UI **terminado** (sin watchdog), y documentación UART. Siguiente paso: **Test1b** con sniffer + registro de `duration_ms` MQTT.
