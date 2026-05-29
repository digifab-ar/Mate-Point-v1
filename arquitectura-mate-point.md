# Arquitectura Mate Point — Firmware ESP32-S3

**Proyecto:** Mate Point — Dispensador de agua caliente  
**OT:** OT-00268 — Etapa 3  
**Última actualización:** 2026-05-27

---

## 1. Diseño de UI — Pantallas y estados visuales

La pantalla de 1024×600 px permite una UI generosa. Se propone una **máquina de estados** con 5 pantallas principales.

### 1.1 Flujo de pantallas

```
┌──────────────────────────────────────────┐
│               IDLE (reposo)              │
│  Logo MATE POINT   │  Precio: $XXX       │
│  "Tocá para pedir" │  Agua caliente      │
└──────────────────────────────────────────┘
             ↓ Toque / acción
┌──────────────────────────────────────────┐
│            REQUESTING (espera)           │
│       ⏳  Generando QR de pago...        │
│       Conectando con Mercado Pago        │
└──────────────────────────────────────────┘
             ↓ QR recibido (MQTT)
┌──────────────────────────────────────────┐
│              QR_SHOW (pago)              │
│  ┌────────┐  Escaneá con                 │
│  │  QR    │  Mercado Pago                │
│  │  320px │  💰 $XXX                     │
│  └────────┘  ⏱ Vence en: 9:59           │
└──────────────────────────────────────────┘
             ↓ Webhook → MQTT command
┌──────────────────────────────────────────┐
│               DISPENSING (ok)            │
│           ✅ ¡Pago exitoso!              │
│       🫖  Preparando tu mate...          │
│       ████████░░  (barra de progreso)    │
└──────────────────────────────────────────┘
             ↓ Timeout / error
┌──────────────────────────────────────────┐
│                ERROR                     │
│    ❌  Ocurrió un problema               │
│    Código: XXXX                          │
│    [Reintentar]   [Cancelar]             │
└──────────────────────────────────────────┘
```

### 1.2 Paleta de colores

| Elemento | Color | HEX |
|----------|-------|-----|
| Fondo principal | Blanco cálido | `#FAFAF8` |
| Acento principal | Verde Mercado Pago | `#00A650` |
| Texto primario | Gris oscuro | `#1A1A1A` |
| QR — módulos | Negro | `#000000` |
| QR — fondo | Blanco | `#FFFFFF` |
| Alerta / error | Rojo | `#E63946` |
| Dispensando | Azul suave | `#457B9D` |

### 1.3 Layout de la pantalla QR (1024×600)

```
┌─────────────────────────────────────────────────────────────────────────────┐
│  [Logo 180×60]          MATE POINT                    ⏱ 09:59              │  ← Header 70px
├─────────────────────────────────────────────────────────────────────────────┤
│                                                                             │
│   ┌──────────────────────┐     Escaneá el QR con                          │
│   │                      │     Mercado Pago para pagar                    │
│   │   QR 320×320 px      │                                                │
│   │  (lv_qrcode widget)  │     💰  $ 1.000,00                             │  ← Body 470px
│   │                      │                                                │
│   └──────────────────────┘     Agua caliente — 1 porción                  │
│                                                                             │
│                                [Cancelar]                                  │
├─────────────────────────────────────────────────────────────────────────────┤
│  WiFi ●   MQTT ●   v1.0.0                                                  │  ← Footer 60px
└─────────────────────────────────────────────────────────────────────────────┘
```

---

## 2. Máquina de estados en firmware

### 2.1 Definición de estados

```c
typedef enum {
    STATE_IDLE,
    STATE_REQUESTING,   // HTTP al backend → espera QR
    STATE_QR_SHOW,      // Mostrando QR, countdown activo
    STATE_DISPENSING,   // Pago confirmado, relé activo
    STATE_ERROR
} machine_state_t;
```

### 2.2 Transiciones

```
IDLE ──[toque]──────────────────────→ REQUESTING
REQUESTING ──[qr_data recibido]────→ QR_SHOW
REQUESTING ──[timeout/error HTTP]──→ ERROR
QR_SHOW ──[MQTT "dispense"]────────→ DISPENSING  ← envía comando UART al PCB Nobana
QR_SHOW ──[countdown = 0]──────────→ ERROR (QR expirado)
QR_SHOW ──[toque "cancelar"]───────→ IDLE
DISPENSING ──[duration_ms elapsed]─→ IDLE         ← detiene comando UART al PCB Nobana
ERROR ──[toque "reintentar"]───────→ REQUESTING
ERROR ──[toque "cancelar"]─────────→ IDLE
```

