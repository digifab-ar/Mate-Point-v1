# UI Figma — mate_point_v0-4

**Proyecto:** Mate Point — OT-00268 Etapa 3  
**Firmware:** [`mate_point_v0-4/`](mate_point_v0-4/)  
**Referencias visuales:** [`UI/figma/pantallas/`](../UI/figma/pantallas/)  
**Plan maestro:** [`PLAN-MATE-POINT-v0-4-UI.md`](PLAN-MATE-POINT-v0-4-UI.md)  
**Última actualización:** 2026-06-18  
**Estado:** Iteración tipografía/layout **implementada** — CTA bold validado en hardware; pendiente bold en títulos/precio QR

---

## 1. Contexto

Tras la primera validación en Waveshare (UI funcional + E2E + rotación 180°), se compararon capturas de hardware contra los PNG de Figma. Se identificaron 14 puntos de ajuste (tipografía, colores, posición de títulos, ícono Parar). Esta iteración cierra la spec y deja el código alineado en `display_ui.cpp` y `ui_theme.h`.

**Origen del análisis:** sesión 2026-06-18 (capturas hardware + referencias Figma en `UI/figma/pantallas/`).

---

## 2. Spec aplicada

### 2.1 Tipografía

| Token | LVGL | Tamaño Figma | Peso Figma | Uso |
|-------|------|--------------|------------|-----|
| `UI_FONT_CTA` | `ui_font_montserrat_bold_32` (custom) | 32 px | **bold** | Botones Iniciar / Parar |
| `UI_FONT_CTA_ICON` | `lv_font_montserrat_32` | 32 px | regular | Flecha `LV_SYMBOL_RIGHT` en CTA |
| `UI_FONT_TITLE` | `lv_font_montserrat_36` | 36 px | bold | Títulos de pantalla |
| `UI_FONT_PRODUCT_DESC` | `lv_font_montserrat_36` | 36 px | bold | Descripción producto (QR) |
| `UI_FONT_PRODUCT_PRICE` | `lv_font_montserrat_32` | 32 px | bold | Precio (QR) |
| `UI_FONT_CARD` | `lv_font_montserrat_20` | 20 px | regular | Texto cards inferiores |
| `UI_FONT_VALUE` | `lv_font_montserrat_44` | — | — | Litros y temperatura (sin cambio) |
| `UI_FONT_LABEL` | `lv_font_montserrat_14` | — | — | Labels ESTADO / TEMPERATURA |

> **LVGL 8 — Montserrat integrado:** solo trae peso **regular**. Los botones CTA usan fuente custom **bold** (§2.6). Títulos, precio y descripción QR siguen en regular hasta exportar variantes 36/32 px bold.

### 2.2 Colores

Paleta **RGB565-safe** (ajustada 2026-06-18 para Waveshare). Ver §3 en [`PLAN-MATE-POINT-v0-4-UI.md`](PLAN-MATE-POINT-v0-4-UI.md).

| Token | Hex | Uso |
|-------|-----|-----|
| `UI_COLOR_GREEN_DARKEST` | `#001000` | Fondo panel derecho QR; fondos split generales |
| `UI_COLOR_GREEN_DARK` | `#001808` | Fondo panel izquierdo QR; fondo card QR |
| `UI_COLOR_GREEN_ACCENT` | `#A0FC50` | Barra título verde, precio, botón Iniciar, litros |
| `UI_COLOR_GREEN_LIGHT` | `#C8FCB8` | Título verde, descripción QR, textos secundarios |
| `UI_COLOR_ORANGE_LIGHT` | `#F8DCB0` | Título naranja (Coloca, Wi-Fi, errores no-pago) |
| `UI_COLOR_ORANGE_SECONDARY` | `#FF8028` | Barra naranja, botón Parar |
| `UI_COLOR_ORANGE_DARK` | `#282008` | Fondo cards Coloca / errores |

### 2.3 Layout título + barra

Helper `add_title_block(panel, title, style, layout)` en `display_ui.cpp`.

| Pantalla | `UiTitleStyle` | `UiTitleLayout` | Barra |
|----------|----------------|-----------------|-------|
| Pago QR | `GREEN` | `CENTER_V` | 44×4 px, `#A0FC50` |
| Coloca termo | `ORANGE` | `CENTER_V` | 44×4 px, `#FF8028` |
| Wi-Fi desconectado | `ORANGE` | `CENTER_V` | idem |
| Cargar termo | `GREEN` | `TOP` (y≈42) | idem verde |
| Finish | `GREEN` | `CENTER_V` | idem verde |
| Error pago | `GREEN` | `CENTER_V` | dinámico vía `show_error()` |
| Error Wi-Fi/agua/bandeja | `ORANGE` | `CENTER_V` | dinámico vía `show_error()` |

