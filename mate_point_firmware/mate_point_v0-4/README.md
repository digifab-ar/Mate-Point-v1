# Mate Point firmware v0-4

**UI Figma** — fork de [`mate_point_v0-3-4`](../mate_point_v0-3-4/) (E2E funcional validado).

Plan: [`PLAN-MATE-POINT-v0-4-UI.md`](../PLAN-MATE-POINT-v0-4-UI.md) · Spec UI: [`UI-DISCREPANCIAS-v0-4.md`](../UI-DISCREPANCIAS-v0-4.md)

**Estado (2026-06-18):** UI Figma operativa en Waveshare — flujo E2E funcional, rotación 180° validada. Iteración tipografía/layout vs Figma **implementada** (fuentes 20/32/36 px, colores QR, títulos centrados). **CTA bold** (Iniciar / Parar) validado en hardware. Pendiente: bold en títulos/precio QR + Test1 banco Nobana.

## Flujo UI

```
Iniciar → QR → pago → Coloca termo → Cargar termo (Iniciar) → dispensado (Parar) → Listo el mate → Iniciar
```

## Cambios vs v0-3-4

| Tema | v0-3-4 | v0-4 |
|------|--------|------|
| UI | Placeholder LVGL | Figma 1024×600, split 512+512 |
| Standby | Botón "Comprar" | Botón **Iniciar** + arte Taragüi full-screen |
| Progreso | Countdown M:SS | **Litros** (`UI_LITERS_FILL_SEC`) |
| WiFi/MQTT | Labels footer | Pantalla **bloqueante** |
| Error pago / timeout QR | Texto / silencioso | **Error-pago** 5 s → Standby |
| Finish | "terminado" / "Listo" | **LISTO EL MATE** |
| Orientación | 0° (montaje Waveshare demo) | **180°** (`lvgl_port.h`) — montaje dispensador |
| Tipografía / layout Figma | Placeholder 14/44 px | **20 / 32 / 36 px** Montserrat; **CTA bold** custom; títulos naranja Coloca/Wi-Fi; QR fondos distintos — ver [`UI-DISCREPANCIAS-v0-4.md`](../UI-DISCREPANCIAS-v0-4.md) |

## Historial UI

| Fecha | Cambio |
|-------|--------|
| 2026-06-18 | Fuente **bold** CTA (`ui_font_montserrat_bold_32`): Iniciar / Parar — ver § Tipografía |
| 2026-06-18 | Iteración tipografía/layout: fuentes 20/32/36 px, fondos QR, títulos centrados/naranja, ícono stop — [`UI-DISCREPANCIAS-v0-4.md`](../UI-DISCREPANCIAS-v0-4.md) |
| 2026-06-18 | Primera UI Figma completa + E2E hardware Waveshare |

## Arduino IDE

Abrir: `mate_point_v0-4/mate_point_v0-4.ino`

| Tools | Valor |
|-------|--------|
| Board | ESP32S3 Dev Module |
| **Flash Size** | **16 MB (128 Mb)** |
| **Partition Scheme** | **Custom Partition Table** |
| **Custom partition CSV** | `partitions.csv` (en esta carpeta) |
| **PSRAM** | **Enabled** (obligatorio con rotación 180°) |
| **USB CDC On Boot** | **Disabled** |
| DIP | **UART2** → Nobana |

Compilación esperada (~77 % de partición APP):

```
Sketch uses ~6440000 bytes of program storage space. Maximum is 8388608 bytes.
```

## Flash y partición

El firmware ocupa ~**6.4 MB** (código ~1.3 MB + assets RGB565 ~4.9 MB). **Max APP (4 MB)** de Arduino no alcanza.

Usar `partitions.csv` incluido:

| Partición | Tamaño | Uso |
|-----------|--------|-----|
| `factory` (APP) | 8 MB | Firmware + assets embebidos |
| `storage` (SPIFFS) | ~8 MB | Reservado OTA / almacenamiento futuro |

