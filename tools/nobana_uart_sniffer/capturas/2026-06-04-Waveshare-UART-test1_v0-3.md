# Captura YYYY-MM-DD — título corto

## Metadatos



## Objetivo

Analizar el comportamiento del wavashare con el mate_point_UART_v0-3 , leyendo el UART con un esp32 sniffer.

## Resultado observable

- Se encendio el ESP32.
- Luego de 1-2seg segundos se prende waveshare.
- El waveshare manda el log registrado.

## Log UART

17:04:05.421 -> [205000] FRAME ARM->NOB len=21 HEX: 4B 40 FF 63 6E 12 2F 00 5B 2E EB 27 D3 72 8E 62 4A 8F 9E 4B E6
17:04:07.528 -> [207116] FRAME ARM->NOB len=34 HEX: 66 85 F2 CE AE 62 CE 4B 04 CC 6F A7 84 52 8C 62 61 CD 02 D2 4D 57 C6 86 51 C2 C6 08 A4 63 42 FF A6 F2
17:04:10.551 -> [210119] FRAME ARM->NOB len=16 HEX: E6 4B 68 8E BF 62 63 10 52 0E CC 0E 77 87 4A F8
17:04:15.724 -> [215299] FRAME ARM->NOB len=9 HEX: 68 01 21 00 00 00 00 00 8A
17:04:18.699 -> [218299] FRAME ARM->NOB len=9 HEX: 68 02 E2 00 00 00 00 55 A1

## Secuencia inferida (opcional)

| Paso | Dir. | Trama / evento |
|------|------|----------------|
| 1 | | |
| 2 | | |

## Notas / anomalías

-

## Conclusión preliminar




