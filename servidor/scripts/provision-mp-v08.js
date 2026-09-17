#!/usr/bin/env node
/**
 * Alta v0-8: sucursal MATEPOINT + 4 cajas (idempotente).
 *
 * Lee MP_ACCESS_TOKEN y MP_USER_ID de servidor/.env (no imprime el token).
 *
 *   cd servidor
 *   npm run provision:mp-v08 -- --dry-run
 *   npm run provision:mp-v08
 *
 * PNGs: servidor/ops/mp-v08/ (gitignored).
 *
 * Si un external_id de caja ya existe en la sucursal POC vieja, se borra
 * esa caja y se recrea bajo MATEPOINT (opción B).
 */
const crypto = require('crypto');
const fs = require('fs');
const path = require('path');
require('dotenv').config({ path: path.join(__dirname, '../.env') });
const { loadRegistry } = require('../src/services/devices');

const MP_API_BASE = 'https://api.mercadopago.com';
const DRY_RUN = process.argv.includes('--dry-run');

const STORE_NAME = 'Mate point';
const STORE_LOCATION = {
  street_number: '1352',
  street_name: 'Santamarina',
  city_name: 'San Fernando',
  state_name: 'Buenos Aires',
  latitude: -34.4568,
  longitude: -58.5612,
  reference: 'Santamarina 1352 — sucursal única flota v0-8',
};
const BUSINESS_HOURS = {
  monday: [{ open: '08:00', close: '22:00' }],
  tuesday: [{ open: '08:00', close: '22:00' }],
  wednesday: [{ open: '08:00', close: '22:00' }],
  thursday: [{ open: '08:00', close: '22:00' }],
  friday: [{ open: '08:00', close: '22:00' }],
  saturday: [{ open: '09:00', close: '20:00' }],
  sunday: [{ open: '10:00', close: '18:00' }],
};

const OUT_DIR = path.join(__dirname, '../ops/mp-v08');

function getAccessToken() {
  const token = process.env.MP_ACCESS_TOKEN;
  if (!token) {
    throw new Error('MP_ACCESS_TOKEN no configurado (servidor/.env o entorno)');
  }
  return token;
}

function getUserId() {
  const id = (process.env.MP_USER_ID || '').toString().trim();
  if (!id) {
    throw new Error('MP_USER_ID no configurado');
  }
  return id;
}

function sleep(ms) {
  return new Promise((resolve) => setTimeout(resolve, ms));
}

async function mpFetch(method, urlPath, body) {
  const headers = {
    Authorization: `Bearer ${getAccessToken()}`,
    'Content-Type': 'application/json',
  };
  if (method === 'POST' || method === 'DELETE') {
    headers['X-Idempotency-Key'] = crypto.randomUUID();
  }
  const res = await fetch(`${MP_API_BASE}${urlPath}`, {
    method,
    headers,
    body: body ? JSON.stringify(body) : undefined,
  });
  const text = await res.text();
  let json = {};
  if (text) {
    try {
      json = JSON.parse(text);
    } catch {
      json = { raw: text };
    }
  }
  return { status: res.status, ok: res.ok, json };
}

function asList(payload) {
  if (Array.isArray(payload)) return payload;
  if (Array.isArray(payload?.results)) return payload.results;
  if (Array.isArray(payload?.stores)) return payload.stores;
  return [];
}

function storeIdOf(store) {
  return String(store.id);
}

function posOnStore(pos, store) {
  if (!pos || !store) return false;
  const sid = storeIdOf(store);
  const posSid = pos.store_id != null ? String(pos.store_id) : '';
  const posExt = (pos.external_store_id || '').toString();
  return posSid === sid || posExt === store.external_id;
}

async function findStore(userId, externalId) {
  const { ok, status, json } = await mpFetch(
    'GET',
    `/users/${userId}/stores/search?external_id=${encodeURIComponent(externalId)}`,
  );
  if (status === 404) return null;
  if (!ok) {
    throw new Error(`GET stores/search ${status}: ${json.message || JSON.stringify(json)}`);
  }
  return asList(json).find((s) => s.external_id === externalId) || null;
}

