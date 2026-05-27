const express = require('express');
const { validateMpSignature } = require('../utils/signature');
const { getOrder, validateOrderForDispense } = require('../services/mercadopago');
const { wasDispensed, markDispensed } = require('../services/idempotency');
const { publishDispense } = require('../services/mqtt');

const router = express.Router();

const PROCESSED_ACTION = 'order.processed';

function resolveOrderId(req) {
  const fromQuery = req.query['data.id'];
  const fromBody = req.body?.data?.id;
  const id = (fromQuery || fromBody || '').toString().trim();
  return id || null;
}

function logEvent(event, fields = {}) {
  console.log(JSON.stringify({ event, ts: new Date().toISOString(), ...fields }));
}

router.post('/mp', async (req, res) => {
  logEvent('webhook_received', {
    action: req.body?.action,
    type: req.body?.type,
    query_id: req.query['data.id'],
    'x-request-id': req.headers['x-request-id'],
  });

  try {
    const action = req.body?.action;
    if (action !== PROCESSED_ACTION) {
      logEvent('webhook_ignored', { action, reason: 'action_not_processed' });
      return res.status(200).json({ ok: true, ignored: true });
    }

    const orderId = resolveOrderId(req);
    if (!orderId) {
      logEvent('webhook_ignored', { reason: 'missing_order_id' });
      return res.status(200).json({ ok: true, ignored: true });
    }

    const sig = validateMpSignature(req);
    if (!sig.checked) {
      logEvent('signature_skipped', { reason: sig.reason, order_id: orderId });
    } else if (sig.valid) {
      logEvent('signature_valid', { order_id: orderId });
    } else {
      logEvent('signature_invalid', { reason: sig.reason, order_id: orderId });
    }

    if (wasDispensed(orderId)) {
      logEvent('dispense_duplicate', { order_id: orderId });
      return res.status(200).json({ ok: true, duplicate: true });
    }

    let order;
    try {
      order = await getOrder(orderId);
      logEvent('order_fetch_ok', { order_id: orderId, status: order.status });
    } catch (err) {
      logEvent('order_fetch_failed', {
        order_id: orderId,
        message: err.message,
        status: err.status,
      });
      return res.status(200).json({ ok: true, dispensed: false, reason: 'order_fetch_failed' });
    }

    const validation = validateOrderForDispense(order);
    if (!validation.ok) {
      logEvent('order_not_ready', {
        order_id: orderId,
        reason: validation.reason,
        ...validation,
      });
      return res.status(200).json({ ok: true, dispensed: false, reason: validation.reason });
    }

    const externalReference = order.external_reference || req.body?.data?.external_reference;

    logEvent('dispense_triggered', {
      order_id: orderId,
      external_reference: externalReference,
      amount: order.total_paid_amount,
    });

    try {
      await publishDispense({
        order_id: orderId,
        external_reference: externalReference,
      });
      markDispensed(orderId);
      return res.status(200).json({ ok: true, dispensed: true, order_id: orderId });
    } catch (mqttErr) {
      logEvent('mqtt_failed', { order_id: orderId, message: mqttErr.message });
      return res.status(200).json({ ok: true, dispensed: false, reason: 'mqtt_failed' });
    }
  } catch (err) {
    logEvent('webhook_error', { message: err.message });
    return res.status(200).json({ ok: false, error: 'internal_error' });
  }
});

module.exports = router;
