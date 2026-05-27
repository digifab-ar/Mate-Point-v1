const express = require('express');
const { getMqttStatus } = require('../services/mqtt');

const router = express.Router();

router.get('/', (_req, res) => {
  const mqtt = getMqttStatus();
  const ok = mqtt === 'connected' || mqtt === 'connecting';
  res.status(ok ? 200 : 503).json({
    status: ok ? 'ok' : 'degraded',
    mqtt,
    ts: new Date().toISOString(),
  });
});

module.exports = router;
