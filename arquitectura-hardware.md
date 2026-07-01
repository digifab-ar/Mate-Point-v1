# Arquitectura de Hardware — Mate Point

**Proyecto:** Mate Point — Dispensador de agua caliente  
**OT:** OT-00268 — Etapa 3  
**Última actualización:** 2026-06-24

---

## 1. Visión general del prototipo

```
┌──────────────────────────────────────────────────────────────┐
│                    PROTOTIPO MATE POINT                      │
│                                                              │
│  ┌───────────────────┐                                       │
│  │  Waveshare        │  ← UI + lógica de negocio            │
│  │  ESP32-S3-Touch   │    (pantalla QR, MP, MQTT)           │
│  │  LCD-7B           │                                       │
│  └──┬──────┬──────┬──┘                                       │
│     │UART2 │ I2C  │ GP6                                      │
│     │      │      │                                          │
│     ▼      │      └──────────────────────────────────┐       │
│  ┌──────────────┐                           ┌────────▼─────┐ │
│  │  TXS0108E    │  ← level shift 3.3V↔5V   │ Sensor nivel │ │
│  │  (3.3V↔5V)   │                           │ recipiente   │ │
│  └──────┬───────┘                           │ de goteo     │ │
│         │ UART 5V                           │ (reed NO)    │ │
│  ┌──────▼───────┐                           └──────────────┘ │
│  │  PCB Nobana  │  ← ctrl electrónica                        │
│  │ (dispensador)│    calentamiento y dispensado              │
│  └──────────────┘                                            │
│                      I2C                                     │
│                  ┌──────────────────────┐                    │
│                  │ VL53L0X (0x29)       │  ← proximidad      │
│                  └──────────────────────┘                    │
└──────────────────────────────────────────────────────────────┘
```

El módulo Waveshare **reemplaza** el panel de control original (display + botones) tomando control total del dispensador. La comunicación UART pasa por un **TXS0108E** que adapta los niveles 3.3 V del ESP32 a los 5 V de la PCB Nobana. El **VL53L0X** conectado por I2C detecta la presencia del recipiente bajo el dispensador. El GPIO GP6 monitorea el **sensor de nivel del recipiente de goteo**.

---

## 2. Integración Waveshare ↔ PCB Nobana

### 2.1 Estrategia de conexión

El módulo Waveshare ESP32-S3 se conecta al **conector UART original** del panel de control del dispensador (el conector al que se enchufaba el display + botones de fábrica).

```
Panel original (display 888 + botones)
         ↕  UART
    PCB Nobana
```

Se convierte en:

```
Waveshare ESP32-S3
         ↕  UART
    PCB Nobana
```

### 2.2 Interfaz UART — Level shifting

El módulo Waveshare ESP32-S3-Touch-LCD-7B expone un conector **UART2 TTL a 3.3 V** en conector PH2.0 (confirmado en inspección física del PCB, 2026-05-27):

| Conector | Pines | Nivel lógico | Uso |
|----------|-------|--------------|-----|
| PH2.0 UART2 | 3V3, GND, RX, TX | **3.3 V TTL** | Comunicación con PCB Nobana |

El PCB Nobana opera a **5 V TTL**. La adaptación de niveles se resuelve con un **TXS0108E** (level shifter bidireccional 3.3 V ↔ 5 V):

```
Waveshare                TXS0108E              PCB Nobana
UART2 TX (3.3V) ──── A1 → B1 ──────────────── RX (5V)
UART2 RX (3.3V) ──── A2 ← B2 ──────────────── TX (5V)
3V3  ─────────── VCCA        VCCB ──────────── 5V
GND  ─────────── GND          GND ──────────── GND
3V3  ─────────── OE
```

> El conector RS485 (PH2.0) queda disponible como alternativa si a futuro se requiere mayor robustez ante ruido o distancias largas.

### 2.3 Comandos a implementar

Con el protocolo UART identificado (ver `plan-de-implementacion.md` §Relevamiento UART), el firmware del Waveshare deberá enviar al menos:

| Acción Mate Point | Comando UART requerido |
|-------------------|----------------------|
| Dispensar agua caliente | Equivalente a botón HOT pulsado |
| Detener dispensado | Equivalente a soltar botón / comando stop |
| Consultar temperatura | Leer valor del display 888 |
| Encender / activar | Equivalente a POWER ON (si aplica) |

---

## 3. Asignación de interfaces — Sistema Mate Point

### 3.1 Mapa de conexiones definitivo

Con **GP6 como único GPIO libre**, la asignación de sensores y periféricos queda:

| Interfaz | Asignación | Componente | Notas |
|----------|-----------|------------|-------|
| **UART2** (PH2.0) | PCB Nobana | — + TXS0108E | Comunicación principal con el dispensador |
| **GPIO GP6** (PH2.0) | Sensor bandeja de goteo | Flotante + **reed NO** | Digital — **HIGH** = seguro · **LOW** = llena |
| **I2C** (PH2.0) | Sensor de proximidad | VL53L0X (0x29) | ToF, detecta recipiente bajo el dispensador |
| **I2C** (PH2.0) | GT911 táctil | — | Integrado, no liberar |
| **I2C** (PH2.0) | CH422G IO expander | — | Integrado, no liberar |
| **RS-485** (PH2.0) | Reserva | — | No asignado |
| **CAN** (PH2.0) | Reserva | — | No asignado |
| **Temp. calefactor** | Vía UART Nobana | — | Leer del protocolo PCB Nobana (ver relevamiento en `plan-de-implementacion.md`) |
| **Temp. agua entrada** | Vía UART Nobana | — | Leer del protocolo PCB Nobana (ver relevamiento en `plan-de-implementacion.md`) |

