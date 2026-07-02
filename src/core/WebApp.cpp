#include "core/WebApp.h"
#include "events/EventType.h"
#include "config.h"
#include <Arduino.h>

// ── index page ───────────────────────────────────────────────────────────────
static const char ROOT_HTML[] = R"html(
<!DOCTYPE html><html><head>
<meta charset="utf-8">
<meta name="viewport" content="width=device-width,initial-scale=1">
<title>BalancaC3</title>
<style>
:root{--bg:#f0f2f5;--card:#fff;--text:#1a1a2e;--sub:#666;--btn:#0066cc;--bh:#0052a3}
html.dark{--bg:#0f1117;--card:#1e2130;--text:#e0e6f0;--sub:#8892a4;--btn:#4d8ef0;--bh:#3a7ae0}
*{box-sizing:border-box}
body{font-family:sans-serif;max-width:440px;margin:0 auto;padding:1em;background:var(--bg);color:var(--text)}
.hdr{display:flex;justify-content:space-between;align-items:center}
h1{margin:.4em 0;font-size:1.4em}
.thm{background:none;border:1px solid var(--sub);border-radius:20px;padding:.3em .85em;cursor:pointer;color:var(--text);font-size:.82em}
.card{background:var(--card);border-radius:10px;padding:1.2em;margin:.8em 0;box-shadow:0 1px 4px rgba(0,0,0,.1);text-align:center}
p{color:var(--sub);margin:.3em 0}
a.btn{display:inline-block;margin-top:.8em;padding:.6em 1.4em;background:var(--btn);color:#fff;border-radius:6px;text-decoration:none;font-size:.95em}
a.btn:active{background:var(--bh)}
</style></head><body>
<div class="hdr"><h1>BalancaC3</h1><button class="thm" id="thm" onclick="tog()"></button></div>
<div class="card"><h2 style="margin:.2em 0">Hello World!</h2><p>Dispositivo online</p><a class="btn" href="/config">&#9881; Configurações</a></div>
<script>
var H=document.documentElement;
function applyDark(d){H.classList.toggle('dark',d);document.getElementById('thm').textContent=d?'Claro':'Escuro'}
function tog(){var d=!H.classList.contains('dark');localStorage.setItem('t',d?'1':'0');applyDark(d)}
(function(){var s=localStorage.getItem('t');applyDark(s!=null?s==='1':window.matchMedia('(prefers-color-scheme:dark)').matches)})();
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
  <h3>Dispositivo</h3>
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

void WebApp::startServer(const char* ip) {
    _server.on("/",               [this]() { handleRoot(); });
    _server.on("/config",         [this]() { handleConfig(); });
    _server.on("/networks",       [this]() { handleNetworks(); });
    _server.on("/config/wifi",    [this]() { handleConfigWifi(); });
    _server.on("/config/restart", [this]() { handleConfigRestart(); });
    _server.on("/config/reset",   [this]() { handleConfigReset(); });
    _server.on("/config/ssid",    [this]() { handleCurrentSsid(); });
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

void WebApp::handleConfigReset() {
    _storage.remove(STORAGE_KEY_WIFI_SSID);
    _storage.remove(STORAGE_KEY_WIFI_PASS);
    _server.send(200, "text/plain", "OK");
    _eventBus.publish({events::EventType::WifiCredentialsCleared});
    _pendingRestart = true;
    _restartAt = millis() + 800;
}

} // namespace core