> La transición hacia `DISPENSING` y la salida del mismo implican enviar comandos UART al PCB del dispensador Nobana. Ver `arquitectura-hardware.md` §2.3.

---

## 3. Configuración Wi-Fi (v1)

La configuración de red en la **primera versión** se realiza exclusivamente por **USB Type-C** y **monitor serie** (Arduino IDE u otra terminal). No hay portal cautivo ni formulario en la pantalla táctil.

| Parámetro | Valor |
|-----------|-------|
| Interfaz | USB Type-C → UART de programación/debug del módulo Waveshare |
| Baudrate | **115200** |
| Timeout de conexión | **15 s** por intento |
| Persistencia | SSID y contraseña en **NVS** solo tras validar conexión exitosa |
| Contraseña en terminal | **Visible** en el monitor serie (sin enmascarado) |

### 3.1 Monitor serie — estado continuo

Desde el arranque, el firmware imprime en serie el estado de Wi-Fi y los **reintentos** de conexión (cuando hay credenciales en NVS o tras un comando `wifi`):

```
WiFi: desconectado — reintento 1/5...
WiFi: desconectado — reintento 2/5...
WiFi: conectado — SSID "MiLocal_2.4G"  IP 192.168.1.42  RSSI -58 dBm
```

Si no hay credenciales válidas en NVS, se indica que hay que ejecutar el comando `wifi`.

### 3.2 Comando `wifi`

El comando **`wifi`** está **siempre disponible** (también con credenciales ya guardadas), para cambiar de red sin re-flashear el firmware.

Flujo interactivo:

```
> wifi
Red: MiLocal_2.4G
Password: mi_clave_secreta
WiFi: conectando — intento 1...
WiFi: conectando — intento 2...
WiFi: conectado — red: MiLocal_2.4G  IP 192.168.1.42  RSSI -58 dBm
MQTT: conectado
Credenciales guardadas en NVS.
```

| Paso | Comportamiento |
|------|----------------|
| 1 | Solicita **nombre de red** (`Red:`). **Enter** confirma y pasa al siguiente paso (sin prompt adicional). |
| 2 | Solicita **contraseña** (`Password:`), visible en el monitor serie. **Enter** confirma e inicia la conexión. |
| 3 | Intenta conectar a Wi-Fi; en serie se muestran mensajes **conectando** y **reintentos** hasta éxito o timeout. |
| 4 | Si la conexión Wi-Fi es exitiva, intenta **MQTT** y reporta `MQTT: conectado` o el error correspondiente. |
| 5 | **Solo si** Wi-Fi (y la validación definida en firmware) fue exitosa: **escribe SSID y contraseña en NVS**. Si falla (SSID inexistente, contraseña incorrecta, timeout 15 s), **no** se actualiza NVS y se mantienen las credenciales anteriores si existían. |

### 3.3 Timeouts y reintentos

- Cada intento de asociación a la red configurada tiene un **timeout de 15 segundos**.
- Los reintentos durante `wifi` y en arranque normal se loguean en serie con numeración (`intento 1`, `intento 2`, …).
- Tras agotar reintentos en el flujo `wifi`, mensaje de error en serie; el operador puede volver a ejecutar `wifi`.

### 3.4 Relación con la UI

- El **footer** de pantalla (WiFi ● / MQTT ●) refleja el mismo estado que el monitor serie, para operación en campo sin laptop.
- La configuración de red **no** usa pantallas LVGL; la pantalla puede mostrar un mensaje genérico de “sin red” si no hay conexión, sin teclado de SSID/clave.

### 3.5 Evolución futura (fuera de v1)

Alternativas documentadas para versiones posteriores: portal HTML vía SoftAP desde celular, o herramienta `nvs_partition_gen` en fábrica. La v1 no las implementa.

---

## 4. Topics MQTT

Extiende la definición base de `integracion-mercadopago-qr.md` (§8).

> **POC Fase 4 (2026-05-29):** implementados `mate/{device_id}/command` y `mate/{device_id}/status` en firmware [`mate_point_v0-1`](../mate_point_firmware/mate_point_v0-1/). Contrato operativo: [`servidor-mate-point.md`](servidor-mate-point.md) §9 · [`mate_point_firmware/PLAN-IMPLEMENTACION.md`](../mate_point_firmware/PLAN-IMPLEMENTACION.md).

