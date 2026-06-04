# Capturas UART Nobana — fuente de verdad

Logs anotados de banco (sniffer, ARMOR, ESP POC) usados para definir
[`mate_point_firmware/PROTOCOLO-UART-NOBANA.md`](../../../mate_point_firmware/PROTOCOLO-UART-NOBANA.md)
y la secuencia de [`mate_point_UART_v0-1`](../../../mate_point_firmware/mate_point_UART_v0-1/).

**Regla:** no actualizar el protocolo ni el firmware POC sin al menos una captura aquí que lo respalde.

---

## Convención de nombres

```
YYYY-MM-DD-<herramienta>-<bebida>-<preset>-<nota>.md
```

| Token | Ejemplos |
|-------|----------|
| `herramienta` | `sniffer`, `esp-poc`, `armor` |
| `bebida` | `coffee`, `milk`, `tea` |
| `preset` | `250ml`, `750ml`, `999ml`, `na` |
| `nota` | `completo`, `unlock`, `stop-manual`, `chime` |

Ejemplo: `2026-06-03-esp-poc-coffee-na-chime.md`

---

## Índice de capturas

| Fecha | Archivo | Herramienta | Resumen | Estado |
|-------|---------|-------------|---------|--------|
| — | — | — | *(añadir filas al subir capturas)* | — |

**Estado:** `borrador` · `revisada` · `incorporada` (ya reflejada en PROTOCOLO / firmware)

---

## Plantilla

Copiar [`_plantilla.md`](_plantilla.md) para cada nueva sesión de banco.

---

## Capturas prioritarias (pendientes)

- [ ] ARMOR + sniffer — Coffee 250 ml, ciclo completo (desbloquear → volumen → dispensar → fin → lock)
- [ ] ARMOR + sniffer — Coffee 750 ml (comparar si salta pasos previos)
- [ ] ARMOR + sniffer — stop manual durante dispensado
- [ ] ESP POC — replicar secuencia completa sin ARMOR (`D60000` o más)
- [ ] ESP POC — solo `C2` vs `01→02→C2` (chime / comportamiento)
