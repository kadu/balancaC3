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
:root{--bg:#f0f2f5;--card:#fff;--text:#1a1a2e;--sub:#666;--bdr:#ddd;--hov:#eef2ff;--inp:#fff;--btn:#0066cc;--bh:#0052a3;--rs:#888;--dan:#cc2200;--danh:#aa1a00;--ok-bg:#d4edda;--ok-c:#155724;--er-bg:#f8d7da;--er-c:#721c24}
html.dark{--bg:#0f1117;--card:#1e2130;--text:#e0e6f0;--sub:#8892a4;--bdr:#2e3548;--hov:#252d40;--inp:#252a3a;--btn:#4d8ef0;--bh:#3a7ae0;--rs:#6b7280;--dan:#e05540;--danh:#c94030;--ok-bg:#1a3a25;--ok-c:#6ee08a;--er-bg:#3a1a1a;--er-c:#f08080}
*{box-sizing:border-box}
body{font-family:sans-serif;max-width:440px;margin:0 auto;padding:1em;background:var(--bg);color:var(--text)}
.hdr{display:flex;justify-content:space-between;align-items:center;margin-bottom:.4em}
h2{margin:0;font-size:1.2em}
.thm{background:none;border:1px solid var(--sub);border-radius:20px;padding:.3em .85em;cursor:pointer;color:var(--text);font-size:.82em}
.back{color:var(--btn);text-decoration:none;font-size:.9em}
.card{background:var(--card);border-radius:10px;padding:1em;margin:.7em 0;box-shadow:0 1px 4px rgba(0,0,0,.1)}
.card h3{margin:.1em 0 .7em;font-size:.78em;color:var(--sub);text-transform:uppercase;letter-spacing:.05em}
.net{display:flex;justify-content:space-between;align-items:center;padding:.55em .4em;border-radius:6px;cursor:pointer;transition:background .15s}
.net:hover,.net:active{background:var(--hov)}
.net.active-net{background:var(--hov);border:1px solid var(--btn)}
.rs{font-size:.78em;color:var(--rs);white-space:nowrap;padding-left:.5em}
.cur{font-size:.75em;background:var(--btn);color:#fff;border-radius:10px;padding:.15em .5em;margin-left:.4em;white-space:nowrap}
label{display:block;font-size:.82em;color:var(--sub);margin:.7em 0 .2em}
input[type=text],input[type=password]{width:100%;padding:.55em .7em;border:1px solid var(--bdr);border-radius:6px;font-size:1em;background:var(--inp);color:var(--text);outline:none}
input:focus{border-color:var(--btn)}
.btn{width:100%;padding:.7em;background:var(--btn);color:#fff;border:none;border-radius:6px;font-size:1em;cursor:pointer;margin-top:.6em;transition:background .15s}
.btn:active{background:var(--bh)}
.btn.dan{background:var(--dan)}
.btn.dan:active{background:var(--danh)}
.sep{border:none;border-top:1px solid var(--bdr);margin:.9em 0}
.msg{padding:.75em;border-radius:6px;text-align:center;margin-top:.6em;font-weight:bold;display:none}
.ok{background:var(--ok-bg);color:var(--ok-c)}
.err{background:var(--er-bg);color:var(--er-c)}
#confirm{display:none;background:var(--er-bg);border-radius:8px;padding:.9em;margin-top:.6em;text-align:center}
#confirm p{margin:0 0 .6em;font-weight:bold;color:var(--er-c)}
#confirm .row{display:flex;gap:.5em}
#confirm button{flex:1;padding:.6em;border:none;border-radius:6px;cursor:pointer;font-size:.95em}
#confirm .yes{background:var(--dan);color:#fff}
#confirm .no{background:var(--bdr);color:var(--text)}
</style></head><body>
<div class="hdr">
  <a class="back" href="/">&#8592; Voltar</a>
  <h2>Configurações</h2>
  <button class="thm" id="thm" onclick="tog()"></button>
</div>
<div class="card">
  <h3>Rede WiFi</h3>
  <div id="nets">Escaneando redes...</div>
  <label>Rede (SSID)</label>
  <input id="ssid" type="text" placeholder="Nome da rede ou rede oculta" autocomplete="off" spellcheck="false">
  <label>Senha</label>
  <input id="pw" type="password" placeholder="Senha da rede" autocomplete="current-password">
  <button class="btn" onclick="saveWifi()">Salvar e reconectar</button>
  <div class="msg" id="wifi-msg"></div>
</div>
<div class="card">
  <h3>Balanca</h3>
  <p style="color:var(--sub);font-size:.85em;margin:.2em 0 .6em">Peso atual: <strong id="wval">--</strong></p>
  <button class="btn" onclick="doTare()" style="margin-bottom:.5em">Tarar (zerar)</button>
  <hr class="sep">
  <p style="color:var(--sub);font-size:.85em;margin:.2em 0 .4em">Calibracao de dois passos:</p>
  <p style="color:var(--sub);font-size:.82em;margin:0 0 .4em"><b>Passo 1:</b> plataforma vazia → Tarar acima.<br><b>Passo 2:</b> coloque o peso conhecido e clique Calibrar.</p>
  <input id="cal-weight" type="number" min="1" max="20000" step="0.1" placeholder="Peso conhecido (g), ex: 1000" style="width:100%;padding:.55em .7em;border:1px solid var(--bdr);border-radius:6px;font-size:1em;background:var(--inp);color:var(--text);margin-bottom:.4em">
  <button class="btn" onclick="doCalibrate()">Calibrar com esse peso</button>
  <div class="msg" id="scale-msg"></div>
</div>
<div class="card">
  <h3>LEDs</h3>
  <label>Brilho (<span id="bval"></span>)</label>
  <input id="bright" type="range" min="10" max="255" style="width:100%;margin:.3em 0"
    oninput="D.getElementById('bval').textContent=this.value" onchange="saveBright(this.value)">
  <div class="msg" id="led-msg"></div>
</div>
<div class="card">
  <h3>Dispositivo</h3>
  <a class="btn" href="/update" style="display:block;text-align:center;text-decoration:none;padding:.7em;background:var(--btn);color:#fff;border-radius:6px;margin-bottom:.4em">&#128190; Atualizar Firmware (OTA)</a>
  <button class="btn" onclick="restartDevice()">Reiniciar dispositivo</button>
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
<script>
var D=document,H=D.documentElement;
function applyDark(d){H.classList.toggle('dark',d);D.getElementById('thm').textContent=d?'Claro':'Escuro'}
function tog(){var d=!H.classList.contains('dark');localStorage.setItem('t',d?'1':'0');applyDark(d)}
(function(){var s=localStorage.getItem('t');applyDark(s!=null?s==='1':window.matchMedia('(prefers-color-scheme:dark)').matches)})();

function sig(r){return r>-60?'&#9602;&#9604;&#9606;&#9608;':r>-70?'&#9602;&#9604;&#9606;_':r>-80?'&#9602;&#9604;__':'&#9602;___'}
var currentSsid='';
Promise.all([
  fetch('/networks').then(function(r){return r.json()}),
  fetch('/config/ssid').then(function(r){return r.text()})
]).then(function(res){
  var ns=res[0]; currentSsid=res[1].trim();
  var d=D.getElementById('nets');
  if(!ns||!ns.length){d.textContent='Nenhuma rede encontrada.';return}
  d.innerHTML='';
  ns.forEach(function(n){
    var isCur=(n.ssid===currentSsid);
    var e=D.createElement('div');
    e.className='net'+(isCur?' active-net':'');
    var badge=isCur?'<span class="cur">&#10003; conectada</span>':'';
    e.innerHTML='<span>'+n.ssid+badge+'</span><span class="rs">'+sig(n.rssi)+' '+n.rssi+'</span>';
    e.onclick=function(){D.getElementById('ssid').value=n.ssid;D.getElementById('pw').value='';D.getElementById('pw').focus()};
    d.appendChild(e);
  });
}).catch(function(){D.getElementById('nets').textContent='Falha no scan.'});

function showMsg(id,ok,txt){var m=D.getElementById(id);m.className='msg '+(ok?'ok':'err');m.textContent=txt;m.style.display='block'}
fetch('/config/led').then(function(r){return r.text()}).then(function(v){
  var s=D.getElementById('bright');s.value=v;D.getElementById('bval').textContent=v;
}).catch(function(){});
// Scale
(function pollWeight(){
  fetch('/scale/weight').then(function(r){return r.json()}).then(function(d){
    var w=d.calibrated?(d.grams>=1000||d.grams<=-1000?(d.grams/1000).toFixed(3)+' kg':d.grams.toFixed(1)+' g'):'Sem calibracao';
    D.getElementById('wval').textContent=w;
  }).catch(function(){}).finally(function(){setTimeout(pollWeight,200)});
})();
function doTare(){
  fetch('/scale/tare',{method:'POST'}).then(function(){showMsg('scale-msg',true,'Tarado!')})
  .catch(function(){showMsg('scale-msg',false,'Erro ao tarar.')});
}
function doCalibrate(){
  var w=parseFloat(D.getElementById('cal-weight').value);
  if(!w||w<=0){D.getElementById('cal-weight').focus();return}
  fetch('/scale/calibrate',{method:'POST',headers:{'Content-Type':'application/x-www-form-urlencoded'},
    body:'weight='+w})
  .then(function(r){return r.text()}).then(function(){showMsg('scale-msg',true,'Calibrado com '+w+'g!')})
  .catch(function(){showMsg('scale-msg',false,'Erro ao calibrar.')});
}
function saveBright(v){
  fetch('/config/led',{method:'POST',headers:{'Content-Type':'application/x-www-form-urlencoded'},
    body:'brightness='+v})
  .then(function(){showMsg('led-msg',true,'Brilho salvo!')})
  .catch(function(){showMsg('led-msg',false,'Erro ao salvar.')});
}

function saveWifi(){
  var ssid=D.getElementById('ssid').value.trim();
  var pw=D.getElementById('pw').value;
  if(!ssid){D.getElementById('ssid').focus();return}
  fetch('/config/wifi',{method:'POST',headers:{'Content-Type':'application/x-www-form-urlencoded'},
    body:'ssid='+encodeURIComponent(ssid)+'&password='+encodeURIComponent(pw)})
  .then(function(r){return r.text()}).then(function(){showMsg('wifi-msg',true,'Salvo! Reconectando...')})
  .catch(function(){showMsg('wifi-msg',false,'Erro ao salvar.')});
}
function restartDevice(){
  fetch('/config/restart',{method:'POST'}).catch(function(){});
  showMsg('wifi-msg',true,'Reiniciando...');
}
function showConfirm(){D.getElementById('confirm').style.display='block'}
function hideConfirm(){D.getElementById('confirm').style.display='none'}
function doReset(){
  hideConfirm();
  fetch('/config/reset',{method:'POST'}).catch(function(){});
  showMsg('wifi-msg',true,'Credenciais apagadas. Reiniciando...');
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
    _server.on("/config/led",     [this]() {
        if (_server.arg("brightness").isEmpty()) handleConfigLedGet();
        else handleConfigLedSet();
    });
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

void WebApp::handleConfigReset() {
    _storage.remove(STORAGE_KEY_WIFI_SSID);
    _storage.remove(STORAGE_KEY_WIFI_PASS);
    _server.send(200, "text/plain", "OK");
    _eventBus.publish({events::EventType::WifiCredentialsCleared});
    _pendingRestart = true;
    _restartAt = millis() + 800;
}

} // namespace core
