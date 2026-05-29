const express = require('express');
const { createStaticQrOrder, cancelOrder } = require('../services/mercadopago');

const router = express.Router();

function logEvent(event, fields = {}) {
  console.log(JSON.stringify({ event, ts: new Date().toISOString(), ...fields }));
}

/**
 * POST /orders/create
 * Crea orden QR estático en Mercado Pago (ESP32 / Postman).
 */
router.post('/create', async (req, res) => {
  const deviceId = (req.body?.device_id || process.env.MQTT_DEVICE_ID || 'MATEPOINT001')
    .toString()
    .trim();

  try {
    const order = await createStaticQrOrder({ deviceId });
    logEvent('order_created', {
      order_id: order.id,
      external_reference: order.external_reference,
      device_id: deviceId,
      status: order.status,
    });

    return res.status(201).json({
      order_id: order.id,
      status: order.status,
      external_reference: order.external_reference,
      total_amount: order.total_amount,
      expiration_time: process.env.MP_ORDER_EXPIRATION || 'PT2M',
      device_id: deviceId,
    });
  } catch (err) {
    logEvent('order_create_failed', {
      device_id: deviceId,
      message: err.message,
      status: err.status,
    });
    return res.status(err.status && err.status >= 400 ? err.status : 502).json({
      error: 'order_create_failed',
      message: err.message,
      details: err.body,
    });
  }
});

/**
 * POST /orders/cancel
 * Cancela orden MP en status created (timeout UI 2 min).
 */
router.post('/cancel', async (req, res) => {
  const orderId = (req.body?.order_id || '').toString().trim();
  if (!orderId) {
    return res.status(400).json({
      error: 'missing_order_id',
      message: 'Body requiere order_id',
    });
  }

  try {
    const order = await cancelOrder(orderId);
    logEvent('order_canceled', {
      order_id: order.id,
      status: order.status,
      status_detail: order.status_detail,
    });

    return res.json({
      ok: true,
      order_id: order.id,
      status: order.status,
      status_detail: order.status_detail,
    });
  } catch (err) {
    logEvent('order_cancel_failed', {
      order_id: orderId,
      message: err.message,
      status: err.status,
    });

    const httpStatus = err.status === 409 ? 409 : (err.status >= 400 ? err.status : 502);
    return res.status(httpStatus).json({
      error: 'order_cancel_failed',
      message: err.message,
      details: err.body,
    });
  }
});

module.exports = router;
