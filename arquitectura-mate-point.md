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

## 3. Configuración Wi-Fi (producto v0-6+)

**Firmware:** [`mate_point_v0-6/`](../mate_point_firmware/mate_point_v0-6/) · Plan: [`PLAN-MATE-POINT-v0-6.md`](../mate_point_firmware/PLAN-MATE-POINT-v0-6.md)

El **dueño del local** configura la red desde la **pantalla táctil** y un **celular**. No se usa USB ni monitor serie.

| Parámetro | Valor |
|-----------|-------|
| Operador | Dueño del local |
| Persistencia | NVS namespace `mate_cfg` — claves `wifi_ssid`, `wifi_pass` |
| Criterio guardado | Solo tras **asociación STA exitosa** en el portal (no exige MQTT) |
| MQTT | Fijo en `config.h` (`broker.hivemq.com:1883`) — no configurable |
| AP provisioning | `MatePoint-XXXX` (abierto) · IP `192.168.4.1` |
| Timeout provisioning | 10 min → cierra AP, vuelve a Error Wi-Fi |
| Post-éxito | `ESP.restart()` |

### 3.1 Flujo dueño del local

```
Error Wi-Fi → Configurar red → pantalla CONFIGURAR RED
  → celular conecta MatePoint-XXXX
  → navegador 192.168.4.1 (portal Mate Point)
  → elegir red 2.4 GHz + contraseña → Conectar
  → NVS guardado → reinicio → operación normal
```

**Cancelar** en pantalla cierra el AP. **Error en servidor** = contactar proveedor (Wi-Fi OK).

### 3.2 Pantallas UI

| Pantalla | Acción |
|----------|--------|
| Error Wi-Fi | CTA **Configurar red** |
| Error servidor | Card **Consulta al proveedor** (reconexión MQTT automática) |
| Configurar red | Instrucciones + **Cancelar** |

Figma: [`UI/figma/pantallas/`](../UI/figma/pantallas/) — `Error-wifi.png`, `Error-mqtt.png`, `Configurar red.png`.

### 3.3 Migración / primera instalación

- Flashear v0-6 → NVS Wi-Fi vacío → reprovisionar desde pantalla.
- Sin `WIFI_SSID` / `WIFI_PASSWORD` en código de producción.

### 3.4 Histórico — configuración USB (POC, supersedido)

<details>
<summary>v1 planificada (no implementada en producto)</summary>

La configuración por **USB Type-C + monitor serie** y comando `wifi` quedó documentada para POC pero fue **reemplazada por SoftAP + portal** en v0-6.

| Parámetro | Valor (histórico) |
|-----------|-------------------|
| Interfaz | USB Type-C → UART debug |
| Baudrate | 115200 |
| Comando | `wifi` interactivo |
| Persistencia | NVS tras validar conexión |

</details>

---

## 4. Topics MQTT

Extiende la definición base de `integracion-mercadopago-qr.md` (§8).

> **POC Fase 4 (2026-05-29):** implementados `mate/{device_id}/command` y `mate/{device_id}/status` en firmware [`mate_point_v0-1`](../mate_point_firmware/mate_point_v0-1/). **POC v0.2:** [`mate_point_v0-2`](../mate_point_firmware/mate_point_v0-2/) — Comprar, HTTP órdenes, QR PROGMEM, timeout 2 min. Contrato operativo: [`servidor-mate-point.md`](servidor-mate-point.md) §9 · [`mate_point_firmware/PLAN-IMPLEMENTACION.md`](../mate_point_firmware/PLAN-IMPLEMENTACION.md) §14–§15.

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

**Implementado (POC v0.1 + v0.2):**

```
mate_point_firmware/
├── PLAN-IMPLEMENTACION.md     ← spec v0.1 + v0.2 (§14–§15)
├── README.md
├── reference/                 ← demo 13 Waveshare (backup)
├── mate_point_v0-1/           ← MQTT + UI simulada
│   ├── mate_point_v0-1.ino
│   ├── config.h
│   └── …
└── mate_point_v0-2/           ← Comprar + QR + HTTP órdenes (POC completa)
    ├── mate_point_v0-2.ino
    ├── app_state.cpp / .h
    ├── order_client.cpp / .h
    ├── qr_static_img.c
    └── …
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
| 2026-06-25 | §3 **Supersedido** — configuración Wi-Fi producto v0-6: SoftAP + portal web + NVS; dueño del local; sin USB |
| 2026-05-27 | §3 Configuración Wi-Fi v1: USB-C + monitor serie 115200, comando `wifi`, NVS tras validar, timeout 15 s, logs de reintentos, MQTT al conectar |
| 2026-05-27 | §3.2: Red y Password se confirman solo con Enter (sin paso `¿Confirmar?`) |
| 2026-05-29 | §4 topics: v0.2 (`mate_point_v0-2`) Comprar + QR PROGMEM; §5 estructura firmware actualizada |
| 2026-05-29 | §4 topics: separado POC (`mate/…`) vs Fase 5 planificado (`matepoint/…`); §5 apunta a [`mate_point_firmware/mate_point_v0-1/`](../mate_point_firmware/mate_point_v0-1/) |
