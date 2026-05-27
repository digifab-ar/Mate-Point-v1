# Plan de implementación — Mate Point

**Proyecto:** Mate Point — Dispensador de agua caliente  
**OT:** OT-00268 — Etapa 3  
**Última actualización:** 2026-05-27  
**Repositorio:** [github.com/digifab-ar/Mate-Point-v1](https://github.com/digifab-ar/Mate-Point-v1)  
**Servidor (Railway):** `https://mate-point-v1-production.up.railway.app`

---

## Estado actual — resumen ejecutivo

| Área | Estado | Notas |
|------|--------|-------|
| **Fases 0–2** | **Completadas** | QR estático, sandbox, Postman + app |
| **Fase 3 — Backend** | **Completada** | Webhook + GET orden + MQTT publicado (prueba e2e 2026-05-27) |
| **Fase 4 — ESP32 / Nobana** | Pendiente | Suscribir firmware a `mate/MATEPOINT001/command` |
| **Fase 5 — Pantalla** | Pendiente | Waveshare + UX |
| **Fase 6 — Producción MP** | Pendiente | Credenciales prod, webhook modo productivo |
| **Opcional** | Pendiente | `POST /orders/create` en servidor (hoy: orden vía Postman) |

**Prueba e2e de referencia (2026-05-27):** orden `ORDTST01KSNFEN3H3FTHXMK9Q1ZPE5NZ`, ref. `mate-001-20260527-003`, $ 500 → logs Railway: `order_fetch_ok` → `dispense_triggered` → `mqtt_published`. Ver § resultados Fase 3.

---

## Fases generales

Ver tabla completa en `integracion-mercadopago-qr.md` §10. Resumen:

| Fase | Entregable | Estado |
|------|------------|--------|
| 0 | Definición precio, tiempo dispensado, `device_id` | **Completado** |
| 1 | App MP + sucursal + caja | **Completado** |
| 2 | Crear órdenes QR estático y probar pago (Postman) | **Completado** |
| 3 | Webhook + backend (Railway + Node.js + MQTT) | **Completado** |
| 4 | MQTT + ESP32 → UART Nobana (TXS0108E) | Pendiente |
| **5** | **Pantalla QR + UX máquina** | **Pendiente** |
| 6 | Producción (credenciales prod, HTTPS) | Pendiente |

---

## Fase 0 — Definición de parámetros del servicio

### Objetivo

Definir los parámetros operativos básicos del Mate Point antes de comenzar la integración con MercadoPago y el firmware.

### Pasos detallados

| Paso | Tarea | Valor / Resultado | Estado |
|------|-------|-------------------|--------|
| 0.1 | Definir precio de venta (en ARS) | **`MP_SALE_AMOUNT=500.00`** (configurable en servidor) | **Completado** |
| 0.2 | Definir tiempo de dispensado | **120 s (120 000 ms)** | **Completado** |
| 0.3 | Asignar `device_id` único | **`MATEPOINT001`** | **Completado** |
| 0.4 | Documentar parámetros en `integracion-mercadopago-qr.md` §10 | — | **Completado** |

### Criterios de aceptación Fase 0

- [x] Precio definido y documentado — **$ 500,00 ARS**
- [x] Tiempo de dispensado definido y documentado — **120 s (120 000 ms)**
- [x] `device_id` asignado, registrado en el backend y en el firmware — **`MATEPOINT001`**

---

## Fase 1 — App MercadoPago + sucursal + caja

### Objetivo

Configurar la cuenta de MercadoPago con la aplicación, sucursal y caja necesarias para cobros con **QR estático**.

### Pasos detallados

| Paso | Tarea | Estado |
|------|-------|--------|
| 1.1 | Crear aplicación en el Portal de Desarrolladores de MP | **Completado** |
| 1.2 | Obtener `access_token` de prueba (sandbox) | **Completado** |
| 1.3 | Crear sucursal (`external_store_id`) via API | **Completado** |
| 1.4 | Crear caja (`external_pos_id`) asociada a la sucursal | **Completado** |
| 1.5 | Verificar credenciales en Postman (GET /users/me) | **Completado** |

### Criterios de aceptación Fase 1

- [x] La aplicación MP existe y tiene credenciales sandbox activas
- [x] La sucursal y la caja están creadas y verificadas via API
- [x] El `access_token` de prueba devuelve datos correctos en `/users/me`

---

## Fase 2 — Crear órdenes QR y probar pago — modo estático (Postman)

### Objetivo

Verificar que el flujo **QR estático** funciona de extremo a extremo en sandbox:  
crear orden → el usuario escanea el QR fijo de la caja → pago aprobado → webhook recibido.

> **Decisión de prototipo:** se usa **modo `static`**. El QR físico de la caja (`MATEPOINT001POS001`) no cambia entre transacciones. El backend crea una orden, la vincula a ese QR, y MercadoPago notifica el pago vía webhook. No se genera `qr_data` por transacción.
>
> **QR estático de la caja:**  
> `https://www.mercadopago.com/instore/merchant/qr/132339357/5507995c943b40ea96c23d3b511b5bb3ad50efc5a5b940f39a305d7a5d413839.png`

### Pasos detallados

| Paso | Tarea | Estado |
|------|-------|--------|
| 2.1 | Crear orden via `POST /v1/orders` con `mode: "static"` y `external_pos_id: "MATEPOINT001POS001"` | **Completado** |
| 2.2 | Verificar en la respuesta que la orden queda en estado `created` / `ready_to_process` | **Completado** |
| 2.3 | Escanear el QR **fijo** de la caja con la app MP (cuenta de prueba comprador) | **Completado** |
| 2.4 | Simular pago aprobado y verificar que la orden pasa a `processed` | **Completado** |
| 2.5 | Verificar campos relevantes en el payload: `status`, `external_reference`, `id` (orden) | **Completado** |
| 2.6 | Documentar estructura de payload de orden y respuesta en `integracion-mercadopago-qr.md` §5.3 | **Completado** |

### Criterios de aceptación Fase 2

- [x] La orden se crea correctamente con `mode: "static"` — estado inicial: `created` / `ready_to_process`
- [x] El QR fijo de la caja es escaneable con la app MP en modo sandbox
- [x] El pago simulado hace que la orden pase a `status: processed` / `accredited`
- [x] Los campos `id`, `external_reference` y `status` están documentados
- [x] El flujo completo (crear orden → escanear QR → pago) está validado en sandbox ✅

---

## Fase 3 — Webhook + backend mínimo

### Objetivo

Implementar el servidor backend en **Railway** (Node.js + Express) que recibe las notificaciones de pago de MercadoPago, las valida contra la API de MP y publica el comando de dispensado vía MQTT.

> **Repositorio:** [github.com/digifab-ar/Mate-Point-v1](https://github.com/digifab-ar/Mate-Point-v1) — código en `servidor/`  
> **Deploy:** Railway — Root Directory `servidor` — `https://mate-point-v1-production.up.railway.app`  
> **Webhook MP (prueba):** `https://mate-point-v1-production.up.railway.app/webhook/mp` — evento **Order (Mercado Pago)**  
> **MQTT POC:** broker público [broker.hivemq.com](https://www.hivemq.com/mqtt/public-mqtt-broker/) — servidor `wss://broker.hivemq.com:8884/mqtt`, ESP32 `mqtt://broker.hivemq.com:1883`  
> Ver `servidor-mate-point.md` §3.2 y checklist §11.

### Pasos detallados

| Paso | Tarea | Estado |
|------|-------|--------|
| 3.1 | Repo [Mate-Point-v1](https://github.com/digifab-ar/Mate-Point-v1) — estructura `servidor/` Node.js + Express | **Completado** |
| 3.2 | Documentar y configurar MQTT (broker público HiveMQ en `.env` / Railway) | **Completado** |
| 3.3 | Deploy en Railway: repo conectado, variables de entorno, `GET /health` | **Completado** |
| 3.4 | `POST /webhook/mp`: recibir IPN, responder 200, log `webhook_received` | **Completado** |
| 3.4b | Validar `x-signature` (estrategia C) con `MP_WEBHOOK_SECRET` | **Completado** |
| 3.5 | Consultar orden via `GET /v1/orders/{id}` y verificar `processed` / `accredited` + `MP_SALE_AMOUNT` | **Completado** |
| 3.6 | Publicar MQTT `{ cmd: "dispense", duration_ms: 120000 }` en `mate/MATEPOINT001/command` | **Completado** |
| 3.7 | Log estructurado `dispense_triggered` / `mqtt_published` / `mqtt_failed` | **Completado** |
| 3.8 | URL webhook registrada en Portal MP (modo prueba) | **Completado** |
| 3.9 | Prueba e2e: pago → webhook → **MQTT** → log | **Completado** |
| 3.10 | Implementar `POST /orders/create` en el servidor (opcional: reemplazar Postman) | Pendiente |

### Criterios de aceptación Fase 3

- [x] `GET /health` en Railway responde (servicio desplegado)
- [x] El backend recibe notificaciones IPN de MP (`order.processed`, HTTP 200)
- [x] Validación `x-signature` implementada (estrategia C — no bloquea si falla)
- [x] Solo órdenes `processed` / `accredited` y monto `MP_SALE_AMOUNT` disparan MQTT
- [x] El mensaje MQTT llega al broker (`mqtt_published` en logs Railway)
- [x] Logs `dispense_triggered`, `signature_*`, `order_fetch_*` implementados
- [x] Flujo completo sandbox: pago → webhook → MQTT → log ✅

### Resultados prueba e2e — 2026-05-27

**Orden:** `ORDTST01KSNFEN3H3FTHXMK9Q1ZPE5NZ` · **Ref.:** `mate-001-20260527-003` · **Pago:** `PAY01KSNFEN4AH15WRVPMS8YQJ0BH` · **Monto:** $ 500,00

| Paso | Resultado |
|------|-----------|
| POST `/v1/orders` | `status: created` |
| Pago app MP (QR estático) | OK |
| Webhook Railway | `order.processed`, HTTP **200** |
| Firma `x-signature` | `signature_invalid` (`hmac_mismatch`) — estrategia **C**, no bloqueó |
| GET orden (servidor) | `order_fetch_ok`, `processed` |
| MQTT | `mqtt_published` → `mate/MATEPOINT001/command`, `duration_ms: 120000` |

**Secuencia de logs Railway:** `mqtt_connected` → `webhook_received` → `signature_invalid` → `order_fetch_ok` → `dispense_triggered` → `mqtt_published`.

**Nota firma:** coherente con doc MP (QR puede no validar HMAC). Seguridad efectiva vía **GET** + `MP_SALE_AMOUNT`. Revisar `MP_WEBHOOK_SECRET` en Railway solo si se quiere auditar `signature_valid`.

**Fuera de alcance de esta prueba:** ESP32 recibiendo MQTT y dispensado físico (Fase 4).

### Pendiente post–Fase 3

- `POST /orders/create` en el servidor (reemplazar Postman).
- (Opcional) Investigar `signature_valid` vs `hmac_mismatch`.
- Fase 4: firmware ESP32 + UART Nobana.

---

## Relevamiento del protocolo UART — Nobana (prerrequisito Fase 4)

El protocolo de comunicación entre el panel de control original (módulo ARMOR) y el PCB Nobana **no está documentado públicamente**. Debe obtenerse mediante **reverse engineering** del equipo físico usando el siguiente plan de tres fases.

### Hardware de relevamiento

| Componente | Rol |
|------------|-----|
| ESP32 (cualquier variante) | Sniffer / logger UART |
| TXS0108E | Level shifter 5V → 3.3V para proteger el ESP32 |
| PC con Serial Monitor / terminal | Captura y análisis de tramas |

Conexión del sniffer **en paralelo** al bus existente (sin desconectar el módulo ARMOR):

```
PCB Nobana TX (5V) ──┬──── RX Módulo ARMOR (5V)
                     └──── TXS0108E B1 → A1 ──── ESP32 UART1 RX (GPIO16)

PCB Nobana RX (5V) ──┬──── TX Módulo ARMOR (5V)
(= ARMOR TX)         └──── TXS0108E B2 → A2 ──── ESP32 UART2 RX (GPIO26)

5V  ──── TXS0108E VCCB
3.3V ─── TXS0108E VCCA + OE
GND común entre ESP32, TXS0108E y PCB Nobana
```

### Fase R1 — Identificar baudrate

**Objetivo:** determinar la velocidad del bus UART antes de intentar capturar tramas.

**Procedimiento:**
1. Conectar sniffer ESP32 + TXS0108E en paralelo al TX del PCB Nobana (línea hacia el módulo ARMOR)
2. Cargar sketch que prueba baudrates secuencialmente: **4800 → 9600 → 19200 → 38400 → 57600 → 115200**
3. Encender el dispensador y observar los bytes recibidos en Serial Monitor
4. El baudrate correcto produce bytes con patrón repetible y coherente (no basura aleatoria)

**Resultado esperado:** baudrate identificado y confirmado.

> Hipótesis inicial: **9600 bps** — es el estándar más común en paneles de electrodomésticos de origen chino de esta generación.

### Fase R2 — Sniffing bidireccional PCB Nobana ↔ Módulo ARMOR

**Objetivo:** capturar todas las tramas en ambas direcciones con el sistema funcionando normalmente.

**Procedimiento:**
1. Con el baudrate ya conocido, conectar el sniffer a **ambas líneas** simultáneamente (ESP32 UART1 + UART2 via TXS0108E)
2. Registrar tráfico en reposo (idle del dispensador)
3. Registrar tráfico al presionar cada botón del módulo ARMOR: Coffee 85°C, Tea 100°C, Honey 65°C, Milk 45°C, UV care, Cooling, Volume ▲/▼
4. Registrar tráfico durante el dispensado activo
5. Registrar tráfico en condición "Water lack" (sin agua)
6. Volcar el log completo con timestamps y dirección de cada trama

**Herramientas:**
- Sketch ESP32 con log HEX + ASCII + timestamp + dirección (`[NOB→ARM]` / `[ARM→NOB]`)
- Serial Monitor a 115200 bps hacia PC
- Copiar log a archivo `.txt` para análisis posterior

### Fase R3 — Documentar tramas y mapear acciones

**Objetivo:** construir el mapa completo del protocolo para implementarlo en el firmware del Waveshare.

**Procedimiento:**
1. Analizar el log de la Fase R2 e identificar patrones por acción
2. Completar la tabla de mapeo:

| Acción | Dirección | Trama HEX | Notas |
|--------|-----------|-----------|-------|
| Dispensar agua caliente (Coffee 85°C) | ARM → NOB | ??? | A relevar |
| Dispensar Tea 100°C | ARM → NOB | ??? | A relevar |
| Dispensar Honey 65°C | ARM → NOB | ??? | A relevar |
| Dispensar Milk 45°C | ARM → NOB | ??? | A relevar |
| Activar UV | ARM → NOB | ??? | A relevar |
| Cooling | ARM → NOB | ??? | A relevar |
| Heartbeat / polling | NOB → ARM | ??? | A relevar |
| Temperatura actual | NOB → ARM | ??? | A relevar |
| Estado water lack | NOB → ARM | ??? | A relevar |

3. Identificar estructura de trama: header, longitud, checksum, terminador
4. Incorporar resultados en este documento (tabla completada arriba)
5. Actualizar `arquitectura-hardware.md` §2.3 con los comandos definitivos

### Parámetros UART — estimación inicial

> A confirmar en Fase R1.

| Parámetro | Valor estimado |
|-----------|---------------|
| Baudrate | 9600 bps |
| Bits de datos | 8 |
| Paridad | None |
| Stop bits | 1 |
| Nivel eléctrico | **5V TTL** (confirmado en inspección física) |

### Consideraciones de integración

| Tema | Detalle |
|------|---------|
| Nivel lógico | Bus a **5V TTL** confirmado. Adaptación mediante **TXS0108E** (3.3V ↔ 5V) entre UART2 del Waveshare y PCB Nobana |
| Level shifter | TXS0108E: VCCA = 3.3V (Waveshare), VCCB = 5V (Nobana), OE = 3.3V (siempre habilitado). Apto para UART push-pull |
| Aislamiento | Considerar optoacoplador si la electrónica de potencia del dispensador genera ruido en el bus UART (a evaluar tras Fase R2 de relevamiento) |
| Panel original | Desconectar el módulo ARMOR (display + botones) una vez completado el relevamiento — el Waveshare toma control total vía UART2 |
| Sniffing sin desconectar | Durante las Fases R1 y R2 el sniffer se conecta **en paralelo** — el módulo ARMOR permanece conectado y el sistema funciona con normalidad |
| Alimentación Waveshare | El módulo Waveshare se alimenta por USB 5V; puede derivarse de la fuente interna del Nobana si entrega 5V estables (a verificar con multímetro) |
| Watchdog | El firmware ESP32-S3 debe implementar watchdog: si pierde comunicación con el PCB Nobana, detener cualquier dispensado activo y reportar error vía MQTT |

---

## Fase 4 — MQTT + ESP32 → UART Nobana

### Objetivo

Establecer el broker MQTT e implementar en el firmware del Waveshare ESP32-S3 la lógica que, al recibir un comando de dispensado, lo traduce en la trama UART correspondiente y la envía al PCB Nobana a través del TXS0108E.

> **No existe relé electromecánico.** El dispensado se activa enviando el comando UART apropiado al PCB Nobana (equivalente al botón HOT del panel original). La interfaz física es: UART2 del Waveshare → TXS0108E (3.3V ↔ 5V) → PCB Nobana UART (5V TTL). Ver `arquitectura-hardware.md` §2.2 y §2.3.

### Pasos detallados

| Paso | Tarea | Dependencia | Estado |
|------|-------|-------------|--------|
| 4.1 | Levantar broker MQTT (Mosquitto) en el servidor backend | — | Pendiente |
| 4.2 | Definir topics: `mate/{device_id}/command` (sub), `mate/{device_id}/status` (pub) | — | Pendiente |
| 4.3 | Implementar cliente MQTT en el firmware Waveshare; suscribirse a `command` | Broker (4.1) | Pendiente |
| 4.4 | Implementar driver UART2 → TXS0108E → PCB Nobana con los parámetros relevados | Relevamiento UART completo | Pendiente |
| 4.5 | Al recibir `{ cmd: "dispense", duration_ms }`: enviar comando UART HOT al Nobana y mantenerlo activo durante `duration_ms` | 4.3, 4.4 | Pendiente |
| 4.6 | Al vencer `duration_ms`: enviar comando UART STOP al Nobana | 4.5 | Pendiente |
| 4.7 | Publicar estado en `mate/{device_id}/status`: `dispensing`, `idle`, `error` | 4.5 | Pendiente |
| 4.8 | Implementar watchdog UART: si no hay respuesta del Nobana en N ms, detener dispensado y publicar error | 4.4 | Pendiente |
| 4.9 | Integrar lectura de temperatura desde el protocolo UART Nobana y publicar en `status` | 4.4 | Pendiente |
| 4.10 | Prueba de integración end-to-end: mensaje MQTT → comando UART → dispensado real | 4.1–4.8 | Pendiente |

### Diagrama de flujo Fase 4

```
Backend                  Broker MQTT              Waveshare ESP32-S3
   │                         │                           │
   │─ PUBLISH ──────────────►│                           │
   │  mate/{id}/command      │──── SUBSCRIBE ───────────►│
   │  { cmd: "dispense",     │     (recibe payload)      │
   │    duration_ms: 120000 }  │                           │
   │                         │                    UART2 TX ──► TXS0108E ──► PCB Nobana
   │                         │                    (trama HOT,  3.3V→5V      (activa
   │                         │                     120 segundos / 2 min)              dispensado)
   │                         │                           │
   │                         │◄── PUBLISH ───────────────│
   │                         │    mate/{id}/status       │
   │                         │    { state: "dispensing" }│
   │                         │                           │
   │                         │         (t = duration_ms) │
   │                         │                    UART2 TX ──► TXS0108E ──► PCB Nobana
   │                         │                    (trama STOP)             (detiene
   │                         │                           │                  dispensado)
   │                         │◄── PUBLISH ───────────────│
   │                         │    { state: "idle" }      │
```

### Criterios de aceptación Fase 4

- [ ] El broker MQTT responde y acepta conexiones desde el firmware y el backend
- [ ] El firmware del Waveshare se suscribe al topic `command` y mantiene la conexión activa
- [ ] Al recibir `cmd: "dispense"` el Waveshare envía la trama UART correcta al PCB Nobana y el dispensador activa el calentador/dispensado
- [ ] El dispensado se detiene automáticamente al vencer `duration_ms`
- [ ] El estado `dispensing` / `idle` / `error` se publica correctamente en el topic `status`
- [ ] El watchdog UART detiene el dispensado y reporta error si el Nobana no responde
- [ ] La temperatura leída del Nobana se incluye en los mensajes de `status`

---

## Fase 5 — Pantalla QR + UX (Waveshare ESP32-S3-Touch-LCD-7B)

Corresponde a la integración del módulo de pantalla descrito en `modulo-waveshare-esp32s3-touch-7b.md` y la arquitectura definida en `arquitectura-mate-point.md`.

### Pasos detallados

| Paso | Tarea | Dependencia | Estado |
|------|-------|-------------|--------|
| 5.1 | Portar LVGL al módulo: hello world + touch test + display encendido | — | Pendiente |
| 5.2 | Implementar máquina de estados + 5 pantallas con datos hardcodeados | 5.1 | Pendiente |
| 5.3 | Integrar cliente MQTT; suscribirse a topics y verificar recepción | Broker MQTT (Fase 4) | Pendiente |
| 5.4 | Mostrar QR estático (imagen descargada al boot por HTTP) | 5.1, Wi-Fi | Pendiente |
| 5.5 | Integrar QR dinámico: recibir `qr_data` por MQTT y renderizar con `lv_lib_qrcode` | 5.3, 5.4 | Pendiente |
| 5.6 | Countdown de expiración, cancelación por toque, manejo de errores | 5.2, 5.5 | Pendiente |
| 5.7 | Ajuste visual final: fuentes, colores, logo, statusbar WiFi/MQTT | 5.2 | Pendiente |

### Criterios de aceptación Fase 5

- [ ] La pantalla arranca en `IDLE` mostrando precio y logo
- [ ] Al tocar, muestra animación de espera mientras el backend genera el QR
- [ ] El QR dinámico se renderiza correctamente y es escaneable con la app MP
- [ ] El countdown refleja el `expiry_ms` recibido en el payload MQTT
- [ ] Al recibir `cmd: "dispense"` pasa a pantalla `DISPENSING` con barra de progreso
- [ ] La pantalla vuelve a `IDLE` automáticamente al terminar el dispensado
- [ ] Los errores de red / timeout muestran pantalla `ERROR` con opción de reintentar
- [ ] Los indicadores WiFi y MQTT en el footer reflejan el estado real de conectividad

---

## Fase 6 — Producción

### Objetivo

Migrar el sistema de sandbox a producción: credenciales reales de MercadoPago, infraestructura con HTTPS fijo y despliegue estable.

### Pasos detallados

| Paso | Tarea | Dependencia | Estado |
|------|-------|-------------|--------|
| 6.1 | Obtener `access_token` de producción en el Portal MP (cuenta real) | — | Pendiente |
| 6.2 | Reemplazar credenciales sandbox por las de producción en el backend | 6.1 | Pendiente |
| 6.3 | Configurar dominio fijo + certificado TLS/HTTPS (Let's Encrypt o similar) | — | Pendiente |
| 6.4 | Actualizar URL del webhook en el Portal de Desarrolladores de MP con el dominio de producción | 6.3 | Pendiente |
| 6.5 | Configurar broker MQTT con TLS (puerto 8883) o tunelizar via WSS | 6.3 | Pendiente |
| 6.6 | Actualizar `wifi_config` y `mqtt_broker_url` en el firmware del Waveshare con los datos de producción | 6.3, 6.5 | Pendiente |
| 6.7 | Prueba de pago real de extremo a extremo: MP → webhook → MQTT → UART Nobana → dispensado | 6.1–6.6 | Pendiente |
| 6.8 | Monitoreo post-deploy: verificar logs de transacciones, alertas de error y uptime del broker | 6.7 | Pendiente |

### Criterios de aceptación Fase 6

- [ ] El backend usa credenciales de producción de MercadoPago
- [ ] El webhook está registrado en MP con URL HTTPS fija (no ngrok)
- [ ] El broker MQTT acepta conexiones seguras (TLS)
- [ ] El firmware del Waveshare se conecta al broker de producción
- [ ] Un pago real en la app MP dispara correctamente el dispensado
- [ ] El sistema opera de forma autónoma sin intervención manual

---

## Notas de dependencias entre fases

```
Relevamiento UART Nobana (§ anterior)
  └── prerrequisito para Fase 4: sin el protocolo UART no se puede enviar los comandos
      correctos al PCB Nobana para activar el dispensado

Fase 3 (Webhook + backend)
  └── necesario para que el servidor valide el pago y publique MQTT "dispense"

Fase 4 (MQTT + ESP32 → UART Nobana)
  └── necesario para tener broker funcional y topic "command" activo
  └── el comando de dispensado es una trama UART al PCB Nobana (NO un relé electromecánico)
  └── Fase 5.3 (cliente MQTT en pantalla) puede desarrollarse en paralelo con broker local (Mosquitto)

Fase 5 (Pantalla)
  └── 5.1–5.2 pueden hacerse sin backend (datos hardcodeados)
  └── 5.3–5.6 requieren Fase 4 completa o broker de prueba

Fase 6 (Producción)
  └── requiere Fases 3, 4 y 5 completas y validadas en sandbox
```

---

## Historial

| Fecha | Cambio |
|-------|--------|
| 2026-05-22 | Documento creado a partir de la reorganización de `modulo-waveshare-esp32s3-touch-7b.md` |
| 2026-05-26 | Actualización de dependencias: relevamiento UART Nobana como prerrequisito de Fase 4 |
| 2026-05-27 | Incorporado plan completo de relevamiento UART Nobana (3 fases) y consideraciones de integración hardware |
| 2026-05-27 | Reordenamiento de secciones: Relevamiento UART Nobana (prereq Fase 4) ahora precede a Fase 5 |
| 2026-05-27 | Documento completo: secciones detalladas para todas las fases (0–6) en orden consecutivo; Fase 4 actualizada — sin relé, comando vía UART2 → TXS0108E → PCB Nobana |
| 2026-05-27 | Fase 0 completada: precio $ 500,00 ARS, tiempo dispensado 120 s (120 000 ms), `device_id` = `MATEPOINT001` |
| 2026-05-27 | Fase 2 completada: flujo QR estático validado en sandbox (orden `ORDTST01KSN8G14TKMBSTCF1G4TXJ355`, pago `accredited`) |
| 2026-05-27 | Fase 3 definida: deploy Railway (Node.js), broker HiveMQ, `servidor-mate-point.md` |
| 2026-05-27 | Fase 3 en curso: Railway `mate-point-v1-production`, webhook MP recibe `order.processed` (orden `ORDTST01KSNCYH61MNGYP5Q27G0Y5RJD`) |
| 2026-05-27 | **Fase 3 completada** — e2e `ORDTST01KSNFEN3H3FTHXMK9Q1ZPE5NZ`: `order_fetch_ok` → `dispense_triggered` → `mqtt_published` |
