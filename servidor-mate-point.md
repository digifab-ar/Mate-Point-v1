# Servidor Mate Point — Backend + MQTT

**Proyecto:** Mate Point — Dispensador de agua caliente  
**OT:** OT-00268 — Etapa 3  
**Repositorio:** [github.com/digifab-ar/Mate-Point-v1](https://github.com/digifab-ar/Mate-Point-v1)  
**Código servidor:** carpeta [`servidor/`](servidor/)  
**Última actualización:** 2026-05-29  
**Deploy:** `https://mate-point-v1-production.up.railway.app`  
**Fase 3:** **Completada** — prueba e2e 2026-05-27 (`ORDTST01KSNFEN3H3FTHXMK9Q1ZPE5NZ`, `mqtt_published`)  
**POC v0.2:** `POST /orders/create` y `/orders/cancel` — firmware [`mate_point_v0-2`](../mate_point_firmware/mate_point_v0-2/)

---

## 1. Rol del servidor

El servidor es el nodo central del sistema. Sus responsabilidades:

| Función | Descripción |
|---------|-------------|
| Crear órdenes MP | `POST /v1/orders` hacia la API de MercadoPago |
| Recibir webhook | `POST /webhook/mp` — notificaciones de pago de MP |
| Verificar pago | `GET /v1/orders/{id}` — confirmar `status: processed / accredited` |
| Publicar MQTT | `mate/{device_id}/command` → `{ cmd: "dispense", duration_ms: 120000 }` |
| Log de transacciones | Registro de cada pago recibido con timestamp y resultado |

> El ESP32 **nunca** se comunica directamente con MercadoPago. Solo escucha el broker MQTT.

---

## 2. Entorno de despliegue — Railway

**Railway** es la plataforma de deploy para el prototipo y actuará como entorno de producción permanente (no hay migración posterior — Railway provee HTTPS desde el día 1).

| Atributo | Valor |
|----------|-------|
| Plataforma | [railway.app](https://railway.app) |
| URL pública | `https://<proyecto>.up.railway.app` (asignada al deploy) |
| HTTPS | Automático — certificado TLS gestionado por Railway |
| Deploy | Desde repositorio GitHub (push → redeploy automático) |
| Variables de entorno | Panel Railway → Variables (nunca en el repo) |
| Costo prototipo | Plan Hobby: ~$5 USD/mes; Trial: crédito inicial gratuito |

> La URL de Railway se registra directamente en el Portal de MP Developers como URL del webhook — sin ngrok, sin redirecciones temporales.

---

## 3. Stack tecnológico

### 3.1 Backend — Node.js + Express

| Componente | Paquete | Rol |
|------------|---------|-----|
| Runtime | Node.js 20 LTS | — |
| Framework HTTP | `express` | Router, middleware |
| Cliente MP | `mercadopago` (SDK oficial) | Crear órdenes, consultar pagos |
| Cliente MQTT | `mqtt` (mqtt.js) | Publicar comandos al broker |
| Variables de entorno | `dotenv` | Carga `.env` local |
| Logger | `morgan` o `pino` | Log de requests y transacciones |

**`package.json` mínimo:**

```json
{
  "name": "servidor-mate-point",
  "version": "1.0.0",
  "engines": { "node": ">=20" },
  "scripts": {
    "start": "node src/index.js",
    "dev": "nodemon src/index.js"
  },
  "dependencies": {
    "dotenv": "^16.0.0",
    "express": "^4.18.0",
    "mercadopago": "^2.0.0",
    "mqtt": "^5.0.0",
    "morgan": "^1.10.0"
  },
  "devDependencies": {
    "nodemon": "^3.0.0"
  }
}
```

### 3.2 Broker MQTT — alternativas para el prototipo

El broker MQTT es **independiente** del backend en Railway. El servidor y el ESP32 se conectan al mismo broker por Internet.

> **¿Por qué no Mosquitto en Railway?**  
> Railway expone HTTP/HTTPS de forma nativa. MQTT TCP (puerto 1883/8883) requiere **TCP Proxy** (plan de pago). Para el prototipo conviene un broker externo gratuito.

#### Comparativa

| Opción | Host / URL | Puerto | Protocolo | Cuenta | ESP32 | Servidor (Node) | Recomendación POC |
|--------|------------|--------|-----------|--------|-------|-----------------|-------------------|
| **A — HiveMQ público (WSS)** | `broker.hivemq.com` | **8884** | MQTT sobre WebSocket seguro (`/mqtt`) | No | Requiere cliente WSS en firmware | `wss://broker.hivemq.com:8884/mqtt` con mqtt.js | **✅ Servidor Railway** |
| **B — HiveMQ público (TCP)** | `broker.hivemq.com` | **1883** | MQTT TCP sin cifrar | No | PubSubClient / estándar | `mqtt://broker.hivemq.com:1883` | **✅ ESP32** (más simple en firmware) |
| **C — HiveMQ Cloud** | `xxxx.s1.eu.hivemq.cloud` | **8883** | MQTT over TLS | Sí (free tier) | Sí, con TLS | `ssl://...:8883` + user/pass | Producción del prototipo |
| **D — EMQX Cloud** | cluster EMQX | 8883 | MQTT TLS | Sí (free tier) | Sí | Igual que C | Alternativa a C |
| **E — Mosquitto en VPS** | IP propia | 1883/8883 | TCP / TLS | Autogestionado | Sí | Sí | Más trabajo de ops |

#### Decisión para Mate Point v1 (POC)

| Cliente | Broker | URL de conexión |
|---------|--------|-----------------|
| **Servidor (Railway)** | HiveMQ público WSS | `MQTT_BROKER_URL=wss://broker.hivemq.com:8884/mqtt` |
| **ESP32-S3** | HiveMQ público TCP | `mqtt://broker.hivemq.com:1883` (mismo broker, otro transporte) |

Ambos publican/suscriben los mismos topics (`mate/MATEPOINT001/...`). No hace falta crear cluster en HiveMQ Cloud para arrancar.

**Limitaciones del broker público** ([broker.hivemq.com](https://www.hivemq.com/mqtt/public-mqtt-broker/)):

- Sin autenticación: cualquiera que adivine el topic puede leer o publicar.
- Uso solo para desarrollo / demo — no datos sensibles.
- Mitigar en POC: topics largos y no obvios (`mate/MATEPOINT001/command`), sin PII en payloads.
- El puerto **8884** es **WSS** (WebSocket), no MQTT/TLS nativo en 8883. La ruta obligatoria es **`/mqtt`** ([documentación listeners HiveMQ](https://docs.hivemq.com/hivemq/latest/user-guide/listeners.html)).

**Cuándo pasar a HiveMQ Cloud (opción C):**

- Credenciales por dispositivo, TLS 8883 en ESP32 y servidor con el mismo host.
- Menor riesgo si el prototipo sale del laboratorio.

Variables en Railway y local: ver [`servidor/.env.example`](servidor/.env.example).

---

## 4. Arquitectura del sistema

```
MercadoPago API
      │
      │  webhook POST /webhook/mp (HTTPS)
      ▼
┌─────────────────────────────┐
│   Railway — servidor-mp     │
│                             │
│  POST /orders/create        │◄── ESP32 v0.2 (Comprar) / Postman
│  POST /orders/cancel        │◄── ESP32 v0.2 (timeout 2 min)
│  POST /webhook/mp           │
│  GET  /health               │
│                             │
│  mqtt.publish(command)      │──► broker.hivemq.com ──► ESP32-S3
└─────────────────────────────┘    (WSS :8884 / TCP :1883)
```

**Flujo completo de un dispensado:**

```
1. [Máquina]  Orden creada vía POST /orders/create → MP responde status: created
2. [Usuario]  Escanea QR estático → paga en app MP
3. [MP]       POST /webhook/mp → { topic: "merchant_order", id: "..." }
4. [Servidor] GET /v1/orders/{id} → verifica status: processed / accredited
5. [Servidor] mqtt.publish("mate/MATEPOINT001/command", { cmd:"dispense", duration_ms:120000 })
6. [ESP32]    Recibe payload MQTT → envía trama UART al PCB Nobana → dispensa 120 s
7. [ESP32]    mqtt.publish("mate/MATEPOINT001/status", { state:"dispensing" })
8. [ESP32]    (t+120s) mqtt.publish("mate/MATEPOINT001/status", { state:"idle" })
```

---

## 5. Endpoints del servidor

| Método | Path | Descripción |
|--------|------|-------------|
| `GET` | `/health` | Healthcheck — Railway lo usa para verificar que el servicio está vivo |
| `POST` | `/orders/create` | Crea una orden QR estático en MP (ESP32 v0.2 al pulsar Comprar, o Postman) |
| `POST` | `/orders/cancel` | Cancela orden en status `created` (timeout UI 2 min en v0.2) |
| `POST` | `/webhook/mp` | Recibe notificaciones IPN de MercadoPago |

### 5.1 `POST /webhook/mp` — flujo interno

```
Recibir POST
  │
  ├─ Leer header x-signature  ──► validar HMAC-SHA256 con MP_WEBHOOK_SECRET
  │    │
  │    └─ Si falla → responder 401, loguear y salir
  │
  ├─ Leer body: { topic, id }
  │    │
  │    └─ Si topic ≠ "merchant_order" → responder 200 y salir (ignorar otros eventos)
  │
  ├─ GET /v1/orders/{id} con ACCESS_TOKEN_SANDBOX
  │    │
  │    └─ Si status ≠ "processed" o status_detail ≠ "accredited" → 200, loguear, salir
  │
  ├─ Publicar MQTT: mate/MATEPOINT001/command
  │    payload: { cmd: "dispense", duration_ms: 120000, order_id: id, ts: Date.now() }
  │
  └─ Responder HTTP 200 en < 22 s (requisito MP)
```

### 5.2 Especificación — `x-signature` + `GET /v1/orders/{id}` (sin implementar aún)

Documento de diseño para cerrar Fase 3. Referencia oficial: [Webhooks — Mercado Pago](https://www.mercadopago.com.ar/developers/es/docs/your-integrations/notifications/webhooks).

#### A. Estrategia de firma — **adoptada: C (híbrida)**

La documentación de MP indica que las notificaciones de **Código QR** pueden no ser verificables con la clave secreta. En pruebas del prototipo **sí llegó** `x-signature` con `order.processed`, pero eso no garantiza que el HMAC valide siempre.

| Estrategia | Descripción | Decisión |
|------------|-------------|----------|
| **A — Firma + GET** | Si firma falla → 401, no dispensar | **No** — arriesga bloquear pagos reales si QR no valida firma |
| **B — Solo GET** | Ignorar firma por completo | **No** — desaprovecha seguridad cuando la firma sí funciona |
| **C — Híbrida** | Intentar firma; **siempre** exigir GET; si firma falla, log y continuar solo con GET | **✅ Adoptada** |

**Por qué C (recomendación del equipo):**

1. **A** es demasiado estricta para QR: si MP no firma bien en este producto, el dispensador nunca activaría pese a cobros legítimos.
2. **B** es simple pero deja de lado una capa gratis cuando la firma sí coincide (tu log ya trae `x-signature`).
3. **C** intenta la firma (auditoría + defensa ante POST falsos si valida), y el **GET a MP es la puerta obligatoria** para dispensar — alineado con la doc QR y con pagos verificados con el token del vendedor.

**Comportamiento concreto (estrategia C):**

- Firma **válida** → log `signature_valid` → seguir a GET.
- Firma **inválida** → log `signature_invalid` → **no** devolver 401; seguir a GET igual.
- GET **OK** + reglas de monto/estado → MQTT.
- GET **falla** o orden no acreditada → HTTP 200, sin MQTT.

---

#### B. Flujo propuesto en `POST /webhook/mp`

```
POST /webhook/mp
│
├─ 1. Responder rápido: objetivo total < 22 s (requisito MP)
│
├─ 2. Extraer datos de la notificación
│     • Query: data.id, data.external_reference, type
│     • Headers: x-signature, x-request-id
│     • Body: action, type, data (JSON)
│
├─ 3. Filtro de evento
│     • Procesar solo si action === "order.processed"
│     • Cualquier otro action → HTTP 200 + log ignore (no dispensar)
│
├─ 4. Validación x-signature (estrategia C)
│     • Ver §5.2.1 — intentar HMAC; si falla → log, continuar a GET (no 401)
│
├─ 5. Resolver order_id
│     • Preferir query data.id; fallback body.data.id
│     • Ejemplo real: ORDTST01KSNCYH61MNGYP5Q27G0Y5RJD
│
├─ 6. Idempotencia (§5.2.3)
│     • Si order_id ya fue dispensado → HTTP 200 + log duplicate (no MQTT)
│
├─ 7. GET /v1/orders/{order_id}  (§5.2.2)
│     • Authorization: Bearer MP_ACCESS_TOKEN (sandbox)
│     • Si HTTP ≠ 200 o timeout → HTTP 200 + log error (no dispensar)
│
├─ 8. Validar orden en respuesta GET
│     • status === "processed"
│     • status_detail === "accredited"
│     • config.qr.external_pos_id === MP_EXTERNAL_POS_ID (opcional, recomendado)
│     • total_paid_amount === total_amount === MP_SALE_AMOUNT (obligatorio, ver §5.2.7)
│     • Si no cumple → HTTP 200 + log skip (no dispensar)
│
├─ 9. Publicar MQTT dispense (§9)
│     • topic mate/MATEPOINT001/command
│     • payload: cmd, duration_ms, order_id, external_reference, ts
│
├─ 10. Log dispense_triggered + marcar idempotencia
│
└─ 11. HTTP 200 { ok: true }
```

> **No usar solo el body del webhook para dispensar.** Aunque traiga `processed/accredited`, el GET confirma contra MP con el token del vendedor y evita spoofing si la firma no aplica en QR.

---

#### 5.2.1 Validación `x-signature` (HMAC-SHA256)

**Variable:** `MP_WEBHOOK_SECRET` — clave del panel → Webhooks → **Clave secreta** (modo prueba; otra clave en modo productivo).

**Header recibido:**

```http
x-signature: ts=1704908010,v1=618c85345248dd820d5fd456117c2ab2ef8eda45a0282ff693eac24131a5e839
x-request-id: 5a9906b9-2088-45f4-ab7d-98fd29c9dc83
```

**Algoritmo (oficial MP):**

1. Parsear `x-signature`: separar por `,`, extraer `ts` y `v1` (hash esperado).
2. Obtener **`data.id` de los query params** de la URL (no del body). En Express: `req.query['data.id']`.
3. Si `data.id` es alfanumérico → usar **minúsculas** en el manifest.
4. Armar el **manifest** (solo incluir partes que existan):

```text
id:{data.id};request-id:{x-request-id};ts:{ts};
```

Ejemplo con la prueba real:

```text
id:ordtst01ksncyh61mngyp5q27g0y5rjd;request-id:5a9906b9-2088-45f4-ab7d-98fd29c9dc83;ts:...;
```

5. Calcular: `HMAC_SHA256(secret, manifest)` en hex.
6. Comparar con `v1` del header (comparación segura, timing-safe si es posible).

**Errores frecuentes (evitar):**

| Error | Correcto |
|-------|----------|
| Usar `req.body.data.id` para el manifest | Usar **query** `data.id` |
| No bajar a minúsculas el id alfanumérico | `toLowerCase()` en id para manifest |
| Usar `req.body.id` (notificación tipo payment legacy) | Para Orders: `data.id` en query |

**Timestamp (opcional, recomendado):**

- Comparar `ts` del header con `Date.now()/1000` (MP documenta `ts` en segundos en ejemplos).
- Tolerancia sugerida: **± 5 minutos** para descartar replays viejos.
- Si está fuera de ventana → no dispensar, log `signature_ts_stale`.

**Si falta `MP_WEBHOOK_SECRET` en Railway:**

- No dispensar; log `config_error`; HTTP 500 o 200 según política (ver consulta abajo).

---

#### 5.2.2 `GET /v1/orders/{id}` antes de dispensar

**Request:**

```http
GET https://api.mercadopago.com/v1/orders/{order_id}
Authorization: Bearer {MP_ACCESS_TOKEN}
```

- `{order_id}` = ID tal como lo devuelve la API (ej. `ORDTST01KSNCYH61MNGYP5Q27G0Y5RJD`) — **mismo casing** que en la respuesta de creación, no necesariamente el del manifest en minúsculas.
- Token: **credenciales de prueba** mientras `live_mode: false` en el webhook.

**Condiciones para disparar MQTT (todas obligatorias):**

| # | Campo en respuesta GET | Valor esperado |
|---|------------------------|----------------|
| 1 | `status` | `processed` |
| 2 | `status_detail` | `accredited` |
| 3 | `type` | `qr` |
| 4 | `config.qr.mode` | `static` |
| 5 | `config.qr.external_pos_id` | `MATEPOINT001POS001` |
| 6 | `transactions.payments[0].status` | `processed` (redundante; refuerzo) |

**Validaciones adicionales:**

| Campo | Regla |
|-------|--------|
| `total_amount` | Debe ser igual a **`MP_SALE_AMOUNT`** (variable de entorno, ej. `500.00`) |
| `total_paid_amount` | Igual a `total_amount` |
| `external_reference` | Prefijo `mate-001-` (opcional) |
| `user_id` | Coincidir con `MP_USER_ID` (opcional en POC) |

> Al cambiar el precio, el dueño actualiza **`MP_SALE_AMOUNT`** en Railway (y en órdenes creadas vía `POST /orders/create` cuando exista). El ESP32 **no** lleva el monto — solo `duration_ms`. Mínimo MP: **$ 15,00 ARS**.

**Si GET devuelve `created` u otro estado:**

- Webhook llegó antes de que MP terminó de procesar → HTTP **200**, log `order_not_ready`, **no** MQTT.
- Opcional (fase posterior): reintento interno 1–2 veces con delay 2 s; no obligatorio en POC.

**Correlación webhook ↔ GET:**

| Fuente | order_id | Uso |
|--------|----------|-----|
| Query `data.id` | ORDTST… | Firma + GET |
| Body `data.id` | ORDTST… | Fallback + logs |
| Body `data.external_reference` | mate-001-… | Log MQTT / trazabilidad |

---

#### 5.2.3 Idempotencia (evitar doble dispensado)

MP puede reenviar el mismo webhook. Sin idempotencia, se publicarían dos MQTT.

**Regla:** antes de MQTT, comprobar si `order_id` ya fue procesado.

| Implementación POC | Descripción |
|--------------------|-------------|
| **Memoria (Map)** | `Set` en RAM del proceso — pierde estado si Railway reinicia |
| **Archivo / Redis** | Mejor para producción; no obligatorio en POC |

**Ventana:** conservar `order_id` procesados al menos **24 h** (o hasta reinicio del dyno en POC).

**Log en duplicado:** `{ event: "dispense_duplicate", order_id }` → HTTP 200.

---

#### 5.2.4 Códigos HTTP de respuesta al webhook

| Situación | HTTP | ¿Dispensar? |
|-----------|------|-------------|
| Firma inválida (estrategia A) | **401** | No |
| Firma inválida (estrategia C) | **200** | Solo si GET OK |
| `action` ≠ `order.processed` | **200** | No |
| Orden no lista en GET | **200** | No |
| Orden validada + MQTT OK | **200** | Sí |
| Orden validada + MQTT falla | **200** | No — log `mqtt_failed` |
| Duplicado idempotencia | **200** | No |

**Decisión equipo:** si MQTT falla pero GET OK → **HTTP 200** + log `mqtt_failed` (no reintentar vía MP; reconciliar manual o reintento interno futuro).

---

#### 5.2.5 Logs estructurados esperados

| event | Cuándo |
|-------|--------|
| `webhook_received` | Siempre (ya existe) |
| `signature_valid` / `signature_invalid` | Tras paso 4 |
| `order_fetch_ok` / `order_fetch_failed` | Tras GET |
| `order_not_ready` | GET con status ≠ processed |
| `dispense_triggered` | Antes de MQTT |
| `mqtt_published` / `mqtt_failed` | Tras MQTT |
| `dispense_duplicate` | Idempotencia |

---

#### 5.2.6 Checklist de implementación

- [x] Parsear query `data.id` para firma (minúsculas si alfanumérico) — `src/utils/signature.js`
- [x] Comparar HMAC con `v1` (estrategia **C** — no bloquear si falla)
- [x] Filtrar `action === "order.processed"` — `src/routes/webhook.js`
- [x] GET orden con `MP_ACCESS_TOKEN` — `src/services/mercadopago.js`
- [x] Validar `processed` + `accredited` + POS + `MP_SALE_AMOUNT`
- [x] Idempotencia por `order_id` — `src/services/idempotency.js`
- [x] `publishDispense` solo si todo OK
- [ ] Prueba en Railway: pago sandbox → `mqtt_published` en logs

---

#### 5.2.7 Decisiones del equipo (2026-05-27)

| # | Tema | Decisión |
|---|------|----------|
| 1 | Estrategia firma | **C (híbrida)** — ver justificación §5.2.A |
| 2 | Firma falla, GET OK | **Dispensar** si GET cumple reglas (no bloquear por firma) |
| 3 | MQTT falla, GET OK | **HTTP 200** + log `mqtt_failed` |
| 4 | Idempotencia POC | **`Set` en memoria** (RAM); persistencia diferida |
| 5 | Monto | Variable **`MP_SALE_AMOUNT`** (ej. `500.00`) en servidor; validar en GET y usar en `POST /orders/create` |

**`MP_SALE_AMOUNT`:**

- Formato: string con dos decimales, ej. `500.00` (mismo formato que `total_amount` en MP).
- Definir en Railway y en `.env` local.
- Al cambiar precio: actualizar variable → redeploy; las órdenes nuevas (API o Postman) deben usar el mismo valor.
- El firmware / MQTT solo recibe `duration_ms`; **no** necesita cambio al subir el precio.

> El `MP_WEBHOOK_SECRET` de **modo prueba** ≠ modo productivo — actualizar en Fase 6.

---

## 6. Variables de entorno

Definir en Railway → Variables (y localmente en `.env`, **nunca commitear**):

```bash
# MercadoPago
MP_ACCESS_TOKEN=APP_USR-...          # Token sandbox (cambiar a prod en Fase 6)
MP_USER_ID=3420512522
MP_EXTERNAL_POS_ID=MATEPOINT001POS001
MP_WEBHOOK_SECRET=                   # Clave secreta de la app en el portal MP
MP_SALE_AMOUNT=500.00                # Precio por porción (ARS); mínimo MP $ 15.00

# MQTT — prototipo: broker público HiveMQ (sin cuenta)
MQTT_BROKER_URL=wss://broker.hivemq.com:8884/mqtt
# HiveMQ Cloud (alternativa con auth): ssl://xxxx.s1.eu.hivemq.cloud:8883
# MQTT_USER=
# MQTT_PASS=
MQTT_DEVICE_ID=MATEPOINT001
DISPENSE_DURATION_MS=120000

# App
PORT=3000
NODE_ENV=production
```

---

## 7. Estructura de archivos del proyecto

```
servidor-mate-point/
├── src/
│   ├── index.js            # Entry point: Express app + MQTT connect
│   ├── routes/
│   │   ├── health.js       # GET /health
│   │   ├── orders.js       # POST /orders/create
│   │   └── webhook.js      # POST /webhook/mp
│   ├── services/
│   │   ├── mercadopago.js  # Wrapper SDK MP: createOrder, getOrder
│   │   └── mqtt.js         # Cliente MQTT: connect, publish
│   └── utils/
│       └── signature.js    # validateMpSignature()
├── .env                    # Local — en .gitignore
├── .gitignore
├── package.json
└── README.md
```

---

## 8. Deploy en Railway — paso a paso

### 8.1 Repositorio

| Campo | Valor |
|-------|-------|
| **GitHub** | [digifab-ar/Mate-Point-v1](https://github.com/digifab-ar/Mate-Point-v1) |
| **Código backend** | carpeta `servidor/` en la raíz del repo |
| **Root Directory en Railway** | `servidor` |

1. Clonar / push al repo remoto
2. `.env` en `.gitignore` — usar `.env.example` como plantilla
3. `package.json` → `"start": "node src/index.js"`

### 8.2 Crear el proyecto en Railway

1. Ir a [railway.app](https://railway.app) → **New Project**
2. Seleccionar **Deploy from GitHub repo** → autorizar Railway → elegir `servidor-mate-point`
3. Railway detecta Node.js automáticamente y ejecuta `npm install` + `npm start`
4. En **Variables** agregar todas las del §6
5. En **Settings → Networking** → habilitar **Public Domain** → Railway asigna `https://<slug>.up.railway.app`

### 8.3 Registrar la URL del webhook en MP

1. Ir a Portal MP Developers → App "Mate point" → **Webhooks**
2. URL: `https://<slug>.up.railway.app/webhook/mp`
3. Eventos: **merchant_order** (órdenes procesadas)
4. Guardar → MP envía un POST de prueba para verificar que el endpoint responde 200

### 8.4 Verificar deploy

```bash
# Healthcheck
curl https://<slug>.up.railway.app/health
# Esperado: { "status": "ok", "mqtt": "connected" }
```

---

## 9. Topics MQTT

| Topic | Dirección | Payload | Descripción |
|-------|-----------|---------|-------------|
| `mate/MATEPOINT001/command` | Servidor → ESP32 | `{ cmd, duration_ms, order_id, ts }` | Comando de dispensado |
| `mate/MATEPOINT001/status` | ESP32 → Servidor | `{ state, ts }` | Estado del dispensador |

**Payload `command` completo:**

```json
{
  "cmd": "dispense",
  "duration_ms": 120000,
  "order_id": "ORDTST01KSNFEN3H3FTHXMK9Q1ZPE5NZ",
  "ts": 1748369220000
}
```

**Payload `status` del ESP32:**

```json
{
  "state": "dispensing",
  "ts": 1748368961000
}
```

Estados posibles: `idle` | `dispensing` | `error`

---

## 10. Log de transacciones

Cada pago procesado se registra en stdout (Railway captura los logs automáticamente):

```json
{
  "event": "dispense_triggered",
  "ts": "2026-05-27T16:47:00Z",
  "order_id": "ORDTST01KSNFEN3H3FTHXMK9Q1ZPE5NZ",
  "external_reference": "mate-001-20260527-003",
  "amount": "500.00",
  "device_id": "MATEPOINT001",
  "mqtt_published": true
}
```

> Railway guarda los logs de los últimos deployments. Para persistencia a largo plazo considerar Railway PostgreSQL (Fase 6).

---

## 11. Checklist de Fase 3

| Ítem | Estado |
|------|--------|
| Repo [Mate-Point-v1](https://github.com/digifab-ar/Mate-Point-v1) con carpeta `servidor/` | **Completado** |
| Deploy en Railway activo (`/health` responde) | **Completado** |
| Variables de entorno configuradas en Railway | **Completado** |
| MQTT: `MQTT_BROKER_URL=wss://broker.hivemq.com:8884/mqtt` — conexión desde Railway | **Completado** |
| Cliente MQTT conecta al broker desde Railway | **Completado** (`mqtt_connected`) |
| `POST /webhook/mp` recibe notificación de prueba de MP | **Completado** |
| Validación `x-signature` (estrategia C) | **Completado** — e2e: `signature_invalid`, flujo continúa |
| Pago sandbox → webhook → MQTT → log verificado end-to-end | **Completado** — ver §11.1 |
| URL webhook registrada en Portal MP Developers | **Completado** (modo prueba) |

### 11.1 Resultados prueba e2e (2026-05-27)

| Campo | Valor |
|-------|-------|
| Orden | `ORDTST01KSNFEN3H3FTHXMK9Q1ZPE5NZ` |
| Ref. | `mate-001-20260527-003` |
| Topic MQTT | `mate/MATEPOINT001/command` |

Logs Railway (orden): `mqtt_connected` → `webhook_received` → `signature_invalid` → `order_fetch_ok` → `dispense_triggered` → `mqtt_published` → `POST /webhook/mp` **200**.

Detalle ampliado: `integracion-mercadopago-qr.md` §0.2 · `plan-de-implementacion.md` §Fase 3.

**Pendiente (Fase 4):** ESP32 suscrito al topic y dispensado físico vía UART Nobana.

---

## 12. Historial

| Fecha | Cambio |
|-------|--------|
| 2026-05-27 | Documento creado — arquitectura, stack, Railway y HiveMQ definidos para Fase 3 |
| 2026-05-27 | Repo Mate-Point-v1; broker público HiveMQ (8884 WSS servidor, 1883 TCP ESP32); scaffold en `servidor/` |
| 2026-05-27 | **Fase 3 completada** — checklist §11; e2e `ORDTST01KSNFEN3H3FTHXMK9Q1ZPE5NZ` con `mqtt_published` |
| 2026-05-29 | **`POST /orders/create`** y **`POST /orders/cancel`** — órdenes desde ESP32 v0.2; `MP_ORDER_EXPIRATION=PT2M` |
