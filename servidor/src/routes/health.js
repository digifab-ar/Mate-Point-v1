const express = require('express');
const { getMqttStatus } = require('../services/mqtt');
const { listDevices, getStoreExternalId } = require('../services/devices');

const router = express.Router();

router.get('/', (_req, res) => {
  const mqtt = getMqttStatus();
  const ok = mqtt === 'connected' || mqtt === 'connecting';
  const devices = listDevices();
  res.status(ok ? 200 : 503).json({
    status: ok ? 'ok' : 'degraded',
    mqtt,
    store_external_id: getStoreExternalId(),
    devices: devices.map((d) => d.deviceId),
    ts: new Date().toISOString(),
  });
});

module.exports = router;
