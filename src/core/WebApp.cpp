#include "core/WebApp.h"
#include "core/ScaleManager.h"
#include "events/EventType.h"
#include "config.h"
#include <Arduino.h>
#include <cstdio>

// ── index page ───────────────────────────────────────────────────────────────
static const char ROOT_HTML[] = R"html(
<!DOCTYPE html><html><head>
<meta charset="utf-8">
<meta name="viewport" content="width=device-width,initial-scale=1">
<title>BalancaC3</title>
<style>
:root{--bg:#f0f2f5;--card:#fff;--text:#1a1a2e;--sub:#666;--btn:#0066cc;--bh:#0052a3;--bdr:#ddd;--warn:#b45309;--warnbg:#fef3c7}
html.dark{--bg:#0f1117;--card:#1e2130;--text:#e0e6f0;--sub:#8892a4;--btn:#4d8ef0;--bh:#3a7ae0;--bdr:#2e3548;--warn:#fbbf24;--warnbg:#1c1408}
*{box-sizing:border-box}
body{font-family:sans-serif;max-width:440px;margin:0 auto;padding:1em;background:var(--bg);color:var(--text)}
.hdr{display:flex;justify-content:space-between;align-items:center}
h1{margin:.4em 0;font-size:1.4em}
.thm{background:none;border:1px solid var(--sub);border-radius:20px;padding:.3em .85em;cursor:pointer;color:var(--text);font-size:.82em}
.card{background:var(--card);border-radius:10px;padding:1.2em;margin:.8em 0;box-shadow:0 1px 4px rgba(0,0,0,.1);text-align:center}
.weight{font-size:3em;font-weight:bold;letter-spacing:-.02em;margin:.1em 0;line-height:1}
.unit{font-size:.4em;font-weight:normal;color:var(--sub);margin-left:.15em}
.raw{font-size:.8em;color:var(--sub);margin:.2em 0}
.warn{background:var(--warnbg);color:var(--warn);border-radius:6px;padding:.5em .8em;font-size:.85em;margin:.5em 0}
.dot{display:inline-block;width:8px;height:8px;border-radius:50%;background:#22c55e;margin-right:.4em}
.dot.off{background:#ef4444}
p{color:var(--sub);margin:.3em 0}
a.btn{display:inline-block;margin-top:.8em;padding:.6em 1.4em;background:var(--btn);color:#fff;border-radius:6px;text-decoration:none;font-size:.95em}
a.btn:active{background:var(--bh)}
</style></head><body>
<div class="hdr"><h1>BalancaC3</h1><div style="display:flex;align-items:center;gap:.5em"><a href="/config" style="display:flex;align-items:center;gap:.3em;color:var(--text);text-decoration:none;font-size:.88em;border:1px solid var(--sub);border-radius:20px;padding:.3em .75em"><span style="font-size:1.1em">&#9881;</span>Configurações</a><button class="thm" id="thm" onclick="tog()"></button></div></div>
<div class="card">
  <div style="font-size:.8em;color:var(--sub);margin-bottom:.4em">
    <span class="dot" id="dot"></span><span id="sensor-status">aguardando...</span>
  </div>
  <div class="weight" id="weight-val">--<span class="unit" id="weight-unit">g</span></div>
  <div class="raw" id="raw-val"></div>
  <div class="warn" id="cal-warn" style="display:none">Sensor sem calibracao — valore raw apenas. Acesse Configurações para calibrar.</div>
  <div style="margin-top:.8em"><button onclick="doTare()" style="padding:.6em 1.6em;background:var(--btn);color:#fff;border:none;border-radius:6px;font-size:.95em;cursor:pointer">Tarar</button></div>
  <div id="tare-msg" style="font-size:.82em;color:var(--sub);margin-top:.4em;min-height:1.2em"></div>
</div>
<script>
var H=document.documentElement,D=document;
function applyDark(d){H.classList.toggle('dark',d);D.getElementById('thm').textContent=d?'Claro':'Escuro'}
function tog(){var d=!H.classList.contains('dark');localStorage.setItem('t',d?'1':'0');applyDark(d)}
(function(){var s=localStorage.getItem('t');applyDark(s!=null?s==='1':window.matchMedia('(prefers-color-scheme:dark)').matches)})();
(function poll(){
  fetch('/scale/weight').then(function(r){return r.json()}).then(function(d){
    var dot=D.getElementById('dot'),st=D.getElementById('sensor-status');
    var wv=D.getElementById('weight-val'),wu=D.getElementById('weight-unit');
    var rv=D.getElementById('raw-val'),cw=D.getElementById('cal-warn');
    if(!d.ready){dot.className='dot off';st.textContent='sensor nao detectado';wv.childNodes[0].textContent='--';rv.textContent='';return}
    dot.className='dot';
    if(d.calibrated){
      st.textContent='calibrado';cw.style.display='none';
      var g=d.grams,abs=Math.abs(g);
      if(abs>=1000){wv.childNodes[0].textContent=(g/1000).toFixed(3);wu.textContent='kg'}
      else{wv.childNodes[0].textContent=g.toFixed(1);wu.textContent='g'}
      rv.textContent='raw: '+d.raw;
    } else {
      st.textContent='sem calibracao';cw.style.display='block';
      wv.childNodes[0].textContent=d.raw;wu.textContent='raw';
      rv.textContent='coloque peso para ver variacao';
    }
  }).catch(function(){D.getElementById('dot').className='dot off'})
  .finally(function(){setTimeout(poll,200)});
})();
function doTare(){
  fetch('/scale/tare',{method:'POST'}).then(function(){
    var m=D.getElementById('tare-msg');m.textContent='Tarado!';setTimeout(function(){m.textContent=''},2000);
  }).catch(function(){D.getElementById('tare-msg').textContent='Erro ao tarar.'});
}
</script></body></html>
)html";

// ── config page ──────────────────────────────────────────────────────────────
static const char CONFIG_HTML[] = R"html(
<!DOCTYPE html><html><head>
<meta charset="utf-8">
<meta name="viewport" content="width=device-width,initial-scale=1">
<title>Configurações</title>
<style>
:root{--bg:#f0f2f5;--card:#fff;--text:#1a1a2e;--sub:#666;--bdr:#ddd;--hov:#eef2ff;--inp:#fff;--btn:#0066cc;--bh:#0052a3;--rs:#888;--dan:#cc2200;--danh:#aa1a00;--ok-bg:#d4edda;--ok-c:#155724;--er-bg:#f8d7da;--er-c:#721c24;--tab-act:#0066cc;--tab-act-t:#fff;--tab-in:#e8edf5;--tab-in-t:#555}
html.dark{--bg:#0f1117;--card:#1e2130;--text:#e0e6f0;--sub:#8892a4;--bdr:#2e3548;--hov:#252d40;--inp:#252a3a;--btn:#4d8ef0;--bh:#3a7ae0;--rs:#6b7280;--dan:#e05540;--danh:#c94030;--ok-bg:#1a3a25;--ok-c:#6ee08a;--er-bg:#3a1a1a;--er-c:#f08080;--tab-act:#4d8ef0;--tab-act-t:#fff;--tab-in:#1a2035;--tab-in-t:#8892a4}
*{box-sizing:border-box}
body{font-family:sans-serif;max-width:440px;margin:0 auto;padding:.8em;background:var(--bg);color:var(--text)}
.hdr{display:flex;justify-content:space-between;align-items:center;margin-bottom:.7em}
h2{margin:0;font-size:1.2em}
.thm{background:none;border:1px solid var(--sub);border-radius:20px;padding:.3em .85em;cursor:pointer;color:var(--text);font-size:.82em}
.back{color:var(--btn);text-decoration:none;font-size:.9em}
.tabs{display:flex;gap:.3em;margin-bottom:.8em;background:var(--tab-in);border-radius:10px;padding:.3em}
.tab{flex:1;padding:.55em .2em;border:none;border-radius:7px;cursor:pointer;font-size:.78em;font-weight:600;transition:background .15s,color .15s;background:transparent;color:var(--tab-in-t)}
.tab.active{background:var(--tab-act);color:var(--tab-act-t);box-shadow:0 1px 3px rgba(0,0,0,.15)}
.pane{display:none}.pane.active{display:block}
.card{background:var(--card);border-radius:10px;padding:1em;margin:.6em 0;box-shadow:0 1px 4px rgba(0,0,0,.1)}
.net{display:flex;justify-content:space-between;align-items:center;padding:.55em .4em;border-radius:6px;cursor:pointer;transition:background .15s}
.net:hover,.net:active{background:var(--hov)}
.net.active-net{background:var(--hov);border:1px solid var(--btn)}
.rs{font-size:.78em;color:var(--rs);white-space:nowrap;padding-left:.5em}
.cur{font-size:.75em;background:var(--btn);color:#fff;border-radius:10px;padding:.15em .5em;margin-left:.4em;white-space:nowrap}
label{display:block;font-size:.82em;color:var(--sub);margin:.7em 0 .2em}
input[type=text],input[type=password],input[type=number]{width:100%;padding:.55em .7em;border:1px solid var(--bdr);border-radius:6px;font-size:1em;background:var(--inp);color:var(--text);outline:none}
input:focus{border-color:var(--btn)}
.btn{width:100%;padding:.7em;background:var(--btn);color:#fff;border:none;border-radius:6px;font-size:1em;cursor:pointer;margin-top:.5em;transition:background .15s}
.btn:active{background:var(--bh)}
.btn.dan{background:var(--dan)}
.btn.dan:active{background:var(--danh)}
.sep{border:none;border-top:1px solid var(--bdr);margin:.8em 0}
.msg{padding:.7em;border-radius:6px;text-align:center;margin-top:.5em;font-weight:bold;display:none}
.ok{background:var(--ok-bg);color:var(--ok-c)}
.err{background:var(--er-bg);color:var(--er-c)}
#confirm{display:none;background:var(--er-bg);border-radius:8px;padding:.9em;margin-top:.6em;text-align:center}
#confirm p{margin:0 0 .6em;font-weight:bold;color:var(--er-c)}
#confirm .row{display:flex;gap:.5em}
#confirm button{flex:1;padding:.6em;border:none;border-radius:6px;cursor:pointer;font-size:.95em}
#confirm .yes{background:var(--dan);color:#fff}
#confirm .no{background:var(--bdr);color:var(--text)}
p.hint{color:var(--sub);font-size:.83em;margin:.2em 0 .5em}
</style></head><body>
<div class="hdr">
  <a class="back" href="/">&#8592; Voltar</a>
  <h2>Configurações</h2>
  <button class="thm" id="thm" onclick="tog()"></button>
</div>

<div class="tabs">
  <button class="tab active" onclick="showTab('balanca',this)">&#9878; Balança</button>
  <button class="tab" onclick="showTab('leds',this)">&#128161; LEDs</button>
  <button class="tab" onclick="showTab('wifi',this)">&#128246; WiFi</button>
  <button class="tab" onclick="showTab('device',this)">&#9881; Dispositivo</button>
</div>

<!-- Balança -->
<div id="pane-balanca" class="pane active">
  <div class="card">
    <p class="hint">Peso atual: <strong id="wval">--</strong></p>
    <button class="btn" onclick="doTare()">Tarar (zerar)</button>
    <hr class="sep">
    <p class="hint"><b>Calibração:</b> com a plataforma vazia clique em Tarar acima, depois coloque o peso conhecido e informe o valor abaixo.</p>
    <input id="cal-weight" type="number" min="1" max="20000" step="0.1" placeholder="Peso conhecido (g), ex: 1000">
    <button class="btn" onclick="doCalibrate()">Calibrar com esse peso</button>
    <div class="msg" id="scale-msg"></div>
  </div>
  <div class="card">
    <p style="font-size:.78em;color:var(--sub);text-transform:uppercase;letter-spacing:.05em;margin:0 0 .8em;font-weight:600">Filtro de leitura</p>
    <label>Suavização <span id="ema-val" style="font-weight:bold"></span>
      <span style="font-size:.78em;color:var(--sub)"> — menor = mais estável, maior = reage mais rápido</span>
    </label>
    <input id="ema" type="range" min="1" max="50" style="width:100%;margin:.2em 0" oninput="updFilter('ema-val',this.value+'%')">
    <label>Sensibilidade <span id="dead-val" style="font-weight:bold"></span>
      <span style="font-size:.78em;color:var(--sub)"> — variação mínima em gramas para atualizar o display</span>
    </label>
    <input id="dead" type="range" min="1" max="50" style="width:100%;margin:.2em 0" oninput="updFilter('dead-val',this.value/10+'g')">
    <label>Zona morta no zero <span id="snap-val" style="font-weight:bold"></span>
      <span style="font-size:.78em;color:var(--sub)"> — trava em 0g quando vazio até esse valor</span>
    </label>
    <input id="snap" type="range" min="1" max="100" style="width:100%;margin:.2em 0" oninput="updFilter('snap-val',this.value/10+'g')">
    <label>Velocidade de leitura <span id="samp-val" style="font-weight:bold"></span>
      <span style="font-size:.78em;color:var(--sub)"> — amostras acumuladas antes de atualizar (menos = mais rápido)</span>
    </label>
    <input id="samp" type="range" min="1" max="20" style="width:100%;margin:.2em 0" oninput="updFilter('samp-val',this.value)">
    <button class="btn" onclick="saveFilter()" style="margin-top:.7em">Salvar configurações do filtro</button>
    <div class="msg" id="filter-msg"></div>
  </div>
</div>

<!-- LEDs -->
<div id="pane-leds" class="pane">
  <div class="card">
    <div style="display:flex;align-items:center;gap:.5em;margin-bottom:.3em">
      <label style="flex:1;margin:0">Brilho <span id="bval" style="font-weight:bold"></span></label>
      <button id="prev-btn" title="Mostrar nos LEDs" onclick="togglePreview()"
        style="background:none;border:none;cursor:pointer;font-size:1.3em;padding:.1em;opacity:.4">&#128161;</button>
      <button title="Salvar brilho" onclick="saveBright(D.getElementById('bright').value)"
        style="background:none;border:none;cursor:pointer;font-size:1.3em;padding:.1em">&#128190;</button>
    </div>
    <input id="bright" type="range" min="10" max="255" style="width:100%;margin:.2em 0" oninput="onSlide(this.value)">
    <div class="msg" id="led-msg"></div>
  </div>
  <div class="card">
    <p style="font-size:.78em;color:var(--sub);text-transform:uppercase;letter-spacing:.05em;margin:0 0 .7em;font-weight:600">Guia de cores</p>
    <table style="width:100%;border-collapse:collapse;font-size:.85em">
      <thead>
        <tr style="border-bottom:1px solid var(--bdr)">
          <th style="text-align:left;padding:.4em .5em;color:var(--sub);font-weight:600;width:36px">Cor</th>
          <th style="text-align:left;padding:.4em .5em;color:var(--sub);font-weight:600;width:38%">Status</th>
          <th style="text-align:left;padding:.4em .5em;color:var(--sub);font-weight:600">Significado</th>
        </tr>
      </thead>
      <tbody>
        <tr style="border-bottom:1px solid var(--bdr)">
          <td style="padding:.55em .5em"><div style="width:13px;height:13px;border-radius:50%;background:#ffffff;border:1px solid var(--bdr);margin:auto"></div></td>
          <td style="padding:.55em .5em;font-weight:500">Branco</td>
          <td style="padding:.55em .5em;color:var(--sub)">Inicializando o dispositivo</td>
        </tr>
        <tr style="border-bottom:1px solid var(--bdr)">
          <td style="padding:.55em .5em"><div style="width:13px;height:13px;border-radius:50%;background:#22c55e;margin:auto;animation:blink 1s step-start infinite"></div></td>
          <td style="padding:.55em .5em;font-weight:500">Verde piscando</td>
          <td style="padding:.55em .5em;color:var(--sub)">Conectando ao WiFi</td>
        </tr>
        <tr style="border-bottom:1px solid var(--bdr)">
          <td style="padding:.55em .5em"><div style="width:13px;height:13px;border-radius:50%;background:#22c55e;margin:auto"></div></td>
          <td style="padding:.55em .5em;font-weight:500">Verde fixo → apaga</td>
          <td style="padding:.55em .5em;color:var(--sub)">WiFi conectado com sucesso</td>
        </tr>
        <tr style="border-bottom:1px solid var(--bdr)">
          <td style="padding:.55em .5em"><div style="width:13px;height:13px;border-radius:50%;background:#fbbf24;margin:auto;animation:breathe 3s ease-in-out infinite"></div></td>
          <td style="padding:.55em .5em;font-weight:500">Amarelo respirando</td>
          <td style="padding:.55em .5em;color:var(--sub)">Modo configuração — sem WiFi, acesse BalancaC3-Config</td>
        </tr>
        <tr style="border-bottom:1px solid var(--bdr)">
          <td style="padding:.55em .5em"><div style="width:13px;height:13px;border-radius:50%;background:#3b82f6;margin:auto"></div></td>
          <td style="padding:.55em .5em;font-weight:500">Azul em barra</td>
          <td style="padding:.55em .5em;color:var(--sub)">Atualização de firmware (OTA) em andamento</td>
        </tr>
        <tr style="border-bottom:1px solid var(--bdr)">
          <td style="padding:.55em .5em"><div style="width:13px;height:13px;border-radius:50%;background:#ef4444;margin:auto;animation:blink .4s step-start infinite"></div></td>
          <td style="padding:.55em .5em;font-weight:500">Vermelho piscando</td>
          <td style="padding:.55em .5em;color:var(--sub)">Erro na atualização OTA</td>
        </tr>
        <tr style="border-bottom:1px solid var(--bdr)">
          <td style="padding:.55em .5em"><div style="width:13px;height:13px;border-radius:50%;background:#22c55e;margin:auto"></div></td>
          <td style="padding:.55em .5em;font-weight:500">Verde (botão 1)</td>
          <td style="padding:.55em .5em;color:var(--sub)">Botão pressionado — inicia ou pausa o timer</td>
        </tr>
        <tr>
          <td style="padding:.55em .5em"><div style="width:13px;height:13px;border-radius:50%;background:#ef4444;margin:auto"></div></td>
          <td style="padding:.55em .5em;font-weight:500">Vermelho (botão 2)</td>
          <td style="padding:.55em .5em;color:var(--sub)">Botão pressionado — executa a tara da balança</td>
        </tr>
      </tbody>
    </table>
  </div>
</div>
<style>
@keyframes blink{0%,100%{opacity:1}50%{opacity:0}}
@keyframes breathe{0%,100%{opacity:.25}50%{opacity:1}}
</style>

<!-- Dispositivo -->
<div id="pane-device" class="pane">
  <div class="card">
    <a class="btn" href="/update" style="display:block;text-align:center;text-decoration:none;padding:.7em;background:var(--btn);color:#fff;border-radius:6px">&#128190; Atualizar Firmware (OTA)</a>
    <button class="btn" onclick="restartDevice()" style="margin-top:.5em">Reiniciar dispositivo</button>
    <div class="msg" id="dev-msg"></div>
    <hr class="sep">
    <button class="btn dan" onclick="showConfirm()">Apagar credenciais e reiniciar</button>
    <div id="confirm">
      <p>Isso apagará o WiFi salvo e reiniciará o dispositivo.</p>
      <div class="row">
        <button class="yes" onclick="doReset()">Sim, apagar</button>
        <button class="no" onclick="hideConfirm()">Cancelar</button>
      </div>
    </div>
  </div>
</div>

<!-- WiFi -->
<div id="pane-wifi" class="pane">
  <div class="card">
    <div id="nets">Escaneando redes...</div>
    <label>Rede (SSID)</label>
    <input id="ssid" type="text" placeholder="Nome da rede ou rede oculta" autocomplete="off" spellcheck="false">
    <label>Senha</label>
    <input id="pw" type="password" placeholder="Senha da rede" autocomplete="current-password">
    <button class="btn" onclick="saveWifi()">Salvar e reconectar</button>
    <div class="msg" id="wifi-msg"></div>
  </div>
</div>

<script>
var D=document,H=D.documentElement;
function applyDark(d){H.classList.toggle('dark',d);D.getElementById('thm').textContent=d?'Claro':'Escuro'}
function tog(){var d=!H.classList.contains('dark');localStorage.setItem('t',d?'1':'0');applyDark(d)}
(function(){var s=localStorage.getItem('t');applyDark(s!=null?s==='1':window.matchMedia('(prefers-color-scheme:dark)').matches)})();

function showTab(id,btn){
  D.querySelectorAll('.pane').forEach(function(p){p.classList.remove('active')});
  D.querySelectorAll('.tab').forEach(function(b){b.classList.remove('active')});
  D.getElementById('pane-'+id).classList.add('active');
  btn.classList.add('active');
  localStorage.setItem('tab',id);
  if(id==='wifi' && !wifiLoaded) loadWifi();
}
// Restore last tab
(function(){var t=localStorage.getItem('tab');if(t){var btn=D.querySelector('.tab[onclick*="\''+t+'\'"]');if(btn) showTab(t,btn);}})();

// Scale polling
var wifiLoaded=false;
(function pollWeight(){
  fetch('/scale/weight').then(function(r){return r.json()}).then(function(d){
    var w=d.calibrated?(d.grams>=1000||d.grams<=-1000?(d.grams/1000).toFixed(3)+' kg':d.grams.toFixed(1)+' g'):'Sem calibracao';
    D.getElementById('wval').textContent=w;
  }).catch(function(){}).finally(function(){setTimeout(pollWeight,200)});
})();
function showMsg(id,ok,txt){var m=D.getElementById(id);m.className='msg '+(ok?'ok':'err');m.textContent=txt;m.style.display='block';if(ok)setTimeout(function(){m.style.display='none'},3000)}
function doTare(){fetch('/scale/tare',{method:'POST'}).then(function(){showMsg('scale-msg',true,'Tarado!')}).catch(function(){showMsg('scale-msg',false,'Erro ao tarar.')})}
function updFilter(id,v){D.getElementById(id).textContent=v}
function saveFilter(){
  var ema=D.getElementById('ema').value/100;
  var dead=D.getElementById('dead').value/10;
  var snap=D.getElementById('snap').value/10;
  var samp=D.getElementById('samp').value;
  fetch('/scale/filter',{method:'POST',headers:{'Content-Type':'application/x-www-form-urlencoded'},
    body:'ema='+ema+'\x26deadband='+dead+'\x26snap='+snap+'\x26samples='+samp})
  .then(function(){showMsg('filter-msg',true,'Filtro salvo!')})
  .catch(function(){showMsg('filter-msg',false,'Erro ao salvar.')});
}
// Load filter config
fetch('/scale/filter').then(function(r){return r.json()}).then(function(f){
  var e=Math.round(f.ema*100);var d=Math.round(f.deadband*10);var s=Math.round(f.snap*10);var sp=f.samples;
  D.getElementById('ema').value=e;updFilter('ema-val',e+'%');
  D.getElementById('dead').value=d;updFilter('dead-val',d/10+'g');
  D.getElementById('snap').value=s;updFilter('snap-val',s/10+'g');
  D.getElementById('samp').value=sp;updFilter('samp-val',sp);
}).catch(function(){});
function doCalibrate(){
  var w=parseFloat(D.getElementById('cal-weight').value);
  if(!w||w<=0){D.getElementById('cal-weight').focus();return}
  fetch('/scale/calibrate',{method:'POST',headers:{'Content-Type':'application/x-www-form-urlencoded'},body:'weight='+w})
  .then(function(r){return r.text()}).then(function(){showMsg('scale-msg',true,'Calibrado com '+w+'g!')})
  .catch(function(){showMsg('scale-msg',false,'Erro ao calibrar.')});
}

// LED
fetch('/config/led').then(function(r){return r.text()}).then(function(v){
  var s=D.getElementById('bright');s.value=v;D.getElementById('bval').textContent=v;
}).catch(function(){});
var previewOn=false;
function togglePreview(){
  previewOn=!previewOn;
  var btn=D.getElementById('prev-btn');btn.style.opacity=previewOn?'1':'0.4';
  if(!previewOn) fetch('/config/led/preview/stop',{method:'POST'}).catch(function(){});
  else sendPreview(D.getElementById('bright').value);
}
function sendPreview(v){if(previewOn) fetch('/config/led/preview',{method:'POST',headers:{'Content-Type':'application/x-www-form-urlencoded'},body:'brightness='+v}).catch(function(){})}
function onSlide(v){D.getElementById('bval').textContent=v;sendPreview(v);}
function saveBright(v){
  fetch('/config/led',{method:'POST',headers:{'Content-Type':'application/x-www-form-urlencoded'},body:'brightness='+v})
  .then(function(){showMsg('led-msg',true,'Brilho salvo!');if(previewOn){previewOn=false;D.getElementById('prev-btn').style.opacity='0.4';fetch('/config/led/preview/stop',{method:'POST'}).catch(function(){});}})
  .catch(function(){showMsg('led-msg',false,'Erro ao salvar.');});
}

// Device
function restartDevice(){fetch('/config/restart',{method:'POST'}).catch(function(){});showMsg('dev-msg',true,'Reiniciando...')}
function showConfirm(){D.getElementById('confirm').style.display='block'}
function hideConfirm(){D.getElementById('confirm').style.display='none'}
function doReset(){hideConfirm();fetch('/config/reset',{method:'POST'}).catch(function(){});showMsg('dev-msg',true,'Credenciais apagadas. Reiniciando...')}

// WiFi — lazy load only when tab is opened
function loadWifi(){
  wifiLoaded=true;
  var sig=function(r){return r>-60?'&#9602;&#9604;&#9606;&#9608;':r>-70?'&#9602;&#9604;&#9606;_':r>-80?'&#9602;&#9604;__':'&#9602;___'};
  Promise.all([fetch('/networks').then(function(r){return r.json()}),fetch('/config/ssid').then(function(r){return r.text()})])
  .then(function(res){
    var ns=res[0],cur=res[1].trim(),d=D.getElementById('nets');
    if(!ns||!ns.length){d.textContent='Nenhuma rede encontrada.';return}
    d.innerHTML='';
    ns.forEach(function(n){
      var isCur=(n.ssid===cur);
      var e=D.createElement('div');e.className='net'+(isCur?' active-net':'');
      var badge=isCur?'<span class="cur">&#10003; conectada</span>':'';
      e.innerHTML='<span>'+n.ssid+badge+'</span><span class="rs">'+sig(n.rssi)+' '+n.rssi+'</span>';
      e.onclick=function(){D.getElementById('ssid').value=n.ssid;D.getElementById('pw').value='';D.getElementById('pw').focus()};
      d.appendChild(e);
    });
  }).catch(function(){D.getElementById('nets').textContent='Falha no scan.'});
}
function saveWifi(){
  var ssid=D.getElementById('ssid').value.trim(),pw=D.getElementById('pw').value;
  if(!ssid){D.getElementById('ssid').focus();return}
  fetch('/config/wifi',{method:'POST',headers:{'Content-Type':'application/x-www-form-urlencoded'},body:'ssid='+encodeURIComponent(ssid)+'&password='+encodeURIComponent(pw)})
  .then(function(r){return r.text()}).then(function(){showMsg('wifi-msg',true,'Salvo! Reconectando...')})
  .catch(function(){showMsg('wifi-msg',false,'Erro ao salvar.')});
}
</script></body></html>
)html";

namespace core {

WebApp::WebApp(hal::IWebServer& server, hal::IWifi& wifi, hal::IStorage& storage,
               hal::IDevice& device, events::EventBus& eventBus)
    : _server(server), _wifi(wifi), _storage(storage),
      _device(device), _eventBus(eventBus) {}

void WebApp::begin() {
    _eventBus.subscribe(events::EventType::WifiConnected,      this);
    _eventBus.subscribe(events::EventType::WifiConfigRequired, this);
    _eventBus.subscribe(events::EventType::WifiDisconnected,   this);
}

void WebApp::loop() {
    if (_running) _server.handle();

    if (_pendingRestart && millis() >= _restartAt) {
        _pendingRestart = false;
        _device.restart();
    }
}

void WebApp::onEvent(const events::Event& event) {
    switch (event.type) {
        case events::EventType::WifiConnected:
            startServer(static_cast<const char*>(event.payload));
            break;
        case events::EventType::WifiConfigRequired:
        case events::EventType::WifiDisconnected:
            stopServer();
            break;
        default:
            break;
    }
}

void WebApp::registerRoutes() {
    _server.on("/",               [this]() { handleRoot(); });
    _server.on("/config",         [this]() { handleConfig(); });
    _server.on("/networks",       [this]() { handleNetworks(); });
    _server.on("/config/wifi",    [this]() { handleConfigWifi(); });
    _server.on("/config/restart", [this]() { handleConfigRestart(); });
    _server.on("/config/reset",   [this]() { handleConfigReset(); });
    _server.on("/config/ssid",    [this]() { handleCurrentSsid(); });
    _server.on("/scale/weight",   [this]() { handleScaleWeight(); });
    _server.on("/scale/tare",     [this]() { handleScaleTare(); });
    _server.on("/scale/calibrate",[this]() { handleScaleCalibrateStep2(); });
    _server.on("/scale/filter",   [this]() {
        if (_server.arg("ema").isEmpty() && _server.arg("deadband").isEmpty())
            handleScaleFilterGet();
        else
            handleScaleFilterSet();
    });
    _server.on("/config/led",         [this]() {
        if (_server.arg("brightness").isEmpty()) handleConfigLedGet();
        else handleConfigLedSet();
    });
    _server.on("/config/led/preview",      [this]() { handleConfigLedPreview(); });
    _server.on("/config/led/preview/stop", [this]() { handleConfigLedPreviewStop(); });
    _routesRegistered = true;
}

void WebApp::startServer(const char* ip) {
    if (!_routesRegistered) registerRoutes();
    _server.begin(80);
    _running = true;
    _eventBus.publish({events::EventType::WebServerStarted, ip});
}

void WebApp::stopServer() {
    if (!_running) return;
    _server.stop();
    _running = false;
}

void WebApp::handleRoot() {
    _server.send(200, "text/html", ROOT_HTML);
}

void WebApp::handleConfig() {
    _server.send(200, "text/html", CONFIG_HTML);
}

void WebApp::handleNetworks() {
    uint8_t count = _wifi.scanNetworks();
    String json = "[";
    for (uint8_t i = 0; i < count; ++i) {
        if (i > 0) json += ",";
        json += "{\"ssid\":\"";
        json += _wifi.scannedSSID(i);
        json += "\",\"rssi\":";
        json += _wifi.scannedRSSI(i);
        json += "}";
    }
    json += "]";
    _server.send(200, "application/json", json.c_str());
}

void WebApp::handleConfigWifi() {
    String ssid = _server.arg("ssid");
    String pass = _server.arg("password");
    if (ssid.isEmpty()) { _server.send(400, "text/plain", "SSID obrigatorio"); return; }
    _storage.putString(STORAGE_KEY_WIFI_SSID, ssid.c_str());
    _storage.putString(STORAGE_KEY_WIFI_PASS, pass.c_str());
    _server.send(200, "text/plain", "OK");
    _eventBus.publish({events::EventType::WifiCredentialsSaved});
    _pendingRestart = true;
    _restartAt = millis() + 800;
}

void WebApp::handleConfigRestart() {
    _server.send(200, "text/plain", "OK");
    _pendingRestart = true;
    _restartAt = millis() + 800;
}

void WebApp::handleCurrentSsid() {
    char ssid[33] = {};
    _storage.getString(STORAGE_KEY_WIFI_SSID, ssid, sizeof(ssid));
    _server.send(200, "text/plain", ssid);
}

void WebApp::handleScaleFilterGet() {
    if (!_scale) { _server.send(200, "application/json", "{}"); return; }
    auto cfg = _scale->filterConfig();
    char buf[128];
    snprintf(buf, sizeof(buf),
        "{\"ema\":%.3f,\"deadband\":%.2f,\"snap\":%.2f,\"samples\":%u}",
        cfg.emaAlpha, cfg.deadbandG, cfg.zeroSnapG, cfg.samples);
    _server.send(200, "application/json", buf);
}

void WebApp::handleScaleFilterSet() {
    if (!_scale) { _server.send(400, "text/plain", "no scale"); return; }
    auto cfg = _scale->filterConfig();
    String v;
    v = _server.arg("ema");      if (!v.isEmpty()) { float f = v.toFloat(); if (f > 0.0f && f <= 1.0f) cfg.emaAlpha  = f; }
    v = _server.arg("deadband"); if (!v.isEmpty()) { float f = v.toFloat(); if (f >= 0.0f) cfg.deadbandG = f; }
    v = _server.arg("snap");     if (!v.isEmpty()) { float f = v.toFloat(); if (f >= 0.0f) cfg.zeroSnapG = f; }
    v = _server.arg("samples");  if (!v.isEmpty()) { int   i = v.toInt();   if (i >= 1 && i <= 20) cfg.samples = static_cast<uint8_t>(i); }
    _scale->setFilterConfig(cfg, true);
    _server.send(200, "text/plain", "OK");
}

void WebApp::handleScaleWeight() {
    float   grams      = _scale ? _scale->lastWeight()   : 0.0f;
    int32_t raw        = _scale ? _scale->lastRaw()      : 0;
    bool    calibrated = _scale ? _scale->isCalibrated() : false;
    bool    ready      = _scale ? _scale->isReady()      : false;
    char    buf[96];
    snprintf(buf, sizeof(buf),
             "{\"grams\":%.2f,\"raw\":%ld,\"calibrated\":%s,\"ready\":%s}",
             grams, (long)raw,
             calibrated ? "true" : "false",
             ready      ? "true" : "false");
    _server.send(200, "application/json", buf);
}

void WebApp::handleScaleTare() {
    if (_scale) _scale->commandTare();
    _server.send(200, "text/plain", "OK");
}

void WebApp::handleScaleCalibrateStep1() {
    if (_scale) _scale->commandCalibrateStep1();
    _server.send(200, "text/plain", "OK");
}

void WebApp::handleScaleCalibrateStep2() {
    String wStr = _server.arg("weight");
    if (wStr.isEmpty()) { _server.send(400, "text/plain", "missing weight"); return; }
    float w = wStr.toFloat();
    if (w <= 0.0f) { _server.send(400, "text/plain", "invalid weight"); return; }
    if (_scale) {
        _scale->commandCalibrateStep1(); // capture raw with weight on
        _scale->commandCalibrateStep2(w);
    }
    _server.send(200, "text/plain", "OK");
}

void WebApp::handleConfigLedGet() {
    char buf[8] = {};
    if (!_storage.getString(STORAGE_KEY_LED_BRIGHTNESS, buf, sizeof(buf))) {
        snprintf(buf, sizeof(buf), "%u", LED_BRIGHTNESS_DEFAULT);
    }
    _server.send(200, "text/plain", buf);
}

void WebApp::handleConfigLedSet() {
    String val = _server.arg("brightness");
    if (val.isEmpty()) { _server.send(400, "text/plain", "missing brightness"); return; }
    int v = val.toInt();
    if (v < 10 || v > 255) { _server.send(400, "text/plain", "out of range"); return; }
    _storage.putString(STORAGE_KEY_LED_BRIGHTNESS, val.c_str());
    static uint8_t bright;
    bright = static_cast<uint8_t>(v);
    _eventBus.publish({events::EventType::LedBrightnessChanged, &bright});
    _server.send(200, "text/plain", "OK");
}

void WebApp::handleConfigLedPreview() {
    String val = _server.arg("brightness");
    if (val.isEmpty()) { _server.send(400, "text/plain", "missing brightness"); return; }
    int v = val.toInt();
    if (v < 10 || v > 255) { _server.send(400, "text/plain", "out of range"); return; }
    static uint8_t bright;
    bright = static_cast<uint8_t>(v);
    _eventBus.publish({events::EventType::LedPreviewChanged, &bright});
    _server.send(200, "text/plain", "OK");
}

void WebApp::handleConfigLedPreviewStop() {
    _eventBus.publish({events::EventType::LedPreviewStopped});
    _server.send(200, "text/plain", "OK");
}

void WebApp::handleConfigReset() {
    _storage.remove(STORAGE_KEY_WIFI_SSID);
    _storage.remove(STORAGE_KEY_WIFI_PASS);
    _server.send(200, "text/plain", "OK");
    _eventBus.publish({events::EventType::WifiCredentialsCleared});
    _pendingRestart = true;
    _restartAt = millis() + 800;
}

} // namespace core