El bloque título+barra se agrupa en un contenedor transparente para centrar o anclar arriba como unidad.

### 2.4 Botones CTA

- Texto: `UI_FONT_CTA` → `ui_font_montserrat_bold_32` (32 px bold), color `#001000`
- Ícono flecha: `UI_FONT_CTA_ICON` (`lv_font_montserrat_32`) + `LV_SYMBOL_RIGHT` — la fuente bold no incluye símbolos LVGL
- Ícono Parar: círculo con borde + cuadrado interior (`set_cta_stop_icon()`), reemplaza el carácter Unicode `■`

### 2.5 Cards inferiores

- Fuente 20 px (`UI_FONT_CARD`)
- Altura card aumentada a **88 px** (antes 72) para acomodar texto multilínea en QR

### 2.6 Fuente bold CTA (custom)

LVGL 8 no incluye Montserrat bold. Los textos **Iniciar** / **Parar** usan una fuente exportada con `lv_font_conv`.

| Archivo | Rol |
|---------|-----|
| [`mate_point_v0-4/ui/fonts/Montserrat-Bold.ttf`](mate_point_v0-4/ui/fonts/Montserrat-Bold.ttf) | Fuente base (OFL, Google Fonts) |
| [`mate_point_v0-4/ui/fonts/lv_font_montserrat_bold_32.c`](mate_point_v0-4/ui/fonts/lv_font_montserrat_bold_32.c) | Bitmap LVGL 32 px, subset `IniciarParar` (~7 KB) |
| [`mate_point_v0-4/ui_assets_bundle.c`](mate_point_v0-4/ui_assets_bundle.c) | `#include` de la fuente + `LV_LVGL_H_INCLUDE_SIMPLE` (include Arduino) |
| [`mate_point_v0-4/ui/ui_theme.h`](mate_point_v0-4/ui/ui_theme.h) | `UI_FONT_CTA` → `&ui_font_montserrat_bold_32` |

**Regenerar** (desde `mate_point_v0-4/ui/fonts/`):

```bash
npx lv_font_conv \
  --font Montserrat-Bold.ttf \
  --size 32 --bpp 4 --format lvgl \
  --no-compress --no-prefilter \
  --lv-font-name ui_font_montserrat_bold_32 \
  --symbols "IniciarParar" \
  -o lv_font_montserrat_bold_32.c
```

**Ajustes manuales** tras regenerar (el exportador no los deja correctos para Arduino/LVGL 8):

| Campo en `ui_font_montserrat_bold_32.c` | Valor | Motivo |
|----------------------------------------|-------|--------|
| `bitmap_format` (en `font_dsc`) | `0` | Obligatorio `--no-compress`; si es `1`, el texto **no se ve** |
| `line_height` | `35` | Alinear con `lv_font_montserrat_32` |
| `base_line` | `6` | Centrado vertical en botón CTA |
| `.fallback` | `&lv_font_montserrat_32` | Respaldo si falta un glifo |

El include de la fuente lo resuelve `ui_assets_bundle.c` (`LV_LVGL_H_INCLUDE_SIMPLE` → `#include "lvgl.h"`). No usar `lvgl/lvgl.h` (falla en Arduino IDE).

---

## 3. Cambios por punto (checklist implementación)

| # | Tema | Estado | Implementación |
|---|------|--------|----------------|
| 1 | Botones 32 px bold | ✅ | `UI_FONT_CTA` → `ui_font_montserrat_bold_32` en `add_cta_button()` — validado hardware |
| 2 | Fondo QR izq. `#001808` / der. `#001000` | ✅ | `build_qr()` → `UI_COLOR_GREEN_DARK` / `GREEN_DARKEST` |
| 3 | Descripción QR 36 px bold | ✅ | `qr_lbl_desc` → `UI_FONT_PRODUCT_DESC` |
| 4 | Precio QR 32 px bold | ✅ | `qr_lbl_price` → `UI_FONT_PRODUCT_PRICE`, alineado bajo descripción |
| 5 | Título QR 36 px, centrado | ✅ | `add_title_block(..., GREEN, CENTER_V)` |
| 6 | Card QR 20 px | ✅ | `add_bottom_card()` + `UI_FONT_CARD` |
| 7 | Coloca: título naranja, centrado | ✅ | `ORANGE` + `CENTER_V` |
| 8 | Card Coloca | ✅ | Sin cambio (OK en spec) |
| 9 | Wi-Fi: título naranja, centrado | ✅ | `build_connectivity()` + `show_error()` estilo naranja |
| 10 | Card Wi-Fi | ✅ | Sin cambio (OK en spec) |
| 11 | Cargar: título 36 px arriba | ✅ | `GREEN` + `TOP` |
| 12 | Botón Parar `#FF8028` | ✅ | Token `UI_COLOR_ORANGE_SECONDARY` |
| 13 | Ícono stop | ✅ | `set_cta_stop_icon()` en `set_cargar_mode(true)` |
| 14 | Finish: título centrado 36 px | ✅ | `GREEN` + `CENTER_V` |

