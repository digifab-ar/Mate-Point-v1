# Mate Point firmware v0-6

**Wi-Fi SoftAP + portal web (NVS)** — fork de [`mate_point_v0-5-2`](../mate_point_v0-5-2/) (pausa/reanudar + bandeja + agua UART + UI Figma).

| Documento | Uso |
|-----------|-----|
| [`PLAN-MATE-POINT-v0-6.md`](../PLAN-MATE-POINT-v0-6.md) | Plan normativo + criterios de aceptación |
| [`arquitectura-mate-point.md`](../../arquitectura-mate-point.md) §3 | Configuración Wi-Fi producto |
| Figma | [`Error-wifi.png`](../../UI/figma/pantallas/Error-wifi.png) · [`Error-mqtt.png`](../../UI/figma/pantallas/Error-mqtt.png) · [`Configurar red.png`](../../UI/figma/pantallas/Configurar%20red.png) |

**Estado:** **E2E OK hardware** (2026-06-25) — provisioning Wi-Fi + portal web validados en campo.

---

## Resumen

El dueño del local configura la red Wi-Fi **desde la pantalla táctil**, sin laptop ni USB:

1. Pantalla **WI-FI DESCONECTADO** → **Configurar red**
2. Pantalla **CONFIGURAR RED** con instrucciones y AP `MatePoint-XXXX`
3. Celular → portal en **192.168.4.1** → elegir red 2.4 GHz + contraseña
4. Credenciales en **NVS** → **`ESP.restart()`** → operación normal

El broker MQTT (`broker.hivemq.com:1883`) permanece **fijo en `config.h`**.

---

## Guía — dueño del local

### Primera configuración o cambio de red

1. Si aparece **WI-FI DESCONECTADO**, tocar **Configurar red**.
2. En **CONFIGURAR RED**, anotar el nombre `MatePoint-XXXX` (suffix = últimos 2 bytes del MAC).
3. En el celular: Ajustes → Wi-Fi → conectar **MatePoint-XXXX**.
4. Abrir el navegador en **192.168.4.1** (el portal cautivo suele abrirse solo).
5. En el portal **Mate Point** → elegir la red del local (2.4 GHz) → **Contraseña** → **Conectar**.
6. El equipo se reinicia solo al conectar correctamente.

**Cancelar** en la pantalla del Mate Point cierra el AP y vuelve a Error Wi-Fi (timeout automático: 10 min).

### Error servidor

Si aparece **ERROR EN SERVIDOR**, el Wi-Fi del local está bien. Contactar al **proveedor** del servicio (MQTT); no reconfigurar red.

---

## Pantallas UI (LVGL)

| Pantalla | Figma | Comportamiento |
|----------|-------|----------------|
| Error Wi-Fi | `Error-wifi.png` | CTA **Configurar red** |
| Error servidor | `Error-mqtt.png` | Card **Consulta al proveedor**; reconexión MQTT en background |
| Provisioning | `Configurar red.png` | Título arriba; card centrada con 3 líneas; **Cancelar** |

**Prioridad overlays:** hardware (agua/bandeja) > Wi-Fi > MQTT > flujo normal.

---

## Portal web (SoftAP)

| Parámetro | Valor |
|-----------|-------|
| SSID AP | `MatePoint-XXXX` (abierto, sin clave) |
| IP | `192.168.4.1` |
| Rutas | `GET /` formulario · `GET /scan` JSON redes · `POST /connect` |
| Éxito STA | Guarda NVS + mensaje + reinicio |
| Fallo STA | Mensaje en web; NVS anterior intacto |

Header del portal: **Mate Point** + **Configurar Wi-Fi**. Sin campo de red oculta.

---

## Módulos firmware

| Archivo | Rol |
|---------|-----|
| `wifi_config.cpp` | NVS namespace `mate_cfg` — claves `wifi_ssid`, `wifi_pass` |
| `wifi_portal.cpp` | SoftAP + DNSServer (captive) + WebServer |
| `wifi_portal_html.h` | HTML del portal |
| `mate_network.cpp` | Orquestación STA/MQTT/provisioning |
| `display_ui.cpp` | Tres pantallas de conectividad + callbacks |

### NVS

- Escritura **solo** tras asociación STA exitosa en el portal (no exige MQTT).
- Sin credenciales en NVS → pantalla Error Wi-Fi al boot.
- Flashear v0-6 sobre v0-5-2 → NVS Wi-Fi vacío → reprovisionar.

### `config.h` relevante

```c
#define MQTT_HOST "broker.hivemq.com"
#define MQTT_PORT 1883
#define MQTT_CLIENT_ID "mate-" DEVICE_ID "-esp32-v060"
#define WIFI_BOOT_ATTEMPT_MS 15000
#define WIFI_BOOT_MAX_ATTEMPTS 5
#define WIFI_PORTAL_STA_TIMEOUT_MS 20000
#define WIFI_PROVISIONING_TIMEOUT_MS 600000
```

No hay `WIFI_SSID` / `WIFI_PASSWORD` en código.

---

## Tipografía CTA (español)

Los botones bold (`Configurar red`, `Cancelar`, etc.) usan `ui_font_montserrat_bold_32` con charset **ASCII + Latin-1** (`0x20-0x7F`, `0xA1-0xFF`).

Regenerar desde esta carpeta:

```bash
npx lv_font_conv --font ui/fonts/Montserrat-Bold.ttf --size 32 --bpp 4 --format lvgl \
  --no-compress --no-prefilter --lv-font-name ui_font_montserrat_bold_32 \
  --range 0x20-0x7F,0xA1-0xFF \
  --symbols "IniciarPararContinuarFinalizarConfigurar redCancelar" \
  -o ui/fonts/lv_font_montserrat_bold_32.c
```

Texto de cards (20 px regular) usa `lv_font_montserrat_20` integrado en LVGL.

---

## Arduino IDE

Abrir: `mate_point_v0-6/mate_point_v0-6.ino`

| Parámetro | Valor |
|-----------|-------|
| Placa | ESP32-S3-Touch-LCD-7 |
| Flash | 16 MB |
| Partición | Custom → `partitions.csv` (8 MB APP) |
| PSRAM | OPI |
| USB CDC On Boot | **Disabled** |
| UART0 | GPIO44/43 @ 9600 — solo Nobana |

---

## Herencia v0-5-2

Sin cambios funcionales: pausa/reanudar Cargar termo, bandeja GPIO6, error agua UART, VL53L0X, UI Figma, flujo E2E compra.

---

## Validación (2026-06-25)

| ID | Criterio | Resultado |
|----|----------|-----------|
| W1 | NVS vacío → Error Wi-Fi + CTA | OK |
| W2 | AP `MatePoint-XXXX` visible en celular | OK |
| W3 | Portal → red 2.4 GHz → NVS guardado | OK |
| W4 | Reinicio post-provisioning → operación normal | OK |
| W6 | Reboot → conexión automática desde NVS | OK |
| W16 | Cancelar → AP off → Error Wi-Fi | OK |
| W17 | UI provisioning alineada con Figma | OK |

Iteraciones post-implementación: fuente CTA español completa; portal sin red oculta + header Mate Point; layout provisioning (título arriba, card centrada).
