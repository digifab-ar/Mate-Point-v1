# Plan de implementación — `mate_point_firmware` (Fase 4.3)

**Proyecto:** Mate Point — OT-00268 Etapa 3  
**Base hardware producto:** Waveshare ESP32-S3-Touch-LCD-7B  
**Device ID:** `MATEPOINT001`  
**Última actualización:** 2026-06-25  

| Hito | Estado |
|------|--------|
| POC v0.1 — MQTT + UI simulada | **Completado** (2026-05-29) |
| POC v0.2 — Comprar + QR + E2E pago | **Completado** (2026-06-03) — §14–§15 |
| POC UART Etapa 1 — replay ARMOR (`mate_point_UART_v0-1`) | **Completado en banco** (2026-06-04) — §16 |
| UART Waveshare auto [`mate_point_UART_v0-3`](mate_point_UART_v0-3/) | **Completado en banco** (2026-06-04) — §16 |
| Producto [`mate_point_v0-3`](mate_point_v0-3/) — UI + MQTT + Nobana (v0-3.0) | **Superseded** — baseline Test1 parcial |
| Producto [`mate_point_v0-3-1`](mate_point_v0-3-1/) — timer MQTT | **E2E OK** Test1 2026-06-05 · [`PLAN-MATE-POINT-v0-3.md`](PLAN-MATE-POINT-v0-3.md) §11.2 |
| Producto [`mate_point_v0-3-2`](mate_point_v0-3-2/) — UI dispensado + Parar | **Implementado** · [`PLAN-MATE-POINT-v0-3.md`](PLAN-MATE-POINT-v0-3.md) §15 |
| Producto [`mate_point_v0-3-4`](mate_point_v0-3-4/) — VL53L0X + Iniciar | **E2E OK** Test1 2026-06-17 · [`PLAN-MATE-POINT-v0-3-4.md`](PLAN-MATE-POINT-v0-3-4.md) |
| Producto [`mate_point_v0-4`](mate_point_v0-4/) — UI Figma | **OK hardware** 2026-06-18 · iteración tipografía/layout **implementada** · pendiente validación visual + Test1 banco · [`PLAN-MATE-POINT-v0-4-UI.md`](PLAN-MATE-POINT-v0-4-UI.md) · [`UI-DISCREPANCIAS-v0-4.md`](UI-DISCREPANCIAS-v0-4.md) |
| Producto [`mate_point_v0-5`](mate_point_v0-5/) — sensor bandeja GPIO6 | **E2E OK** Test1 2026-06-24 · [`PLAN-MATE-POINT-v0-5.md`](PLAN-MATE-POINT-v0-5.md) |
| Producto [`mate_point_v0-5-1`](mate_point_v0-5-1/) — error agua UART | **E2E OK** banco 2026-06-24 (V6/V7) · [`PLAN-MATE-POINT-v0-5-1.md`](PLAN-MATE-POINT-v0-5-1.md) |
| Producto [`mate_point_v0-5-2`](mate_point_v0-5-2/) — pausa / reanudar Cargar termo | **Implementado** — UI pausa OK hardware · [`PLAN-MATE-POINT-v0-5-2.md`](PLAN-MATE-POINT-v0-5-2.md) |
| Producto [`mate_point_v0-6`](mate_point_v0-6/) — Wi-Fi SoftAP + portal web (NVS) | **E2E OK** hardware 2026-06-25 · [`PLAN-MATE-POINT-v0-6.md`](PLAN-MATE-POINT-v0-6.md) · [`mate_point_v0-6/README.md`](mate_point_v0-6/README.md) |

Referencias: [`fase-4-plan-4.1-4.3-TEMP.md`](../fase-4-plan-4.1-4.3-TEMP.md) · [`plan-de-implementacion.md`](../plan-de-implementacion.md) § Fase 4 · [`PROTOCOLO-UART-NOBANA.md`](PROTOCOLO-UART-NOBANA.md) · [`PLAN-POC-NOBANA-UART.md`](PLAN-POC-NOBANA-UART.md) · [`PLAN-MATE-POINT-v0-3.md`](PLAN-MATE-POINT-v0-3.md) · [`PLAN-MATE-POINT-v0-3-4.md`](PLAN-MATE-POINT-v0-3-4.md) · [`PLAN-MATE-POINT-v0-4-UI.md`](PLAN-MATE-POINT-v0-4-UI.md) · [`PLAN-MATE-POINT-v0-5.md`](PLAN-MATE-POINT-v0-5.md) · [`PLAN-MATE-POINT-v0-5-1.md`](PLAN-MATE-POINT-v0-5-1.md) · [`PLAN-MATE-POINT-v0-5-2.md`](PLAN-MATE-POINT-v0-5-2.md) · [`PLAN-MATE-POINT-v0-6.md`](PLAN-MATE-POINT-v0-6.md) · [`UI-DISCREPANCIAS-v0-4.md`](UI-DISCREPANCIAS-v0-4.md) · [`PLAN-MATE-POINT-UART-v0-3.md`](PLAN-MATE-POINT-UART-v0-3.md) · [`servidor-mate-point.md`](../servidor-mate-point.md) §9 · [`servidor/src/services/mqtt.js`](../servidor/src/services/mqtt.js) · [`UI/figma/`](../UI/figma/)

---

## Estado v0.1 (2026-05-29)

| Ítem | Estado |
|------|--------|
| Sketch `mate_point_v0-1` | **Operativo en hardware** |
| Wi‑Fi + MQTT HiveMQ | OK |
| UI simulada (Dispensar → terminado → Listo) | OK |
| `status` MQTT (30 s + transiciones) | OK |
| Dedup `order_id` | OK |
| UART Nobana | Fuera de alcance |
| E2E Railway (pago → pantalla) | **Completado** (2026-05-29) |
| POC v0.2 (Comprar + QR) | **Completado** (E2E hardware 2026-06-03) — §14 · §15 |

**Abrir en Arduino:** [`mate_point_v0-1/mate_point_v0-1.ino`](mate_point_v0-1/mate_point_v0-1.ino)

---

## Estado v0.2 (2026-06-03)

