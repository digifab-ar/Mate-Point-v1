# Mate Point v1

Dispensador de agua caliente con pago por **QR estático** de Mercado Pago, backend en Railway y comando de dispensado vía MQTT hacia ESP32-S3 (Waveshare).

**OT:** OT-00268 — Etapa 3  
**Organización:** [digifab-ar](https://github.com/digifab-ar)  
**Repositorio:** [Mate-Point-v1](https://github.com/digifab-ar/Mate-Point-v1)

---

## Contenido del repositorio

| Ruta | Descripción |
|------|-------------|
| [`servidor/`](servidor/) | Backend Node.js (Express) — deploy en **Railway** |
| [`servidor-mate-point.md`](servidor-mate-point.md) | Arquitectura del servidor, MQTT, Railway, variables de entorno |
| [`integracion-mercadopago-qr.md`](integracion-mercadopago-qr.md) | Integración MP: QR estático, Postman, sandbox |
| [`plan-de-implementacion.md`](plan-de-implementacion.md) | Plan por fases (0–6) |
| [`arquitectura-mate-point.md`](arquitectura-mate-point.md) | Arquitectura general del sistema |
| [`arquitectura-hardware.md`](arquitectura-hardware.md) | Hardware, UART Nobana, TXS0108E |
| [`modulo-waveshare-esp32s3-touch-7b.md`](modulo-waveshare-esp32s3-touch-7b.md) | Pantalla táctil 7" |
| [`mate_point_firmware/`](mate_point_firmware/) | **Firmware ESP32** — POC v0.1 MQTT · **v0.2 Comprar+QR** |
| [`dispensador-nobana.md`](dispensador-nobana.md) | Dispensador Nobana |

---

## Estado del proyecto

| Fase | Entregable | Estado |
|------|------------|--------|
| 0 | Parámetros (precio, tiempo, `device_id`) | Completado |
| 1 | App MP + sucursal + caja | Completado |
| 2 | QR estático + pago sandbox | Completado |
| 3 | Webhook + backend (Railway) + MQTT | Completado |
| 4 | MQTT + ESP32 (POC 4.1–4.3 + E2E Railway) | **Completado** — [`mate_point_v0-1`](mate_point_firmware/mate_point_v0-1/) |
| 4 | POC completa v0.2 (Comprar → QR → pago) | **Implementado** — [`mate_point_v0-2`](mate_point_firmware/mate_point_v0-2/) · E2E hardware pendiente |
| 4 | UART Nobana (4.4–4.10) | Pendiente |
| 5 | Pantalla QR + UX | **Parcial** — v0.2: Comprar, QR PROGMEM, countdown, cancel |
| 6 | Producción MP | Pendiente |

Detalle Fase 4 POC: [`fase-4-plan-4.1-4.3-TEMP.md`](fase-4-plan-4.1-4.3-TEMP.md) · firmware: [`mate_point_firmware/PLAN-IMPLEMENTACION.md`](mate_point_firmware/PLAN-IMPLEMENTACION.md).

Última prueba e2e backend (Fase 3): orden `ORDTST01KSNFEN3H3FTHXMK9Q1ZPE5NZ` — detalle en [`integracion-mercadopago-qr.md`](integracion-mercadopago-qr.md) §0.2.

---

## Inicio rápido — servidor local

```bash
cd servidor
cp .env.example .env
# Completar .env (MP_ACCESS_TOKEN, MQTT_BROKER_URL, etc.)
npm install
npm run dev
```

- Health: `GET http://localhost:3000/health`
- Ver [`servidor/README.md`](servidor/README.md) y [`servidor-mate-point.md`](servidor-mate-point.md)

---

## Deploy en Railway

1. Conectar este repo en [Railway](https://railway.app) → **Root Directory:** `servidor`
2. Variables de entorno según [`servidor/.env.example`](servidor/.env.example)
3. Habilitar dominio público → registrar `https://<slug>.up.railway.app/webhook/mp` en Mercado Pago

---

## Parámetros del prototipo

| Parámetro | Valor |
|-----------|-------|
| Precio | $ 500,00 ARS |
| Dispensado | 120 s (120 000 ms) |
| `device_id` | `MATEPOINT001` |
| QR | Estático (`MATEPOINT001POS001`) |