```
Waveshare ESP32-S3-Touch-7B
│
├── UART2 PH2.0 ─── TXS0108E ─────────────── PCB Nobana (UART 5V)
│                                             └── temp. calefactor (vía protocolo)
│                                             └── temp. agua entrada (vía protocolo)
│
├── I2C PH2.0 ──┬── GT911  (0x14/0x5D)      ← táctil (integrado)
│               ├── CH422G (0x24)            ← IO expander (integrado)
│               └── VL53L0X (0x29)           ← sensor de proximidad
│
├── GPIO PH2.0 ──┬── 3V3 ──[10 kΩ]──┬── GP6 (GPIO_NUM_6)   ← sensor bandeja (§3.4)
│               │                  └── reed NO ── GND
│               └── GND
│
├── RS-485 PH2.0 ─────────────────────────── [reserva]
└── CAN PH2.0 ────────────────────────────── [reserva]
```

└── CAN PH2.0 ────────────────────────────── [reserva]
```

### 3.2 Consideraciones I2C

Con el VL53L0X agregado, el bus I2C tiene tres dispositivos:

| Dirección | Dispositivo | Origen |
|-----------|-------------|--------|
| 0x14 / 0x5D | GT911 (táctil) | Integrado |
| 0x24 | CH422G (IO expander) | Integrado |
| 0x29 | VL53L0X (proximidad) | Externo — sin conflicto |

No hay colisiones. El cableado externo debe ser corto (<30 cm) y usar las resistencias pull-up ya presentes en el módulo para el táctil.

### 3.3 Expansión futura de GPIOs

Si en el futuro se requieren señales digitales adicionales (botones físicos de emergencia, indicadores LED, etc.), agregar un **PCF8574A** al bus I2C:
- Dirección configurable: 0x38–0x3F (sin conflicto con ningún dispositivo actual)
- 8 GPIOs adicionales a 3.3V (o 5V según VCC del chip)
- Sin cambios de hardware en el módulo Waveshare

### 3.4 Sensor bandeja de goteo — reed switch @ GPIO6

Montaje validado en prototipo (2026-06-24). Esquema: [`docs/hardware/sensor-bandeja-reed-gpio6.png`](docs/hardware/sensor-bandeja-reed-gpio6.png).

| Ítem | Valor |
|------|--------|
| Conector Waveshare | PH2.0 **GPIO** — pines **3V3**, **GND**, **GP6** |
| Pin firmware | `GPIO_NUM_6` |
| Sensor | Flotante con **reed switch normalmente abierto (NO)** |
| Pull-up | Resistencia **10 kΩ** entre **3V3** y nodo **GPIO6** (externa al módulo) |
| Reed | Un terminal al nodo GPIO6; el otro a **GND** |
| Imán | Integrado en el flotante — al subir cierra el reed |

**Cableado (pull-up externo):**

```
3V3 ──[10 kΩ]──┬── GPIO6 (GP6)     ← nodo leído por firmware
               │
               └── reed NO ── GND
```

| Condición física | Reed | GPIO6 |
|------------------|------|-------|
| Bandeja **segura** (flotante abajo, poco agua) | **Abierto** | **HIGH** (3,3 V vía 10 kΩ) |
| Bandeja **llena** (flotante arriba, imán activo) | **Cerrado** | **LOW** (derivado a GND) |

Firmware v0-5: poll cada **5 s**; `DRIP_TRAY_FULL_LEVEL = LOW`. Ver [`mate_point_firmware/PLAN-MATE-POINT-v0-5.md`](mate_point_firmware/PLAN-MATE-POINT-v0-5.md).

> El reed **NO** implica circuito abierto en reposo (bandeja segura). No confundir con el sensor de **tanque de agua** del Nobana, que se lee por **UART** (`b2` / byte 7 en `PROTOCOLO-UART-NOBANA.md` §4.4).

---

## 4. Relación con otros documentos

| Documento | Relación |
|-----------|----------|
| `dispensador-nobana.md` | Especificaciones del hardware Nobana (producto, PCB, panel original) |
| `modulo-waveshare-esp32s3-touch-7b.md` | Especificaciones del módulo ESP32-S3 (pinout, entorno de desarrollo) |
| `arquitectura-mate-point.md` | Máquina de estados del firmware; el estado `DISPENSING` se materializa enviando el comando UART al Nobana (ver §2.3) |
| `plan-de-implementacion.md` | Plan de fases; incluye el relevamiento UART del Nobana como prerrequisito de Fase 4 |
| `mate_point_firmware/PLAN-MATE-POINT-v0-5.md` | Firmware sensor bandeja — poll, UI error, auto-Parar |
| `docs/hardware/sensor-bandeja-reed-gpio6.png` | Esquema cableado reed + pull-up 10 kΩ @ GPIO6 |

---

## 5. Historial

| Fecha | Cambio |
|-------|--------|
| 2026-06-24 | §3.4 — Cableado validado reed NO @ GPIO6: pull-up **10 kΩ** a 3V3, reed a GND; **HIGH** = seguro, **LOW** = llena; esquema en `docs/hardware/` |
| 2026-05-27 | Documento creado — consolidación de arquitectura hardware desde `dispensador-nobana.md` y `modulo-waveshare-esp32s3-touch-7b.md` |
| 2026-05-27 | §1 — Diagrama de visión general expandido: TXS0108E (level shift UART), VL53L0X (I2C proximidad), sensor de nivel recipiente de goteo (GP6) |
