const crypto = require('crypto');

const MP_API_BASE = 'https://api.mercadopago.com';

function getAccessToken() {
  const token = process.env.MP_ACCESS_TOKEN;
  if (!token) throw new Error('MP_ACCESS_TOKEN no configurado');
  return token;
}

function normalizeAmount(value) {
  const n = Number(value);
  if (!Number.isFinite(n)) return null;
  return n.toFixed(2);
}

/**
 * GET /v1/orders/{id}
 * @param {string} orderId
 * @returns {Promise<object>}
 */
async function getOrder(orderId) {
  const res = await fetch(`${MP_API_BASE}/v1/orders/${encodeURIComponent(orderId)}`, {
    method: 'GET',
    headers: {
      Authorization: `Bearer ${getAccessToken()}`,
      'Content-Type': 'application/json',
    },
  });

  const body = await res.json().catch(() => ({}));
  if (!res.ok) {
    const err = new Error(body.message || `MP API ${res.status}`);
    err.status = res.status;
    err.body = body;
    throw err;
  }
  return body;
}

/**
 * Valida si la orden autoriza dispensado (§5.2.2 + MP_SALE_AMOUNT).
 * @returns {{ ok: boolean, reason: string }}
 */
function validateOrderForDispense(order) {
  const expectedAmount = normalizeAmount(process.env.MP_SALE_AMOUNT || '500.00');
  const expectedPos = process.env.MP_EXTERNAL_POS_ID || 'MATEPOINT001POS001';

  if (order.status !== 'processed') {
    return { ok: false, reason: `status_${order.status}` };
  }
  if (order.status_detail !== 'accredited') {
    return { ok: false, reason: `status_detail_${order.status_detail}` };
  }
  if (order.type !== 'qr') {
    return { ok: false, reason: `type_${order.type}` };
  }

  const qr = order.config?.qr;
  if (qr?.mode !== 'static') {
    return { ok: false, reason: `qr_mode_${qr?.mode}` };
  }
  if (qr?.external_pos_id !== expectedPos) {
    return { ok: false, reason: 'external_pos_mismatch' };
  }

  const total = normalizeAmount(order.total_amount);
  const paid = normalizeAmount(order.total_paid_amount);
  if (total !== expectedAmount || paid !== expectedAmount) {
    return {
      ok: false,
      reason: 'amount_mismatch',
      expected: expectedAmount,
      total,
      paid,
    };
  }

  const payment = order.transactions?.payments?.[0];
  if (payment && payment.status !== 'processed') {
    return { ok: false, reason: `payment_status_${payment.status}` };
  }

  return { ok: true, reason: 'ok' };
}

function generateExternalReference(deviceId) {
  const now = new Date();
  const y = now.getUTCFullYear();
  const m = String(now.getUTCMonth() + 1).padStart(2, '0');
  const d = String(now.getUTCDate()).padStart(2, '0');
  const suffix = crypto.randomBytes(3).toString('hex');
  const prefix = (deviceId || 'MATEPOINT001').toLowerCase().replace(/[^a-z0-9]/g, '');
  return `mate-${prefix}-${y}${m}${d}-${suffix}`;
}

/**
 * POST /v1/orders — QR estático (§5.3 integracion-mercadopago-qr.md)
 * @param {{ deviceId?: string, externalReference?: string }} params
 */
async function createStaticQrOrder(params = {}) {
  const amount = normalizeAmount(process.env.MP_SALE_AMOUNT || '500.00');
  const posId = process.env.MP_EXTERNAL_POS_ID || 'MATEPOINT001POS001';
  const expiration = process.env.MP_ORDER_EXPIRATION || 'PT2M';
  const externalReference = params.externalReference
    || generateExternalReference(params.deviceId);

  const body = {
    type: 'qr',
    total_amount: amount,
    description: 'Agua caliente - 1 porcion',
    external_reference: externalReference,
    expiration_time: expiration,
    config: {
      qr: {
        external_pos_id: posId,
        mode: 'static',
      },
    },
    transactions: {
      payments: [{ amount }],
    },
    items: [
      {
        title: 'Agua caliente',
        unit_price: amount,
        quantity: 1,
        unit_measure: 'unit',
        external_code: 'AGUA001',
      },
    ],
  };

  const res = await fetch(`${MP_API_BASE}/v1/orders`, {
    method: 'POST',
    headers: {
      Authorization: `Bearer ${getAccessToken()}`,
      'Content-Type': 'application/json',
      'X-Idempotency-Key': crypto.randomUUID(),
    },
    body: JSON.stringify(body),
  });

  const order = await res.json().catch(() => ({}));
  if (!res.ok) {
    const err = new Error(order.message || `MP API ${res.status}`);
    err.status = res.status;
    err.body = order;
    throw err;
  }
  return order;
}

/**
 * POST /v1/orders/{id}/cancel — solo status created / action_required
 * @param {string} orderId
 */
async function cancelOrder(orderId) {
  const res = await fetch(
    `${MP_API_BASE}/v1/orders/${encodeURIComponent(orderId)}/cancel`,
    {
      method: 'POST',
      headers: {
        Authorization: `Bearer ${getAccessToken()}`,
        'Content-Type': 'application/json',
        'X-Idempotency-Key': crypto.randomUUID(),
      },
    },
  );

  const order = await res.json().catch(() => ({}));
  if (!res.ok) {
    const err = new Error(order.message || `MP API ${res.status}`);
    err.status = res.status;
    err.body = order;
    throw err;
  }
  return order;
}

module.exports = {
  getOrder,
  validateOrderForDispense,
  createStaticQrOrder,
  cancelOrder,
  generateExternalReference,
  normalizeAmount,
};
