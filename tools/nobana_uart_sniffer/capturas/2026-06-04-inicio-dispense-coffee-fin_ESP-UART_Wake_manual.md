# Captura YYYY-MM-DD — título corto

## Metadatos



## Objetivo

Analizar el comportamiento del ESP32 con el nobana con el firmware con wake manual.

## Resultado observable

- Se encendio el ESP32.
- Luego de 1-2seg segundos se prende ON  nobana.
- Se manda el comando W.
- Luego de unos segundos con la confirmacion se manda el comando "R".
- Se escuchan 3 chimes , y luego inicia el dispensado.
- Se escucha un chime al momento "cooldown" y corte del dispensado.
- Un ultimo chime luego de unos segundos de haber al "lock". 

## Log UART

11:39:44.501 -> 
11:39:44.501 -> [wake] INICIO — escuchando bus antes de F8
11:39:44.501 -> [wake] fase=RX_BOOT (5000 ms)
11:39:44.501 -> [wake] Encender Nobana si aun OFF; ver NOB->ESP en log
11:39:44.534 -> [133471] NOB->ESP (raw) len=2 HEX: 00 00
11:39:49.481 -> [wake] fase=POST_F8 — enviando F8, escucha 3000 ms
11:39:49.481 -> [138437] ESP->NOB len=1 HEX: F8
11:39:52.661 -> 
11:39:52.661 -> [wake] LISTO — bytes_rx=2 tramas_68_ok=0
11:39:52.661 -> [wake] Sin trama 0x68 valida tras F8.
11:39:52.661 -> [wake] Repetir W con Nobana ON o revisar cableado/baud.
11:39:52.661 -> [wake] R = replay 21 -> 23 -> E2 (sin repetir F8)
11:39:52.661 -> 
11:39:59.337 -> 
11:39:59.337 -> [replay] INICIO — Coffee 180 ml (captura ref.)
11:39:59.337 -> [replay] 21 -> 23 -> E2 -> pre-stop -> 22 -> lock
11:39:59.337 -> [replay] fase=START_21 seq=1
11:39:59.402 -> [148367] telem replay=START_21 T_obj=85 T_act=56 b2=0x12 fase=0x14 (activo/precalent) progress=0
11:40:03.350 -> [replay] fase=IDLE_23 seq=2
11:40:06.347 -> [replay] fase=DISPENSE seq=3
11:40:10.210 -> [159159] telem replay=DISPENSE T_obj=85 T_act=56 b2=0x12 fase=0x14 (activo/precalent) progress=6
11:40:10.505 -> [159459] telem replay=DISPENSE T_obj=85 T_act=57 b2=0x12 fase=0x14 (activo/precalent) progress=6
11:40:11.003 -> [159959] telem replay=DISPENSE T_obj=85 T_act=58 b2=0x12 fase=0x14 (activo/precalent) progress=6
11:40:11.198 -> [160159] telem replay=DISPENSE T_obj=85 T_act=58 b2=0x12 fase=0x14 (activo/precalent) progress=13
11:40:11.393 -> [160359] telem replay=DISPENSE T_obj=85 T_act=59 b2=0x12 fase=0x14 (activo/precalent) progress=13
11:40:11.922 -> [160859] telem replay=DISPENSE T_obj=85 T_act=60 b2=0x12 fase=0x14 (activo/precalent) progress=13
11:40:12.221 -> [161159] telem replay=DISPENSE T_obj=85 T_act=61 b2=0x12 fase=0x14 (activo/precalent) progress=20
11:40:12.319 -> [161259] telem replay=DISPENSE T_obj=85 T_act=62 b2=0x12 fase=0x14 (activo/precalent) progress=20
11:40:12.517 -> [161461] telem replay=DISPENSE T_obj=85 T_act=63 b2=0x12 fase=0x14 (activo/precalent) progress=20
11:40:12.714 -> [161659] telem replay=DISPENSE T_obj=85 T_act=64 b2=0x12 fase=0x14 (activo/precalent) progress=20
11:40:12.813 -> [161759] telem replay=DISPENSE T_obj=85 T_act=65 b2=0x12 fase=0x14 (activo/precalent) progress=20
11:40:13.012 -> [161959] telem replay=DISPENSE T_obj=85 T_act=66 b2=0x12 fase=0x14 (activo/precalent) progress=20
11:40:13.211 -> [162159] telem replay=DISPENSE T_obj=85 T_act=67 b2=0x12 fase=0x14 (activo/precalent) progress=27
11:40:13.404 -> [162359] telem replay=DISPENSE T_obj=85 T_act=68 b2=0x12 fase=0x14 (activo/precalent) progress=27
11:40:13.603 -> [162559] telem replay=DISPENSE T_obj=85 T_act=69 b2=0x12 fase=0x14 (activo/precalent) progress=27
11:40:13.801 -> [162759] telem replay=DISPENSE T_obj=85 T_act=70 b2=0x12 fase=0x14 (activo/precalent) progress=27
11:40:13.901 -> [162859] telem replay=DISPENSE T_obj=85 T_act=71 b2=0x12 fase=0x14 (activo/precalent) progress=27
11:40:14.100 -> [163061] telem replay=DISPENSE T_obj=85 T_act=72 b2=0x12 fase=0x14 (activo/precalent) progress=27
11:40:14.197 -> [163159] telem replay=DISPENSE T_obj=85 T_act=72 b2=0x12 fase=0x14 (activo/precalent) progress=34
11:40:14.295 -> [163259] telem replay=DISPENSE T_obj=85 T_act=73 b2=0x12 fase=0x14 (activo/precalent) progress=34
11:40:14.392 -> [163359] telem replay=DISPENSE T_obj=85 T_act=74 b2=0x12 fase=0x14 (activo/precalent) progress=34
11:40:14.725 -> [163659] telem replay=DISPENSE T_obj=85 T_act=75 b2=0x12 fase=0x14 (activo/precalent) progress=34
11:40:15.138 -> [164059] telem replay=DISPENSE T_obj=85 T_act=76 b2=0x12 fase=0x14 (activo/precalent) progress=34
11:40:15.204 -> [164159] telem replay=DISPENSE T_obj=85 T_act=76 b2=0x12 fase=0x14 (activo/precalent) progress=42
11:40:15.302 -> [164261] telem replay=DISPENSE T_obj=85 T_act=77 b2=0x12 fase=0x14 (activo/precalent) progress=42
11:40:15.598 -> [164559] telem replay=DISPENSE T_obj=85 T_act=78 b2=0x12 fase=0x14 (activo/precalent) progress=42
11:40:15.797 -> [164759] telem replay=DISPENSE T_obj=85 T_act=79 b2=0x12 fase=0x14 (activo/precalent) progress=42
11:40:16.195 -> [165159] telem replay=DISPENSE T_obj=85 T_act=80 b2=0x12 fase=0x14 (activo/precalent) progress=49
11:40:16.623 -> [165559] telem replay=DISPENSE T_obj=85 T_act=81 b2=0x12 fase=0x14 (activo/precalent) progress=49
11:40:17.018 -> [165959] telem replay=DISPENSE T_obj=85 T_act=82 b2=0x12 fase=0x14 (activo/precalent) progress=49
11:40:17.218 -> [166159] telem replay=DISPENSE T_obj=85 T_act=82 b2=0x12 fase=0x14 (activo/precalent) progress=56
11:40:17.617 -> [166559] telem replay=DISPENSE T_obj=85 T_act=83 b2=0x12 fase=0x14 (activo/precalent) progress=56
11:40:18.209 -> [167159] telem replay=DISPENSE T_obj=85 T_act=83 b2=0x12 fase=0x14 (activo/precalent) progress=63
11:40:18.902 -> [167859] telem replay=DISPENSE T_obj=85 T_act=84 b2=0x12 fase=0x14 (activo/precalent) progress=63
11:40:19.197 -> [168161] telem replay=DISPENSE T_obj=85 T_act=84 b2=0x12 fase=0x14 (activo/precalent) progress=71
11:40:20.219 -> [169159] telem replay=DISPENSE T_obj=85 T_act=85 b2=0x12 fase=0x14 (activo/precalent) progress=78
11:40:21.217 -> [170161] telem replay=DISPENSE T_obj=85 T_act=85 b2=0x12 fase=0x14 (activo/precalent) progress=86
11:40:22.207 -> [171159] telem replay=DISPENSE T_obj=85 T_act=85 b2=0x12 fase=0x14 (activo/precalent) progress=93
11:40:23.195 -> [172159] telem replay=DISPENSE T_obj=85 T_act=84 b2=0x12 fase=0x14 (activo/precalent) progress=101
11:40:24.192 -> [173159] telem replay=DISPENSE T_obj=85 T_act=84 b2=0x12 fase=0x14 (activo/precalent) progress=108
11:40:24.825 -> [173759] telem replay=DISPENSE T_obj=85 T_act=83 b2=0x12 fase=0x14 (activo/precalent) progress=108
11:40:25.221 -> [174159] telem replay=DISPENSE T_obj=85 T_act=83 b2=0x12 fase=0x14 (activo/precalent) progress=116
11:40:26.212 -> [175159] telem replay=DISPENSE T_obj=85 T_act=83 b2=0x12 fase=0x14 (activo/precalent) progress=124
11:40:27.202 -> [176159] telem replay=DISPENSE T_obj=85 T_act=83 b2=0x12 fase=0x14 (activo/precalent) progress=131
11:40:27.410 -> [176361] telem replay=DISPENSE T_obj=85 T_act=82 b2=0x12 fase=0x14 (activo/precalent) progress=131
11:40:28.218 -> [177161] telem replay=DISPENSE T_obj=85 T_act=82 b2=0x12 fase=0x14 (activo/precalent) progress=139
11:40:29.108 -> [178061] telem replay=DISPENSE T_obj=85 T_act=82 b2=0x12 fase=0x14 (activo/precalent) progress=146
11:40:29.999 -> [178959] telem replay=DISPENSE T_obj=85 T_act=83 b2=0x12 fase=0x14 (activo/precalent) progress=146
11:40:30.096 -> [179061] telem replay=DISPENSE T_obj=85 T_act=83 b2=0x12 fase=0x14 (activo/precalent) progress=153
11:40:30.330 -> [replay] T_DISPENSE -> PRE_STOP
11:40:30.363 -> [replay] fase=PRE_STOP seq=3
11:40:31.098 -> [180059] telem replay=PRE_STOP T_obj=85 T_act=83 b2=0x12 fase=0x14 (activo/precalent) progress=161
11:40:31.293 -> [180259] telem replay=PRE_STOP T_obj=85 T_act=84 b2=0x12 fase=0x14 (activo/precalent) progress=161
11:40:32.122 -> [181059] telem replay=PRE_STOP T_obj=85 T_act=84 b2=0x12 fase=0x14 (activo/precalent) progress=168
11:40:32.420 -> [181361] telem replay=PRE_STOP T_obj=85 T_act=83 b2=0x12 fase=0x14 (activo/precalent) progress=168
11:40:32.816 -> [181761] telem replay=PRE_STOP T_obj=85 T_act=82 b2=0x12 fase=0x14 (activo/precalent) progress=168
11:40:33.112 -> [182059] telem replay=PRE_STOP T_obj=85 T_act=82 b2=0x12 fase=0x14 (activo/precalent) progress=175
11:40:33.212 -> [182161] telem replay=PRE_STOP T_obj=85 T_act=81 b2=0x12 fase=0x14 (activo/precalent) progress=175
11:40:33.310 -> [182259] telem replay=PRE_STOP T_obj=85 T_act=80 b2=0x12 fase=0x14 (activo/precalent) progress=175
11:40:33.612 -> [182561] telem replay=PRE_STOP T_obj=85 T_act=79 b2=0x12 fase=0x14 (activo/precalent) progress=175
11:40:33.811 -> [182759] telem replay=PRE_STOP T_obj=85 T_act=78 b2=0x12 fase=0x14 (activo/precalent) progress=175
11:40:33.911 -> [182859] telem replay=PRE_STOP T_obj=85 T_act=77 b2=0x12 fase=0x14 (activo/precalent) progress=175
11:40:34.108 -> [183059] telem replay=PRE_STOP T_obj=85 T_act=77 b2=0x12 fase=0x14 (activo/precalent) progress=183
11:40:34.205 -> [183159] telem replay=PRE_STOP T_obj=85 T_act=76 b2=0x12 fase=0x14 (activo/precalent) progress=183
11:40:34.238 -> [replay] fase=STOP_22_04 seq=3
11:40:34.238 -> [replay] fase=CLOSE_22_00 seq=3
11:40:34.409 -> [183359] telem replay=CLOSE_22_00 T_obj=85 T_act=74 b2=0x11 fase=0x14 (activo/precalent) progress=0
11:40:34.609 -> [183559] telem replay=CLOSE_22_00 T_obj=85 T_act=73 b2=0x11 fase=0x14 (activo/precalent) progress=0
11:40:34.708 -> [183661] telem replay=CLOSE_22_00 T_obj=85 T_act=72 b2=0x11 fase=0x14 (activo/precalent) progress=0
11:40:34.805 -> [183759] telem replay=CLOSE_22_00 T_obj=85 T_act=71 b2=0x11 fase=0x14 (activo/precalent) progress=0
11:40:35.001 -> [183959] telem replay=CLOSE_22_00 T_obj=85 T_act=70 b2=0x11 fase=0x14 (activo/precalent) progress=0
11:40:35.100 -> [184061] telem replay=CLOSE_22_00 T_obj=85 T_act=69 b2=0x11 fase=0x14 (activo/precalent) progress=0
11:40:35.199 -> [184159] telem replay=CLOSE_22_00 T_obj=85 T_act=67 b2=0x11 fase=0x14 (activo/precalent) progress=0
11:40:35.298 -> [184259] telem replay=CLOSE_22_00 T_obj=85 T_act=66 b2=0x11 fase=0x14 (activo/precalent) progress=0
11:40:35.494 -> [184459] telem replay=CLOSE_22_00 T_obj=85 T_act=65 b2=0x11 fase=0x14 (activo/precalent) progress=0
11:40:35.627 -> [184559] telem replay=CLOSE_22_00 T_obj=85 T_act=64 b2=0x11 fase=0x14 (activo/precalent) progress=0
11:40:35.723 -> [184659] telem replay=CLOSE_22_00 T_obj=85 T_act=63 b2=0x11 fase=0x14 (activo/precalent) progress=0
11:40:35.821 -> [184759] telem replay=CLOSE_22_00 T_obj=85 T_act=61 b2=0x11 fase=0x14 (activo/precalent) progress=0
11:40:36.019 -> [184959] telem replay=CLOSE_22_00 T_obj=85 T_act=60 b2=0x11 fase=0x14 (activo/precalent) progress=0
11:40:36.118 -> [185059] telem replay=CLOSE_22_00 T_obj=85 T_act=59 b2=0x11 fase=0x14 (activo/precalent) progress=0
11:40:36.218 -> [185159] telem replay=CLOSE_22_00 T_obj=85 T_act=58 b2=0x11 fase=0x14 (activo/precalent) progress=0
11:40:36.253 -> [replay] NOB b2=0x11 -> COOLDOWN
11:40:36.253 -> [replay] fase=COOLDOWN_22 seq=4
11:40:36.417 -> [185359] telem replay=COOLDOWN_22 T_obj=85 T_act=57 b2=0x11 fase=0x14 (activo/precalent) progress=0
11:40:36.617 -> [185561] telem replay=COOLDOWN_22 T_obj=85 T_act=56 b2=0x11 fase=0x14 (activo/precalent) progress=0
11:40:36.917 -> [185859] telem replay=COOLDOWN_22 T_obj=85 T_act=55 b2=0x11 fase=0x14 (activo/precalent) progress=0
11:40:37.616 -> [186559] telem replay=COOLDOWN_22 T_obj=85 T_act=54 b2=0x11 fase=0x14 (activo/precalent) progress=0
11:40:39.797 -> [188759] telem replay=COOLDOWN_22 T_obj=85 T_act=55 b2=0x11 fase=0x14 (activo/precalent) progress=0
11:40:41.817 -> [190759] telem replay=COOLDOWN_22 T_obj=85 T_act=56 b2=0x11 fase=0x14 (activo/precalent) progress=0
11:40:44.123 -> [193059] telem replay=COOLDOWN_22 T_obj=85 T_act=56 b2=0x12 fase=0x14 (activo/precalent) progress=0
11:40:44.615 -> [193559] telem replay=COOLDOWN_22 T_obj=85 T_act=57 b2=0x12 fase=0x14 (activo/precalent) progress=0
11:40:45.900 -> [194859] telem replay=COOLDOWN_22 T_obj=85 T_act=58 b2=0x12 fase=0x14 (activo/precalent) progress=0
11:40:46.922 -> [195859] telem replay=COOLDOWN_22 T_obj=85 T_act=59 b2=0x12 fase=0x14 (activo/precalent) progress=0
11:40:49.513 -> [198459] telem replay=COOLDOWN_22 T_obj=85 T_act=58 b2=0x12 fase=0x14 (activo/precalent) progress=0
11:40:51.233 -> [replay] fase=LOCK_23 seq=5
11:40:54.237 -> [203189] telem replay=LOCK_23 T_obj=85 T_act=58 b2=0x12 fase=0x14 (activo/precalent) progress=0
11:40:54.237 -> [replay] FIN (ciclo_completo)


## Secuencia inferida (opcional)

| Paso | Dir. | Trama / evento |
|------|------|----------------|
| 1 | | |
| 2 | | |

## Notas / anomalías

-

## Conclusión preliminar

**POC ESP32 maestro validado** (2026-06-04). Procedimiento **`W` → `R`**: ciclo Coffee 180 ml timer completo (`FIN ciclo_completo`), dispensado y cierre hidráulico OK.

Documentado en [`PROTOCOLO-UART-NOBANA.md`](../../../mate_point_firmware/PROTOCOLO-UART-NOBANA.md) §7.6–7.7 y [`PLAN-POC-NOBANA-UART.md`](../../../mate_point_firmware/PLAN-POC-NOBANA-UART.md) §8 (Etapa 1 cerrada).

## Referencias

- ARMOR ref.: [`2026-06-03-inicio-dispense-coffee-fin.md`](2026-06-03-inicio-dispense-coffee-fin.md)
- Otra corrida ESP: [`2026-06-04-inicio-dispense-coffee-fin_ESP-UART.md`](2026-06-04-inicio-dispense-coffee-fin_ESP-UART.md)


