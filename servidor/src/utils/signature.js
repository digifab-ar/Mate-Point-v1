const crypto = require('crypto');

/**
 * Valida x-signature de Mercado Pago (Fase 3).
 * @see https://www.mercadopago.com.ar/developers/es/docs/your-integrations/notifications/webhooks
 */
function validateMpSignature(_req) {
  // TODO Fase 3: implementar según manifest oficial MP
  return false;
}

function hmacSha256Hex(secret, manifest) {
  return crypto.createHmac('sha256', secret).update(manifest).digest('hex');
}

module.exports = { validateMpSignature, hmacSha256Hex };
