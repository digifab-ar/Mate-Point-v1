# Servidor Mate Point

Backend Node.js para webhooks de Mercado Pago y publicación MQTT.

Documentación completa: [`../servidor-mate-point.md`](../servidor-mate-point.md)

## Endpoints

| Método | Ruta | Estado |
|--------|------|--------|
| `GET` | `/health` | Implementado |
| `POST` | `/webhook/mp` | Esqueleto (Fase 3) |
| `POST` | `/orders/create` | Esqueleto (Fase 3) |

## Desarrollo local

```bash
npm install
cp .env.example .env
npm run dev
```

## Railway

- **Root Directory:** `servidor` (configurar en el proyecto Railway)
- **Start command:** `npm start` (por defecto)
- Variables: copiar desde `.env.example`
