# Validación banco — Waveshare UART v0-3 + Nobana (OK)

## Metadatos

| Campo | Valor |
|-------|--------|
| **Fecha** | 2026-06-04 |
| **Firmware** | [`mate_point_UART_v0-3.ino`](../../../mate_point_firmware/mate_point_UART_v0-3/mate_point_UART_v0-3.ino) (bus limpio: solo `F8` + tramas `0x68` en TX) |
| **Placa** | Waveshare ESP32-S3-Touch-LCD-7B |
| **Nobana** | PCB con agua, ARMOR desconectado |
| **Level shifter** | TXS0108E 3.3 V ↔ 5 V |
| **DIP** | **UART2** (PH2.0 → UART0 GPIO43/44 @ 9600) |
| **Cableado** | Nobana Tx → GPIO44 · Nobana Rx ← GPIO43 · GND común |
| **Resultado** | **Éxito** — ciclo completo W→S→R sin intervención manual |

## Objetivo

Validar en banco que el Waveshare actúa como **maestro Nobana** con el ciclo automático **una pasada por encendido** (wake `F8` → standby poll `21` → dispensado Coffee timer `E2`/`22` sin lock `23`).

## Resultado observable (banco)

- Tras reset del Waveshare (Nobana ya encendida o encendiéndose ~1–2 s después): **secuencia automática completa**.
- **Chimes** y comportamiento del panel Nobana coherentes con dispensado Coffee ~180 ml (referencia §7.6–7.7 PROTOCOLO).
- **Temperatura / flujo** según expectativa del ciclo caliente (T ~85 °C en panel).
- **Fin de ciclo** correcto (cierre sin depender de lock `23`+`d7=55`).
- **Una sola pasada** por encendido; sin segundo dispensado hasta nuevo reset / power-cycle.
- Operador: **funcionó perfecto** (criterio principal de aceptación Etapa 2a-WS).

## Evidencia UART (sniffer, opcional)

Captura TX Waveshare con firmware anterior (debug en Serial): [`2026-06-04-Waveshare-UART-test2_v0-3.md`](2026-06-04-Waveshare-UART-test2_v0-3.md) — tramas `0x68` y tiempos alineados al plan; blobs previos explicados por build con logs @ 115200.

Con firmware **bus limpio**, el sniffer en TX debe mostrar únicamente:

| Fase | TX esperado |
|------|-------------|
| Wake | `F8` ×1 tras ventana escucha |
| Standby | `68 01 21 00 00 00 00 00 8A` (poll ~100 ms) |
| Dispense | `68 02 E2 … 55` → `E2` + `b5=04` → `68 02 22` preset + `22` cierre → cooldown `22` |
| Fin | **Silencio en TX** (standby apagado tras ciclo) |

## Criterios de aceptación (PLAN v0-3 §7)

| ID | Criterio | Estado |
|----|----------|--------|
| W1 | Wake `F8` efectivo (Nobana responde al guion) | [x] |
| S1 | Poll `21` estable antes de dispensar | [x] |
| R1 | Dispensado Coffee ~180 ml; T ~85 °C en Nobana | [x] |
| R2 | Fin sin `23` lock; chime fin | [x] |
| R3 | Evidencia banco (chimes + panel + opcional sniffer) | [x] |
| R4 | Una pasada; sin segundo ciclo hasta reset | [x] |
| R5 | Repetible en nuevo encendido | [x] (misma sesión de prueba exitosa) |

## Secuencia inferida

| Paso | Evento |
|------|--------|
| 1 | Power-on Waveshare, DIP UART2, Nobana en banco |
| 2 | `BOOT_WAIT` 3 s |
| 3 | Wake: escucha 5 s → `F8` → escucha 3 s |
| 4 | Delay 2 s → standby poll `21` (3 s) |
| 5 | Dispense: `E2` café → pre-stop → `22` cierre → cooldown |
| 6 | `DONE` — sin más TX hacia Nobana |

## Notas

- Firmware sin `Serial` de debug: validación **sensorial en Nobana** es el criterio principal; sniffer confirma tramas si se repite prueba.
- Siguiente hito producto: port del driver a [`mate_point_v0-2`](../../../mate_point_firmware/mate_point_v0-2/) (MQTT `dispense` → UART físico) — ver [`PLAN-IMPLEMENTACION.md`](../../../mate_point_firmware/PLAN-IMPLEMENTACION.md) §12.

## Conclusión

**Etapa 2a-WS cerrada en banco (2026-06-04).** El Waveshare con `mate_point_UART_v0-3` controla el Nobana de extremo a extremo con el ciclo automático planificado.
