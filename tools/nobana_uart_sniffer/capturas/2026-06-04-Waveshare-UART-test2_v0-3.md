# Captura sniffer — Waveshare v0-3 test2 (pre-bus-limpio)

> **Validación banco cerrada:** ver [`2026-06-04-Waveshare-UART-v0-3_banco-validacion-OK.md`](2026-06-04-Waveshare-UART-v0-3_banco-validacion-OK.md) (prueba exitosa en Nobana con firmware bus limpio).

## Metadatos

| Campo | Valor |
|-------|--------|
| Fecha | 2026-06-04 |
| Firmware | `mate_point_UART_v0-3` (build con debug Serial @ 115200) |
| Sniffer | ESP32 `nobana_uart_sniffer.ino`, solo TX Waveshare |

## Objetivo

Analizar el comportamiento del Waveshare con `mate_point_UART_v0-3`, leyendo el UART con un ESP32 sniffer.

## Resultado observable

- Se encendio el ESP32.
- Luego de 1-2seg segundos se prende waveshare.
- El waveshare manda el log registrado.

## Log UART

17:28:25.849 -> [41693] FRAME ARM->NOB len=20 HEX: 4B 48 31 4A C7 AE 72 A5 DE 72 4E FB 72 8E 62 4A 8F 9E 4B E6
17:28:27.962 -> [43810] FRAME ARM->NOB len=35 HEX: 66 8D EA CE A6 62 C6 4B 04 CC 6F AF 84 52 8C 62 69 CD 12 08 C5 8D EB 6A AC 73 09 57 87 4B C5 4D 4A 43 B2
17:28:30.925 -> [46784] FRAME ARM->NOB len=10 HEX: C4 CE AC C0 00 CE 5F A0 E3 E1
17:28:36.137 -> [51965] FRAME ARM->NOB len=9 HEX: 68 01 21 00 00 00 00 00 8A
17:28:39.119 -> [54965] FRAME ARM->NOB len=9 HEX: 68 02 E2 00 00 00 00 55 A1
17:29:03.130 -> [78974] FRAME ARM->NOB len=18 HEX: 68 02 E2 00 00 00 00 55 A1 68 02 E2 00 00 04 00 55 A5
17:29:03.222 -> [79066] FRAME ARM->NOB len=9 HEX: 68 02 E2 00 00 04 00 55 A5
17:29:07.022 -> [82874] FRAME ARM->NOB len=18 HEX: 68 02 22 00 00 04 00 55 E5 68 02 22 00 00 00 00 55 E1
17:29:07.123 -> [82966] FRAME ARM->NOB len=9 HEX: 68 02 22 00 00 00 00 55 E1
17:29:30.010 -> [105864] FRAME ARM->NOB len=9 HEX: 68 01 21 00 00 00 00 00 8A

## Secuencia inferida (opcional)

| Paso | Dir. | Trama / evento |
|------|------|----------------|
| 1 | | |
| 2 | | |

## Notas / anomalías

-

## Conclusión preliminar

Tramas `0x68` y tiempos coherentes con el ciclo auto. Blobs iniciales atribuibles a debug en el mismo `Serial` @ 115200 (corregido en firmware bus limpio). **Banco final:** documento de validación OK enlazado arriba.

