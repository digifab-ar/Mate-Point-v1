const fs = require('fs');
const path = require('path');

const DEFAULT_CONFIG_PATH = path.join(__dirname, '../config/devices.json');

let registry = null;

function configPath() {
  return process.env.DEVICES_CONFIG_PATH || DEFAULT_CONFIG_PATH;
}

function loadRegistry() {
  const filePath = configPath();
  const raw = fs.readFileSync(filePath, 'utf8');
  const data = JSON.parse(raw);
  if (!data || typeof data !== 'object') {
    throw new Error('devices.json inválido');
  }

  const storeExternalId = (data.store_external_id || '').toString().trim();
  const devices = Array.isArray(data.devices) ? data.devices : [];
  if (!storeExternalId) {
    throw new Error('devices.json: falta store_external_id');
  }
  if (devices.length === 0) {
    throw new Error('devices.json: flota vacía');
  }

  const byId = new Map();
  const byPos = new Map();
  for (const row of devices) {
    const deviceId = (row.device_id || '').toString().trim();
    const externalPosId = (row.external_pos_id || '').toString().trim();
    const name = (row.name || '').toString().trim();
    if (!deviceId || !externalPosId) {
      throw new Error('devices.json: cada fila requiere device_id y external_pos_id');
    }
    if (byId.has(deviceId)) {
      throw new Error(`devices.json: device_id duplicado ${deviceId}`);
    }
    if (byPos.has(externalPosId)) {
      throw new Error(`devices.json: external_pos_id duplicado ${externalPosId}`);
    }
    const entry = { deviceId, externalPosId, name };
    byId.set(deviceId, entry);
    byPos.set(externalPosId, entry);
  }

  registry = { storeExternalId, devices: [...byId.values()], byId, byPos, filePath };
  return registry;
}

function getRegistry() {
  if (!registry) loadRegistry();
  return registry;
}

function getDeviceById(deviceId) {
  const id = (deviceId || '').toString().trim();
  if (!id) return null;
  return getRegistry().byId.get(id) || null;
}

function getDeviceByPos(externalPosId) {
  const pos = (externalPosId || '').toString().trim();
  if (!pos) return null;
  return getRegistry().byPos.get(pos) || null;
}

function listDevices() {
  return getRegistry().devices;
}

function getStoreExternalId() {
  return getRegistry().storeExternalId;
}

module.exports = {
  loadRegistry,
  getRegistry,
  getDeviceById,
  getDeviceByPos,
  listDevices,
  getStoreExternalId,
};