| Ítem | Estado |
|------|--------|
| Sketch `mate_point_v0-2` | **Implementado** — compila tras fix `display_ui.h` + `lvgl.h` |
| Botón **Comprar** + máquina de estados | OK en código |
| HTTP `POST /orders/create` y `/orders/cancel` | OK (`order_client.cpp` → Railway) |
| QR estático LVGL (PROGMEM) | OK — `qr_static_img.c` 320×320 |
| Timer 2 min + cancel en timeout | OK (`QR_TIMEOUT_MS`) |
| MQTT `dispense` solo en `QR_SHOW` | OK |
| UI **Dispensado** → terminado → Listo | OK (hereda v0.1) |
| **E2E hardware** (escaneo QR + pago sandbox) | **Completado** (2026-06-03) |

**Abrir en Arduino:** [`mate_point_v0-2/mate_point_v0-2.ino`](mate_point_v0-2/mate_point_v0-2.ino)

Detalle de archivos y conversor LVGL: **§15**.

---

## 1. Decisiones cerradas (equipo)

| # | Tema | Decisión |
|---|------|----------|
| 1 | Heartbeat `status` (cada 30 s) | Publicar **último estado conocido** (`idle` o `dispensing`) |
| 2 | Fin de ciclo MQTT | Tras `duration_ms` → UI **"terminado"** → **3 s** → UI **"Listo"** → recién ahí **`state: idle`** |
| 3 | Segundo `dispense` en curso | **Fuera de alcance** en esta iteración (no UI error, no `state: error`) |
| 4 | Wi‑Fi | **v0-6:** NVS + SoftAP + portal web (dueño del local); sin USB. **v0-5-2 y anteriores:** hardcoded en `config.h` |
| 5 | `device_id` en `status` | String **`"MATEPOINT001"`** (igual que `DEVICE_ID`) |
| 6 | `external_reference` en `command` | Campo opcional del servidor; **ignorar** o log Serial; no UI ni `status` |
| 7 | Comandos duplicados (QoS 1 / reconnect) | **Deduplicar por `order_id`** — si ya se procesó, ignorar |
| 8 | Validación `duration_ms` | Asumir payload válido del servidor |
| 9 | Topics MQTT | Solo `mate/MATEPOINT001/command` y `mate/MATEPOINT001/status` |
| 10 | Base LVGL | Mantener **port del demo 13** (`lvgl_v8_port` / panel Waveshare); no reimplementar drivers |
| 11 | Carpeta repo | **`mate_point_firmware/`** |

### Broker y toolchain (sin cambios)

| Tema | Valor |
|------|--------|
| Broker POC | `broker.hivemq.com:1883` (TCP, sin TLS) |
| Toolchain | Arduino IDE + esp32 ≥ 3.0 + placa Waveshare ESP32-S3-Touch-LCD-7 |
| Flash / PSRAM | 16 MB / OPI |
| Monitor serie | 115200 |

---

## 2. Contrato MQTT

### 2.1 Topics

| Topic | Dirección | QoS |
|-------|-----------|-----|
| `mate/MATEPOINT001/command` | Servidor → ESP32 | 1 (subscribe) |
| `mate/MATEPOINT001/status` | ESP32 → broker | 0 o 1 (publish) |

### 2.2 Payload `command` (entrada)

```json
{
  "cmd": "dispense",
  "duration_ms": 120000,
  "order_id": "ORDTST01...",
  "external_reference": "mate-001-20260527-003",
  "ts": 1748369220000
}
```

| Campo | Uso firmware 4.3 |
|-------|------------------|
| `cmd` | Procesar solo si `"dispense"` |
| `duration_ms` | Timer de pantalla "Dispensar" |
| `order_id` | Dedup + log Serial |
| `external_reference` | Log Serial opcional |
| `ts` | Log opcional |

### 2.3 Payload `status` (salida)

Mínimo:

```json
{
  "device_id": "MATEPOINT001",
  "state": "idle",
  "ts": 1748368961000
}
```

Recomendado (cuando aplique):

```json
{
  "device_id": "MATEPOINT001",
  "state": "dispensing",
  "ts": 1748368961000,
  "uptime_ms": 123456,
  "wifi_rssi": -58,
  "mqtt_connected": true,
  "order_id": "ORDTST01..."
}
```

Estados en 4.3: **`idle`**, **`dispensing`** (no `error` en esta iteración).

---

## 3. Copiar archivos del demo 13 (tu parte — antes de codear)

### 3.1 Origen

En el ZIP **ESP32-S3-Touch-LCD-7B Demo**, carpeta Arduino, ejemplo que ya compilaste:

```
.../Arduino/examples/13_LVGL_TRANSPLANT/
```

(o ruta equivalente en tu descarga).

### 3.2 Destino

Copiar **todo el contenido** de esa carpeta a:

```
Mate_point_software/mate_point_firmware/mate_point_v0-1/
```

(Los archivos del port Waveshare van **dentro** de `mate_point_v0-1/`, junto al `.ino`.)

### 3.3 Qué copiar y qué no

| Acción | Detalle |
|--------|---------|
| **Copiar** | Todos los `.h`, `.c`, `.cpp` y auxiliares del folder `13_LVGL_TRANSPLANT` |
| **Renombrar / no usar** | `13_LVGL_TRANSPLANT.ino` → mover a `reference/` como backup |
| **Sketch activo** | Tras implementar: `mate_point_v0-1/mate_point_v0-1.ino` |
| **No copiar al repo** | Carpeta `lvgl/` — queda en `~/Documents/Arduino/libraries/lvgl` |
| **No copiar al repo** | `lv_conf.h` — queda en `~/Documents/Arduino/libraries/lv_conf.h` (junto a la carpeta `lvgl`) |
| **No copiar** | Librerías `ESP32_Display_Panel`, `ESP32_IO_Expander` — siguen en `Arduino/libraries/` |

### 3.4 Archivos típicos del demo 13 (referencia)

Según la versión del ZIP, el folder 13 suele incluir parte o todo lo siguiente. **Copiá todo lo que tenga tu carpeta 13** (nombres pueden variar levemente):

```
13_LVGL_TRANSPLANT.ino          → respaldar, no dejar como .ino activo
lvgl_v8_port.h
lvgl_v8_port.cpp
esp_panel_board_custom_conf.h   (o ESP_Panel_Conf.h / similar)
i2c.h / i2c.c
io_extension.h / io_extension.c
rgb_lcd_port.h / rgb_lcd_port.c
touch*.h / touch*.c               (GT911, si aplica)
```

Si tu demo 13 usa **`esp_display_panel.hpp`** en lugar de `rgb_lcd_port`, copiá esos headers del sketch; la regla es la misma: **todo lo que no sea librería global de Arduino**.

### 3.5 Verificación después de copiar

