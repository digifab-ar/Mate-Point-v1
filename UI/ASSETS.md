# UI assets — mate_point_v0-4

Imágenes exportadas desde Figma (`UI/figma/imagenes/`) convertidas a arrays C **RGB565** para LVGL 8 (`LV_COLOR_DEPTH=16`, `LV_COLOR_16_SWAP=0`).

## Inventario

| PNG | Símbolo C | Resolución | Uso |
|-----|-----------|------------|-----|
| `full-image-ad1.png` | `img_full_image_ad1` | 1024×600 | Standby (full-screen) |
| `Split-image-Ad1.png` | `img_split_image_ad1` | 512×600 | Cargar termo (panel izq.) |
| `Split-image-coloca-termo.png` | `img_split_image_coloca_termo` | 512×600 | Coloca termo |
| `Split-image-retira-termo.png` | `img_split_image_retira_termo` | 512×600 | Finish |
| `Split-image-error-pago.png` | `img_split_image_error_pago` | 512×600 | Error pago |
| `Split-image-error-wifi-mqtt.png` | `img_split_image_error_wifi_mqtt` | 512×600 | Error conectividad |
| `Split-image-agua.png` | `img_split_image_agua` | 512×600 | Error agua/bandeja (UI preparada) |

Símbolos declarados en `mate_point_firmware/mate_point_v0-4/ui/ui_assets.h`.

## Generación

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

Requisito: `pip install Pillow`

## Formato C (importante)

El array debe ser `uint8_t[]` con **bytes little-endian por píxel**, no palabras de 16 bits:

```c
// Correcto (como qr_static_img.c):
0xa5, 0xa8, 0xc5, 0xa0, ...

// Incorrecto — trunca al byte bajo → pantalla negra:
0xa8a5, 0xa0c5, ...
```

El script `tools/png2lvgl_rgb565.py` emite el formato correcto desde 2026-06-18.

## Arduino IDE

Los `.c` generados viven en `ui/ui_assets/`, pero **Arduino IDE no compila subcarpetas**.

El sketch incluye `ui_assets_bundle.c` en la raíz del sketch, que `#include` todos los assets.

## Tamaño flash

| Componente | ~Tamaño |
|------------|---------|
| 7 imágenes RGB565 | ~4.9 MB |
| `qr_static_img.c` | ~0.2 MB |
| Código firmware | ~1.3 MB |
| **Total sketch** | **~6.4 MB** |

Requiere partición APP ≥ 8 MB — ver `mate_point_v0-4/partitions.csv` y [`README.md`](../mate_point_firmware/mate_point_v0-4/README.md).
