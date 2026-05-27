# Módulo Waveshare ESP32-S3-Touch-LCD-7B — Documentación técnica

**Proyecto:** Mate Point — Dispensador de agua caliente  
**OT:** OT-00268 — Etapa 3  
**Última actualización:** 2026-05-22

---

## 1. Especificaciones del módulo

### 1.1 Procesador y memoria

| Parámetro | Valor |
|-----------|-------|
| SoC | Espressif ESP32-S3 |
| Arquitectura | Xtensa LX7 dual-core, 32 bits |
| Frecuencia máxima | 240 MHz |
| SRAM interna | 512 KB |
| ROM interna | 384 KB |
| Flash onboard | 16 MB |
| PSRAM onboard | 8 MB (OPI) |

### 1.2 Conectividad

| Parámetro | Valor |
|-----------|-------|
| Wi-Fi | 2.4 GHz 802.11 b/g/n |
| Bluetooth | BLE 5, antena integrada |

### 1.3 Pantalla

| Parámetro | Valor |
|-----------|-------|
| Tamaño | 7 pulgadas |
| Resolución | **1024 × 600** px |
| Colores | 65K (RGB565) |
| Interfaz | RGB paralelo |
| Panel | IPS |
| Ángulo de visión | 170° |
| Brillo | 235 cd/m² |
| Táctil | Capacitivo, 5 puntos |
| Superficie táctil | Vidrio templado |
| Controlador táctil | I2C, con soporte interrupt |

### 1.4 Interfaces periféricas

| Interfaz | Conector | Nivel lógico | Pines expuestos | Notas |
|----------|----------|--------------|-----------------|-------|
| UART2 | PH2.0 | **3.3 V** | 3V3, GND, RX, TX | UART TTL directo — uso general |
| I2C | PH2.0 | 3.3 V | 3V3, GND, SDA, SCL | GPIO8/GPIO9, compartido con táctil GT911 |
| GPIO | PH2.0 | 3.3 V | 3V3, GND, GP6, ? | Verificar con esquemático |
| CAN | PH2.0 | diferencial | L, H | Transceptor integrado |
| RS-485 | PH2.0 | diferencial | A, B | Transceptor integrado |
| POWER | PH2.0 | — | 5V, GND | Alimentación externa |
| UART1 | USB Type-C | — | — | Programación / debug |
| USB | USB Type-C | — | — | Alimentación / datos |
| UART2_UART1 | Header lateral | 3.3 V | RX, TX (UART1 y UART2) | Header de depuración, verificar pinout |
| TF Card | SPI / MMC | 3.3 V | CS → EXIO4 | Micro SD |

> **Corrección respecto a documentación previa:** el módulo expone un puerto **UART2 TTL a 3.3 V** en conector PH2.0, no documentado en la wiki oficial al momento de redacción. Esto fue confirmado visualmente en el silkscreen del PCB.

### 1.5 Parámetros eléctricos

| Parámetro | Valor |
|-----------|-------|
| Alimentación | USB Type-C 5 V |
| Consumo típico | 350 mA |
| Temperatura de operación | 0 °C – 65 °C |
| Dimensiones (con táctil) | 192.96 × 110.76 mm |
| Dimensiones (sin táctil) | 165.72 × 97.60 mm |

---

## 2. Pinout y recursos del sistema

### 2.4 Direcciones I2C reservadas

No usar dispositivos I2C con las siguientes direcciones (ya ocupadas por el módulo):

| Dirección | Dispositivo |
|-----------|-------------|
| 0x24 | CH422G (IO expander) |
| 0x14 (o 0x5D) | GT911 (controlador táctil) |

### 2.5 GPIO disponibles (libres)

Los siguientes pines están expuestos en conectores PH2.0 para uso en la aplicación:

| Conector | Interfaz | Pines expuestos | Observación |
|----------|----------|-----------------|-------------|
| PH2.0 UART2 | UART TTL 3.3 V | 3V3, GND, RX, TX | Asignado a comunicación con PCB Nobana (via TXS0108E) |
| PH2.0 I2C | I2C 3.3 V | 3V3, GND, SDA, SCL | GPIO8/GPIO9 — compartido con GT911. Evitar dir. 0x14, 0x5D, 0x24 |
| PH2.0 GPIO | GPIO 3.3 V | 3V3, GND, **GP6** | **Un único GPIO disponible** — confirmado en inspección física |
| PH2.0 CAN | CAN diferencial | L, H | Transceptor integrado |
| PH2.0 RS-485 | RS-485 diferencial | A, B | Transceptor integrado |

> **Restricción crítica:** el conector GPIO expone únicamente **GP6** como pin de uso general (3 pines totales: 3V3, GND, GP6). No hay otros GPIO libres en el módulo. Todo sensor digital adicional debe conectarse por **I2C** o mediante un **expansor I2C** (PCF8574A).