1. En `mate_point_v0-1/` debe haber **un solo** `.ino`: `mate_point_v0-1.ino`.
2. Abrí temporalmente el `.ino` respaldado en Arduino y confirmá que la lista de tabs (archivos del sketch) coincide con lo copiado al repo.
3. Avisá en el chat cuando esté listo para continuar.

---

## 4. Estructura objetivo del proyecto (después de implementar)

```
mate_point_firmware/
├── PLAN-IMPLEMENTACION.md          ← este documento (índice maestro)
├── PLAN-MATE-POINT-v0-3.md         ← producto Waveshare UI + Nobana
├── PLAN-MATE-POINT-v0-3-4.md       ← VL53L0X + gate Iniciar (E2E OK)
├── PLAN-MATE-POINT-v0-4-UI.md      ← UI Figma v0-4
├── PLAN-MATE-POINT-v0-5.md         ← sensor bandeja GPIO6 (reed NO)
├── PLAN-MATE-POINT-v0-5-1.md       ← error agua UART (E2E OK)
├── PLAN-MATE-POINT-v0-5-2.md       ← pausa / reanudar (cooldown pausa 5 s)
├── UI-DISCREPANCIAS-v0-4.md        ← Spec tipografía/layout vs Figma (implementado)
├── PROTOCOLO-UART-NOBANA.md        ← diccionario bus UART
├── PLAN-POC-NOBANA-UART.md         ← POC Etapa 1 replay (ESP32 v0-1)
├── PLAN-MATE-POINT-UART-v0-2.md    ← POC kiosco W/S/R (ESP32 UART v0-2)
├── PLAN-MATE-POINT-UART-v0-3.md    ← POC UART Waveshare auto (cerrada)
├── README.md
├── reference/                      ← backup demo 13
├── mate_point_v0-1/                ← MQTT + UI simulada
├── mate_point_v0-2/                ← Comprar + QR + MQTT (dispensado simulado)
├── mate_point_v0-3/                ← producto: v0-2 + driver Nobana
├── mate_point_v0-4/                ← UI Figma
├── mate_point_v0-5/                ← sensor bandeja GPIO6
├── mate_point_v0-5-1/              ← error agua UART
├── mate_point_v0-5-2/                ← pausa / reanudar
├── mate_point_UART_v0-1/           ← POC UART Etapa 1 cerrada
├── mate_point_UART_v0-2/           ← POC UART kiosco (sin lock 23)
└── mate_point_UART_v0-3/           ← POC UART Waveshare auto (cerrada banco)
```

Librerías Arduino adicionales (Library Manager): **PubSubClient**, **ArduinoJson** v7.

---

## 5. `config.h` (plantilla)

```c
#pragma once

#define DEVICE_ID           "MATEPOINT001"

#define WIFI_SSID           "TU_RED_AQUI"
#define WIFI_PASSWORD       "TU_CLAVE_AQUI"

#define MQTT_HOST           "broker.hivemq.com"
#define MQTT_PORT           1883

#define TOPIC_COMMAND       "mate/" DEVICE_ID "/command"
#define TOPIC_STATUS        "mate/" DEVICE_ID "/status"

#define STATUS_INTERVAL_MS      30000
#define TERMINADO_TO_LISTO_MS   3000

#define MQTT_CLIENT_ID      "mate-" DEVICE_ID "-esp32"
```

Antes de flashear en el dispensador: reemplazar `TU_RED_AQUI` / `TU_CLAVE_AQUI`.

En el `.ino`, **antes** de `#include <PubSubClient.h>`:

```cpp
#define MQTT_MAX_PACKET_SIZE 512
```

---

## 6. Flujo de estados

### 6.1 Arranque

```
power-on
  → init display (port demo 13)
  → UI "Listo" (o "Iniciando..." breve)
  → WiFi.begin(SSID, PASS)
  → MQTT connect + SUBSCRIBE command
  → publish status { state: "idle" }
  → footer: Wifi / MQTT según conexión
```

### 6.2 Al recibir `command`

```
MQTT callback
  → parse JSON
  → if cmd != "dispense" → return
  → if order_id ya procesado → return (dedup)
  → UI "Dispensar"
  → publish status { state: "dispensing", order_id }
  → timer duration_ms

timer expired
  → UI "terminado"
  → esperar TERMINADO_TO_LISTO_MS (3 s)
  → UI "Listo"
  → publish status { state: "idle" }
```

### 6.3 Heartbeat

Cada `STATUS_INTERVAL_MS` (30 s), si MQTT conectado: republicar **estado actual** (`idle` o `dispensing`).

### 6.4 UI mínima (1024×600)

```
┌────────────────────────────────────────────┐
│  Mate Point — POC Fase 4                   │
│                                            │
│           [  mensaje central  ]            │
│         Listo / Dispensar / terminado      │
│                                            │
│  Wifi: Conectado                           │
│  MQTT: conectado                           │
└────────────────────────────────────────────┘
```

Textos en **español**. `order_id` solo en Serial, no en pantalla.

### 6.5 LVGL y threads

El demo 13 inicializa LVGL con tarea dedicada (`lvgl_port_init` / `lvgl_v8_port`). Cualquier cambio de UI debe hacerse con el **mismo patrón de lock** que use el demo 13 (`lvgl_port_lock` / `unlock` o equivalente).

---

## 7. Tareas de implementación (orden)

| Fase | # | Tarea | Verificación |
|------|---|--------|--------------|
| **Prep** | — | Copiar archivos demo 13 (§3) | Archivos en repo |
| **A** | 8 | Crear `mate_point_firmware.ino` + `config.h` | Compila |
| **A** | 9 | `display_ui`: labels Wifi / MQTT / mensaje "Listo" | Pantalla estática |
| **B** | 10 | Wi‑Fi + reconexión + labels | Serial + UI |
| **B** | 11 | MQTT PubSubClient + subscribe | `MQTT: conectado` |
| **C** | 12 | `publish_status()` heartbeat + transiciones | `mosquitto_sub` |
| **C** | 13 | Callback parse `dispense` + dedup `order_id` | CLI → Serial |
| **D** | 14 | `dispense_sim`: ciclo completo con `duration_ms` | CLI 10 s |
| **E** | 16 | E2E Railway (pago sandbox) | **Completado** |
| **E** | 17 | Reconexión Wi‑Fi | Labels OK |

**Fuera de alcance ahora:** tarea 15 (segundo dispense / error), UART 4.4+, QR Fase 5, NVS `wifi`.

