# Servidor Mate Point

Backend Node.js para webhooks de Mercado Pago y publicación MQTT.

Documentación completa: [`../servidor-mate-point.md`](../servidor-mate-point.md)  
Flota v0-8: [`../mate_point_firmware/PLAN-MATE-POINT-v0-8.md`](../mate_point_firmware/PLAN-MATE-POINT-v0-8.md)

## Endpoints

| Método | Ruta | Estado |
|--------|------|--------|
| `GET` | `/health` | Implementado — incluye `devices[]` |
| `POST` | `/webhook/mp` | **Implementado** — firma (C) + GET orden + MQTT por `device_id` |
| `POST` | `/orders/create` | **Implementado** — body `{ "device_id": "MATEPOINT00n" }` (obligatorio) |
| `POST` | `/orders/cancel` | **Implementado** — cancela orden en status `created` |

`device_id` desconocido o ausente → `400`. El POS sale de [`src/config/devices.json`](src/config/devices.json).

## Desarrollo local

```bash
npm install
cp .env.example .env
npm run dev
```

Alta sucursal `MATEPOINT` + 4 cajas (token en `.env`, no en git):

```bash
npm run provision:mp-v08 -- --dry-run
npm run provision:mp-v08
```

## Railway

- **Root Directory:** `servidor` (configurar en el proyecto Railway)
- **Start command:** `npm start` (por defecto)
- Variables: copiar desde `.env.example`
- **No** hace falta `MQTT_DEVICE_ID` ni `MP_EXTERNAL_POS_ID` (v0-8)
- Tras el primer deploy v0-8: la máquina v0-9 con QR POC **deja de cobrar** hasta flashear el PNG nuevo
