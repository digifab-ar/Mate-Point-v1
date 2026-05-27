require('dotenv').config();

const express = require('express');
const morgan = require('morgan');

const healthRouter = require('./routes/health');
const ordersRouter = require('./routes/orders');
const webhookRouter = require('./routes/webhook');
const { connectMqtt, getMqttStatus } = require('./services/mqtt');

const PORT = process.env.PORT || 3000;

const app = express();
app.use(morgan('combined'));
app.use(express.json());

app.use('/health', healthRouter);
app.use('/orders', ordersRouter);
app.use('/webhook', webhookRouter);

app.get('/', (_req, res) => {
  res.json({
    service: 'mate-point-server',
    version: '1.0.0',
    mqtt: getMqttStatus(),
  });
});

connectMqtt();

app.listen(PORT, () => {
  console.log(JSON.stringify({
    event: 'server_start',
    port: PORT,
    mqtt: getMqttStatus(),
    broker: process.env.MQTT_BROKER_URL ? '(configured)' : '(missing MQTT_BROKER_URL)',
  }));
});
