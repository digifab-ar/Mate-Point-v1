const crypto = require('crypto');

const TS_TOLERANCE_SEC = 5 * 60;

/**
 * Parsea x-signature: "ts=1704908010,v1=abc..."
 * @returns {{ ts: string|null, v1: string|null }}
 */
function parseXSignature(header) {
  if (!header || typeof header !== 'string') {
    return { ts: null, v1: null };
  }
  let ts = null;
  let v1 = null;
  for (const part of header.split(',')) {
    const [key, ...rest] = part.split('=');
    const value = rest.join('=').trim();
    const k = key?.trim();
    if (k === 'ts') ts = value;
    if (k === 'v1') v1 = value;
  }
  return { ts, v1 };
}

/**
 * data.id para manifest: query param, minúsculas si es alfanumérico.
 */
function manifestDataId(req) {
  const raw = req.query['data.id'] ?? req.body?.data?.id ?? '';
  const id = String(raw).trim();
  if (!id) return '';
  if (/^[a-zA-Z0-9]+$/.test(id)) return id.toLowerCase();
  return id;
}

function buildManifest(req) {
  const dataId = manifestDataId(req);
  const xRequestId = req.headers['x-request-id'] || '';
  const { ts } = parseXSignature(req.headers['x-signature']);
  const parts = [];
  if (dataId) parts.push(`id:${dataId}`);
  if (xRequestId) parts.push(`request-id:${xRequestId}`);
  if (ts) parts.push(`ts:${ts}`);
  return parts.length ? `${parts.join(';')};` : '';
}

function timingSafeEqualHex(a, b) {
  if (!a || !b || a.length !== b.length) return false;
  try {
    return crypto.timingSafeEqual(Buffer.from(a, 'hex'), Buffer.from(b, 'hex'));
  } catch {
    return false;
  }
}

function isTimestampStale(tsSec) {
  const ts = Number(tsSec);
  if (!Number.isFinite(ts)) return false;
  const nowSec = Math.floor(Date.now() / 1000);
  return Math.abs(nowSec - ts) > TS_TOLERANCE_SEC;
}

/**
 * Estrategia C: intenta validar; no bloquea el flujo si falla.
 * @returns {{ checked: boolean, valid: boolean|null, reason: string }}
 */
function validateMpSignature(req) {
  const secret = process.env.MP_WEBHOOK_SECRET;
  const header = req.headers['x-signature'];

  if (!secret) {
    return { checked: false, valid: null, reason: 'missing_secret' };
  }
  if (!header) {
    return { checked: true, valid: null, reason: 'missing_header' };
  }

  const { ts, v1 } = parseXSignature(header);
  if (!ts || !v1) {
    return { checked: true, valid: false, reason: 'malformed_header' };
  }
  if (isTimestampStale(ts)) {
    return { checked: true, valid: false, reason: 'stale_timestamp' };
  }

  const manifest = buildManifest(req);
  if (!manifest.includes('id:')) {
    return { checked: true, valid: false, reason: 'missing_data_id' };
  }

  const expected = crypto.createHmac('sha256', secret).update(manifest).digest('hex');
  const valid = timingSafeEqualHex(expected, v1);
  return { checked: true, valid, reason: valid ? 'ok' : 'hmac_mismatch' };
}

function hmacSha256Hex(secret, manifest) {
  return crypto.createHmac('sha256', secret).update(manifest).digest('hex');
}

module.exports = {
  validateMpSignature,
  hmacSha256Hex,
  buildManifest,
  manifestDataId,
  parseXSignature,
};
