# Servidor Mate Point — Backend + MQTT

**Proyecto:** Mate Point — Dispensador de agua caliente  
**OT:** OT-00268 — Etapa 3  
**Repositorio:** [github.com/digifab-ar/Mate-Point-v1](https://github.com/digifab-ar/Mate-Point-v1)  
**Código servidor:** carpeta [`servidor/`](servidor/)  
**Última actualización:** 2026-05-27

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
│  POST /orders/create        │◄── (futuro: ESP32 / frontend)
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
| `POST` | `/orders/create` | Crea una orden QR estático en MP (llamado desde la máquina o manualmente) |
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

### 5.2 Validación del webhook (`x-signature`)

MP envía en el header `x-signature` la firma HMAC-SHA256 del body usando el **secret** de la app.

```javascript
const crypto = require('crypto');

function validateMpSignature(req) {
  const signature = req.headers['x-signature'];
  const secret    = process.env.MP_WEBHOOK_SECRET;
  if (!signature || !secret) return false;

  // Formato: "ts=...,v1=<hash>"
  const [, v1] = signature.split(',').map(p => p.split('='));
  const ts     = signature.match(/ts=(\d+)/)?.[1];
  const manifest = `id:${req.body.id};request-id:${req.headers['x-request-id']};ts:${ts};`;

  const expected = crypto
    .createHmac('sha256', secret)
    .update(manifest)
    .digest('hex');

  return v1[1] === expected;
}
```

> El `MP_WEBHOOK_SECRET` se obtiene en: Portal MP Developers → App "Mate point" → Webhooks → Clave secreta.

---

## 6. Variables de entorno

Definir en Railway → Variables (y localmente en `.env`, **nunca commitear**):

```bash
# MercadoPago
MP_ACCESS_TOKEN=APP_USR-...          # Token sandbox (cambiar a prod en Fase 6)
MP_USER_ID=3420512522
MP_EXTERNAL_POS_ID=MATEPOINT001POS001
MP_WEBHOOK_SECRET=                   # Clave secreta de la app en el portal MP

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
  "order_id": "ORDTST01KSN8G14TKMBSTCF1G4TXJ355",
  "ts": 1748368960000
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
  "ts": "2026-05-27T17:43:47Z",
  "order_id": "ORDTST01KSN8G14TKMBSTCF1G4TXJ355",
  "external_reference": "mate-001-20260527-001",
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
| Repo [Mate-Point-v1](https://github.com/digifab-ar/Mate-Point-v1) con carpeta `servidor/` | En curso |
| Deploy en Railway activo (`/health` responde) | Pendiente |
| Variables de entorno configuradas en Railway | Pendiente |
| MQTT: `MQTT_BROKER_URL=wss://broker.hivemq.com:8884/mqtt` — conexión desde Railway | Pendiente |
| Cliente MQTT conecta al broker desde Railway | Pendiente |
| `POST /webhook/mp` recibe notificación de prueba de MP | Pendiente |
| Validación `x-signature` implementada y testeada | Pendiente |
| Pago sandbox → webhook → MQTT → log verificado end-to-end | Pendiente |
| URL webhook registrada en Portal MP Developers | Pendiente |

---

## 12. Historial

| Fecha | Cambio |
|-------|--------|
| 2026-05-27 | Documento creado — arquitectura, stack, Railway y HiveMQ definidos para Fase 3 |
| 2026-05-27 | Repo Mate-Point-v1; broker público HiveMQ (8884 WSS servidor, 1883 TCP ESP32); scaffold en `servidor/` |
