const express = require('express');

const router = express.Router();

/**
 * POST /orders/create
 * Crea orden QR estático en Mercado Pago (Fase 3 — implementación pendiente).
 */
router.post('/create', async (_req, res) => {
  res.status(501).json({
    error: 'not_implemented',
    message: 'POST /orders/create — pendiente Fase 3',
  });
});

module.exports = router;