Si el IDE no ofrece *Custom Partition Table*, actualizar core **esp32** (≥ 2.0.14).

## Assets UI

PNG Figma → C arrays RGB565. Ver [`../../UI/ASSETS.md`](../../UI/ASSETS.md).

Arduino IDE **no compila** `.c` en subcarpetas → el sketch incluye `ui_assets_bundle.c` en la raíz, que `#include` todos los archivos de `ui/ui_assets/` y `ui/fonts/`.

Regenerar tras cambiar PNG:

```bash
python3 tools/png2lvgl_rgb565.py \
  UI/figma/imagenes/full-image-ad1.png \
  UI/figma/imagenes/Split-image-Ad1.png \
  UI/figma/imagenes/Split-image-coloca-termo.png \
  UI/figma/imagenes/Split-image-retira-termo.png \
  UI/figma/imagenes/Split-image-error-pago.png \
  UI/figma/imagenes/Split-image-error-wifi-mqtt.png \
  UI/figma/imagenes/Split-image-agua.png \
  -o mate_point_firmware/mate_point_v0-4/ui/ui_assets
```

## Orientación de pantalla (180°)

Montaje físico del dispensador requiere rotación 180° respecto al demo Waveshare.

En `lvgl_port.h`:

```c
#define EXAMPLE_LVGL_PORT_ROTATION_DEGREE  (180)   // 0 | 90 | 180 | 270
```

| Rotación | Efecto |
|----------|--------|
| Display | `rotate_copy_pixel()` en flush RGB |
| Touch GT911 | Mirror X + Y (180°) |
| Framebuffers | **3** en PSRAM (`LVGL_PORT_LCD_RGB_BUFFER_NUMS`) |

`rgb_lcd_port.cpp` inicializa el panel con `num_fbs = LVGL_PORT_LCD_RGB_BUFFER_NUMS` (debe coincidir; ver § Troubleshooting).

Para montaje sin rotación, usar `(0)` y volver a 2 framebuffers.

## Tipografía UI (`lv_conf.h`)

La UI v0-4 usa Montserrat **20 / 32 / 36 px** además de 14 y 44. Habilitar en `~/Documents/Arduino/libraries/lv_conf.h`:

```c
#define LV_FONT_MONTSERRAT_20 1
#define LV_FONT_MONTSERRAT_32 1
#define LV_FONT_MONTSERRAT_36 1
```

### Tokens (`ui/ui_theme.h`)

| Token | Fuente | Uso |
|-------|--------|-----|
| `UI_FONT_CTA` | `ui_font_montserrat_bold_32` (custom) | Texto botones **Iniciar** / **Parar** |
| `UI_FONT_CTA_ICON` | `lv_font_montserrat_32` | Flecha en CTA (`LV_SYMBOL_RIGHT`) |
| `UI_FONT_TITLE` | `lv_font_montserrat_36` | Títulos de pantalla (regular; bold pendiente) |
| `UI_FONT_PRODUCT_DESC` | `lv_font_montserrat_36` | Descripción QR |
| `UI_FONT_PRODUCT_PRICE` | `lv_font_montserrat_32` | Precio QR |
| `UI_FONT_CARD` | `lv_font_montserrat_20` | Cards inferiores |

LVGL 8 no incluye Montserrat **bold** integrado. Los CTA usan fuente custom en `ui/fonts/`.

### Regenerar fuente CTA bold

Desde `mate_point_v0-4/ui/fonts/`:

```bash
npx lv_font_conv \
  --font Montserrat-Bold.ttf \
  --size 32 --bpp 4 --format lvgl \
  --no-compress --no-prefilter \
  --lv-font-name ui_font_montserrat_bold_32 \
  --symbols "IniciarParar" \
  -o lv_font_montserrat_bold_32.c
```

Tras regenerar, verificar en el `.c` generado:

