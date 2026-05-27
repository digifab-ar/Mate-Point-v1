# Dispensador Nobana — Base de hardware del prototipo

**Proyecto:** Mate Point — Dispensador de agua caliente  
**OT:** OT-00268 — Etapa 3  
**Última actualización:** 2026-05-26

---

## 1. Identificación del producto

| Parámetro | Valor |
|-----------|-------|
| Marca | Nobana |
| Modelo | Electric UV Hot & Cold Mini Dispenser |
| Referencia Alibaba | `1601560564989` |
| Función en el proyecto | Base de hardware del prototipo Mate Point |
| URL de producto | [Alibaba — Nobana Mini Dispenser](https://www.alibaba.com/product-detail/Nobana-Electric-UV-Hot-Cold-Mini_1601560564989.html) |

---

## 2. Descripción del hardware base

El dispensador Nobana es una unidad compacta de agua caliente/fría con purificación UV. En el contexto del proyecto **Mate Point** se utiliza únicamente en modo **agua caliente**.

El equipo incluye de fábrica:

- PCB de control principal (electrónica de dispensado)
- Panel de control original compuesto por:
  - Display de 7 segmentos formato **(888)** — no es pantalla LCD
  - Conjunto de botones físicos con funcionalidades predefinidas (temperatura, modo, etc.)
- Electrónica de calentamiento y dispensado
- Sistema de purificación UV integrado (no utilizado activamente en Mate Point)

## 3. Panel de control original

### 3.1 Display

- **Tipo:** 7 segmentos, 3 dígitos — formato **(888)**
- **NO es pantalla LCD** — no hay framebuffer ni protocolo de video
- Muestra valores numéricos: temperatura, temporizadores, códigos de estado
- Conectado al PCB principal mediante bus **UART**

### 3.2 Botones

El panel original cuenta con botones de función predefinida (número exacto a confirmar durante el relevamiento físico del equipo). Funciones típicas de este tipo de dispensador:

| Botón (típico) | Función probable |
|----------------|-----------------|
| HOT / CALIENTE | Dispensar agua caliente |
| COLD / FRÍO | Dispensar agua fría (no usado en Mate Point) |
| ▲ / ▼ | Ajuste de temperatura |
| POWER / STAND BY | Encendido / modo ahorro |
| UV | Activar purificación UV |

> **Nota:** El mapeo exacto de botones debe verificarse con el equipo físico. Los botones quedan **inhabilitados** en el prototipo — el Waveshare toma control total vía UART.

### 3.3 Conexión al PCB

El display (888) y los botones se comunican con el PCB principal del dispensador mediante **UART**. Este mismo bus es el punto de integración para el módulo Waveshare.

---

## 4. Relación con otros documentos

| Documento | Relación |
|-----------|----------|
| `arquitectura-hardware.md` | Arquitectura de integración Waveshare ↔ PCB Nobana: estrategia de conexión, level shifting, mapa de interfaces |
| `modulo-waveshare-esp32s3-touch-7b.md` | Especificaciones del módulo que se conecta al Nobana vía UART |
| `arquitectura-mate-point.md` | Máquina de estados del firmware; el estado `DISPENSING` se materializa enviando el comando UART al Nobana |
| `integracion-mercadopago-qr.md` | Flujo de pago; el evento de pago confirmado desencadena el comando UART de dispensado |
| `plan-de-implementacion.md` | Plan de fases; incluye el relevamiento UART del Nobana como prerrequisito de Fase 4 |

---

## 5. Historial

| Fecha | Cambio |
|-------|--------|
| 2026-05-26 | Documento inicial — base hardware, panel de control original, estrategia de integración UART |
| 2026-05-27 | §4.2 actualizado: interfaz definitiva UART2 (3.3V) + TXS0108E. §5 reescrito con plan de 3 fases (baudrate → sniffing bidireccional → mapeo de tramas). §6 actualizado con hardware confirmado |
| 2026-05-27 | Reorganización: §2.1 (rol en el prototipo), §4 (integración), §5 (protocolo UART), §6 (consideraciones) movidos a `arquitectura-hardware.md` y `plan-de-implementacion.md` |
