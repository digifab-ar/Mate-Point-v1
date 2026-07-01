#pragma once

static const char PORTAL_HTML[] = R"rawliteral(
<!DOCTYPE html>
<html lang="es">
<head>
<meta charset="utf-8">
<meta name="viewport" content="width=device-width,initial-scale=1">
<title>Mate Point Wi-Fi</title>
<style>
body{font-family:sans-serif;margin:16px;background:#001808;color:#c8fcb8}
.brand{font-size:1.5em;color:#a0fc50;font-weight:bold;margin:0 0 4px}
h1{font-size:1.25em;color:#a0fc50;margin:0 0 8px}
p{margin:0 0 12px;font-size:0.95em}
label{display:block;margin:12px 0 4px;font-size:0.9em}
input,select{width:100%;padding:10px;box-sizing:border-box;border-radius:8px;border:none;font-size:1em}
button{margin-top:16px;width:100%;padding:14px;background:#ff8028;color:#001000;font-weight:bold;border:none;border-radius:12px;font-size:1em}
#msg{margin-top:14px;min-height:1.2em}
</style>
</head>
<body>
<p class="brand">Mate Point</p>
<h1>Configurar Wi-Fi</h1>
<p>Elegí la red de tu local (2.4 GHz).</p>
<label>Red Wi-Fi</label>
<select id="ssid"><option value="">Cargando redes...</option></select>
<label>Contraseña</label>
<input id="pass" type="password" autocomplete="off">
<button type="button" onclick="connect()">Conectar</button>
<div id="msg"></div>
<script>
function loadScan(){
  fetch('/scan').then(function(r){return r.json();}).then(function(nets){
    var s=document.getElementById('ssid');
    s.innerHTML='';
    if(!nets.length){s.innerHTML='<option value="">No se encontraron redes</option>';return;}
    nets.forEach(function(n){
      var o=document.createElement('option');
      o.value=n.ssid;
      o.textContent=n.ssid+' ('+n.rssi+' dBm)';
      s.appendChild(o);
    });
  }).catch(function(){document.getElementById('msg').textContent='Error al buscar redes.';});
}
function connect(){
  var ssid=document.getElementById('ssid').value;
  var pass=document.getElementById('pass').value;
  if(!ssid){document.getElementById('msg').textContent='Elegí una red.';return;}
  document.getElementById('msg').textContent='Conectando...';
  var body='ssid='+encodeURIComponent(ssid)+'&password='+encodeURIComponent(pass);
  fetch('/connect',{method:'POST',headers:{'Content-Type':'application/x-www-form-urlencoded'},body:body})
    .then(function(r){return r.json();})
    .then(function(d){
      document.getElementById('msg').textContent=d.message||'';
    })
    .catch(function(){document.getElementById('msg').textContent='Error de conexion.';});
}
loadScan();
</script>
</body>
</html>
)rawliteral";