---

## 4. Archivos modificados

| Archivo | Cambio |
|---------|--------|
| [`mate_point_v0-4/ui/ui_theme.h`](mate_point_v0-4/ui/ui_theme.h) | Tokens fuente, `UI_FONT_CTA` bold, `UI_FONT_CTA_ICON`, enums layout |
| [`mate_point_v0-4/display_ui.cpp`](mate_point_v0-4/display_ui.cpp) | Helpers refactorizados; pantallas `build_*()`; ícono stop; flecha con `UI_FONT_CTA_ICON` |
| [`mate_point_v0-4/ui_assets_bundle.c`](mate_point_v0-4/ui_assets_bundle.c) | Bundle fuente bold + `LV_LVGL_H_INCLUDE_SIMPLE` |
| [`mate_point_v0-4/ui/fonts/lv_font_montserrat_bold_32.c`](mate_point_v0-4/ui/fonts/lv_font_montserrat_bold_32.c) | Fuente CTA bold 32 px (subset) |
| [`mate_point_v0-4/README.md`](mate_point_v0-4/README.md) | Tipografía, regeneración fuente, troubleshooting |
| `~/Documents/Arduino/libraries/lv_conf.h` | `LV_FONT_MONTSERRAT_20/32/36` → `1` (fuera del repo) |

### Helpers nuevos / refactorizados (`display_ui.cpp`)

| Función | Rol |
|---------|-----|
| `apply_title_style()` | Aplica paleta verde u naranja a título + barra |
| `add_title_block()` | Bloque título+barra parametrizado |
| `get_title_bar()` | Obtiene barra desde label título (pantalla error) |
| `set_cta_arrow_icon()` | Flecha derecha en botón |
| `set_cta_stop_icon()` | Ícono stop (círculo + cuadrado) |
| `add_cta_button()` | CTA con fuente bold 32 px e ícono en contenedor |
| `add_bottom_card()` | Card 88 px, fuente 20 px |

---

## 5. Configuración LVGL requerida

En `~/Documents/Arduino/libraries/lv_conf.h` (no versionado en repo):

```c
#define LV_FONT_MONTSERRAT_20 1
#define LV_FONT_MONTSERRAT_32 1
#define LV_FONT_MONTSERRAT_36 1
```

Mantener habilitados los ya usados: `_12`, `_14`, `_16`, `_26`, `_44`.

Tras modificar `lv_conf.h` → **recompilar completo** en Arduino IDE.

---

## 6. Pendiente / fuera de esta iteración

| Tema | Notas |
|------|-------|
| **Bold títulos / precio / desc. QR** | Exportar Montserrat Bold 36 px y 32 px; sustituir `UI_FONT_TITLE`, `UI_FONT_PRODUCT_DESC`, `UI_FONT_PRODUCT_PRICE` |
| **Validación hardware** | Screenshot diff vs `UI/figma/pantallas/` en Waveshare con rotación 180° |
| **Test1 banco Nobana** | E2E formal v0-4 (equivalente v0-3-4) |
| **Precio/descripción backend** | Placeholder `$500` / `"Recarga de 1 litro"` en `config.h` hasta extender API |
| **Ajuste fino posiciones** | Si tras captura hardware el centrado vertical difiere ±pocos px, tunear offsets en `add_title_block()` |

---

## 7. Validación sugerida (hardware)

- [x] Standby: botón **Iniciar** legible a 32 px **bold**
- [ ] QR: paneles con tonos distintos izq./der.; título centrado; card Mercado Pago 20 px
- [ ] Coloca / Wi-Fi: título beige `#F8DCB0`, barra naranja, centrado
- [x] Cargar idle: título arriba; botón Iniciar 32 px **bold**
- [x] Cargar dispensing: botón naranja + ícono stop; texto **Parar** bold
- [ ] Finish: **LISTO EL MATE** centrado
- [ ] Área táctil botones sin regresión
- [ ] Regresión E2E flujo completo

---

## 8. Referencias

- Plan UI v0-4: [`PLAN-MATE-POINT-v0-4-UI.md`](PLAN-MATE-POINT-v0-4-UI.md)
- README firmware: [`mate_point_v0-4/README.md`](mate_point_v0-4/README.md)
- Assets: [`UI/ASSETS.md`](../UI/ASSETS.md)
- Figma: [`UI/figma/pantallas/`](../UI/figma/pantallas/)
- Wireframe: [Mate Point — Figma](https://www.figma.com/design/TmPeOvLv6q247UMZsAD8fl/Mate-Point?node-id=0-1)
