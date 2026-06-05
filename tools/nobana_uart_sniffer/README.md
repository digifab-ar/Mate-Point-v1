# Nobana UART sniffer (ESP32 NodeMCU 38p)

Sketch de **relevamiento** para capturar tramas entre PCB Nobana y módulo ARMOR.  
**No** es el firmware del Waveshare Mate Point (`mate_point_v0-2`).

## Hardware

| Cable Nobana | TXS0108E | ESP32 NodeMCU |
|--------------|----------|---------------|
| **G** | GND | GND |
| **5V** | VCCB | — (ESP32 por USB) |
| **Tx** | B1 → A1 | **GPIO25** (`Serial2` RX) — NOB→ARM |
| **Rx** | B2 → A2 | **GPIO17** (`Serial1` RX) — ARM→NOB |
| ESP32 **3.3V** | VCCA + OE | — |

ARMOR permanece conectado al cable de 4 pines (sniffer en paralelo).

## Arduino IDE

1. Placa: **ESP32 Dev Module** (o ESP32-WROOM-32)
2. Puerto USB correcto
3. Abrir `nobana_uart_sniffer.ino`
4. Monitor Serie: **115200** + salto de línea **NL** o **CRLF**

## Uso

1. Flashear y abrir Monitor Serie.
2. **Fase R1 — baudrate:** `B9600` luego `T` (15 s) y pulsar botones ARMOR. Si `ARM->NOB` sigue en 0, revisar cableado o probar `S`.
3. El comando `S` mira **ambas** direcciones; los **botones** generan tráfico en **ARM->NOB** (GPIO17 / pin Rx del cable).
4. Fijar baud: `B9600` (o el valor con más bytes en `T` o `S`).
5. **Fase R2 — mapeo:** `M HOT` y pulsar botón → buscar `FRAME ARM->NOB`.

### Formato de log

```
[12345] FRAME ARM->NOB len=4 HEX: AA BB CC DD | ASCII:"...."
[12380] MARK | HOT Coffee
```

- `NOB->ARM` = pin **Tx** del cable (Nobana transmite).
- `ARM->NOB` = pin **Rx** del cable (ARMOR transmite).

### Comandos

| Comando | Acción |
|---------|--------|
| `?` / `h` | Ayuda |
| `S` | Escanear baudrates (ambas direcciones) |
| `T` | Test 15 s: contador bytes por dirección |
| `B9600` | Fijar baudrate del bus |
| `V` | Alternar verbose (todas las FRAME) / solo cambios |
| `M texto` | Marca manual en el log |

Por defecto solo imprime **FRAME** cuando cambia el contenido (ignora byte `seq` y checksum). Comando **`V`** para ver todas las tramas.

## Capturas anotadas (fuente de verdad)

Guardar cada sesión de banco como `.md` en [`capturas/`](capturas/README.md):

1. Copiar [`capturas/_plantilla.md`](capturas/_plantilla.md) con nombre `YYYY-MM-DD-…`.
2. Completar metadatos, resultado observable y log UART.
3. Añadir fila al índice en [`capturas/README.md`](capturas/README.md).

A partir de esas capturas se actualiza
[`PROTOCOLO-UART-NOBANA.md`](../../mate_point_firmware/PROTOCOLO-UART-NOBANA.md)
y la secuencia de [`mate_point_UART_v0-1`](../../mate_point_firmware/mate_point_UART_v0-1/).

## Ajustes

Editar el bloque **Configuración** al inicio de `nobana_uart_sniffer.ino` (pines, baud, `FRAME_GAP_MS`).
