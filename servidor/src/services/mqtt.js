const mqtt = require('mqtt');

let client = null;
let status = 'disconnected';

function getMqttStatus() {
  return status;
}

function commandTopic(deviceId) {
  const id = (deviceId || '').toString().trim();
  if (!id) {
    throw new Error('deviceId requerido para el topic MQTT');
  }
  return `mate/${id}/command`;
}

function connectMqtt() {
  const url = process.env.MQTT_BROKER_URL;
  if (!url) {
    console.warn('MQTT_BROKER_URL no definida — MQTT deshabilitado');
    status = 'disabled';
    return;
  }

  const options = {
    clientId: `mate-server-${Math.random().toString(16).slice(2, 10)}`,
    reconnectPeriod: 5000,
    connectTimeout: 30000,
  };

  if (process.env.MQTT_USER) {
    options.username = process.env.MQTT_USER;
    options.password = process.env.MQTT_PASS;
  }

  status = 'connecting';
  client = mqtt.connect(url, options);

  client.on('connect', () => {
    status = 'connected';
    console.log(JSON.stringify({ event: 'mqtt_connected', url: url.replace(/\/\/.*@/, '//***@') }));
  });

  client.on('reconnect', () => {
    status = 'connecting';
  });

  client.on('close', () => {
    status = 'disconnected';
  });

  client.on('error', (err) => {
    console.error(JSON.stringify({ event: 'mqtt_error', message: err.message }));
  });
}

function publishDispense(payload = {}) {
  const deviceId = (payload.device_id || '').toString().trim();
  if (!deviceId) {
    return Promise.reject(new Error('device_id requerido para publicar dispense'));
  }

  const durationMs = Number(process.env.DISPENSE_DURATION_MS || 120000);
  const pauseTimeoutMs = Number(process.env.PAUSE_TIMEOUT_MS || 20000);
  const topic = commandTopic(deviceId);
  const message = {
    cmd: 'dispense',
    duration_ms: durationMs,
    pause_timeout_ms: pauseTimeoutMs,
    ts: Date.now(),
    ...payload,
    device_id: deviceId,
  };

  return new Promise((resolve, reject) => {
    if (!client || status !== 'connected') {
      return reject(new Error(`MQTT no conectado (status: ${status})`));
    }
    client.publish(topic, JSON.stringify(message), { qos: 1 }, (err) => {
      if (err) return reject(err);
      console.log(JSON.stringify({ event: 'mqtt_published', topic, message }));
      resolve(message);
    });
  });
}

module.exports = { connectMqtt, getMqttStatus, publishDispense, commandTopic };
