# Plan de implementación — Mate Point v0-8

**Proyecto:** Mate Point — OT-00268 Etapa 3  
**Carpeta firmware:** [`mate_point_v0-8/`](mate_point_v0-8/) — fork de [`mate_point_v0-9/`](mate_point_v0-9/)  
**Base validada:** v0-9 (oferta / litros / retiro termo — E2E OK hardware 2026-09-17) · v0-7 VL6180 · v0-6 Wi-Fi NVS · servidor Fase 3 + órdenes v0.2  
**Plataforma:** Waveshare ESP32-S3-Touch-LCD-7B + Nobana UART + MQTT + QR estático MP  
**Última actualización:** 2026-09-17  
**Estado:** **Implementado** (código) — pendiente alta MP sandbox (4 cajas) + PNG LVGL por unidad + Railway + QA E2E

| Documento | Uso |
|-----------|-----|
| [`PLAN-MATE-POINT-v0-9.md`](PLAN-MATE-POINT-v0-9.md) | **Producto vigente** — oferta servidor, litros, `WAIT_TERMO_RESUME` |
| [`PLAN-MATE-POINT-v0-6.md`](PLAN-MATE-POINT-v0-6.md) | `DEVICE_ID` fijo en `config.h`, Wi-Fi en NVS |
| [`PLAN-MATE-POINT-v0-7.md`](PLAN-MATE-POINT-v0-7.md) | VL6180 — **cerrado** 2026-09-08 |
| [`integracion-mercadopago-qr.md`](../integracion-mercadopago-qr.md) | QR estático, sucursal/caja, webhook (POC DigiFAB) |
| [`servidor-mate-point.md`](../servidor-mate-point.md) | Backend Railway, MQTT, `POST /orders/create` |
| [`arquitectura-mate-point.md`](../arquitectura-mate-point.md) | Topics `mate/{device_id}/command` · `status` |

Docs oficiales MP:

- [Crear aplicación QR](https://www.mercadopago.com.ar/developers/es/docs/qr-code/create-application)
- [Crear sucursal y caja](https://www.mercadopago.com.ar/developers/es/docs/qr-code/create-store-and-pos)
- [Credenciales](https://www.mercadopago.com.ar/developers/es/docs/credentials)
- [Notificaciones / webhooks](https://www.mercadopago.com.ar/developers/es/docs/qr-code/notifications)
- Panel: [Tus integraciones](https://www.mercadopago.com.ar/developers/panel/app)

---

## 1. Objetivo

Habilitar **varios Mate Point** cobrando en **la misma cuenta de Mercado Pago del cliente**, con:

1. **Una sucursal (store)** para toda la marca / flota.
2. **Una caja (POS) y un QR estático por máquina.**
3. **Un `device_id` único** por máquina hacia el servidor y MQTT.
4. La **aplicación Developers y las credenciales en poder del cliente** (no de DigiFAB).

Hoy el prototipo es 1:1:1 (`MATEPOINT001` = sucursal = caja = topic MQTT). El firmware ya envía `device_id` en `POST /orders/create`, pero el servidor **ignora** ese campo al elegir el POS y **siempre** publica en `mate/MATEPOINT001/command`.

v0-8 cierra ese ruteo y documenta el alta operativa en la cuenta del cliente.

> **Alcance v0-8:** registro `device_id` ↔ `external_pos_id` en servidor; MQTT por máquina; flash por unidad (`DEVICE_ID` + PNG de su caja); guía de cuenta Developers del cliente y alta de sucursal/cajas. **Sin** QR dinámico, **sin** `DEVICE_ID` en portal Wi-Fi, **sin** segunda sucursal por local, **sin** cambio de UART/Nobana/UI Figma.

---

## 2. Decisiones cerradas (2026-09-02)

| # | Tema | Decisión |
|---|------|----------|
| D1 | Sucursales MP | **Una sucursal para todos** los Mate Point |
| D2 | Cajas / QR | **Una caja + un QR estático por máquina.** No compartir POS entre equipos |
| D3 | Titular MP | Cuenta **del cliente** (quien cobra). App Developers creada **en esa cuenta** |
| D4 | Rol DigiFAB | Implementa servidor/firmware. Recibe credenciales por **compartir en el portal** (no por chat) |
| D5 | `external_id` sucursal | **`MATEPOINT`** — identidad de marca, **no** igual a un `device_id` |
| D6 | `device_id` | `MATEPOINT001`, `MATEPOINT002`, … — firmware + HTTP + MQTT |
| D7 | `external_pos_id` | `MATEPOINT001POS001`, `MATEPOINT002POS001`, … — alfanumérico, máx. 40, **sin guiones** |
| D8 | Firmware identidad | Hereda v0-6 **D2**: `DEVICE_ID` y QR en flash; el dueño del local solo configura Wi-Fi |
| D9 | Servidor | Tabla `device_id` → `external_pos_id`. Crear orden y webhook usan ese mapa. Dejar de usar un único `MQTT_DEVICE_ID` / `MP_EXTERNAL_POS_ID` como fuente de verdad |
| D10 | Webhook | **Una URL** Railway. MP notifica todas las cajas de la app. El GET de la orden trae el POS |
| D11 | MQTT | Mismo broker POC (`broker.hivemq.com`). Topics `mate/{device_id}/command` y `…/status`. `MQTT_CLIENT_ID` único por unidad |
| D12 | Precio / duración | Siguen globales: `MP_SALE_AMOUNT`, `DISPENSE_DURATION_MS` (igual para toda la flota en v0-8) |
| D13 | Cuenta POC DigiFAB | Sandbox actual (`store 77230109`, POS `132339357`) **no** se migra. En la cuenta del cliente se crea sucursal y cajas **nuevas** |
| D14 | Domicilio de la sucursal única | Dirección **fiscal / sede** del cliente (CUIT). No es la dirección de cada kiosco. MP usa ese dato para impuestos y mapa; es el trade-off de D1 |
| D15 | `fixed_amount` | **`true`** — igual que el POS POC |
| D16 | Categoría POS | **`621102`** (gastronomía) — igual POC; cambiar solo si el rubro del cliente lo exige |
| D17 | Base firmware | Fork **v0-9** (producto vigente, E2E OK 2026-09-17). MQTT/QR/servidor de v0-8 no dependen del ToF. Plan original (2026-09-02) decía v0-7; v0-9 se intercaló y quedó cerrado primero |
| D18 | `MQTT_CLIENT_ID` | Sufijo **`v080`** |
| D19 | Persistencia registro dispositivos | Archivo/config en servidor (JSON o módulo). **Sin** PostgreSQL en v0-8 |
| D20 | OAuth | **No** en v0-8. Integración propia del cliente: Access Token de **su** app (prueba, luego producción) |

### 2.1 Cambio respecto a v0-6 / servidor actual

| Aspecto | Hoy (POC) | v0-8 |
|---------|-----------|------|
| Cuenta MP | Sandbox DigiFAB | Cuenta **cliente** |
| Sucursales | 1 (`MATEPOINT001` = también el device) | 1 (`MATEPOINT`) para **N** máquinas |
| Cajas | 1 (`MATEPOINT001POS001`) | N — una por `device_id` |
| `POST /orders/create` | `device_id` solo en `external_reference` | Elige el POS del `device_id` |
| Webhook → MQTT | Siempre `mate/MATEPOINT001/command` | `mate/{device_id}/command` según POS de la orden |
| QR en pantalla | Un PNG PROGMEM | PNG **de esa caja**, flasheado por unidad |
| Validación POS | `=== MP_EXTERNAL_POS_ID` | POS ∈ registro de dispositivos |

### 2.2 Modelo de identidad (obligatorio 1:1)

```
Cuenta MP del cliente
  └── App Developers "Mate point"     ← 1 (webhook + tokens)
        └── Sucursal MATEPOINT        ← 1 (D1)
              ├── Caja MATEPOINT001POS001  ↔  device MATEPOINT001  ↔  QR PNG 1
              ├── Caja MATEPOINT002POS001  ↔  device MATEPOINT002  ↔  QR PNG 2
              ├── Caja MATEPOINT003POS001  ↔  device MATEPOINT003  ↔  QR PNG 3
              └── Caja MATEPOINT004POS001  ↔  device MATEPOINT004  ↔  QR PNG 4
```

No reutilizar un QR entre dos máquinas: Mercado Pago mantiene **una orden abierta por POS**. Dos equipos con el mismo POS se pisan.

---

## 3. Quién hace qué

| Actor | Responsabilidad |
|-------|-----------------|
| **Cliente** | Cuenta MP vendedora; app Developers; verificación de identidad; **comparte** credenciales; guarda Access Token y clave webhook. No las pega en issues/git |
| **DigiFAB (ops MP)** | Con las credenciales compartidas: `GET /users/me`, alta sucursal `MATEPOINT`, alta de **cada caja**, descarga PNG, llena el registro `device_id` ↔ POS |
| **DigiFAB (servidor)** | Railway: token, user_id, secret webhook, registro de dispositivos, ruteo MQTT |
| **DigiFAB (firmware)** | Por unidad: `DEVICE_ID` + `qr_static_img.c` del PNG de **su** caja; flash |
| **Dueño del local** | Solo Wi-Fi (portal v0-6). No crea cajas ni toca Developers |

---

## 4. Guía del cliente — cuenta Mercado Pago Developers

Mercado Pago **no** tiene una “cuenta developer” aparte. Se entra a [developers.mercadopago.com.ar](https://www.mercadopago.com.ar/developers) con la **misma** cuenta MP que **cobra**. Quien es dueño de esa cuenta es dueño de la app, de las cajas y del dinero.

Usar la cuenta **empresa / CUIT del negocio**, no la de un técnico ni la de DigiFAB.

### 4.1 Si todavía no tiene cuenta Mercado Pago

1. Crear cuenta en [mercadopago.com.ar](https://www.mercadopago.com.ar) con el mail corporativo que va a quedar como titular.
2. Completar datos personales o de empresa (CUIT, domicilio fiscal).
3. Instalar la **app Mercado Pago** en un celular del titular.
4. Completar **verificación de identidad** (MP la pide al crear la aplicación).
5. Confirmar que puede ver el saldo / actividad de esa cuenta (es la que recibirá los cobros QR).

### 4.2 Crear la aplicación (Código QR)

Doc: [Crear aplicación](https://www.mercadopago.com.ar/developers/es/docs/qr-code/create-application).

1. Abrir [Mercado Pago Developers](https://www.mercadopago.com.ar/developers) → **Ingresar** con la cuenta del paso 4.1.
2. Arriba a la derecha: **Tus integraciones** → **Crear aplicación** (o **Ver todas** si ya hay otras).
3. Completar verificación de identidad o reautenticación si el portal lo pide.
4. **Nombre:** `Mate point` (máx. 50 caracteres).
5. Tipo de pago: **Pagos presenciales** → Continuar.
6. Producto: **Código QR** → Continuar.
7. Confirmar, aceptar Declaración de Privacidad y Términos → **Confirmar**.

Queda creada la app. Las **credenciales de prueba** se generan solas (no hay que “activarlas”).

### 4.3 Anotar credenciales de prueba

En la app → menú izquierdo **Pruebas → Credenciales de prueba**:

| Dato | Dónde | ¿Lo usa Mate Point? |
|------|--------|---------------------|
| **Access Token** (empieza con `APP_USR-`) | Credenciales de prueba | **Sí** — backend (`MP_ACCESS_TOKEN`) |
| **Public Key** | Misma pantalla | **No** en v0-8 (no hay frontend MP) |
| **User ID** (vendedor de prueba) | Datos de credenciales de prueba | **Sí** — path de sucursales (`MP_USER_ID`) |
| **N.° de aplicación** | Datos de integración | Trazabilidad; no va en el firmware |

El Access Token **nunca** va en el firmware ni en el repo. Solo Railway / `.env` local (gitignored).

### 4.4 Compartir credenciales con DigiFAB (recomendado)

Doc: [Credenciales — compartir](https://www.mercadopago.com.ar/developers/es/docs/credentials).

Así el cliente **no manda el token por WhatsApp** y puede revocar el acceso después.

1. Tus integraciones → aplicación **Mate point**.
2. **Pruebas** (o **Producción** cuando corresponda).
3. **Comparte las credenciales con un desarrollador** → **Compartir credenciales**.
4. Mail de la cuenta Mercado Pago de DigiFAB (el mail **tiene** que ser una cuenta MP).
5. Máximo **10** compartidos; se pueden quitar sin romper la app.

Si no pueden usar compartir: carga conjunta en el panel Railway en una llamada, sin dejar el token en tickets.

### 4.5 Webhook (cuando DigiFAB indique la URL)

1. Tus integraciones → app **Mate point** → **Webhooks** (o Notificaciones → Webhooks).
2. URL (POC actual): `https://mate-point-v1-production.up.railway.app/webhook/mp`
3. Modo **Prueba** primero. Evento: **Order (Mercado Pago)** / órdenes — el que corresponda a QR `order.processed` (mismo criterio que [`integracion-mercadopago-qr.md`](../integracion-mercadopago-qr.md) §7).
4. Guardar. Copiar la **clave secreta** → `MP_WEBHOOK_SECRET` en Railway.
5. MP suele mandar un POST de prueba; el servidor debe responder **200**.

La clave de **modo prueba** ≠ la de **modo productivo**. Al pasar a Fase 6 hay que actualizar Railway.

### 4.6 Credenciales de producción (más adelante — Fase 6)

No hace falta el día 1. Cuando el flujo sandbox esté cerrado:

1. App → **Producción → Credenciales de producción**.
2. Completar **Industria** y **Sitio web** (obligatorio).
3. Aceptar términos + reCAPTCHA → **Activar credenciales de producción**.
4. Copiar Access Token de **producción** (también `APP_USR-`) y User ID **real** (el que cobra).
5. Reemplazar en Railway `MP_ACCESS_TOKEN` y `MP_USER_ID`.
6. Registrar el webhook en pestaña **producción** (otra clave secreta).
7. Recrear sucursal/cajas si el alta de prueba quedó atada al user_id sandbox — en la práctica sucursal y POS de **producción** se crean con el token de producción y el `user_id` real.

v0-8 se valida primero en **prueba**. Producción es el mismo procedimiento de cajas (§6) con el otro token.

### 4.7 Usuarios de prueba (pagos sandbox)

- **Vendedor:** la propia app (token de prueba).
- **Comprador:** otra cuenta test. Crear usuarios test: `POST https://api.mercadopago.com/users/test` con **Access Token de producción** del titular (`40311` si se usa el de sandbox). Ver [`integracion-mercadopago-qr.md`](../integracion-mercadopago-qr.md) §5.5.
- La app del celular del tester se loguea con el **comprador**, no con el vendedor.

---

## 5. Datos que DigiFAB necesita para cablear Mate Point

Entrega del cliente (o leídos por DigiFAB con credenciales compartidas). **No versionar secretos.**

### 5.1 Obligatorios — servidor (Railway / `.env`)

| Variable / dato | Origen | Uso |
|-----------------|--------|-----|
| `MP_ACCESS_TOKEN` | Credenciales de prueba (luego prod) | Crear/consultar/cancelar órdenes |
| `MP_USER_ID` | Credenciales o `GET /users/me` | Alta sucursal `POST /users/{id}/stores`; chequeos |
| `MP_WEBHOOK_SECRET` | Portal → Webhooks | Firma `x-signature` (estrategia C; el GET sigue siendo la puerta) |
| N.° de aplicación | Datos de integración | Documentación / soporte MP |
| URL webhook confirmada | Misma que Railway | Debe coincidir con el portal |

### 5.2 Obligatorios — sucursal única (se crean en §6)

| Dato | Ejemplo | Uso |
|------|---------|-----|
| `name` | `Mate point` | Display MP |
| `external_id` | `MATEPOINT` | `MP_EXTERNAL_STORE_ID` |
| `id` (numérico MP) | p. ej. `77230109` en POC | `store_id` al crear cajas |
| Domicilio fiscal | calle, `city_name` de lista MP, provincia, lat/long, CP | Impuestos / mapa MP (D14) |

### 5.3 Obligatorios — por cada máquina

| Dato | Ejemplo máquina 1 | Ejemplo máquina 2 | Uso |
|------|-------------------|-------------------|-----|
| `device_id` | `MATEPOINT001` | `MATEPOINT002` | Firmware `DEVICE_ID`, body create, topic MQTT |
| Nombre de caja | `Mate point - Dispensador 1` | `Mate point - Dispensador 2` | Alta POS |
| `external_id` POS | `MATEPOINT001POS001` | `MATEPOINT002POS001` | `external_pos_id` en órdenes |
| `id` POS (MP) | numérico | numérico | URL del QR |
| `uuid` del QR | hex | hex | Trazabilidad |
| `qr.image` | URL PNG | URL PNG | Convertir a `qr_static_img.c` |
| `status` | `active` | `active` | No flashear cajas inactivas |

### 5.4 Ya los define DigiFAB (no los pide al cliente)

| Variable | Valor POC / producto | Notas |
|----------|----------------------|--------|
| `MP_SALE_AMOUNT` | `500.00` | Cambiar si el cliente fija otro precio (≥ $ 15) |
| `MP_ORDER_EXPIRATION` | `PT2M` | Alineado al timer UI |
| `DISPENSE_DURATION_MS` | `30000` (v0-3-1+) | Tiempo total dispensado firmware |
| `MQTT_BROKER_URL` | `wss://broker.hivemq.com:8884/mqtt` | Servidor |
| `MQTT_HOST` firmware | `broker.hivemq.com:1883` | v0-6 D26 |
| `SERVER_HOST` | `mate-point-v1-production.up.railway.app` | Firmware |
| `PORT` / `NODE_ENV` | Railway | — |

### 5.5 No hacen falta en v0-8

| Dato | Motivo |
|------|--------|
| Public Key | No hay Brick / frontend MP |
| Client ID / Client Secret | OAuth fuera de alcance (D20) |
| Access Token en el ESP32 | El ESP32 **nunca** habla con MP |

### 5.6 Plantilla de entrega (completar y guardar fuera del git)

```text
Cliente: ____________________
Fecha: ____________________

App: Mate point
N.° aplicación: ____________________
User ID prueba: ____________________
User ID producción (si aplica): ____________________
Webhook URL: https://mate-point-v1-production.up.railway.app/webhook/mp
Webhook modo: prueba / producción
Access Token: (Railway / 1Password — no acá)
Webhook secret: (Railway / 1Password — no acá)

Sucursal
  name: Mate point
  external_id: MATEPOINT
  store_id: ____________________
  domicilio: ____________________
  city_name (lista MP): ____________________

Máquinas
  device_id | external_pos_id | pos_id | uuid | URL PNG
  MATEPOINT001 | MATEPOINT001POS001 |  |  |
  MATEPOINT002 | MATEPOINT002POS001 |  |  |
```

---

## 6. Paso a paso — sucursal y cajas (cuenta del cliente)

Hacerlo con el **Access Token de prueba** DigiFAB y `MP_USER_ID=3420512522`. Script idempotente:

```bash
cd servidor
# token en .env (gitignored), no en el chat
npm run provision:mp-v08 -- --dry-run
npm run provision:mp-v08
```

Crea sucursal `MATEPOINT` (Santamarina 1352, San Fernando) y las 4 cajas del registro [`servidor/src/config/devices.json`](../servidor/src/config/devices.json). PNGs → `servidor/ops/mp-v08/` (gitignored).

Mismos requests que [`integracion-mercadopago-qr.md`](../integracion-mercadopago-qr.md) §5.1–§5.2, con IDs **nuevos**. El store/POS POC (`MATEPOINT001` / `77230109`) **no** se reutiliza (opción B).

Herramientas: el script, o Postman / `curl`. Header siempre:

```http
Authorization: Bearer <MP_ACCESS_TOKEN>
Content-Type: application/json
```

### 6.1 Verificar token y User ID

```bash
curl -H "Authorization: Bearer $MP_ACCESS_TOKEN" \
  https://api.mercadopago.com/users/me
```

Anotar `id` → `MP_USER_ID`. Si 401: token mal copiado o de otra app.

### 6.2 Crear **una** sucursal (una sola vez)

`POST https://api.mercadopago.com/users/{MP_USER_ID}/stores`

Sustituir domicilio por el **fiscal del cliente**. `city_name` debe ser un valor **exacto** de la lista MP (el POC usó `San Fernando`; `Victoria` falló).

```json
{
  "name": "Mate point",
  "external_id": "MATEPOINT",
  "location": {
    "street_number": "1352",
    "street_name": "Santamarina",
    "city_name": "San Fernando",
    "state_name": "Buenos Aires",
    "latitude": -34.4568,
    "longitude": -58.5612,
    "reference": "Santamarina 1352 — sucursal única flota"
  },
  "business_hours": {
    "monday": [{"open": "08:00", "close": "22:00"}],
    "tuesday": [{"open": "08:00", "close": "22:00"}],
    "wednesday": [{"open": "08:00", "close": "22:00"}],
    "thursday": [{"open": "08:00", "close": "22:00"}],
    "friday": [{"open": "08:00", "close": "22:00"}],
    "saturday": [{"open": "09:00", "close": "20:00"}],
    "sunday": [{"open": "10:00", "close": "18:00"}]
  }
}
```

Guardar de la respuesta:

- `id` → `store_id` (número, sin comillas en el JSON de la caja)
- `external_id` = `MATEPOINT`

Si `city_name was invalid`: usar el nombre que MP lista en el error.

No crear una segunda sucursal por kiosco (D1).

### 6.3 Crear **una caja por máquina** (repetir)

`POST https://api.mercadopago.com/pos`

`external_store_id` **siempre** `MATEPOINT`. `store_id` **siempre** el de §6.2. Cambian `name` y `external_id`.

**Máquina 1**

```json
{
  "name": "Mate point - Dispensador 1",
  "fixed_amount": true,
  "store_id": 0,
  "external_store_id": "MATEPOINT",
  "external_id": "MATEPOINT001POS001",
  "category": 621102
}
```

**Máquina 2** — `"name": "Mate point - Dispensador 2"`, `"external_id": "MATEPOINT002POS001"`.  
**Máquina 3** — Dispensador 3 / `MATEPOINT003POS001`.  
**Máquina 4** — Dispensador 4 / `MATEPOINT004POS001`.

`store_id: 0` es placeholder: poner el `id` numérico real de §6.2.

Reglas:

- `external_id`: solo letras y números, máx. 40, **sin guiones**.
- `point_of_sale_exists` → ese `external_id` ya existe; usar el siguiente (`…POS001` de otro `device_id`).
- `EXTERNAL_STORE_ID_NOT_MATCH` → `external_store_id` distinto del `external_id` de la sucursal.
- `INVALID_EXTERNAL_ID` → hay un `-` o carácter inválido.

### 6.4 Guardar el QR de cada caja

La respuesta **201** incluye:

```json
{
  "id": 2711382,
  "uuid": "0977011a027c4b4387e52069da4264deae2946af4dcc44ee98a8f1dbb376c8a1",
  "external_id": "MATEPOINT001POS001",
  "external_store_id": "MATEPOINT",
  "status": "active",
  "qr": {
    "image": "https://www.mercadopago.com/instore/merchant/qr/2711382/<uuid>.png",
    "template_document": "https://www.mercadopago.com/instore/merchant/qr/2711382/template_<uuid>.pdf",
    "template_image": "https://www.mercadopago.com/instore/merchant/qr/2711382/template_<uuid>.png"
  }
}
```

Para cada máquina:

1. Descargar `qr.image` (PNG del código, no la plantilla con marco).
2. Anotar `id`, `uuid`, `external_id` en la plantilla §5.6.
3. Ese PNG es el que se convierte a LVGL (`qr_static_img.c`) **solo** para esa unidad.

El QR **no cambia** entre ventas. Si se da de baja la caja y se crea otra, hay que re-flashear el PNG nuevo.

### 6.5 Verificar listado

```bash
# sucursales
curl -H "Authorization: Bearer $MP_ACCESS_TOKEN" \
  "https://api.mercadopago.com/users/$MP_USER_ID/stores/search?external_id=MATEPOINT"

# cajas (filtrar a mano por external_store_id MATEPOINT)
curl -H "Authorization: Bearer $MP_ACCESS_TOKEN" \
  "https://api.mercadopago.com/pos"
```

Esperado: 1 store `MATEPOINT`, N POS `MATEPOINT00nPOS001`, todos `active`.

### 6.6 Alta de una máquina extra (operación repetible)

1. Asignar próximo `device_id` (`MATEPOINT005`, …).
2. `POST /pos` con `external_id` `MATEPOINT005POS001` y la **misma** sucursal.
3. Descargar PNG.
4. Agregar fila al registro del servidor.
5. Flashear firmware con ese `DEVICE_ID` y ese PNG.
6. No tocar webhook ni Access Token.

---

## 7. Servidor — ruteo por dispositivo

Implementado en `servidor/` (2026-09-17). Sandbox DigiFAB: opción B, 4 máquinas, Santamarina 1352 / San Fernando, precio/copy igual v0-9. Railway lo carga ops.

Registro: [`src/config/devices.json`](../servidor/src/config/devices.json). `POST /orders/create` exige `device_id` de esa lista (400 si falta o es desconocido). Webhook: POS ∈ registro → `mate/{device_id}/command`.

Contrato objetivo (cerrado en código):

### 7.1 Registro

```json
{
  "store_external_id": "MATEPOINT",
  "devices": [
    {
      "device_id": "MATEPOINT001",
      "external_pos_id": "MATEPOINT001POS001"
    },
    {
      "device_id": "MATEPOINT002",
      "external_pos_id": "MATEPOINT002POS001"
    },
    {
      "device_id": "MATEPOINT003",
      "external_pos_id": "MATEPOINT003POS001"
    },
    {
      "device_id": "MATEPOINT004",
      "external_pos_id": "MATEPOINT004POS001"
    }
  ]
}
```

`device_id` desconocido → `400` en create (no crear orden en el POS equivocado).

### 7.2 `POST /orders/create`

1. Leer `body.device_id`.
2. Resolver `external_pos_id`.
3. `POST /v1/orders` con `mode: "static"` y **ese** POS (hoy `mercadopago.js` usa un env único).
4. `external_reference` puede seguir incluyendo el device (`mate-matepoint002-…`).

### 7.3 `POST /webhook/mp`

Sin cambio de URL ni de filtro `order.processed`.

1. GET orden (igual que ahora).
2. Validar monto, `processed` / `accredited`, `type: qr`, `mode: static`.
3. `config.qr.external_pos_id` ∈ registro (reemplaza igualdad a un solo env).
4. POS → `device_id`.
5. `publish` en `mate/{device_id}/command`.
6. Idempotencia por `order_id` (ya sirve para N máquinas).

### 7.4 MQTT

`commandTopic(deviceId)` en lugar de `MQTT_DEVICE_ID` global. Un cliente servidor publica a N topics. El ESP32 de cada unidad **solo** se suscribe al suyo.

---

## 8. Firmware — una imagen por unidad

Hereda v0-6: el dueño configura Wi-Fi; identidad de fábrica.

Por cada ESP32, antes de flash:

| Archivo | Cambio |
|---------|--------|
| `config.h` | `#define DEVICE_ID "MATEPOINT00n"` · `MQTT_CLIENT_ID` `…-v080` |
| `qr_static_img.c` | PNG de **esa** caja, LVGL 8, `CF_TRUE_COLOR`, 320×320, fondo blanco ([`PLAN-IMPLEMENTACION.md`](PLAN-IMPLEMENTACION.md) §15.3) |
| Resto | Igual v0-6 (o v0-7): `order_client` ya manda `device_id` |

No hace falta que el servidor envíe el QR: sigue siendo estático en PROGMEM.

Checklist flash:

1. `DEVICE_ID` = fila del registro servidor.
2. PNG = `qr.image` de **ese** `external_pos_id`.
3. `SERVER_HOST` y broker iguales en toda la flota.
4. Tras boot: portal Wi-Fi si NVS vacío; MQTT `mate/MATEPOINT00n/#`.

---

## 9. Tareas de implementación

| ID | Área | Tarea | Criterio |
|----|------|-------|----------|
| T1 | Cliente | Cuenta MP + app QR + compartir credenciales (§4) | DigiFAB entra a la app / tiene token de prueba |
| T2 | Ops MP | `GET /users/me` + sucursal `MATEPOINT` (§6.1–6.2) | `store_id` anotado |
| T3 | Ops MP | Cajas 1–4 + PNG (§6.3–6.4) | Cuatro `external_id` distintos, QR escaneables |
| T4 | Ops MP | Webhook prueba + `MP_WEBHOOK_SECRET` (§4.5) | POST prueba → 200 |
| T5 | Servidor | Registro dispositivos + create por POS | Create `MATEPOINT002` no usa POS de 001 |
| T6 | Servidor | Webhook: POS → topic MQTT | Pago caja 2 → `mate/MATEPOINT002/command` |
| T7 | Servidor | Validar POS desconocido / device desconocido | Sin MQTT cruzado |
| T8 | Firmware | Fork `mate_point_v0-8/` desde v0-9 · `MQTT_CLIENT_ID` v080 | Compila sobre base v0-9 |
| T9 | Firmware | Unidades 2–4: `DEVICE_ID` + `qr_static_img.c` | QR en pantalla = PNG de esa caja |
| T10 | QA | E2E máquinas (mín. 001 vs otra vía Postman/MQTT) | Sin cruce; idempotencia por `order_id` |
| T11 | Docs | Completar plantilla §5.6 (fuera del git) | Mapa 1:1 cerrado |
| T12 | Docs | Enlace desde README firmware / plan maestro **al implementar** | Índice actualizado |

---

## 10. Criterios de aceptación

| ID | Criterio |
|----|----------|
| S1 | Una sola sucursal `external_id=MATEPOINT` en la cuenta del **cliente** |
| S2 | N = 4 cajas activas bajo esa sucursal, cada una con PNG propio |
| S3 | App Developers y tokens son del cliente (compartidos, no copiados de DigiFAB sandbox) |
| S4 | Railway usa `MP_ACCESS_TOKEN` / `MP_USER_ID` / `MP_WEBHOOK_SECRET` de esa app |
| S5 | `POST /orders/create` `{ device_id: "MATEPOINT002" }` crea orden con `external_pos_id=MATEPOINT002POS001` |
| S6 | Pago QR máquina 2 → MQTT **solo** en `mate/MATEPOINT002/command` |
| S7 | Pago QR máquina 1 no cambia el estado de la máquina 2 |
| S8 | Firmware máquina *n* muestra el QR de **su** caja y publica `device_id` correcto en `status` |
| S9 | Timeout 2 min / cancel siguen igual por `order_id` |
| S10 | Provisioning Wi-Fi v0-6 intacto |
| S11 | Dispensado / Nobana / (si aplica) VL6180: sin regresión de flujo |
| S12 | Secretos no están en el repositorio |

**Procedimiento QA sugerido**

1. `mosquitto_sub -h broker.hivemq.com -p 1883 -t 'mate/+/command' -v`
2. Máquina 1: Comprar → pagar QR 1 → mensaje solo en `…/MATEPOINT001/command`.
3. Máquina 2: igual con `…/002/…`.
4. Dos compras solapadas (cada una en su QR dentro de `PT2M`).
5. Create con `device_id` inexistente → error, sin orden MP.

---

## 11. Riesgos y mitigaciones

| Riesgo | Mitigación |
|--------|------------|
| Token del cliente por chat | D4 — compartir en portal MP |
| Flash de QR de otra caja | Checklist T9: PNG = `external_pos_id` de ese `DEVICE_ID` |
| `DEVICE_ID` duplicado en dos ESP32 | Ambos reciben el mismo `dispense` — IDs de fábrica únicos |
| Sucursal única vs. impuestos por local | Aceptado (D1, D14). Si AFIP/MP lo exige después: v0-x multi-store |
| HiveMQ público: `dispense` falsificable | Aceptado en POC; Fase 6 → broker con auth |
| Alta sandbox vs. prod: POS distintos | Recrear cajas con token prod; re-flash PNG |
| `city_name` inválido | Lista del error MP; no improvisar barrios |
| Firma webhook `hmac_mismatch` | Igual POC (estrategia C); GET + monto + POS del registro |

---

## 12. Fuera de alcance v0-8

- Segunda sucursal por domicilio de kiosco
- Precio o `duration_ms` distintos por máquina
- `DEVICE_ID` / QR configurables por SoftAP
- QR dinámico MQTT (`qr_show`)
- OAuth / marketplace de terceros
- HiveMQ Cloud / TLS 8883 en ESP32
- PostgreSQL / panel admin de flota
- Migración de la sucursal/caja sandbox DigiFAB
- Cambios UART Nobana, Figma, VL6180 (eso es v0-7)

---

## 13. Estimación

| Fase | Días |
|------|------|
| Onboarding cliente (§4) + sucursal + 2 cajas + webhook | 0.5–1.0 |
| Registro servidor + create/webhook MQTT por device | 1.0–1.5 |
| Fork firmware + QR unidad 2 + flash | 0.5 |
| QA E2E dos máquinas + concurrencia | 1.0 |
| Plantilla entrega + este plan en índice (al implementar) | 0.5 |
| **Total** | **~3.5–4.5 días** |

Sketch: [`mate_point_v0-8/`](mate_point_v0-8/). Servidor: `src/config/devices.json` + create/webhook por POS. Alta MP: `servidor/scripts/provision-mp-v08.js`.

---

## 14. Estructura

```
mate_point_firmware/mate_point_v0-8/
├── mate_point_v0-8.ino
├── config.h                 ← DEVICE_ID por unidad; MQTT_CLIENT_ID v080
├── qr_static_img.c          ← PNG de ESA caja (reemplazar tras provision)
└── … (resto igual a v0-9)

servidor/
├── src/config/devices.json  ← 4 device_id ↔ POS
├── src/services/devices.js
├── scripts/provision-mp-v08.js
└── … create/webhook rutean por POS
```

---

## Changelog

| Fecha | Cambio |
|-------|--------|
| 2026-09-17 | **Implementado** — fork v0-9; registro 4 máquinas; create/webhook MQTT por POS; script sucursal `MATEPOINT` + 4 cajas (opción B, Santamarina 1352). Pendiente: correr provision, PNG LVGL, Railway, QA |
| 2026-09-17 | Base de fork → **v0-9** (producto vigente E2E OK). D17 actualizado |
| 2026-09-02 | Plan inicial v0-8 — flota N máquinas, **una sucursal `MATEPOINT`**, una caja/QR por `device_id`; cuenta Developers del cliente; guía de datos para Railway |