async function waitForStore(userId, externalId, fallback) {
  for (let i = 0; i < 8; i++) {
    const found = await findStore(userId, externalId);
    if (found) return found;
    await sleep(1500);
  }
  return fallback || null;
}

async function findPosByExternalId(externalId) {
  const { ok, status, json } = await mpFetch(
    'GET',
    `/pos?external_id=${encodeURIComponent(externalId)}`,
  );
  if (!ok) {
    const all = await mpFetch('GET', '/pos');
    if (!all.ok) {
      throw new Error(`GET pos ${status}/${all.status}: ${JSON.stringify(json)}`);
    }
    return asList(all.json).find((p) => p.external_id === externalId) || null;
  }
  return asList(json).find((p) => p.external_id === externalId) || asList(json)[0] || null;
}

async function deletePos(posId) {
  const id = encodeURIComponent(String(posId));
  let res = await mpFetch('DELETE', `/pos/${id}`);
  if (res.ok || res.status === 204) return res;
  res = await mpFetch('DELETE', `/v2/pos/${id}`);
  if (res.ok || res.status === 204) return res;
  throw new Error(`DELETE pos ${posId} ${res.status}: ${JSON.stringify(res.json)}`);
}

async function createPos(store, device) {
  const base = {
    name: device.name || `Mate point - ${device.deviceId}`,
    fixed_amount: true,
    store_id: storeIdOf(store),
    external_id: device.externalPosId,
    category: 621102,
  };

  for (let attempt = 1; attempt <= 6; attempt++) {
    const withExt = { ...base, external_store_id: store.external_id };
    let created = await mpFetch('POST', '/pos', attempt === 1 ? withExt : base);
    if (created.ok) return created.json;

    const err = (created.json?.error || '').toString();
    if (err === 'point_of_sale_exists') {
      return { conflict: true };
    }
    if (
      err === 'non_existent_external_store_id'
      || err === 'INEXISTENT_EXTERNAL_STORE_ID'
    ) {
      created = await mpFetch('POST', '/pos', base);
      if (created.ok) return created.json;
      console.log(JSON.stringify({
        event: 'pos_create_retry',
        device_id: device.deviceId,
        attempt,
        status: created.status,
        error: created.json?.error,
      }));
      await sleep(2000);
      continue;
    }

    throw new Error(
      `POST pos ${device.externalPosId} ${created.status}: ${JSON.stringify(created.json)}`,
    );
  }

  throw new Error(`POST pos ${device.externalPosId}: reintentos agotados`);
}

async function downloadQr(url, destPath) {
  const res = await fetch(url);
  if (!res.ok) {
    throw new Error(`download QR ${res.status} ${url}`);
  }
  const buf = Buffer.from(await res.arrayBuffer());
  fs.writeFileSync(destPath, buf);
  return destPath;
}

async function ensurePos(store, device) {
  let pos = await findPosByExternalId(device.externalPosId);

  if (pos && !posOnStore(pos, store)) {
    console.log(JSON.stringify({
      event: 'pos_deleted_old_store',
      device_id: device.deviceId,
      pos_id: pos.id,
      old_store_id: pos.store_id,
      old_external_store_id: pos.external_store_id,
      new_store_id: store.id,
    }));
    await deletePos(pos.id);
    pos = null;
  }

  if (pos && posOnStore(pos, store)) {
    console.log(JSON.stringify({
      event: 'pos_exists',
      device_id: device.deviceId,
      pos_id: pos.id,
      external_id: pos.external_id,
      store_id: pos.store_id,
    }));
    return pos;
  }

  const created = await createPos(store, device);
  if (created?.conflict) {
    pos = await findPosByExternalId(device.externalPosId);
    if (!pos) {
      throw new Error(`POS ${device.externalPosId} conflict sin GET`);
    }
    if (!posOnStore(pos, store)) {
      await deletePos(pos.id);
      const retry = await createPos(store, device);
      if (retry?.conflict || !retry?.id) {
        throw new Error(`No se pudo recrear POS ${device.externalPosId} en sucursal ${store.id}`);
      }
      pos = retry;
    }
    console.log(JSON.stringify({
      event: 'pos_exists_after_conflict',
      device_id: device.deviceId,
      pos_id: pos.id,
    }));
    return pos;
  }

  console.log(JSON.stringify({
    event: 'pos_created',
    device_id: device.deviceId,
    pos_id: created.id,
    external_id: created.external_id,
    uuid: created.uuid,
  }));
  return created;
}