### 4.1 Tabla de topics

**En uso (POC y servidor):**

| Topic | Dirección | Payload relevante |
|-------|-----------|-------------------|
| `mate/{device_id}/command` | Servidor → ESP32 | `cmd: "dispense"`, `duration_ms`, `order_id` |
| `mate/{device_id}/status` | ESP32 → broker | `state`, `device_id`, `uptime_ms`, `wifi_rssi` |

**Planificados (Fase 5 — prefijo legacy `matepoint/` en diseño original):**

| Topic | Dirección | Payload relevante |
|-------|-----------|-------------------|
| `matepoint/{device_id}/qr_show` | Servidor → ESP32 | `qr_data`, `amount`, `expiry_ms` |
| `matepoint/{device_id}/cancel` | Servidor → ESP32 | — (cancela QR activo) |

### 4.2 Payload `qr_show`

```json
{
  "qr_data": "00020101021243650016COM.MERCADOLIBRE02133420512522...",
  "amount": "1000.00",
  "currency": "ARS",
  "description": "Agua caliente - 1 porcion",
  "expiry_ms": 600000,
  "order_id": "12345678"
}
```

### 4.3 Payload `status` (ESP32 → broker)

**POC v0.1** (`idle` / `dispensing`):

```json
{
  "device_id": "MATEPOINT001",
  "state": "dispensing",
  "ts": 1748368961000,
  "uptime_ms": 123456,
  "wifi_rssi": -65,
  "mqtt_connected": true,
  "order_id": "ORDTST01..."
}
```

**Fase 5 (referencia):**

```json
{
  "device_id": "MATEPOINT001",
  "state": "QR_SHOW",
  "uptime_ms": 123456,
  "wifi_rssi": -65,
  "mqtt_connected": true
}
```

---

## 5. Estructura de archivos — firmware

**Implementado (POC v0.1):**

```
mate_point_firmware/
├── PLAN-IMPLEMENTACION.md
├── README.md
├── reference/                 ← demo 13 Waveshare (backup)
└── mate_point_v0-1/           ← abrir en Arduino IDE
    ├── mate_point_v0-1.ino
    ├── config.h
    ├── display_ui.h / .cpp
    ├── dispense_sim.h / .cpp
    ├── mate_network.h / .cpp
    └── [port Waveshare: lvgl_port, rgb_lcd, gt911, …]
```

**Planificado (Fase 5 — referencia):**

```
mate_point_display/
├── lv_conf.h
├── src/
│   ├── config.h
│   ├── state_machine.cpp
│   ├── mqtt_client.cpp
│   └── ui/
│       ├── screen_idle.cpp
│       ├── screen_qr.cpp
│       └── …
```

**Dependencias** (ver también `modulo-waveshare-esp32s3-touch-7b.md` §3.3 y `arquitectura-hardware.md` §3):

```
ESP32_Display_Panel
ESP32_IO_Expander
lvgl (v8.3.x)
lv_lib_qrcode
ArduinoJson        ← parseo de payloads MQTT
PubSubClient       ← cliente MQTT (o AsyncMQTT)
```

---

## 6. Historial

| Fecha | Cambio |
|-------|--------|
| 2026-05-22 | Documento creado a partir de la reorganización de `modulo-waveshare-esp32s3-touch-7b.md` |
| 2026-05-26 | Referencia agregada al dispensador Nobana (`dispensador-nobana.md`) como base de hardware |
| 2026-05-27 | Referencias actualizadas: §2.2 apunta a `arquitectura-hardware.md` §2.3 para comandos UART. Dependencias §5 actualizadas con nuevo `arquitectura-hardware.md` |
| 2026-05-27 | §3 Configuración Wi-Fi v1: USB-C + monitor serie 115200, comando `wifi`, NVS tras validar, timeout 15 s, logs de reintentos, MQTT al conectar |
| 2026-05-27 | §3.2: Red y Password se confirman solo con Enter (sin paso `¿Confirmar?`) |
| 2026-05-29 | §4 topics: separado POC (`mate/…`) vs Fase 5 planificado (`matepoint/…`); §5 apunta a [`mate_point_firmware/mate_point_v0-1/`](../mate_point_firmware/mate_point_v0-1/) |