- `bitmap_format = 0` (con `--no-compress`; si queda en `1`, el texto del botón **no se ve**)
- `line_height = 35`, `base_line = 6` (alinear con `lv_font_montserrat_32`)
- `.fallback = &lv_font_montserrat_32`

El include Arduino (`lvgl.h` vs `lvgl/lvgl.h`) lo resuelve `ui_assets_bundle.c` vía `LV_LVGL_H_INCLUDE_SIMPLE`.

Spec completa: [`../UI-DISCREPANCIAS-v0-4.md`](../UI-DISCREPANCIAS-v0-4.md) §2.6.

## Config UI (`config.h`)

```c
#define UI_LITERS_FILL_SEC           120
#define UI_PRODUCT_LITERS_DEFAULT    1.0f
#define UI_ERROR_PAGO_MS             5000
#define UI_PRODUCT_DESC_PLACEHOLDER  "Recarga de 1 litro"
#define UI_PRODUCT_PRICE_PLACEHOLDER "$500"
```

## Estructura relevante

```
mate_point_v0-4/
├── mate_point_v0-4.ino
├── partitions.csv              ← 8 MB APP
├── ui_assets_bundle.c          ← bundle assets + fuente CTA (Arduino IDE)
├── display_ui.cpp / .h         ← pantallas LVGL
├── ui/
│   ├── ui_theme.h, ui_strings.h, ui_assets.h
│   ├── ui_assets/*.c           ← imágenes RGB565
│   └── fonts/
│       ├── Montserrat-Bold.ttf
│       └── lv_font_montserrat_bold_32.c
├── lvgl_port.h / .cpp          ← rotación, flush RGB
├── rgb_lcd_port.h / .cpp       ← num_fbs sincronizado con lvgl_port
└── config.h
```

## Troubleshooting

| Síntoma | Causa | Solución |
|---------|-------|----------|
| Pantalla negra, compila OK | Assets RGB565 mal formateados (`0xABCD` en `uint8_t[]` en lugar de `0xCD, 0xAB`) | Regenerar con `tools/png2lvgl_rgb565.py` corregido |
| Error flash 156 % (> 4 MB) | Partición Max APP 4 MB | `partitions.csv` + Custom Partition Table |
| Pantalla negra tras rotación 180° | Panel RGB con 2 FB, lvgl_port pide 3 (`fbs[2]` inválido) | `rgb_lcd_port.cpp`: `num_fbs = LVGL_PORT_LCD_RGB_BUFFER_NUMS` |
| Error compile C++ con rotación | Casts `void*` implícitos en `lvgl_port.cpp` | Casts explícitos en rutas `#if EXAMPLE_LVGL_PORT_ROTATION_DEGREE != 0` |
| UI muy oscura al boot | Overlay WiFi/MQTT bloqueante (`#001000`) sin conexión | Normal hasta conectar; con assets OK se ve ilustración + texto error |
| Texto botón CTA invisible | Fuente bold con `bitmap_format = 1` (comprimida sin datos válidos) | Regenerar con `--no-compress`; ver § Tipografía |
| Error `lvgl/lvgl.h: No such file` al compilar fuente | Include por defecto de `lv_font_conv` | `ui_assets_bundle.c` define `LV_LVGL_H_INCLUDE_SIMPLE` antes del `#include` |
| Pantalla negra con rotación | PSRAM disabled | **Tools → PSRAM → Enabled** |

## Referencias

- Plan UI: [`PLAN-MATE-POINT-v0-4-UI.md`](../PLAN-MATE-POINT-v0-4-UI.md)
- Base funcional: [`mate_point_v0-3-4/`](../mate_point_v0-3-4/)
- Assets: [`../../UI/ASSETS.md`](../../UI/ASSETS.md)
- Wireframe: [Figma Mate Point](https://www.figma.com/design/TmPeOvLv6q247UMZsAD8fl/Mate-Point?node-id=0-1)
