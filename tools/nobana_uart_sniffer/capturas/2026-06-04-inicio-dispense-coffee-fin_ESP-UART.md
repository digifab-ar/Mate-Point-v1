# Captura YYYY-MM-DD — título corto

## Metadatos



## Objetivo

Analizar el comportamiento del ESP32 con el nobana.

## Resultado observable

- Se encendio el ESP32.
- Luego de 1-2seg segundos se prende ON  nobana.
- Luego de unos segundos,se manda el comando "R"
- Se escuchan 3 chimes , y luego inicia el dispensado.
- Se escucha un chime al momento "cooldown" y corte del dispensado.
- Un ultimo chime luego de unos segundos de haber al "lock". 

## Log UART

10:46:05.731 -> [5374] ESP->NOB len=1 HEX: F8
10:46:05.733 -> [6870] NOB->ESP (raw) len=1 HEX: 00
10:46:05.733 -> 
10:46:05.733 -> [wake] LISTO — bytes_rx=2 tramas_68_ok=0
10:46:05.733 -> [wake] Sin trama 0x68 valida tras F8.
10:46:05.733 -> [wake] Si Nobana estaba OFF: encenderlo, pulsar W, revisar log, luego R.
10:46:05.733 -> [wake] R = replay 21 -> 23 -> E2 (sin repetir F8)
10:46:05.733 -> 
10:46:05.733 -> 
10:46:05.733 -> [replay] INICIO — Coffee 180 ml (captura ref.)
10:46:05.733 -> [replay] 21 -> 23 -> E2 -> pre-stop -> 22 -> lock
10:46:05.733 -> [replay] fase=START_21 seq=1
10:46:05.733 -> [17246] telem replay=START_21 T_obj=85 T_act=20 b2=0x12 fase=0x14 (activo/precalent) progress=0
10:46:05.733 -> [replay] fase=IDLE_23 seq=2
10:46:05.734 -> [replay] fase=DISPENSE seq=3
10:46:05.734 -> [29140] telem replay=DISPENSE T_obj=85 T_act=21 b2=0x12 fase=0x14 (activo/precalent) progress=0
10:46:05.734 -> [29538] telem replay=DISPENSE T_obj=85 T_act=22 b2=0x12 fase=0x14 (activo/precalent) progress=0
10:46:05.734 -> [29738] telem replay=DISPENSE T_obj=85 T_act=23 b2=0x12 fase=0x14 (activo/precalent) progress=0
10:46:05.734 -> [29838] telem replay=DISPENSE T_obj=85 T_act=24 b2=0x12 fase=0x14 (activo/precalent) progress=0
10:46:05.734 -> [30040] telem replay=DISPENSE T_obj=85 T_act=25 b2=0x12 fase=0x14 (activo/precalent) progress=0
10:46:05.734 -> [30238] telem replay=DISPENSE T_obj=85 T_act=26 b2=0x12 fase=0x14 (activo/precalent) progress=0
10:46:05.734 -> [30538] telem replay=DISPENSE T_obj=85 T_act=27 b2=0x12 fase=0x14 (activo/precalent) progress=0
10:46:05.734 -> [30638] telem replay=DISPENSE T_obj=85 T_act=28 b2=0x12 fase=0x14 (activo/precalent) progress=0
10:46:05.734 -> [30940] telem replay=DISPENSE T_obj=85 T_act=29 b2=0x12 fase=0x14 (activo/precalent) progress=0
10:46:05.734 -> [31038] telem replay=DISPENSE T_obj=85 T_act=30 b2=0x12 fase=0x14 (activo/precalent) progress=0
10:46:05.734 -> [31138] telem replay=DISPENSE T_obj=85 T_act=31 b2=0x12 fase=0x14 (activo/precalent) progress=0
10:46:05.734 -> [31238] telem replay=DISPENSE T_obj=85 T_act=33 b2=0x12 fase=0x14 (activo/precalent) progress=8
10:46:05.734 -> [31438] telem replay=DISPENSE T_obj=85 T_act=35 b2=0x12 fase=0x14 (activo/precalent) progress=8
10:46:05.734 -> [31538] telem replay=DISPENSE T_obj=85 T_act=37 b2=0x12 fase=0x14 (activo/precalent) progress=8
10:46:05.734 -> [31638] telem replay=DISPENSE T_obj=85 T_act=40 b2=0x12 fase=0x14 (activo/precalent) progress=8
10:46:05.734 -> [31840] telem replay=DISPENSE T_obj=85 T_act=43 b2=0x12 fase=0x14 (activo/precalent) progress=8
10:46:05.734 -> [31938] telem replay=DISPENSE T_obj=85 T_act=46 b2=0x12 fase=0x14 (activo/precalent) progress=8
10:46:05.734 -> [32038] telem replay=DISPENSE T_obj=85 T_act=49 b2=0x12 fase=0x14 (activo/precalent) progress=8
10:46:05.734 -> [32138] telem replay=DISPENSE T_obj=85 T_act=53 b2=0x12 fase=0x14 (activo/precalent) progress=8
10:46:05.734 -> [32240] telem replay=DISPENSE T_obj=85 T_act=53 b2=0x12 fase=0x14 (activo/precalent) progress=17
10:46:05.734 -> [32338] telem replay=DISPENSE T_obj=85 T_act=55 b2=0x12 fase=0x14 (activo/precalent) progress=17
10:46:05.734 -> [32438] telem replay=DISPENSE T_obj=85 T_act=58 b2=0x12 fase=0x14 (activo/precalent) progress=17
10:46:05.734 -> [32538] telem replay=DISPENSE T_obj=85 T_act=61 b2=0x12 fase=0x14 (activo/precalent) progress=17
10:46:05.734 -> [32638] telem replay=DISPENSE T_obj=85 T_act=63 b2=0x12 fase=0x14 (activo/precalent) progress=17
10:46:05.734 -> [32838] telem replay=DISPENSE T_obj=85 T_act=66 b2=0x12 fase=0x14 (activo/precalent) progress=17
10:46:05.734 -> [32938] telem replay=DISPENSE T_obj=85 T_act=67 b2=0x12 fase=0x14 (activo/precalent) progress=17
10:46:05.734 -> [33038] telem replay=DISPENSE T_obj=85 T_act=69 b2=0x12 fase=0x14 (activo/precalent) progress=17
10:46:05.734 -> [33238] telem replay=DISPENSE T_obj=85 T_act=71 b2=0x12 fase=0x14 (activo/precalent) progress=24
10:46:05.734 -> [33338] telem replay=DISPENSE T_obj=85 T_act=72 b2=0x12 fase=0x14 (activo/precalent) progress=24
10:46:05.734 -> [33438] telem replay=DISPENSE T_obj=85 T_act=73 b2=0x12 fase=0x14 (activo/precalent) progress=24
10:46:05.734 -> [33538] telem replay=DISPENSE T_obj=85 T_act=74 b2=0x12 fase=0x14 (activo/precalent) progress=24
10:46:05.735 -> [33738] telem replay=DISPENSE T_obj=85 T_act=75 b2=0x12 fase=0x14 (activo/precalent) progress=24
10:46:05.735 -> [33838] telem replay=DISPENSE T_obj=85 T_act=76 b2=0x12 fase=0x14 (activo/precalent) progress=24
10:46:05.735 -> [34040] telem replay=DISPENSE T_obj=85 T_act=77 b2=0x12 fase=0x14 (activo/precalent) progress=24
10:46:05.735 -> [34238] telem replay=DISPENSE T_obj=85 T_act=77 b2=0x12 fase=0x14 (activo/precalent) progress=32
10:46:05.735 -> [34338] telem replay=DISPENSE T_obj=85 T_act=78 b2=0x12 fase=0x14 (activo/precalent) progress=32
10:46:05.735 -> [34838] telem replay=DISPENSE T_obj=85 T_act=79 b2=0x12 fase=0x14 (activo/precalent) progress=32
10:46:05.735 -> [35238] telem replay=DISPENSE T_obj=85 T_act=79 b2=0x12 fase=0x14 (activo/precalent) progress=39
10:46:05.735 -> [35638] telem replay=DISPENSE T_obj=85 T_act=80 b2=0x12 fase=0x14 (activo/precalent) progress=39
10:46:05.735 -> [36240] telem replay=DISPENSE T_obj=85 T_act=80 b2=0x12 fase=0x14 (activo/precalent) progress=46
10:46:05.735 -> [36538] telem replay=DISPENSE T_obj=85 T_act=81 b2=0x12 fase=0x14 (activo/precalent) progress=46
10:46:05.735 -> [37238] telem replay=DISPENSE T_obj=85 T_act=81 b2=0x12 fase=0x14 (activo/precalent) progress=54
10:46:05.735 -> [37438] telem replay=DISPENSE T_obj=85 T_act=82 b2=0x12 fase=0x14 (activo/precalent) progress=54
10:46:05.735 -> [38238] telem replay=DISPENSE T_obj=85 T_act=83 b2=0x12 fase=0x14 (activo/precalent) progress=61
10:46:05.735 -> [39238] telem replay=DISPENSE T_obj=85 T_act=83 b2=0x12 fase=0x14 (activo/precalent) progress=68
10:46:05.735 -> [39438] telem replay=DISPENSE T_obj=85 T_act=84 b2=0x12 fase=0x14 (activo/precalent) progress=68
10:46:05.735 -> [40238] telem replay=DISPENSE T_obj=85 T_act=84 b2=0x12 fase=0x14 (activo/precalent) progress=75
10:46:05.735 -> [41238] telem replay=DISPENSE T_obj=85 T_act=84 b2=0x12 fase=0x14 (activo/precalent) progress=83
10:46:05.735 -> [41738] telem replay=DISPENSE T_obj=85 T_act=85 b2=0x12 fase=0x14 (activo/precalent) progress=83
10:46:05.735 -> [42238] telem replay=DISPENSE T_obj=85 T_act=85 b2=0x12 fase=0x14 (activo/precalent) progress=90
10:46:05.735 -> [42638] telem replay=DISPENSE T_obj=85 T_act=84 b2=0x12 fase=0x14 (activo/precalent) progress=90
10:46:05.735 -> [43240] telem replay=DISPENSE T_obj=85 T_act=84 b2=0x12 fase=0x14 (activo/precalent) progress=98
10:46:05.735 -> [44238] telem replay=DISPENSE T_obj=85 T_act=84 b2=0x12 fase=0x14 (activo/precalent) progress=105
10:46:05.735 -> [45138] telem replay=DISPENSE T_obj=85 T_act=84 b2=0x12 fase=0x14 (activo/precalent) progress=113
10:46:07.982 -> [46138] telem replay=DISPENSE T_obj=85 T_act=84 b2=0x12 fase=0x14 (activo/precalent) progress=120
10:46:07.982 -> [47140] telem replay=DISPENSE T_obj=85 T_act=84 b2=0x12 fase=0x14 (activo/precalent) progress=128
10:46:07.982 -> [48138] telem replay=DISPENSE T_obj=85 T_act=84 b2=0x12 fase=0x14 (activo/precalent) progress=135
10:46:07.982 -> [replay] T_DISPENSE -> PRE_STOP
10:46:07.982 -> [replay] fase=PRE_STOP seq=3
10:46:07.982 -> [48338] telem replay=PRE_STOP T_obj=85 T_act=83 b2=0x12 fase=0x14 (activo/precalent) progress=135
10:46:08.002 -> [49138] telem replay=PRE_STOP T_obj=85 T_act=83 b2=0x12 fase=0x14 (activo/precalent) progress=142
10:46:08.002 -> [50140] telem replay=PRE_STOP T_obj=85 T_act=83 b2=0x12 fase=0x14 (activo/precalent) progress=150
10:46:08.002 -> [50738] telem replay=PRE_STOP T_obj=85 T_act=82 b2=0x12 fase=0x14 (activo/precalent) progress=150
10:46:08.095 -> [51040] telem replay=PRE_STOP T_obj=85 T_act=81 b2=0x12 fase=0x14 (activo/precalent) progress=150
10:46:08.195 -> [51138] telem replay=PRE_STOP T_obj=85 T_act=81 b2=0x12 fase=0x14 (activo/precalent) progress=157
10:46:08.294 -> [51238] telem replay=PRE_STOP T_obj=85 T_act=80 b2=0x12 fase=0x14 (activo/precalent) progress=157
10:46:08.489 -> [51440] telem replay=PRE_STOP T_obj=85 T_act=79 b2=0x12 fase=0x14 (activo/precalent) progress=157
10:46:08.605 -> [51538] telem replay=PRE_STOP T_obj=85 T_act=78 b2=0x12 fase=0x14 (activo/precalent) progress=157
10:46:08.885 -> [51838] telem replay=PRE_STOP T_obj=85 T_act=77 b2=0x12 fase=0x14 (activo/precalent) progress=157
10:46:08.988 -> [51938] telem replay=PRE_STOP T_obj=85 T_act=76 b2=0x12 fase=0x14 (activo/precalent) progress=157
10:46:09.087 -> [52038] telem replay=PRE_STOP T_obj=85 T_act=75 b2=0x12 fase=0x14 (activo/precalent) progress=157
10:46:09.118 -> [replay] fase=STOP_22_04 seq=3
10:46:09.118 -> [replay] fase=CLOSE_22_00 seq=3
10:46:09.296 -> [52238] telem replay=CLOSE_22_00 T_obj=85 T_act=74 b2=0x11 fase=0x14 (activo/precalent) progress=0
10:46:09.396 -> [52338] telem replay=CLOSE_22_00 T_obj=85 T_act=73 b2=0x11 fase=0x14 (activo/precalent) progress=0
10:46:09.496 -> [52438] telem replay=CLOSE_22_00 T_obj=85 T_act=72 b2=0x11 fase=0x14 (activo/precalent) progress=0
10:46:09.613 -> [52538] telem replay=CLOSE_22_00 T_obj=85 T_act=71 b2=0x11 fase=0x14 (activo/precalent) progress=0
10:46:09.680 -> [52638] telem replay=CLOSE_22_00 T_obj=85 T_act=70 b2=0x11 fase=0x14 (activo/precalent) progress=0
10:46:09.879 -> [52838] telem replay=CLOSE_22_00 T_obj=85 T_act=69 b2=0x11 fase=0x14 (activo/precalent) progress=0
10:46:09.978 -> [52938] telem replay=CLOSE_22_00 T_obj=85 T_act=68 b2=0x11 fase=0x14 (activo/precalent) progress=0
10:46:10.076 -> [53038] telem replay=CLOSE_22_00 T_obj=85 T_act=67 b2=0x11 fase=0x14 (activo/precalent) progress=0
10:46:10.278 -> [53238] telem replay=CLOSE_22_00 T_obj=85 T_act=66 b2=0x11 fase=0x14 (activo/precalent) progress=0
10:46:10.378 -> [53338] telem replay=CLOSE_22_00 T_obj=85 T_act=64 b2=0x11 fase=0x14 (activo/precalent) progress=0
10:46:10.483 -> [53438] telem replay=CLOSE_22_00 T_obj=85 T_act=63 b2=0x11 fase=0x14 (activo/precalent) progress=0
10:46:10.586 -> [53540] telem replay=CLOSE_22_00 T_obj=85 T_act=62 b2=0x11 fase=0x14 (activo/precalent) progress=0
10:46:10.837 -> [53738] telem replay=CLOSE_22_00 T_obj=85 T_act=61 b2=0x11 fase=0x14 (activo/precalent) progress=0
10:46:10.905 -> [53838] telem replay=CLOSE_22_00 T_obj=85 T_act=60 b2=0x11 fase=0x14 (activo/precalent) progress=0
10:46:10.969 -> [53938] telem replay=CLOSE_22_00 T_obj=85 T_act=59 b2=0x11 fase=0x14 (activo/precalent) progress=0
10:46:11.069 -> [54038] telem replay=CLOSE_22_00 T_obj=85 T_act=58 b2=0x11 fase=0x14 (activo/precalent) progress=0
10:46:11.136 -> [replay] NOB b2=0x11 -> COOLDOWN
10:46:11.136 -> [replay] fase=COOLDOWN_22 seq=4
10:46:11.270 -> [54238] telem replay=COOLDOWN_22 T_obj=85 T_act=57 b2=0x11 fase=0x14 (activo/precalent) progress=0
10:46:11.368 -> [54338] telem replay=COOLDOWN_22 T_obj=85 T_act=56 b2=0x11 fase=0x14 (activo/precalent) progress=0
10:46:11.695 -> [54638] telem replay=COOLDOWN_22 T_obj=85 T_act=55 b2=0x11 fase=0x14 (activo/precalent) progress=0
10:46:12.292 -> [55238] telem replay=COOLDOWN_22 T_obj=85 T_act=54 b2=0x11 fase=0x14 (activo/precalent) progress=0
10:46:13.191 -> [56140] telem replay=COOLDOWN_22 T_obj=85 T_act=55 b2=0x11 fase=0x14 (activo/precalent) progress=0
10:46:13.987 -> [56938] telem replay=COOLDOWN_22 T_obj=85 T_act=56 b2=0x11 fase=0x14 (activo/precalent) progress=0
10:46:15.076 -> [58038] telem replay=COOLDOWN_22 T_obj=85 T_act=57 b2=0x11 fase=0x14 (activo/precalent) progress=0
10:46:17.886 -> [60838] telem replay=COOLDOWN_22 T_obj=85 T_act=57 b2=0x12 fase=0x14 (activo/precalent) progress=0
10:46:18.294 -> [61238] telem replay=COOLDOWN_22 T_obj=85 T_act=58 b2=0x12 fase=0x14 (activo/precalent) progress=0
10:46:19.706 -> [62640] telem replay=COOLDOWN_22 T_obj=85 T_act=57 b2=0x12 fase=0x14 (activo/precalent) progress=0
10:46:25.879 -> [68838] telem replay=COOLDOWN_22 T_obj=85 T_act=58 b2=0x12 fase=0x14 (activo/precalent) progress=0
10:46:26.112 -> [replay] fase=LOCK_23 seq=5
10:46:27.078 -> [70038] telem replay=LOCK_23 T_obj=85 T_act=57 b2=0x12 fase=0x14 (activo/precalent) progress=0
10:46:29.113 -> [72068] telem replay=LOCK_23 T_obj=85 T_act=57 b2=0x12 fase=0x14 (activo/precalent) progress=0
10:46:29.113 -> [replay] FIN (ciclo_completo)


## Secuencia inferida (opcional)

| Paso | Dir. | Trama / evento |
|------|------|----------------|
| 1 | | |
| 2 | | |

## Notas / anomalías

-

## Conclusión preliminar


## Referencias


