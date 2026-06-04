# Captura YYYY-MM-DD — título corto

## Metadatos



## Objetivo

Analizar el comportamiento del ESP32 con el nobana cuando se queda sin agua, por corte de linea.

## Resultado observable

- Se encendio el ESP32.
- Luego de 1-2seg segundos se prende ON  nobana.
- Luego de unos segundos,se manda el comando "W"
- Luego de unos segundos,se manda el comando "S", y se escucha 1 chime
- Luego de unos segundos,se manda el comando "R" , y se escucha un chime.
- Inicia el dispensado.
- Se escucha un chime al momento "cooldown" y corte del dispensado.
- Se lee en vervose desde el NOBANA 68 01 11 37 14 00 00 00 00 00 C5
- se manda el comando "R" , y se escucha un chime.
- Inicia el dispensado.
- se cierra dispensado mediante corte manual.
- - Luego de unos segundos,se manda el comando "R" , y se escucha un chime.
- Inicia el dispensado. (no sale agua)
- se cierra dispensado mediante corte manual.
- Se lee en vervose desde el NOBANA 68 01 10 58 14 00 00 01 00 00 E6

## Log UART

14:00:35.436 -> [cfg] bus 9600 8N1 RX=GPIO25 TX=GPIO17
14:00:35.436 -> 
14:00:35.436 -> === Mate Point UART v0-2 — kiosco W/S/R ===
14:00:35.436 -> USB 115200 | Bus 9600 8N1 | Poll 100 ms | ARMOR OFF
14:00:35.436 -> W wake | S standby (21) | R dispensar | X abortar/salir standby
14:00:35.436 -> 
14:00:35.436 -> Comandos (Enter):
14:00:35.436 ->   W           wake: escucha 5s -> F8 -> log 3s
14:00:35.436 ->   S           standby: poll 21 (tras [wake] LISTO)
14:00:35.469 ->   R           dispensar Coffee 180 ml (requiere S; sin lock 23)
14:00:35.469 ->   X           abortar dispensado O salir standby
14:00:35.469 ->   ? / h       ayuda
14:00:35.469 ->   B9600       baud bus Nobana
14:00:35.469 ->   V           verbose HEX
14:00:35.469 ->   I           estado / telemetria
14:00:35.469 -> 
14:00:35.469 -> Wake: IDLE | Standby: OFF | Dispense: OFF | Baud: 9600
14:00:35.469 -> [ready] Nobana ON -> W -> S -> R (ver PLAN-MATE-POINT-UART-v0-2.md)
14:00:44.956 -> 
14:00:44.956 -> [wake] INICIO — escuchando bus antes de F8
14:00:44.956 -> [wake] fase=RX_BOOT (5000 ms)
14:00:44.956 -> [wake] Encender Nobana si aun OFF; ver NOB->ESP en log
14:00:44.988 -> [11587] NOB->ESP (raw) len=70 HEX: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 3F 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
14:00:49.939 -> [wake] fase=POST_F8 — enviando F8, escucha 3000 ms
14:00:49.939 -> [16553] ESP->NOB len=1 HEX: F8
14:00:53.113 -> 
14:00:53.113 -> [wake] LISTO — bytes_rx=70 tramas_68_ok=0
14:00:53.113 -> [wake] Sin trama 0x68 valida tras F8.
14:00:53.113 -> [wake] Repetir W con Nobana ON o revisar cableado/baud.
14:00:53.147 -> [wake] S = standby (21 poll) | R = Coffee 180 ml (sin lock 23)
14:00:53.147 -> 
14:00:58.254 -> [standby] ON — poll 21 (auto tras fin o comando S)
14:01:09.641 -> 
14:01:09.641 -> [dispense] INICIO — Coffee 180 ml (E2 -> 22, sin lock 23)
14:01:09.641 -> [dispense] fase=DISPENSE seq=2
14:01:09.750 -> [36318] telem disp=DISPENSE standby=ON T_obj=85 T_act=24 b2=0x11 fase=0x14 (activo/precalent) progress=0
14:01:09.809 -> [36418] telem disp=DISPENSE standby=ON T_obj=85 T_act=24 b2=0x12 fase=0x14 (activo/precalent) progress=0
14:01:14.507 -> [41118] telem disp=DISPENSE standby=ON T_obj=85 T_act=25 b2=0x12 fase=0x14 (activo/precalent) progress=0
14:01:15.003 -> [41618] telem disp=DISPENSE standby=ON T_obj=85 T_act=26 b2=0x12 fase=0x14 (activo/precalent) progress=0
14:01:15.434 -> [42018] telem disp=DISPENSE standby=ON T_obj=85 T_act=27 b2=0x12 fase=0x14 (activo/precalent) progress=0
14:01:15.730 -> [42318] telem disp=DISPENSE standby=ON T_obj=85 T_act=28 b2=0x12 fase=0x14 (activo/precalent) progress=8
14:01:15.927 -> [42518] telem disp=DISPENSE standby=ON T_obj=85 T_act=29 b2=0x12 fase=0x14 (activo/precalent) progress=8
14:01:16.028 -> [42620] telem disp=DISPENSE standby=ON T_obj=85 T_act=30 b2=0x12 fase=0x14 (activo/precalent) progress=8
14:01:16.226 -> [42818] telem disp=DISPENSE standby=ON T_obj=85 T_act=31 b2=0x12 fase=0x14 (activo/precalent) progress=8
14:01:16.326 -> [42918] telem disp=DISPENSE standby=ON T_obj=85 T_act=33 b2=0x12 fase=0x14 (activo/precalent) progress=8
14:01:16.427 -> [43020] telem disp=DISPENSE standby=ON T_obj=85 T_act=35 b2=0x12 fase=0x14 (activo/precalent) progress=8
14:01:16.527 -> [43118] telem disp=DISPENSE standby=ON T_obj=85 T_act=37 b2=0x12 fase=0x14 (activo/precalent) progress=8
14:01:16.725 -> [43318] telem disp=DISPENSE standby=ON T_obj=85 T_act=40 b2=0x12 fase=0x14 (activo/precalent) progress=17
14:01:16.825 -> [43418] telem disp=DISPENSE standby=ON T_obj=85 T_act=42 b2=0x12 fase=0x14 (activo/precalent) progress=17
14:01:16.923 -> [43518] telem disp=DISPENSE standby=ON T_obj=85 T_act=45 b2=0x12 fase=0x14 (activo/precalent) progress=17
14:01:17.120 -> [43718] telem disp=DISPENSE standby=ON T_obj=85 T_act=48 b2=0x12 fase=0x14 (activo/precalent) progress=17
14:01:17.220 -> [43818] telem disp=DISPENSE standby=ON T_obj=85 T_act=51 b2=0x12 fase=0x14 (activo/precalent) progress=17
14:01:17.321 -> [43920] telem disp=DISPENSE standby=ON T_obj=85 T_act=54 b2=0x12 fase=0x14 (activo/precalent) progress=17
14:01:17.420 -> [44018] telem disp=DISPENSE standby=ON T_obj=85 T_act=56 b2=0x12 fase=0x14 (activo/precalent) progress=17
14:01:17.622 -> [44218] telem disp=DISPENSE standby=ON T_obj=85 T_act=58 b2=0x12 fase=0x14 (activo/precalent) progress=17
14:01:17.722 -> [44320] telem disp=DISPENSE standby=ON T_obj=85 T_act=61 b2=0x12 fase=0x14 (activo/precalent) progress=24
14:01:17.821 -> [44418] telem disp=DISPENSE standby=ON T_obj=85 T_act=62 b2=0x12 fase=0x14 (activo/precalent) progress=24
14:01:17.927 -> [44518] telem disp=DISPENSE standby=ON T_obj=85 T_act=64 b2=0x12 fase=0x14 (activo/precalent) progress=24
14:01:18.120 -> [44718] telem disp=DISPENSE standby=ON T_obj=85 T_act=66 b2=0x12 fase=0x14 (activo/precalent) progress=24
14:01:18.221 -> [44818] telem disp=DISPENSE standby=ON T_obj=85 T_act=67 b2=0x12 fase=0x14 (activo/precalent) progress=24
14:01:18.319 -> [44918] telem disp=DISPENSE standby=ON T_obj=85 T_act=68 b2=0x12 fase=0x14 (activo/precalent) progress=24
14:01:18.517 -> [45118] telem disp=DISPENSE standby=ON T_obj=85 T_act=69 b2=0x12 fase=0x14 (activo/precalent) progress=24
14:01:18.614 -> [45220] telem disp=DISPENSE standby=ON T_obj=85 T_act=70 b2=0x12 fase=0x14 (activo/precalent) progress=24
14:01:18.713 -> [45318] telem disp=DISPENSE standby=ON T_obj=85 T_act=70 b2=0x12 fase=0x14 (activo/precalent) progress=32
14:01:18.812 -> [45418] telem disp=DISPENSE standby=ON T_obj=85 T_act=71 b2=0x12 fase=0x14 (activo/precalent) progress=32
14:01:19.011 -> [45620] telem disp=DISPENSE standby=ON T_obj=85 T_act=72 b2=0x12 fase=0x14 (activo/precalent) progress=32
14:01:19.213 -> [45818] telem disp=DISPENSE standby=ON T_obj=85 T_act=73 b2=0x12 fase=0x14 (activo/precalent) progress=32
14:01:19.312 -> [45918] telem disp=DISPENSE standby=ON T_obj=85 T_act=74 b2=0x12 fase=0x14 (activo/precalent) progress=32
14:01:19.611 -> [46218] telem disp=DISPENSE standby=ON T_obj=85 T_act=75 b2=0x12 fase=0x14 (activo/precalent) progress=32
14:01:19.711 -> [46318] telem disp=DISPENSE standby=ON T_obj=85 T_act=75 b2=0x12 fase=0x14 (activo/precalent) progress=39
14:01:20.006 -> [46618] telem disp=DISPENSE standby=ON T_obj=85 T_act=76 b2=0x12 fase=0x14 (activo/precalent) progress=39
14:01:20.202 -> [46818] telem disp=DISPENSE standby=ON T_obj=85 T_act=77 b2=0x12 fase=0x14 (activo/precalent) progress=39
14:01:20.513 -> [47118] telem disp=DISPENSE standby=ON T_obj=85 T_act=78 b2=0x12 fase=0x14 (activo/precalent) progress=39
14:01:20.711 -> [47320] telem disp=DISPENSE standby=ON T_obj=85 T_act=78 b2=0x12 fase=0x14 (activo/precalent) progress=46
14:01:20.914 -> [47518] telem disp=DISPENSE standby=ON T_obj=85 T_act=79 b2=0x12 fase=0x14 (activo/precalent) progress=46
14:01:21.314 -> [47918] telem disp=DISPENSE standby=ON T_obj=85 T_act=80 b2=0x12 fase=0x14 (activo/precalent) progress=46
14:01:21.508 -> [48118] telem disp=DISPENSE standby=ON T_obj=85 T_act=81 b2=0x12 fase=0x14 (activo/precalent) progress=46
14:01:21.705 -> [48318] telem disp=DISPENSE standby=ON T_obj=85 T_act=81 b2=0x12 fase=0x14 (activo/precalent) progress=53
14:01:22.001 -> [48620] telem disp=DISPENSE standby=ON T_obj=85 T_act=82 b2=0x12 fase=0x14 (activo/precalent) progress=53
14:01:22.730 -> [49318] telem disp=DISPENSE standby=ON T_obj=85 T_act=83 b2=0x12 fase=0x14 (activo/precalent) progress=61
14:01:23.715 -> [50320] telem disp=DISPENSE standby=ON T_obj=85 T_act=83 b2=0x12 fase=0x14 (activo/precalent) progress=68
14:01:24.215 -> [50818] telem disp=DISPENSE standby=ON T_obj=85 T_act=84 b2=0x12 fase=0x14 (activo/precalent) progress=68
14:01:24.711 -> [51318] telem disp=DISPENSE standby=ON T_obj=85 T_act=84 b2=0x12 fase=0x14 (activo/precalent) progress=75
14:01:25.632 -> [52218] telem disp=DISPENSE standby=ON T_obj=85 T_act=85 b2=0x12 fase=0x14 (activo/precalent) progress=75
14:01:25.731 -> [52318] telem disp=DISPENSE standby=ON T_obj=85 T_act=85 b2=0x12 fase=0x14 (activo/precalent) progress=83
14:01:26.728 -> [53320] telem disp=DISPENSE standby=ON T_obj=85 T_act=85 b2=0x12 fase=0x14 (activo/precalent) progress=90
14:01:27.733 -> [54318] telem disp=DISPENSE standby=ON T_obj=85 T_act=85 b2=0x12 fase=0x14 (activo/precalent) progress=98
14:01:28.527 -> [55118] telem disp=DISPENSE standby=ON T_obj=85 T_act=84 b2=0x12 fase=0x14 (activo/precalent) progress=98
14:01:28.725 -> [55318] telem disp=DISPENSE standby=ON T_obj=85 T_act=84 b2=0x12 fase=0x14 (activo/precalent) progress=105
14:01:29.711 -> [56320] telem disp=DISPENSE standby=ON T_obj=85 T_act=84 b2=0x12 fase=0x14 (activo/precalent) progress=113
14:01:30.708 -> [57318] telem disp=DISPENSE standby=ON T_obj=85 T_act=84 b2=0x12 fase=0x14 (activo/precalent) progress=120
14:01:31.631 -> [58218] telem disp=DISPENSE standby=ON T_obj=85 T_act=84 b2=0x12 fase=0x14 (activo/precalent) progress=128
14:01:31.828 -> [58418] telem disp=DISPENSE standby=ON T_obj=85 T_act=83 b2=0x12 fase=0x14 (activo/precalent) progress=128
14:01:32.602 -> [59218] telem disp=DISPENSE standby=ON T_obj=85 T_act=83 b2=0x12 fase=0x14 (activo/precalent) progress=135
14:01:33.629 -> [60218] telem disp=DISPENSE standby=ON T_obj=85 T_act=83 b2=0x12 fase=0x14 (activo/precalent) progress=143
14:01:33.663 -> [dispense] T_DISPENSE -> PRE_STOP
14:01:33.663 -> [dispense] fase=PRE_STOP seq=2
14:01:34.612 -> [61220] telem disp=PRE_STOP standby=ON T_obj=85 T_act=83 b2=0x12 fase=0x14 (activo/precalent) progress=150
14:01:35.633 -> [62220] telem disp=PRE_STOP standby=ON T_obj=85 T_act=83 b2=0x12 fase=0x14 (activo/precalent) progress=157
14:01:35.831 -> [62420] telem disp=PRE_STOP standby=ON T_obj=85 T_act=82 b2=0x12 fase=0x14 (activo/precalent) progress=157
14:01:36.226 -> [62820] telem disp=PRE_STOP standby=ON T_obj=85 T_act=81 b2=0x12 fase=0x14 (activo/precalent) progress=157
14:01:36.425 -> [63020] telem disp=PRE_STOP standby=ON T_obj=85 T_act=80 b2=0x12 fase=0x14 (activo/precalent) progress=157
14:01:36.621 -> [63220] telem disp=PRE_STOP standby=ON T_obj=85 T_act=80 b2=0x12 fase=0x14 (activo/precalent) progress=165
14:01:36.720 -> [63320] telem disp=PRE_STOP standby=ON T_obj=85 T_act=79 b2=0x12 fase=0x14 (activo/precalent) progress=165
14:01:36.817 -> [63420] telem disp=PRE_STOP standby=ON T_obj=85 T_act=78 b2=0x12 fase=0x14 (activo/precalent) progress=165
14:01:36.918 -> [63520] telem disp=PRE_STOP standby=ON T_obj=85 T_act=77 b2=0x12 fase=0x14 (activo/precalent) progress=165
14:01:37.215 -> [63820] telem disp=PRE_STOP standby=ON T_obj=85 T_act=76 b2=0x12 fase=0x14 (activo/precalent) progress=165
14:01:37.312 -> [63920] telem disp=PRE_STOP standby=ON T_obj=85 T_act=75 b2=0x12 fase=0x14 (activo/precalent) progress=165
14:01:37.410 -> [64022] telem disp=PRE_STOP standby=ON T_obj=85 T_act=74 b2=0x12 fase=0x14 (activo/precalent) progress=165
14:01:37.546 -> [dispense] fase=STOP_22_04 seq=2
14:01:37.546 -> [dispense] fase=CLOSE_22_00 seq=2
14:01:37.712 -> [64320] telem disp=CLOSE_22_00 standby=ON T_obj=85 T_act=72 b2=0x11 fase=0x14 (activo/precalent) progress=0
14:01:37.811 -> [64420] telem disp=CLOSE_22_00 standby=ON T_obj=85 T_act=70 b2=0x11 fase=0x14 (activo/precalent) progress=0
14:01:37.911 -> [64520] telem disp=CLOSE_22_00 standby=ON T_obj=85 T_act=69 b2=0x11 fase=0x14 (activo/precalent) progress=0
14:01:38.109 -> [64720] telem disp=CLOSE_22_00 standby=ON T_obj=85 T_act=68 b2=0x11 fase=0x14 (activo/precalent) progress=0
14:01:38.211 -> [64822] telem disp=CLOSE_22_00 standby=ON T_obj=85 T_act=67 b2=0x11 fase=0x14 (activo/precalent) progress=0
14:01:38.306 -> [64920] telem disp=CLOSE_22_00 standby=ON T_obj=85 T_act=66 b2=0x11 fase=0x14 (activo/precalent) progress=0
14:01:38.506 -> [65120] telem disp=CLOSE_22_00 standby=ON T_obj=85 T_act=65 b2=0x11 fase=0x14 (activo/precalent) progress=0
14:01:38.604 -> [65222] telem disp=CLOSE_22_00 standby=ON T_obj=85 T_act=64 b2=0x11 fase=0x14 (activo/precalent) progress=0
14:01:38.704 -> [65320] telem disp=CLOSE_22_00 standby=ON T_obj=85 T_act=63 b2=0x11 fase=0x14 (activo/precalent) progress=0
14:01:38.836 -> [65420] telem disp=CLOSE_22_00 standby=ON T_obj=85 T_act=62 b2=0x11 fase=0x14 (activo/precalent) progress=0
14:01:39.033 -> [65622] telem disp=CLOSE_22_00 standby=ON T_obj=85 T_act=60 b2=0x11 fase=0x14 (activo/precalent) progress=0
14:01:39.133 -> [65720] telem disp=CLOSE_22_00 standby=ON T_obj=85 T_act=59 b2=0x11 fase=0x14 (activo/precalent) progress=0
14:01:39.239 -> [65820] telem disp=CLOSE_22_00 standby=ON T_obj=85 T_act=58 b2=0x11 fase=0x14 (activo/precalent) progress=0
14:01:39.301 -> [65920] telem disp=CLOSE_22_00 standby=ON T_obj=85 T_act=57 b2=0x11 fase=0x14 (activo/precalent) progress=0
14:01:39.535 -> [66120] telem disp=CLOSE_22_00 standby=ON T_obj=85 T_act=56 b2=0x11 fase=0x14 (activo/precalent) progress=0
14:01:39.535 -> [dispense] NOB b2=0x11 -> COOLDOWN
14:01:39.535 -> [dispense] fase=COOLDOWN_22 seq=3
14:01:39.731 -> [66320] telem disp=COOLDOWN_22 standby=ON T_obj=85 T_act=55 b2=0x11 fase=0x14 (activo/precalent) progress=0
14:01:40.127 -> [66720] telem disp=COOLDOWN_22 standby=ON T_obj=85 T_act=54 b2=0x11 fase=0x14 (activo/precalent) progress=0
14:01:40.630 -> [67220] telem disp=COOLDOWN_22 standby=ON T_obj=85 T_act=53 b2=0x11 fase=0x14 (activo/precalent) progress=0
14:01:42.908 -> [69520] telem disp=COOLDOWN_22 standby=ON T_obj=85 T_act=54 b2=0x11 fase=0x14 (activo/precalent) progress=0
14:01:44.622 -> [71220] telem disp=COOLDOWN_22 standby=ON T_obj=85 T_act=55 b2=0x11 fase=0x14 (activo/precalent) progress=0
14:01:46.109 -> [72722] telem disp=COOLDOWN_22 standby=ON T_obj=85 T_act=56 b2=0x11 fase=0x14 (activo/precalent) progress=0
14:01:47.930 -> [74520] telem disp=COOLDOWN_22 standby=ON T_obj=85 T_act=57 b2=0x11 fase=0x14 (activo/precalent) progress=0
14:01:50.639 -> [77220] telem disp=COOLDOWN_22 standby=ON T_obj=85 T_act=56 b2=0x11 fase=0x14 (activo/precalent) progress=0
14:01:54.539 -> [81150] telem disp=COOLDOWN_22 standby=ON T_obj=85 T_act=56 b2=0x11 fase=0x14 (activo/precalent) progress=0
14:01:54.539 -> [dispense] FIN (ciclo_sin_lock)
14:01:54.572 -> [standby] ON — poll 21 (auto tras fin o comando S)
14:02:31.863 -> [cfg] verbose ON
14:02:31.929 -> [118519] NOB->ESP len=11 HEX: 68 01 11 37 14 00 00 00 00 00 C5
14:02:31.961 -> [118551] ESP->NOB len=9 HEX: 68 01 21 00 00 00 00 00 8A
14:02:32.027 -> [118619] NOB->ESP len=11 HEX: 68 01 11 37 14 00 00 00 00 00 C5
14:02:32.059 -> [118651] ESP->NOB len=9 HEX: 68 01 21 00 00 00 00 00 8A
14:02:32.125 -> [118721] NOB->ESP len=11 HEX: 68 01 11 37 14 00 00 00 00 00 C5
14:02:32.160 -> [118751] ESP->NOB len=9 HEX: 68 01 21 00 00 00 00 00 8A
14:02:32.226 -> [118819] NOB->ESP len=11 HEX: 68 01 11 37 14 00 00 00 00 00 C5
14:02:32.257 -> [118851] ESP->NOB len=9 HEX: 68 01 21 00 00 00 00 00 8A
14:02:32.324 -> [118919] NOB->ESP len=11 HEX: 68 01 11 37 14 00 00 00 00 00 C5
14:02:32.355 -> [118951] ESP->NOB len=9 HEX: 68 01 21 00 00 00 00 00 8A
14:02:32.434 -> [119019] NOB->ESP len=11 HEX: 68 01 11 37 14 00 00 00 00 00 C5
14:02:32.434 -> [119051] ESP->NOB len=9 HEX: 68 01 21 00 00 00 00 00 8A
14:02:32.535 -> [119121] NOB->ESP len=11 HEX: 68 01 11 37 14 00 00 00 00 00 C5
14:02:32.535 -> [119151] ESP->NOB len=9 HEX: 68 01 21 00 00 00 00 00 8A
14:02:32.633 -> [119219] NOB->ESP len=11 HEX: 68 01 11 37 14 00 00 00 00 00 C5
14:02:32.665 -> [119251] ESP->NOB len=9 HEX: 68 01 21 00 00 00 00 00 8A
14:02:32.731 -> [119319] NOB->ESP len=11 HEX: 68 01 11 37 14 00 00 00 00 00 C5
14:02:32.765 -> [119351] ESP->NOB len=9 HEX: 68 01 21 00 00 00 00 00 8A
14:02:32.828 -> [119419] NOB->ESP len=11 HEX: 68 01 11 37 14 00 00 00 00 00 C5
14:02:32.862 -> [119451] ESP->NOB len=9 HEX: 68 01 21 00 00 00 00 00 8A
14:02:32.927 -> [119521] NOB->ESP len=11 HEX: 68 01 11 37 14 00 00 00 00 00 C5
14:02:32.960 -> [119551] ESP->NOB len=9 HEX: 68 01 21 00 00 00 00 00 8A
14:02:33.025 -> [119619] NOB->ESP len=11 HEX: 68 01 11 37 14 00 00 00 00 00 C5
14:02:33.059 -> [119651] ESP->NOB len=9 HEX: 68 01 21 00 00 00 00 00 8A
14:02:33.125 -> [119719] NOB->ESP len=11 HEX: 68 01 11 37 14 00 00 00 00 00 C5
14:02:33.162 -> [119751] ESP->NOB len=9 HEX: 68 01 21 00 00 00 00 00 8A
14:02:33.226 -> [119819] NOB->ESP len=11 HEX: 68 01 11 37 14 00 00 00 00 00 C5
14:02:33.258 -> [119851] ESP->NOB len=9 HEX: 68 01 21 00 00 00 00 00 8A
14:02:33.322 -> [119921] NOB->ESP len=11 HEX: 68 01 11 37 14 00 00 00 00 00 C5
14:02:33.355 -> [119951] ESP->NOB len=9 HEX: 68 01 21 00 00 00 00 00 8A
14:02:33.421 -> [120019] NOB->ESP len=11 HEX: 68 01 11 37 14 00 00 00 00 00 C5
14:02:33.455 -> [120051] ESP->NOB len=9 HEX: 68 01 21 00 00 00 00 00 8A
14:02:33.519 -> [120119] NOB->ESP len=11 HEX: 68 01 11 37 14 00 00 00 00 00 C5
14:02:33.552 -> [120151] ESP->NOB len=9 HEX: 68 01 21 00 00 00 00 00 8A
14:02:33.618 -> [120219] NOB->ESP len=11 HEX: 68 01 11 37 14 00 00 00 00 00 C5
14:02:33.650 -> [120251] ESP->NOB len=9 HEX: 68 01 21 00 00 00 00 00 8A
14:02:33.719 -> [120321] NOB->ESP len=11 HEX: 68 01 11 37 14 00 00 00 00 00 C5
14:02:33.749 -> [120351] ESP->NOB len=9 HEX: 68 01 21 00 00 00 00 00 8A
14:02:33.814 -> [120419] NOB->ESP len=11 HEX: 68 01 11 37 14 00 00 00 00 00 C5
14:02:33.848 -> [120451] ESP->NOB len=9 HEX: 68 01 21 00 00 00 00 00 8A
14:02:33.913 -> [120519] NOB->ESP len=11 HEX: 68 01 11 37 14 00 00 00 00 00 C5
14:02:33.947 -> [120551] ESP->NOB len=9 HEX: 68 01 21 00 00 00 00 00 8A
14:02:34.013 -> [120619] NOB->ESP len=11 HEX: 68 01 11 37 14 00 00 00 00 00 C5
14:02:34.046 -> [120651] ESP->NOB len=9 HEX: 68 01 21 00 00 00 00 00 8A
14:02:34.110 -> [120721] NOB->ESP len=11 HEX: 68 01 11 37 14 00 00 00 00 00 C5
14:02:34.143 -> [120751] ESP->NOB len=9 HEX: 68 01 21 00 00 00 00 00 8A
14:02:34.249 -> [120819] NOB->ESP len=11 HEX: 68 01 11 37 14 00 00 00 00 00 C5
14:02:34.249 -> [120851] ESP->NOB len=9 HEX: 68 01 21 00 00 00 00 00 8A
14:02:34.315 -> [120919] NOB->ESP len=11 HEX: 68 01 11 37 14 00 00 00 00 00 C5
14:02:34.348 -> [120951] ESP->NOB len=9 HEX: 68 01 21 00 00 00 00 00 8A
14:02:34.415 -> [121019] NOB->ESP len=11 HEX: 68 01 11 37 14 00 00 00 00 00 C5
14:02:34.447 -> [121051] ESP->NOB len=9 HEX: 68 01 21 00 00 00 00 00 8A
14:02:34.514 -> [121121] NOB->ESP len=11 HEX: 68 01 11 37 14 00 00 00 00 00 C5
14:02:34.547 -> [121151] ESP->NOB len=9 HEX: 68 01 21 00 00 00 00 00 8A
14:02:34.617 -> [121219] NOB->ESP len=11 HEX: 68 01 11 37 14 00 00 00 00 00 C5
14:02:34.646 -> [121251] ESP->NOB len=9 HEX: 68 01 21 00 00 00 00 00 8A
14:02:34.712 -> [121319] NOB->ESP len=11 HEX: 68 01 11 37 14 00 00 00 00 00 C5
14:02:34.745 -> [121351] ESP->NOB len=9 HEX: 68 01 21 00 00 00 00 00 8A
14:02:34.811 -> [121419] NOB->ESP len=11 HEX: 68 01 11 37 14 00 00 00 00 00 C5
14:02:34.844 -> [121451] ESP->NOB len=9 HEX: 68 01 21 00 00 00 00 00 8A
14:02:34.910 -> [cfg] verbose OFF
14:02:49.930 -> 
14:02:49.930 -> [dispense] INICIO — Coffee 180 ml (E2 -> 22, sin lock 23)
14:02:49.930 -> [dispense] fase=DISPENSE seq=2
14:02:49.996 -> [136599] telem disp=DISPENSE standby=ON T_obj=85 T_act=55 b2=0x11 fase=0x14 (activo/precalent) progress=0
14:02:50.094 -> [136699] telem disp=DISPENSE standby=ON T_obj=85 T_act=55 b2=0x12 fase=0x14 (activo/precalent) progress=0
14:02:50.193 -> [136799] telem disp=DISPENSE standby=ON T_obj=85 T_act=54 b2=0x12 fase=0x14 (activo/precalent) progress=0
14:02:53.806 -> [140401] telem disp=DISPENSE standby=ON T_obj=85 T_act=54 b2=0x12 fase=0x14 (activo/precalent) progress=6
14:02:53.905 -> [140499] telem disp=DISPENSE standby=ON T_obj=85 T_act=55 b2=0x12 fase=0x14 (activo/precalent) progress=6
14:02:54.505 -> [141099] telem disp=DISPENSE standby=ON T_obj=85 T_act=56 b2=0x12 fase=0x14 (activo/precalent) progress=6
14:02:54.804 -> [141399] telem disp=DISPENSE standby=ON T_obj=85 T_act=56 b2=0x12 fase=0x14 (activo/precalent) progress=13
14:02:54.902 -> [141499] telem disp=DISPENSE standby=ON T_obj=85 T_act=57 b2=0x12 fase=0x14 (activo/precalent) progress=13
14:02:55.299 -> [141899] telem disp=DISPENSE standby=ON T_obj=85 T_act=58 b2=0x12 fase=0x14 (activo/precalent) progress=13
14:02:55.497 -> [142099] telem disp=DISPENSE standby=ON T_obj=85 T_act=59 b2=0x12 fase=0x14 (activo/precalent) progress=13
14:02:55.797 -> [142401] telem disp=DISPENSE standby=ON T_obj=85 T_act=59 b2=0x12 fase=0x14 (activo/precalent) progress=20
14:02:55.896 -> [142499] telem disp=DISPENSE standby=ON T_obj=85 T_act=60 b2=0x12 fase=0x14 (activo/precalent) progress=20
14:02:55.995 -> [142599] telem disp=DISPENSE standby=ON T_obj=85 T_act=61 b2=0x12 fase=0x14 (activo/precalent) progress=20
14:02:56.289 -> [142899] telem disp=DISPENSE standby=ON T_obj=85 T_act=62 b2=0x12 fase=0x14 (activo/precalent) progress=20
14:02:56.387 -> [142999] telem disp=DISPENSE standby=ON T_obj=85 T_act=63 b2=0x12 fase=0x14 (activo/precalent) progress=20
14:02:56.682 -> [143299] telem disp=DISPENSE standby=ON T_obj=85 T_act=64 b2=0x12 fase=0x14 (activo/precalent) progress=20
14:02:56.782 -> [143399] telem disp=DISPENSE standby=ON T_obj=85 T_act=65 b2=0x12 fase=0x14 (activo/precalent) progress=27
14:02:56.881 -> [143499] telem disp=DISPENSE standby=ON T_obj=85 T_act=66 b2=0x12 fase=0x14 (activo/precalent) progress=27
14:02:57.210 -> [143799] telem disp=DISPENSE standby=ON T_obj=85 T_act=67 b2=0x12 fase=0x14 (activo/precalent) progress=27
14:02:57.308 -> [143899] telem disp=DISPENSE standby=ON T_obj=85 T_act=68 b2=0x12 fase=0x14 (activo/precalent) progress=27
14:02:57.614 -> [144199] telem disp=DISPENSE standby=ON T_obj=85 T_act=69 b2=0x12 fase=0x14 (activo/precalent) progress=27
14:02:57.714 -> [144299] telem disp=DISPENSE standby=ON T_obj=85 T_act=70 b2=0x12 fase=0x14 (activo/precalent) progress=27
14:02:57.811 -> [144399] telem disp=DISPENSE standby=ON T_obj=85 T_act=70 b2=0x12 fase=0x14 (activo/precalent) progress=34
14:02:58.009 -> [144599] telem disp=DISPENSE standby=ON T_obj=85 T_act=71 b2=0x12 fase=0x14 (activo/precalent) progress=34
14:02:58.107 -> [144699] telem disp=DISPENSE standby=ON T_obj=85 T_act=72 b2=0x12 fase=0x14 (activo/precalent) progress=34
14:02:58.207 -> [144799] telem disp=DISPENSE standby=ON T_obj=85 T_act=73 b2=0x12 fase=0x14 (activo/precalent) progress=34
14:02:58.500 -> [145101] telem disp=DISPENSE standby=ON T_obj=85 T_act=74 b2=0x12 fase=0x14 (activo/precalent) progress=34
14:02:58.698 -> [145299] telem disp=DISPENSE standby=ON T_obj=85 T_act=75 b2=0x12 fase=0x14 (activo/precalent) progress=34
14:02:58.796 -> [145399] telem disp=DISPENSE standby=ON T_obj=85 T_act=75 b2=0x12 fase=0x14 (activo/precalent) progress=41
14:02:58.995 -> [145599] telem disp=DISPENSE standby=ON T_obj=85 T_act=76 b2=0x12 fase=0x14 (activo/precalent) progress=41
14:02:59.192 -> [145799] telem disp=DISPENSE standby=ON T_obj=85 T_act=77 b2=0x12 fase=0x14 (activo/precalent) progress=41
14:02:59.486 -> [146099] telem disp=DISPENSE standby=ON T_obj=85 T_act=78 b2=0x12 fase=0x14 (activo/precalent) progress=41
14:02:59.782 -> [146399] telem disp=DISPENSE standby=ON T_obj=85 T_act=78 b2=0x12 fase=0x14 (activo/precalent) progress=48
14:02:59.916 -> [146499] telem disp=DISPENSE standby=ON T_obj=85 T_act=79 b2=0x12 fase=0x14 (activo/precalent) progress=48
14:03:00.218 -> [146799] telem disp=DISPENSE standby=ON T_obj=85 T_act=80 b2=0x12 fase=0x14 (activo/precalent) progress=48
14:03:00.482 -> [147099] telem disp=DISPENSE standby=ON T_obj=85 T_act=81 b2=0x12 fase=0x14 (activo/precalent) progress=48
14:03:00.789 -> [147399] telem disp=DISPENSE standby=ON T_obj=85 T_act=81 b2=0x12 fase=0x14 (activo/precalent) progress=55
14:03:00.887 -> [147499] telem disp=DISPENSE standby=ON T_obj=85 T_act=82 b2=0x12 fase=0x14 (activo/precalent) progress=55
14:03:01.285 -> [147899] telem disp=DISPENSE standby=ON T_obj=85 T_act=83 b2=0x12 fase=0x14 (activo/precalent) progress=55
14:03:01.787 -> [148399] telem disp=DISPENSE standby=ON T_obj=85 T_act=83 b2=0x12 fase=0x14 (activo/precalent) progress=63
14:03:01.891 -> [148499] telem disp=DISPENSE standby=ON T_obj=85 T_act=84 b2=0x12 fase=0x14 (activo/precalent) progress=63
14:03:01.956 -> [148563] telem disp=DISPENSE standby=ON T_obj=85 T_act=84 b2=0x12 fase=0x14 (activo/precalent) progress=63
14:03:01.956 -> [dispense] FIN (usuario_X)
14:03:01.956 -> [standby] ON — poll 21 (auto tras fin o comando S)
14:03:17.115 -> 
14:03:17.115 -> [dispense] INICIO — Coffee 180 ml (E2 -> 22, sin lock 23)
14:03:17.115 -> [dispense] fase=DISPENSE seq=2
14:03:17.182 -> [163780] telem disp=DISPENSE standby=ON T_obj=85 T_act=88 b2=0x11 fase=0x14 (activo/precalent) progress=0
14:03:17.278 -> [163880] telem disp=DISPENSE standby=ON T_obj=85 T_act=88 b2=0x12 fase=0x14 (activo/precalent) progress=0
14:03:18.178 -> [164780] telem disp=DISPENSE standby=ON T_obj=85 T_act=88 b2=0x12 fase=0x14 (activo/precalent) progress=4
14:03:19.169 -> [165780] telem disp=DISPENSE standby=ON T_obj=85 T_act=88 b2=0x12 fase=0x14 (activo/precalent) progress=12
14:03:19.972 -> [166580] telem disp=DISPENSE standby=ON T_obj=85 T_act=87 b2=0x12 fase=0x14 (activo/precalent) progress=12
14:03:20.169 -> [166782] telem disp=DISPENSE standby=ON T_obj=85 T_act=87 b2=0x12 fase=0x14 (activo/precalent) progress=19
14:03:20.570 -> [167180] telem disp=DISPENSE standby=ON T_obj=85 T_act=86 b2=0x12 fase=0x14 (activo/precalent) progress=19
14:03:21.195 -> [167780] telem disp=DISPENSE standby=ON T_obj=85 T_act=86 b2=0x12 fase=0x14 (activo/precalent) progress=26
14:03:21.293 -> [167882] telem disp=DISPENSE standby=ON T_obj=85 T_act=85 b2=0x12 fase=0x14 (activo/precalent) progress=26
14:03:21.693 -> [168282] telem disp=DISPENSE standby=ON T_obj=85 T_act=84 b2=0x12 fase=0x14 (activo/precalent) progress=26
14:03:21.991 -> [168580] telem disp=DISPENSE standby=ON T_obj=85 T_act=83 b2=0x12 fase=0x14 (activo/precalent) progress=26
14:03:22.190 -> [168780] telem disp=DISPENSE standby=ON T_obj=85 T_act=83 b2=0x12 fase=0x14 (activo/precalent) progress=33
14:03:22.288 -> [168880] telem disp=DISPENSE standby=ON T_obj=85 T_act=82 b2=0x12 fase=0x14 (activo/precalent) progress=33
14:03:22.487 -> [169078] telem disp=DISPENSE standby=ON T_obj=85 T_act=81 b2=0x12 fase=0x14 (activo/precalent) progress=33
14:03:22.786 -> [169380] telem disp=DISPENSE standby=ON T_obj=85 T_act=80 b2=0x12 fase=0x14 (activo/precalent) progress=33
14:03:23.184 -> [169782] telem disp=DISPENSE standby=ON T_obj=85 T_act=80 b2=0x12 fase=0x14 (activo/precalent) progress=40
14:03:23.284 -> [169880] telem disp=DISPENSE standby=ON T_obj=85 T_act=79 b2=0x12 fase=0x14 (activo/precalent) progress=40
14:03:23.584 -> [170182] telem disp=DISPENSE standby=ON T_obj=85 T_act=80 b2=0x12 fase=0x14 (activo/precalent) progress=40
14:03:24.178 -> [170780] telem disp=DISPENSE standby=ON T_obj=85 T_act=80 b2=0x12 fase=0x14 (activo/precalent) progress=47
14:03:24.771 -> [171380] telem disp=DISPENSE standby=ON T_obj=85 T_act=81 b2=0x12 fase=0x14 (activo/precalent) progress=47
14:03:25.169 -> [171780] telem disp=DISPENSE standby=ON T_obj=85 T_act=81 b2=0x12 fase=0x14 (activo/precalent) progress=54
14:03:25.298 -> [171910] telem disp=DISPENSE standby=ON T_obj=85 T_act=81 b2=0x12 fase=0x14 (activo/precalent) progress=54
14:03:25.331 -> [dispense] FIN (usuario_X)
14:03:25.331 -> [standby] ON — poll 21 (auto tras fin o comando S)
14:03:43.097 -> [cfg] verbose ON
14:03:43.097 -> [189711] ESP->NOB len=9 HEX: 68 01 21 00 00 00 00 00 8A
14:03:43.194 -> [189781] NOB->ESP len=11 HEX: 68 01 10 58 14 00 00 01 00 00 E6
14:03:43.194 -> [189811] ESP->NOB len=9 HEX: 68 01 21 00 00 00 00 00 8A
14:03:43.294 -> [189879] NOB->ESP len=11 HEX: 68 01 10 58 14 00 00 01 00 00 E6
14:03:43.327 -> [189911] ESP->NOB len=9 HEX: 68 01 21 00 00 00 00 00 8A
14:03:43.393 -> [189979] NOB->ESP len=11 HEX: 68 01 10 58 14 00 00 01 00 00 E6
14:03:43.426 -> [190011] ESP->NOB len=9 HEX: 68 01 21 00 00 00 00 00 8A
14:03:43.493 -> [190079] NOB->ESP len=11 HEX: 68 01 10 58 14 00 00 01 00 00 E6
14:03:43.526 -> [190111] ESP->NOB len=9 HEX: 68 01 21 00 00 00 00 00 8A
14:03:43.596 -> [190179] NOB->ESP len=11 HEX: 68 01 10 58 14 00 00 01 00 00 E6
14:03:43.596 -> [190211] ESP->NOB len=9 HEX: 68 01 21 00 00 00 00 00 8A
14:03:43.661 -> [190279] NOB->ESP len=11 HEX: 68 01 10 58 14 00 00 01 00 00 E6
14:03:43.695 -> [190311] ESP->NOB len=9 HEX: 68 01 21 00 00 00 00 00 8A
14:03:43.793 -> [190379] NOB->ESP len=11 HEX: 68 01 10 58 14 00 00 01 00 00 E6
14:03:43.827 -> [190411] ESP->NOB len=9 HEX: 68 01 21 00 00 00 00 00 8A
14:03:43.893 -> [190479] NOB->ESP len=11 HEX: 68 01 10 58 14 00 00 01 00 00 E6
14:03:43.926 -> [190511] ESP->NOB len=9 HEX: 68 01 21 00 00 00 00 00 8A
14:03:43.993 -> [190579] NOB->ESP len=11 HEX: 68 01 10 58 14 00 00 01 00 00 E6
14:03:44.026 -> [190611] ESP->NOB len=9 HEX: 68 01 21 00 00 00 00 00 8A
14:03:44.093 -> [190679] NOB->ESP len=11 HEX: 68 01 10 58 14 00 00 01 00 00 E6
14:03:44.126 -> [190711] ESP->NOB len=9 HEX: 68 01 21 00 00 00 00 00 8A
14:03:44.191 -> [190779] NOB->ESP len=11 HEX: 68 01 10 58 14 00 00 01 00 00 E6
14:03:44.223 -> [190811] ESP->NOB len=9 HEX: 68 01 21 00 00 00 00 00 8A
14:03:44.289 -> [190881] NOB->ESP len=11 HEX: 68 01 10 58 14 00 00 01 00 00 E6
14:03:44.325 -> [190911] ESP->NOB len=9 HEX: 68 01 21 00 00 00 00 00 8A
14:03:44.393 -> [190979] NOB->ESP len=11 HEX: 68 01 10 58 14 00 00 01 00 00 E6
14:03:44.421 -> [191011] ESP->NOB len=9 HEX: 68 01 21 00 00 00 00 00 8A
14:03:44.487 -> [191079] NOB->ESP len=11 HEX: 68 01 10 58 14 00 00 01 00 00 E6
14:03:44.522 -> [191111] ESP->NOB len=9 HEX: 68 01 21 00 00 00 00 00 8A
14:03:44.585 -> [191179] NOB->ESP len=11 HEX: 68 01 10 58 14 00 00 01 00 00 E6
14:03:44.618 -> [191211] ESP->NOB len=9 HEX: 68 01 21 00 00 00 00 00 8A
14:03:44.683 -> [191279] NOB->ESP len=11 HEX: 68 01 10 58 14 00 00 01 00 00 E6
14:03:44.718 -> [191311] ESP->NOB len=9 HEX: 68 01 21 00 00 00 00 00 8A


## Secuencia inferida (opcional)

| Paso | Dir. | Trama / evento |
|------|------|----------------|
| 1 | | |
| 2 | | |

## Notas / anomalías

-

## Conclusión preliminar

**Sensor de tanque (validado en banco):**

| UART NOB→ARM | Tanque | Nobana permite dispensar |
|--------------|--------|---------------------------|
| **`b2=0x11`**, byte 7 = `00` | Hay agua, **nivel bajo** | **Sí** |
| **`b2=0x10`**, byte 7 = **`0x01`** | **Sin agua** | **No** |

Referencias HEX en verbose: `68 01 11 37 14 00 00 00 00 00 C5` (bajo) · `68 01 10 58 14 00 00 01 00 00 E6` (vacío).

Documentado en [`PROTOCOLO-UART-NOBANA.md`](../../../mate_point_firmware/PROTOCOLO-UART-NOBANA.md) §4.4 y §0.2.2 (V22–V24).

## Referencias


