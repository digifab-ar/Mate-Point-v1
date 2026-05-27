const mqtt = require('mqtt');

let client = null;
let status = 'disconnected';

function getMqttStatus() {
  return status;
}

function commandTopic() {
  const deviceId = process.env.MQTT_DEVICE_ID || 'MATEPOINT001';
  return `mate/${deviceId}/command`;
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
  const durationMs = Number(process.env.DISPENSE_DURATION_MS || 120000);
  const message = {
    cmd: 'dispense',
    duration_ms: durationMs,
    ts: Date.now(),
    ...payload,
  };

  return new Promise((resolve, reject) => {
    if (!client || status !== 'connected') {
      return reject(new Error(`MQTT no conectado (status: ${status})`));
    }
    client.publish(commandTopic(), JSON.stringify(message), { qos: 1 }, (err) => {
      if (err) return reject(err);
      console.log(JSON.stringify({ event: 'mqtt_published', topic: commandTopic(), message }));
      resolve(message);
    });
  });
}

module.exports = { connectMqtt, getMqttStatus, publishDispense, commandTopic };
