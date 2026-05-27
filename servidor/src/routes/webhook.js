const express = require('express');
const { publishDispense } = require('../services/mqtt');

const router = express.Router();

/**
 * POST /webhook/mp
 * Recibe notificaciones de Mercado Pago (Fase 3 — validación y consulta orden pendiente).
 */
router.post('/mp', async (req, res) => {
  console.log(JSON.stringify({
    event: 'webhook_received',
    ts: new Date().toISOString(),
    headers: {
      'x-signature': req.headers['x-signature'],
      'x-request-id': req.headers['x-request-id'],
    },
    body: req.body,
  }));

  // TODO Fase 3: validateMpSignature(req), GET /v1/orders/{id}, status processed/accredited
  res.status(200).json({ received: true });
});

module.exports = router;