---

## 8. Pruebas

### 8.1 CLI — escuchar

```bash
mosquitto_sub -h broker.hivemq.com -p 1883 -t 'mate/MATEPOINT001/#' -v
```

### 8.2 CLI — dispense rápido (10 s)

```bash
mosquitto_pub -h broker.hivemq.com -p 1883 \
  -t 'mate/MATEPOINT001/command' \
  -m '{"cmd":"dispense","duration_ms":10000,"order_id":"TEST-QUICK","ts":1748369220000}'
```

### 8.3 Secuencia esperada

1. Pantalla **Dispensar** ~10 s  
2. Pantalla **terminado** 3 s  
3. Pantalla **Listo**  
4. MQTT: `dispensing` → (tras 10 s + 3 s) `idle`  
5. Heartbeat cada 30 s con último estado

### 8.4 E2E Railway

Pago sandbox → webhook → servidor publica `command` → misma secuencia en pantalla (`duration_ms` default 120000).

**Estado:** **Completado** (2026-05-29).

---

## 9. Criterios de aceptación 4.3

- [x] Firmware compila y corre sobre base demo 13
- [x] `Wifi: Conectado` y `MQTT: conectado` con HiveMQ público
- [x] `mosquitto_sub` recibe `status` cada 30 s y en transiciones
- [x] `command` `dispense` → **Dispensar** durante `duration_ms`
- [x] **terminado** → 3 s → **Listo** → `idle` publicado
- [x] Dedup: mismo `order_id` no reinicia ciclo
- [x] Serial 115200: trazas Wi‑Fi, MQTT, payloads

---

## 10. Fuera de alcance (referencia)

| Paso | Contenido |
|------|-----------|
| 4.4+ | UART2 → TXS0108E → Nobana |
| 4.3 tarea 15 | Segundo `dispense` / pantalla Ocupado |
| Fase 5 | QR, `qr_show`, cancel, 5 pantallas |
| Fase 6 | Broker TLS, credenciales prod |
| Wi‑Fi v1 (USB + serie) | ~~Comando serie `wifi` + NVS~~ — **supersedido por v0-6** SoftAP + portal ([`arquitectura-mate-point.md`](../arquitectura-mate-point.md) §3) |

---

## 11. Known issues (v0.1)

| Issue | Severidad | Notas |
|-------|-----------|-------|
| Franja corrida en labels footer WiFi/MQTT | Cosmético / intermitente | Solo al conectar red; label central OK. Causa probable: partial refresh LVGL + RGB direct mode. Reconectar suele limpiar. |

---

## 12. Próximo hito

**Cerrado (2026-06-03):** POC v0.2 Waveshare — Comprar + QR + E2E pago — ver **§15**.

**Cerrado (2026-06-04):** POC UART Etapa 1 — replay captura ARMOR en banco — ver **§16** y [`PLAN-POC-NOBANA-UART.md`](PLAN-POC-NOBANA-UART.md).

**Cerrado (2026-06-04):** UART Waveshare auto [`mate_point_UART_v0-3/`](mate_point_UART_v0-3/) — banco OK — [`2026-06-04-Waveshare-UART-v0-3_banco-validacion-OK.md`](../tools/nobana_uart_sniffer/capturas/2026-06-04-Waveshare-UART-v0-3_banco-validacion-OK.md).

**Implementado (2026-06-05)** — [`mate_point_v0-3/`](mate_point_v0-3/) · plan: [`PLAN-MATE-POINT-v0-3.md`](PLAN-MATE-POINT-v0-3.md)

**Validación banco pendiente:**

| # | Entregable | Notas |
|---|------------|--------|
| 1 | Fork `mate_point_v0-2` → `mate_point_v0-3` | Misma UI/flujo QR y MQTT |
| 2 | Driver Nobana desde [`mate_point_UART_v0-3`](mate_point_UART_v0-3/) | `W` antes de LVGL; `S` en Comprar/QR; `R` en dispensado |
| 3 | **Bus sin logs** | Solo `F8` + tramas `0x68` — sin `Serial` firmware (USB ni UART) |
| 4 | Ciclos repetidos | Tras Listo → Comprar: **`S`** + nueva compra QR → nuevo **`R`** |
| 5 | E2E banco | Pago sandbox + dispensado físico; captura sniffer |

**Opcional en la misma iteración:** validar [`mate_point_UART_v0-2/`](mate_point_UART_v0-2/) en ESP32 Dev (`W`/`S`/`R` manual) — no bloquea v0-3 Waveshare.

**Después:** Fase 4.4–4.10 (watchdog, temperatura en `status`, tanque vacío en UI).

---

## 14. POC completa v0.2 — spec firmware

**Carpeta objetivo:** `mate_point_v0-2/` (fork de v0.1).  
**Alcance:** flujo de compra en pantalla; dispensado sigue **simulado** (sin UART).

### 14.1 Flujo de usuario

1. **Reposo:** pantalla con botón **Comprar** (footer WiFi/MQTT como v0.1).
2. **Toque Comprar:** HTTP `POST` al servidor Railway `/orders/create` → mostrar **QR estático**.
3. **Espera pago (max 2 min):**
   - **Pago OK:** webhook → MQTT `command` → pantalla **Dispensado** → **terminado** (3 s) → **Listo** → vuelve a **Comprar**.
   - **Sin pago:** al cumplir 2 min → `POST /orders/cancel` (o expiración MP) → vuelve a **Comprar**.

### 14.2 Máquina de estados

| Estado interno | UI central | MQTT `status` | Transiciones |
|----------------|------------|---------------|--------------|
| `COMPRAR` | Botón Comprar | `idle` | Toque → `CREATING` |
| `CREATING` | "Creando orden…" (opcional) | `idle` | HTTP OK → `QR_SHOW`; error → `COMPRAR` |
| `QR_SHOW` | Imagen QR + timer opcional | `idle` | MQTT dispense → `DISPENSING`; timeout → `COMPRAR` |
| `DISPENSING` | **v0-3-1:** **Dispensado**. **v0-3-2:** **`83°`** + countdown + **Parar** | `dispensing` | `duration_ms` → `TERMINADO` |
| `TERMINADO` | **terminado** | `dispensing` | 3 s → `LISTO` |
| `LISTO` | **Listo** | `dispensing` | publicar `idle` → `COMPRAR` |

**Reglas (heredadas v0.1 + nuevas):**

