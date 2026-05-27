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

async function createStaticQrOrder(_params) {
  throw new Error('createStaticQrOrder — pendiente');
}

module.exports = {
  getOrder,
  validateOrderForDispense,
  createStaticQrOrder,
  normalizeAmount,
};