async function main() {
  const fleet = loadRegistry();
  const userId = getUserId();
  getAccessToken();

  fs.mkdirSync(OUT_DIR, { recursive: true });

  console.log(JSON.stringify({
    event: 'provision_start',
    dry_run: DRY_RUN,
    user_id: userId,
    store_external_id: fleet.storeExternalId,
    devices: fleet.devices.map((d) => d.deviceId),
  }));

  const storeBody = {
    name: STORE_NAME,
    external_id: fleet.storeExternalId,
    location: STORE_LOCATION,
    business_hours: BUSINESS_HOURS,
  };

  let store = null;
  if (DRY_RUN) {
    console.log(JSON.stringify({ event: 'dry_run_store', body: storeBody }));
  } else {
    store = await findStore(userId, fleet.storeExternalId);
    if (store) {
      console.log(JSON.stringify({
        event: 'store_exists',
        store_id: store.id,
        external_id: store.external_id,
      }));
    } else {
      const created = await mpFetch('POST', `/users/${userId}/stores`, storeBody);
      if (!created.ok) {
        store = await findStore(userId, fleet.storeExternalId);
        if (!store) {
          throw new Error(`POST store ${created.status}: ${JSON.stringify(created.json)}`);
        }
        console.log(JSON.stringify({
          event: 'store_exists_after_post',
          store_id: store.id,
          status: created.status,
        }));
      } else {
        console.log(JSON.stringify({
          event: 'store_created',
          store_id: created.json.id,
          external_id: created.json.external_id,
        }));
        store = await waitForStore(userId, fleet.storeExternalId, created.json);
      }
    }
    if (!store?.id) {
      throw new Error('No hay store_id para crear cajas');
    }
  }

  const posResults = [];
  for (const device of fleet.devices) {
    if (DRY_RUN) {
      console.log(JSON.stringify({
        event: 'dry_run_pos',
        body: {
          name: device.name,
          store_id: store ? storeIdOf(store) : '(pendiente)',
          external_id: device.externalPosId,
        },
      }));
      posResults.push({
        device_id: device.deviceId,
        external_pos_id: device.externalPosId,
        dry_run: true,
      });
      continue;
    }

    const pos = await ensurePos(store, device);
    const qrUrl = pos.qr?.image || '';
    let pngPath = null;
    if (qrUrl) {
      pngPath = path.join(OUT_DIR, `${device.deviceId}.png`);
      await downloadQr(qrUrl, pngPath);
      console.log(JSON.stringify({
        event: 'qr_downloaded',
        device_id: device.deviceId,
        file: pngPath,
      }));
    } else {
      console.log(JSON.stringify({
        event: 'qr_missing',
        device_id: device.deviceId,
        pos_id: pos.id,
      }));
    }

    posResults.push({
      device_id: device.deviceId,
      name: device.name,
      external_pos_id: device.externalPosId,
      pos_id: pos.id,
      uuid: pos.uuid,
      status: pos.status,
      store_id: pos.store_id,
      qr_image: qrUrl,
      png: pngPath,
    });
  }

  const result = {
    ts: new Date().toISOString(),
    dry_run: DRY_RUN,
    user_id: userId,
    store: store
      ? {
        id: store.id,
        external_id: store.external_id,
        name: store.name,
      }
      : { external_id: fleet.storeExternalId, dry_run: true },
    devices: posResults,
  };

  const resultPath = path.join(OUT_DIR, 'provision-result.json');
  if (!DRY_RUN) {
    fs.writeFileSync(resultPath, `${JSON.stringify(result, null, 2)}\n`);
  }

  console.log(JSON.stringify({ event: 'provision_done', result_file: DRY_RUN ? null : resultPath }));
  console.log(JSON.stringify(result, null, 2));
}

main().catch((err) => {
  console.error(JSON.stringify({ event: 'provision_failed', message: err.message }));
  process.exit(1);
});