- Solo procesar `command` `dispense` en estado `QR_SHOW` (ignorar en `COMPRAR` salvo dedup).
- Dedup por `order_id` igual que v0.1.
- Segundo `dispense` en curso: fuera de alcance v0.2.
- Heartbeat 30 s = último `status` conocido.

### 14.3 Configuración nueva (`config.h` v0.2)

| Constante | Valor sugerido | Uso |
|-----------|----------------|-----|
| `SERVER_HOST` | `mate-point-v1-production.up.railway.app` | HTTPS (puerto 443) |
| `SERVER_PORT` | `443` | TLS |
| `QR_TIMEOUT_MS` | `120000` | 2 min sin pago |
| QR imagen | `qr_static_img.c` | PROGMEM — ver §15 |

### 14.4 Módulos nuevos / cambios

| Módulo | Responsabilidad |
|--------|-----------------|
| `order_client.cpp` / `.h` | HTTP `POST /orders/create` y `/orders/cancel` → Railway |
| `app_state.cpp` / `.h` | Máquina de estados §14.2 + timer QR |
| `display_ui.cpp` / `.h` | Botón Comprar, panel QR, countdown, `display_ui_set_qr_image()` |
| `qr_static_img.c` + `qr_image.h` | Array C LVGL 8 (`qr_static_img`, `qr_static_map`) |
| `mate_network.cpp` | Solo aceptar `dispense` si `QR_SHOW` |
| `dispense_sim.cpp` | Label **Dispensado** (simulación v0.1) |

### 14.5 Dependencias servidor (bloqueantes)

| Endpoint | Estado | Uso |
|----------|--------|-----|
| `POST /orders/create` | **Implementado** (`servidor/src/routes/orders.js`) | Toque Comprar |
| `POST /orders/cancel` | **Implementado** | Timeout UI 2 min |

Orden MP: `expiration_time: PT2M` vía `MP_ORDER_EXPIRATION` (alineado `QR_TIMEOUT_MS`).

### 14.6 QR estático en pantalla

Opciones (POC):

1. **PROGMEM** — PNG embebido (~320×320), sin red extra al mostrar QR. Ver [`integracion-mercadopago-qr.md`](../integracion-mercadopago-qr.md) §13.1.
2. **HTTP al boot** — descargar una vez; más lento al arranque.

Recomendación POC: **PROGMEM**.

### 14.7 Criterios de aceptación v0.2

**Implementación (código):**

- [x] Botón **Comprar** visible en reposo
- [x] Toque crea orden (HTTP) y muestra imagen QR en LVGL
- [x] MQTT `dispense` en `QR_SHOW` → **Dispensado** → terminado → Listo → Comprar
- [x] Timer 2 min → `POST /orders/cancel` → vuelve a **Comprar**
- [x] WiFi/MQTT footer operativo (known issue v0.1 aceptable)

**Validación en hardware (2026-06-03):**

- [x] QR escaneable con app MP sandbox
- [x] E2E: Comprar → pago → **Dispensado** sin Postman
- [x] E2E: timeout 2 min sin pago

### 14.8 Orden de implementación (registro)

| # | Tarea | Estado |
|---|-------|--------|
| 1 | Servidor `/orders/create` | **Hecho** |
| 2 | Servidor `/orders/cancel` | **Hecho** |
| 3 | Fork `mate_point_v0-2` | **Hecho** |
| 4 | UI Comprar + estados | **Hecho** |
| 5 | HTTP create | **Hecho** |
| 6 | QR en LVGL (PROGMEM) | **Hecho** |
| 7 | Timer + cancel | **Hecho** |
| 8 | MQTT dispense → **Dispensado** | **Hecho** |
| 9 | E2E completo en hardware | **Hecho** (2026-06-03) |

---

## 15. Implementación v0.2 — registro técnico (2026-05-29)

### 15.1 Estructura `mate_point_v0-2/`

```
mate_point_v0-2/
├── mate_point_v0-2.ino
├── config.h                 ← SERVER_HOST, QR_TIMEOUT_MS, MQTT topics
├── app_state.cpp / .h       ← COMPRAR → CREATING → QR_SHOW → DISPENSE → LISTO
├── order_client.cpp / .h    ← HTTPS create/cancel
├── display_ui.cpp / .h      ← Comprar, panel QR, countdown
├── qr_static_img.c          ← imagen QR (LVGL 8, 320×320)
├── qr_image.h               ← extern qr_static_img
├── mate_network.cpp         ← dispense solo en QR_SHOW
├── dispense_sim.cpp         ← texto "Dispensado"
└── [port Waveshare: lvgl_port, rgb_lcd, gt911, …]
```

### 15.2 Servidor (Railway)

| Endpoint | Body | Respuesta |
|----------|------|-----------|
| `POST /orders/create` | `{ "device_id": "MATEPOINT001" }` (opcional) | `order_id`, `external_reference`, `expiration_time: PT2M` |
| `POST /orders/cancel` | `{ "order_id": "ORD…" }` | `ok`, `status` |

Implementación: [`servidor/src/routes/orders.js`](../servidor/src/routes/orders.js), [`mercadopago.js`](../servidor/src/services/mercadopago.js).

### 15.3 QR estático en LVGL 8

PNG fuente: [`integracion-mercadopago-qr.md`](../integracion-mercadopago-qr.md) §13.1 (320×320, fondo blanco opaco).

