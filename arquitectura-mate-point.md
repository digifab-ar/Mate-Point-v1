# Arquitectura Mate Point — Firmware ESP32-S3

**Proyecto:** Mate Point — Dispensador de agua caliente  
**OT:** OT-00268 — Etapa 3  
**Última actualización:** 2026-05-22

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

## 3. Topics MQTT

Extiende la definición base de `integracion-mercadopago-qr.md` (§8).

### 3.1 Tabla de topics

| Topic | Dirección | Payload relevante |
|-------|-----------|-------------------|
| `matepoint/{device_id}/qr_show` | Servidor → ESP32 | `qr_data`, `amount`, `expiry_ms` |
| `matepoint/{device_id}/command` | Servidor → ESP32 | `cmd: "dispense"`, `duration_ms` |
| `matepoint/{device_id}/status` | ESP32 → Servidor | `state`, `uptime_ms`, `wifi_rssi` |
| `matepoint/{device_id}/cancel` | Servidor → ESP32 | — (cancela QR activo) |

### 3.2 Payload `qr_show`

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

### 3.3 Payload `status` (ESP32 → Servidor)

```json
{
  "device_id": "matepoint-001",
  "state": "QR_SHOW",
  "uptime_ms": 123456,
  "wifi_rssi": -65,
  "mqtt_connected": true
}
```

---

## 4. Estructura de archivos — Arduino / PlatformIO

```
mate_point_display/
├── platformio.ini              ← o sketch .ino para Arduino
├── lv_conf.h                   ← config LVGL (resolución, memoria)
├── src/
│   ├── main.cpp
│   ├── config.h                ← SSID, broker, device_id (desde NVS)
│   ├── state_machine.cpp       ← lógica de estados
│   ├── wifi_manager.cpp        ← conexión / reconexión Wi-Fi
│   ├── mqtt_client.cpp         ← subscribe/publish MQTT
│   └── ui/
│       ├── ui.h / ui.cpp       ← inicialización LVGL + pantallas
│       ├── screen_idle.cpp
│       ├── screen_requesting.cpp
│       ├── screen_qr.cpp
│       ├── screen_dispensing.cpp
│       └── screen_error.cpp
└── data/                       ← SPIFFS: logo, fuentes custom
    └── logo_matepoint.png
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

## 5. Historial

| Fecha | Cambio |
|-------|--------|
| 2026-05-22 | Documento creado a partir de la reorganización de `modulo-waveshare-esp32s3-touch-7b.md` |
| 2026-05-26 | Referencia agregada al dispensador Nobana (`dispensador-nobana.md`) como base de hardware |
| 2026-05-27 | Referencias actualizadas: §2.2 apunta a `arquitectura-hardware.md` §2.3 para comandos UART. Dependencias §4 actualizadas con nuevo `arquitectura-hardware.md` |