---

## 3. Entorno de desarrollo

### 3.1 Frameworks soportados

| Framework | Estado |
|-----------|--------|
| Arduino (ESP32 board package ≥ 3.0.0) | Soportado |
| ESP-IDF v5.x | Soportado |
| PlatformIO | Soportado (target `esp32-s3-devkitc-1`) |

### 3.2 Librería gráfica — LVGL

- **Versión recomendada:** LVGL v8.3.x (Arduino) o LVGL v9.x (ESP-IDF)
- LVGL v8 corre a ~17 FPS promedio con PCLK ≈ 30 MHz en core único
- Para mejor rendimiento: habilitar **bounce buffer** y alojar buffers LVGL en SRAM interna (no PSRAM) para evitar flickering causado por contención de bus cuando Wi-Fi está activo

```c
// lv_conf.h — ajustes clave para 7B
#define LV_HOR_RES_MAX   1024
#define LV_VER_RES_MAX    600
#define LV_COLOR_DEPTH     16   // RGB565
#define LV_MEM_SIZE     (128 * 1024U)  // 128 KB en SRAM interna
```

### 3.3 Dependencias Arduino

```
ESP32_Display_Panel   (Waveshare / ESP_Panel_Conf.h)
ESP32_IO_Expander     (para CH422G)
lvgl                  (v8.3.x, con lv_conf.h configurado)
lv_lib_qrcode         (para QR dinámico)
VL53L0X               (pololu/vl53l0x-arduino — sensor de proximidad)
```

> Ver `arquitectura-hardware.md` §3 para la asignación de interfaces y sensores externos del sistema Mate Point.

---

## 4. Consideraciones de performance

| Tema | Recomendación |
|------|--------------|
| Flickering con Wi-Fi | Usar **bounce buffer** + buffers LVGL en SRAM interna (no PSRAM) |
| FPS en LVGL benchmark | ~17 FPS promedio con PCLK 30 MHz; suficiente para UI de kiosco |
| Tamaño del QR widget | 320×320 px cabe cómodo en 1024×600; usar tamaño fijo para QR denso (qr_data EMVCo es largo) |
| Quiet zone | Agregar borde blanco de al menos 4 módulos (≈ 10 px) alrededor del QR |
| Countdown timer | Usar `lv_timer_create` en lugar de `delay()` para no bloquear LVGL |
| Actualización de QR | `lv_qrcode_update()` es síncrono; llamar desde task LVGL (no desde ISR) |
| Desconexión Wi-Fi | Implementar reconexión automática; mostrar indicador en statusbar |

---

## 5. Referencias

| Recurso | URL |
|---------|-----|
| Wiki oficial Waveshare | https://www.waveshare.com/wiki/ESP32-S3-Touch-LCD-7B |
| Docs Waveshare (pinout, ejemplos) | https://docs.waveshare.com/ESP32-S3-Touch-LCD-7B |
| LVGL — Widget QR code | https://docs.lvgl.io/8.3/libs/qrcode.html |
| LVGL — lv_lib_qrcode (GitHub) | https://github.com/lvgl/lv_lib_qrcode |
| ESP32 Display Panel lib | https://github.com/esp-arduino-libs/ESP32_Display_Panel |
| CH422G datasheet | https://files.waveshare.com/wiki/common/CH422DS1_EN.pdf |
| Integración Mercado Pago QR | `integracion-mercadopago-qr.md` |
| Arquitectura Mate Point | `arquitectura-mate-point.md` |
| Arquitectura de hardware | `arquitectura-hardware.md` |
| Dispensador Nobana (base hardware) | `dispensador-nobana.md` |

---

## 6. Historial

| Fecha | Cambio |
|-------|--------|
| 2026-05-22 | Documento inicial — especificaciones, diseño UI, estrategia QR |
| 2026-05-22 | Reorganización: secciones de UI/arquitectura movidas a archivos dedicados |
| 2026-05-26 | Referencia agregada al dispensador Nobana (`dispensador-nobana.md`) como base de hardware |
| 2026-05-27 | Corrección interfaces periféricas: agregado UART2 TTL 3.3V (PH2.0), GPIO (PH2.0), POWER (PH2.0), header UART2_UART1 — confirmado visualmente en PCB físico |
| 2026-05-27 | Confirmado: conector GPIO expone solo GP6 (3 pines: 3V3, GND, GP6). Agregada §3 con mapa de asignación de interfaces y sensores del sistema Mate Point |
| 2026-05-27 | Reorganización: §2 reducida a §2.4/2.5 (pinout detallado movido a wikis/esquemático). §3 (asignación de interfaces) movida a `arquitectura-hardware.md` |