**Conversor** ([LVGL Image Converter](https://lvgl.io/tools/imageconverter), pestaña **LVGL v8**):

| Opción | Valor |
|--------|--------|
| Color format | `CF_TRUE_COLOR` |
| Output format | **C array** (no Binary RGB565) |
| Dither | off |
| Big-endian | off |

Símbolos en firmware: `qr_static_map[]`, `qr_static_img` (`lv_img_dsc_t`, `LV_IMG_CF_TRUE_COLOR`).

**Integración UI:** al entrar en `QR_SHOW`, `app_state` llama `display_ui_set_qr_image(&qr_static_img)`.

### 15.4 Compilación Arduino IDE

`display_ui.h` declara `lv_img_dsc_t` → debe incluir `#include "lvgl.h"` (header autocontenido).

Placa: Waveshare ESP32-S3-Touch-LCD-7 · `LV_COLOR_DEPTH=16` (RGB565).

### 15.5 Prueba E2E en hardware — **Completada** (2026-06-03)

| Caso | Resultado |
|------|-----------|
| QR escaneable (MP sandbox) | OK |
| Comprar → pago → **Dispensado** → Listo → Comprar | OK |
| Timeout 2 min sin pago → vuelve a Comprar | OK |

Monitor serie observado: `[app] order created`, `[app] QR timeout`, `[mqtt] dispense accepted`.

---

## 16. Integración Nobana UART (índice)

Resumen de la cadena UART; **detalle normativo del bus** en [`PROTOCOLO-UART-NOBANA.md`](PROTOCOLO-UART-NOBANA.md). **Procedimiento y criterios Etapa 1** en [`PLAN-POC-NOBANA-UART.md`](PLAN-POC-NOBANA-UART.md).

| Etapa | Sketch / plan | Estado | Alcance |
|-------|---------------|--------|---------|
| **1** | [`mate_point_UART_v0-1/`](mate_point_UART_v0-1/) · [PLAN-POC](PLAN-POC-NOBANA-UART.md) | **Cerrada** 2026-06-04 | Replay captura ARMOR: comandos **`W`** + **`R`**; Coffee 180 ml timer |
| **1b** | — | Pendiente | Fin manual, otros ml, `D`/`T`/`M` — ver PLAN-POC §9 |
| **2a-ESP** | [`mate_point_UART_v0-2/`](mate_point_UART_v0-2/) · [PLAN v0-2](PLAN-MATE-POINT-UART-v0-2.md) | Implementado; banco pendiente | Kiosco manual `W`/`S`/`R` (ESP32 Dev) |
| **2a-WS** | [`mate_point_UART_v0-3/`](mate_point_UART_v0-3/) · [PLAN v0-3](PLAN-MATE-POINT-UART-v0-3.md) | **Cerrada** 2026-06-04 | Waveshare: ciclo **auto** `W→S→R`, **1×/boot**, GPIO **44/43** — [captura OK](../tools/nobana_uart_sniffer/capturas/2026-06-04-Waveshare-UART-v0-3_banco-validacion-OK.md) |
| **2b** | [`mate_point_v0-3/`](mate_point_v0-3/) · [PLAN producto](PLAN-MATE-POINT-v0-3.md) | Superseded | v0-3.0 baseline |
| **2b-1** | [`mate_point_v0-3-1/`](mate_point_v0-3-1/) · [PLAN §14](PLAN-MATE-POINT-v0-3.md) | **OK** Test1 2026-06-05 | `duration_ms` MQTT; **terminado** → **Listo** → Comprar |
| **2b-2** | [`mate_point_v0-3-2/`](mate_point_v0-3-2/) · [PLAN §15](PLAN-MATE-POINT-v0-3.md) | Implementado | UI dispensado: **T_viva**, countdown, **Parar** (§7.4 manual ~1,8 s) |
| **2b-3** | [`mate_point_v0-3-3/`](mate_point_v0-3-3/) · [PLAN §16](PLAN-MATE-POINT-v0-3.md) | **E2E OK** Test1 2026-06-05 | Parar, timer natural, 2.ª compra tras Parar |
| **2b-4** | [`mate_point_v0-3-4/`](mate_point_v0-3-4/) · [PLAN v0-3-4](PLAN-MATE-POINT-v0-3-4.md) | **E2E OK** Test1 2026-06-17 | VL53L0X, Iniciar, auto-Parar, offset 85 mm |
| **2b-5** | [`mate_point_v0-4/`](mate_point_v0-4/) · [PLAN UI v0-4](PLAN-MATE-POINT-v0-4-UI.md) · [Spec UI](UI-DISCREPANCIAS-v0-4.md) | **OK hardware** 2026-06-18 | UI Figma; tipografía/layout iteración 2; litros; rotación 180°; pendiente validación visual + Test1 banco |
| **2b-6** | [`mate_point_v0-5/`](mate_point_v0-5/) · [PLAN v0-5](PLAN-MATE-POINT-v0-5.md) | **E2E OK** Test1 2026-06-24 | Sensor bandeja reed NO @ GPIO6; poll 5 s; error bloqueante; auto-Parar + `order_cancel` |
| **2b-7** | [`mate_point_v0-5-1/`](mate_point_v0-5-1/) · [PLAN v0-5-1](PLAN-MATE-POINT-v0-5-1.md) | **E2E OK** banco 2026-06-24 (V6/V7) | Tanque vacío UART (`b2=0x10`, byte 7); `UI_ERR_AGUA`; `ensure_tank_monitor_poll()` |
| **2b-8** | [`mate_point_v0-5-2/`](mate_point_v0-5-2/) · [PLAN v0-5-2](PLAN-MATE-POINT-v0-5-2.md) | **Implementado** — UI pausa OK hardware | Pausa: cooldown **5 s**; Continuar oculto → visible; Finalizar fijo; timer 20 s |
| **2b-9** | [`mate_point_v0-6/`](mate_point_v0-6/) · [PLAN v0-6](PLAN-MATE-POINT-v0-6.md) | **E2E OK** hardware 2026-06-25 | Wi-Fi NVS + SoftAP + portal web; pantallas Error-wifi / Error-mqtt / Configurar red |
| *(ref)* | [`mate_point_v0-2/`](mate_point_v0-2/) | Completado | Dispensado simulado — base de fork v0-3 |

| Tema | Referencia |
|------|------------|
| Tramas, `cmd`, telemetría, tanque vacío | [`PROTOCOLO-UART-NOBANA.md`](PROTOCOLO-UART-NOBANA.md) |
| Coffee validado (timer) | Inicio **`E2`** + **`d7=0x55`** (UV on); no usar solo `C2` — §5.1 PROTOCOLO |
| Pines ESP Dev **maestro** | **GPIO25** RX · **GPIO17** TX — PLAN-POC §3 |
| Pines Waveshare **maestro** | **GPIO44** RX · **GPIO43** TX (UART2 PH2.0) — [PLAN v0-3](PLAN-MATE-POINT-UART-v0-3.md) §3 |
| Sniffer (ARMOR en paralelo) | [`tools/nobana_uart_sniffer/`](../tools/nobana_uart_sniffer/) — GPIO25 NOB→ARM, GPIO17 ARM→NOB |
| Criterios aceptación Etapa 1 | [`PLAN-POC-NOBANA-UART.md` §8](PLAN-POC-NOBANA-UART.md) |
| Relación Fase 4.x | Etapa 1 ≈ 4.4–4.6 banco; Etapa 2b ≈ producto + 4.7+ |

---

## 17. Producto `mate_point_v0-3` … `mate_point_v0-5-2` (índice)

Plan normativo: [`PLAN-MATE-POINT-v0-3.md`](PLAN-MATE-POINT-v0-3.md). Plan v0-3-4: [`PLAN-MATE-POINT-v0-3-4.md`](PLAN-MATE-POINT-v0-3-4.md). Plan UI v0-4: [`PLAN-MATE-POINT-v0-4-UI.md`](PLAN-MATE-POINT-v0-4-UI.md). Plan v0-5: [`PLAN-MATE-POINT-v0-5.md`](PLAN-MATE-POINT-v0-5.md). Plan v0-5-1: [`PLAN-MATE-POINT-v0-5-1.md`](PLAN-MATE-POINT-v0-5-1.md). Plan v0-5-2: [`PLAN-MATE-POINT-v0-5-2.md`](PLAN-MATE-POINT-v0-5-2.md). Plan v0-6: [`PLAN-MATE-POINT-v0-6.md`](PLAN-MATE-POINT-v0-6.md).

| Tema | v0-3-3 — **banco OK** | v0-3-4 — **banco OK** | v0-4 — **hardware OK** | v0-5 — **banco OK** |
|------|------------------------|------------------------|-------------------------|-------------------------|
| Base | Fork v0-3-2 | Fork v0-3-3 | Fork v0-3-4 | Fork v0-4 |
| Post-pago | Dispensado inmediato | **Coloque el termo** / **Iniciar** | Igual v0-3-4 | Igual v0-4 |
| Sensor termo | — | VL53L0X; offset **85 mm**; `< 15 mm` corr. | Igual v0-3-4 | Igual v0-4 |
| Sensor bandeja | — | — | UI `UI_ERR_BANDEJA` sin trigger | Reed NO @ **GPIO6**; poll **5 s**; error bloqueante |
| MQTT `dispensing` | Al pago | Solo al **Iniciar** | Igual v0-3-4 | Igual v0-4 |
| Termo retirado | — | Pre-Iniciar: mensaje; dispensando: auto-Parar | Igual v0-3-4 | Igual v0-4 |
| Bandeja llena | — | — | — | Auto-Parar + **`order_cancel`**; sin Finish; recuperación Standby |
| Dispensado activo | Parar 200 ms; terminado @ countdown 0 | Igual v0-3-3 | Igual v0-3-4 | Igual v0-4 |
| UI | Placeholder LVGL | Placeholder LVGL | **Figma** 1024×600; litros; WiFi/MQTT bloqueante; **rotación 180°** | Igual v0-4 |
| Flash | ~2 MB APP | ~2 MB APP | **~6.4 MB** → partición custom 8 MB APP | Igual v0-4 |
| Validación | [`Test1`](../tools/nobana_uart_sniffer/capturas/2026-06-05-Waveshare-Mate_point-v0-3-3_Test1.md) | [`Test1`](../tools/nobana_uart_sniffer/capturas/2026-06-17-Waveshare-Mate_point-v0-3-4_Test1.md) | E2E funcional Waveshare 2026-06-18; Test1 banco pendiente | [`Test1`](../tools/nobana_uart_sniffer/capturas/2026-06-24-Waveshare-Mate_point-v0-5_Test1.md) |

Assets UI: [`../UI/figma/`](../UI/figma/) · Wireframe: [Figma Mate Point](https://www.figma.com/design/TmPeOvLv6q247UMZsAD8fl/Mate-Point?node-id=0-1)

### 17.1 Producto v0-5-1 … v0-5-2

| Tema | v0-5-1 — **banco OK** | v0-5-2 — **implementado** |
|------|------------------------|-------------------------|
| Base | Fork v0-5 | Fork v0-5-1 |
| Tanque vacío | UART `b2=0x10` + byte 7; `UI_ERR_AGUA`; poll en error | Hereda v0-5-1 |
| Bandeja llena | Hereda v0-5 | Hereda v0-5-1 |
| Parar | Fin abrupto Nobana; contrato UI sigue hasta `duration_ms` | **Pausa** — abort corto + cooldown **5 s**; fase `PAUSED` |
| UI Cargar termo (dispensando) | Solo **Parar** | Igual v0-5-1 |
| UI Cargar termo (pausado) | — | Cooldown: solo **Finalizar**; luego **Continuar** + **Finalizar** — [`Cargar-termo-pause.png`](../UI/figma/pantallas/Cargar-termo-pause.png) |
| Continuar en pausa | — | Oculto durante cooldown (~7 s); `display_ui_set_continuar_visible` |
| Timer decisión pausa | — | **20 s**; reinicia en cada pausa |
| Reanudar | — | `nobana_dispense_start(remaining_ms)` tras cooldown |
| Fin por presupuesto | Cierre gradual Nobana | Igual; sin `order_cancel` |
| Retiro termo (dispensando) | Auto-Parar; contrato UI sigue | **Cierre sesión** → Finish |
| Validación | V6/V7 banco · [`PLAN v0-5-1`](PLAN-MATE-POINT-v0-5-1.md) §12 | Test1 banco pendiente · [`PLAN v0-5-2`](PLAN-MATE-POINT-v0-5-2.md) §12 |

### 17.2 Producto v0-6 — Wi-Fi provisioning

| Tema | v0-6 — **E2E OK** hardware 2026-06-25 |
|------|----------------------------------------|
| Base | Fork v0-5-2 |
| Wi-Fi | NVS (`mate_cfg`); sin credenciales en `config.h` |
| Provisioning | SoftAP `MatePoint-XXXX` + portal `192.168.4.1` + `ESP.restart()` |
| UI conectividad | `Error-wifi` (CTA Configurar red) · `Error-mqtt` (proveedor) · `Configurar red` |
| MQTT | Fijo `broker.hivemq.com:1883` |
| Herencia | Pausa/reanudar, bandeja, agua UART, VL53L0X, E2E compra |
| Doc | [`mate_point_v0-6/README.md`](mate_point_v0-6/README.md) · [`PLAN-MATE-POINT-v0-6.md`](PLAN-MATE-POINT-v0-6.md) |

---

## Changelog

| Fecha | Cambio |
|-------|--------|
| 2026-06-25 | **v0-6 E2E OK hardware** — Wi-Fi NVS + SoftAP + portal; doc [`mate_point_v0-6/README.md`](mate_point_v0-6/README.md) · [`arquitectura-mate-point.md`](../arquitectura-mate-point.md) §3 |
| 2026-06-24 | **v0-5-2 UI pausa** — Continuar oculto en cooldown; solo Finalizar; visible tras ~7 s |
| 2026-06-24 | **v0-5-2 fix pausa** — `abort_pause` cooldown 5 s; `standby_enable` al reanudar |
| 2026-06-24 | **Etapa 2b-8 implementada** — [`mate_point_v0-5-2/`](mate_point_v0-5-2/) pausa/reanudar; Test1 banco pendiente |
| 2026-06-24 | **Etapa 2b-8 planificada** — [`mate_point_v0-5-2/`](mate_point_v0-5-2/) pausa/reanudar Cargar termo; plan [`PLAN-MATE-POINT-v0-5-2.md`](PLAN-MATE-POINT-v0-5-2.md) |
| 2026-06-24 | **Etapa 2b-7** — [`mate_point_v0-5-1/`](mate_point_v0-5-1/) error agua UART; E2E OK banco (V6/V7); índice §16 |
| 2026-06-24 | **Etapa 2b-6 E2E OK** — [`mate_point_v0-5/`](mate_point_v0-5/); captura [`2026-06-24-Waveshare-Mate_point-v0-5_Test1.md`](../tools/nobana_uart_sniffer/capturas/2026-06-24-Waveshare-Mate_point-v0-5_Test1.md) |
| 2026-06-24 | **Etapa 2b-6 implementada** — [`mate_point_v0-5/`](mate_point_v0-5/); sensor bandeja GPIO6; Test1 banco pendiente |
| 2026-06-24 | **Cableado bandeja documentado** — reed NO @ GPIO6, pull-up 10 kΩ; esquema [`docs/hardware/sensor-bandeja-reed-gpio6.png`](../docs/hardware/sensor-bandeja-reed-gpio6.png); [`arquitectura-hardware.md`](../arquitectura-hardware.md) §3.4 · [`PLAN-MATE-POINT-v0-5.md`](PLAN-MATE-POINT-v0-5.md) |
| 2026-06-24 | **Etapa 2b-6 planificada** — [`mate_point_v0-5/`](mate_point_v0-5/) sensor bandeja reed NO @ GPIO6; plan [`PLAN-MATE-POINT-v0-5.md`](PLAN-MATE-POINT-v0-5.md) |
| 2026-06-18 | **Iteración tipografía/layout v0-4** — fuentes 20/32/36 px, fondos QR, títulos naranja/verde, ícono stop; doc [`UI-DISCREPANCIAS-v0-4.md`](UI-DISCREPANCIAS-v0-4.md) · [`PLAN-MATE-POINT-v0-4-UI.md`](PLAN-MATE-POINT-v0-4-UI.md) §18 |
| 2026-06-18 | **Etapa 2b-5 OK hardware** — [`mate_point_v0-4/`](mate_point_v0-4/); UI Figma + rotación 180° + partición 8 MB; E2E funcional Waveshare; doc [`README`](mate_point_v0-4/README.md) · [`UI/ASSETS.md`](../UI/ASSETS.md) |
| 2026-06-18 | **Etapa 2b-5 implementada** — [`mate_point_v0-4/`](mate_point_v0-4/); UI Figma LVGL; plan [`PLAN-MATE-POINT-v0-4-UI.md`](PLAN-MATE-POINT-v0-4-UI.md) |
| 2026-06-18 | **Etapa 2b-5 planificada** — UI Figma v0-4; plan [`PLAN-MATE-POINT-v0-4-UI.md`](PLAN-MATE-POINT-v0-4-UI.md); assets [`UI/figma/`](../UI/figma/) |
| 2026-06-17 | **Etapa 2b-4 E2E OK** — [`mate_point_v0-3-4/`](mate_point_v0-3-4/); VL53L0X + Iniciar; captura [`2026-06-17-Waveshare-Mate_point-v0-3-4_Test1.md`](../tools/nobana_uart_sniffer/capturas/2026-06-17-Waveshare-Mate_point-v0-3-4_Test1.md) |
| 2026-06-05 | **Etapa 2b-3 E2E OK** — fin natural timer + 2.ª compra tras Parar; captura [`2026-06-05-Waveshare-Mate_point-v0-3-3_Test1.md`](../tools/nobana_uart_sniffer/capturas/2026-06-05-Waveshare-Mate_point-v0-3-3_Test1.md) |
| 2026-06-05 | **Etapa 2b-3 OK** — [`mate_point_v0-3-3/`](mate_point_v0-3-3/) Test1 banco; corte ~300–500 ms |
| 2026-06-05 | **Etapa 2b-3** — [`mate_point_v0-3-3/`](mate_point_v0-3-3/); Parar 200 ms; UI contrato desacoplado; [`PLAN-MATE-POINT-v0-3.md`](PLAN-MATE-POINT-v0-3.md) §16 |
| 2026-06-05 | **Etapa 2b-2** — [`mate_point_v0-3-2/`](mate_point_v0-3-2/); UI dispensado (T_viva, countdown, Parar); [`PLAN-MATE-POINT-v0-3.md`](PLAN-MATE-POINT-v0-3.md) §15 |
| 2026-06-05 | **Etapa 2b-1 OK** — [`mate_point_v0-3-1/`](mate_point_v0-3-1/); Test1 E2E timer MQTT; captura [`2026-06-05-Waveshare-Mate_point-v0-3-1_Test1.md`](../tools/nobana_uart_sniffer/capturas/2026-06-05-Waveshare-Mate_point-v0-3-1_Test1.md) |
| 2026-06-05 | **v0-3-1 planificado e implementado** — timer MQTT, fin UI en stop |
| 2026-06-05 | **Etapa 2b** — [`mate_point_v0-3/`](mate_point_v0-3/); Test1 E2E parcial (superseded) |
| 2026-06-05 | **Etapa 2b planificada** — [`PLAN-MATE-POINT-v0-3.md`](PLAN-MATE-POINT-v0-3.md); §12/§16 |
| 2026-06-04 | **2a-WS cerrada en banco** — [`mate_point_UART_v0-3`](mate_point_UART_v0-3/); captura validación OK |
| 2026-06-04 | §16: Etapa **2a-WS** [`mate_point_UART_v0-3`](mate_point_UART_v0-3/); §12 próximo hito |
| 2026-06-04 | §16 índice UART; cabecera estado global; §4 estructura; pines 25/17 y `E2` |
| 2026-06-03 | POC v0.2 E2E hardware completado |
| 2026-05-29 | POC v0.1 operativo |
