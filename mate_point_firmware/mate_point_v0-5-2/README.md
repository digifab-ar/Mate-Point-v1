# Mate Point firmware v0-5-2

**Pausa / reanudar Cargar termo** — fork de [`mate_point_v0-5-1`](../mate_point_v0-5-1/) (bandeja GPIO6 + error agua UART + UI Figma).

Plan: [`PLAN-MATE-POINT-v0-5-2.md`](../PLAN-MATE-POINT-v0-5-2.md)

**Estado:** implementado — UI pausa validada en hardware; Test1 banco formal pendiente

## Flujo UI

```
Iniciar → QR → pago → Coloca termo → Cargar termo (Iniciar)
    → dispensando (Parar) ⇄ pausado (Finalizar; Continuar tras cooldown)
    → Listo el mate → Iniciar
```

## Pausa UART (Parar)

Igual corte corto que v0-4 (pre-stop **200 ms** + `22+00` sin `22+04`), pero con **cooldown de pausa 5 s** (no 15 s de fin de sesión):

```
Parar → corte ~200 ms → cierre ~2 s → cooldown pausa 5 s → Continuar visible
```

- Timer de decisión en UI: **20 s** (reinicia en cada pausa).
- Fin automático (presupuesto agotado): cooldown **15 s** (sin cambio).

## UI en pausa

Tras **Parar**, la pantalla pasa a `ESTADO: ESPERA` con dos fases visuales:

| Fase | Duración aprox. | Botones visibles |
|------|-----------------|------------------|
| Cooldown Nobana | ~7 s (cierre + 5 s) | Solo **Finalizar** |
| Listo para reanudar | Hasta timeout 20 s o acción usuario | **Continuar** + **Finalizar** |

- **Finalizar** permanece siempre en la misma posición (CTA inferior, `bottom_y = -48`).
- **Continuar** está oculto durante el cooldown y aparece arriba de Finalizar (`bottom_y = -136`) cuando Nobana termina el ciclo (`!busy` + `standby_active`).
- API: `display_ui_set_continuar_visible(bool)` — no se usa estado deshabilitado para Continuar.

## Cambios vs v0-5-1

| Tema | v0-5-1 | v0-5-2 |
|------|--------|--------|
| Parar | Abort; contrato UI seguía | **Pausa** — presupuesto congelado |
| UART pausa | — | `nobana_dispense_abort_pause()` + **5 s** cooldown |
| UI pausado (cooldown) | — | Solo **Finalizar** |
| UI pausado (listo) | — | **Continuar** + **Finalizar** |
| Reanudar | — | `standby_enable()` + `start(remaining_ms)` |
| Retiro termo (dispensando) | Auto-Parar | **Cierre sesión** → Finish |

## `config.h`

```c
#define PAUSE_DECISION_TIMEOUT_MS  20000
#define PAUSE_COOLDOWN_MS          5000
```

## Arduino IDE

Abrir: `mate_point_v0-5-2/mate_point_v0-5-2.ino` — misma config que v0-5-1 (partición 8 MB, PSRAM, UART2).
