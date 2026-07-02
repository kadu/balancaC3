#include "hal/Esp32CaptivePortal.h"
#include <Arduino.h>
#include <cstring>

// Captive portal detection endpoints (Android, Apple, Windows)
static const char* CAPTIVE_PROBES[] = {
    "/generate_204",
    "/hotspot-detect.html",
    "/fwlink",
    "/connecttest.txt",
    nullptr
};

static const char PORTAL_HTML[] PROGMEM = R"html(
<!DOCTYPE html><html><head>
<meta charset="utf-8">
<meta name="viewport" content="width=device-width,initial-scale=1">
<title>WiFi Setup</title>
<style>
:root{--bg:#f0f2f5;--card:#fff;--text:#1a1a2e;--sub:#666;--bdr:#ddd;--hov:#eef2ff;--inp:#fff;--btn:#0066cc;--bh:#0052a3;--rs:#888;--ok-bg:#d4edda;--ok-c:#155724;--er-bg:#f8d7da;--er-c:#721c24}
html.dark{--bg:#0f1117;--card:#1e2130;--text:#e0e6f0;--sub:#8892a4;--bdr:#2e3548;--hov:#252d40;--inp:#252a3a;--btn:#4d8ef0;--bh:#3a7ae0;--rs:#6b7280;--ok-bg:#1a3a25;--ok-c:#6ee08a;--er-bg:#3a1a1a;--er-c:#f08080}
*{box-sizing:border-box}
body{font-family:sans-serif;max-width:440px;margin:0 auto;padding:1em;background:var(--bg);color:var(--text);transition:background .2s,color .2s}
.hdr{display:flex;justify-content:space-between;align-items:center;margin-bottom:.3em}
h2{margin:0;font-size:1.25em}
.thm{background:none;border:1px solid var(--bdr);border-radius:20px;padding:.3em .85em;cursor:pointer;color:var(--text);font-size:.82em;transition:border-color .2s}
p{color:var(--sub);margin:.1em 0 .8em;font-size:.88em}
.card{background:var(--card);border-radius:10px;padding:1em;margin:.6em 0;box-shadow:0 1px 4px rgba(0,0,0,.1)}
.net{display:flex;justify-content:space-between;align-items:center;padding:.55em .4em;border-radius:6px;cursor:pointer;transition:background .15s}
.net:hover,.net:active{background:var(--hov)}
.rs{font-size:.78em;color:var(--rs);white-space:nowrap;padding-left:.5em}
label{display:block;font-size:.82em;color:var(--sub);margin:.75em 0 .25em}
input[type=text],input[type=password]{width:100%;padding:.55em .7em;border:1px solid var(--bdr);border-radius:6px;font-size:1em;background:var(--inp);color:var(--text);outline:none;transition:border-color .15s}
input:focus{border-color:var(--btn)}
.btn{width:100%;padding:.7em;background:var(--btn);color:#fff;border:none;border-radius:6px;font-size:1em;cursor:pointer;margin-top:.75em;transition:background .15s}
.btn:active{background:var(--bh)}
.msg{padding:.8em;border-radius:6px;text-align:center;margin-top:.6em;font-weight:bold;display:none}
.ok{background:var(--ok-bg);color:var(--ok-c)}
.err{background:var(--er-bg);color:var(--er-c)}
</style>
</head><body>
<div class="hdr"><h2>WiFi Setup</h2><button class="thm" id="thm" onclick="tog()"></button></div>
<p>Selecione uma rede ou digite o SSID manualmente (redes ocultas)</p>
<div class="card"><div id="nets">Escaneando...</div></div>
<div class="card">
<label>Rede (SSID)</label>
<input id="ssid" type="text" placeholder="Nome da rede ou rede oculta" autocomplete="off" spellcheck="false">
<label>Senha</label>
<input id="pw" type="password" placeholder="Senha da rede" autocomplete="current-password">
<button class="btn" onclick="conn()">Conectar</button>
</div>
<div class="msg" id="msg"></div>
<script>
var D=document,H=D.documentElement;
function applyDark(d){H.classList.toggle('dark',d);D.getElementById('thm').textContent=d?'☀ Claro':'🌙 Escuro'}
function tog(){var d=!H.classList.contains('dark');localStorage.setItem('t',d?'1':'0');applyDark(d)}
(function(){var s=localStorage.getItem('t');applyDark(s!=null?s==='1':window.matchMedia('(prefers-color-scheme:dark)').matches)})();
function sig(r){return r>-60?'▂▄▆█':r>-70?'▂▄▆_':r>-80?'▂▄__':'▂___'}
fetch('/networks').then(function(r){return r.json()}).then(function(ns){
  var d=D.getElementById('nets');
  if(!ns||!ns.length){d.textContent='Nenhuma rede encontrada. Recarregue.';return}
  d.innerHTML='';
  ns.forEach(function(n){
    var e=D.createElement('div');e.className='net';
    e.innerHTML='<span>'+n.ssid+'</span><span class="rs">'+sig(n.rssi)+' '+n.rssi+'</span>';
    e.onclick=function(){D.getElementById('ssid').value=n.ssid;D.getElementById('pw').value='';D.getElementById('pw').focus()};
    d.appendChild(e);
  });
}).catch(function(){D.getElementById('nets').textContent='Falha no scan. Recarregue a página.'});
function conn(){
  var ssid=D.getElementById('ssid').value.trim();
  var pw=D.getElementById('pw').value;
  if(!ssid){D.getElementById('ssid').focus();return}
  fetch('/save',{method:'POST',headers:{'Content-Type':'application/x-www-form-urlencoded'},
    body:'ssid='+encodeURIComponent(ssid)+'&password='+encodeURIComponent(pw)})
  .then(function(r){return r.text()}).then(function(){
    var m=D.getElementById('msg');m.className='msg ok';
    m.textContent='Credenciais salvas! O dispositivo irá conectar em instantes.';m.style.display='block';
  }).catch(function(){
    var m=D.getElementById('msg');m.className='msg err';
    m.textContent='Erro ao salvar. Tente novamente.';m.style.display='block';
  });
}
</script></body></html>
)html";

namespace hal {

Esp32CaptivePortal::Esp32CaptivePortal(IWifi& wifi) : _wifi(wifi) {}

void Esp32CaptivePortal::begin() {
    _hasCredentials = false;
    registerRoutes();
    _dns.start(53, "*", IPAddress(192, 168, 4, 1));
    _server.begin();
}

void Esp32CaptivePortal::handle() {
    _dns.processNextRequest();
    _server.handleClient();
}

void Esp32CaptivePortal::stop() {
    _server.stop();
    _dns.stop();
}

bool Esp32CaptivePortal::hasNewCredentials() {
    return _hasCredentials;
}

WifiCredentials Esp32CaptivePortal::consumeCredentials() {
    _hasCredentials = false;
    return _pending;
}

void Esp32CaptivePortal::registerRoutes() {
    _server.on("/", [this]() { handleRoot(); });
    _server.on("/networks", [this]() { handleNetworks(); });
    _server.on("/save", HTTP_POST, [this]() { handleSave(); });

    for (int i = 0; CAPTIVE_PROBES[i] != nullptr; ++i) {
        _server.on(CAPTIVE_PROBES[i], [this]() { redirectToRoot(); });
    }

    _server.onNotFound([this]() { redirectToRoot(); });
}

void Esp32CaptivePortal::handleRoot() {
    _server.send_P(200, "text/html", PORTAL_HTML);
}

void Esp32CaptivePortal::handleNetworks() {
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

    _server.send(200, "application/json", json);
}

void Esp32CaptivePortal::handleSave() {
    String ssid     = _server.arg("ssid");
    String password = _server.arg("password");

    if (ssid.isEmpty()) {
        _server.send(400, "text/plain", "SSID obrigatorio");
        return;
    }

    strncpy(_pending.ssid,     ssid.c_str(),     sizeof(_pending.ssid)     - 1);
    strncpy(_pending.password, password.c_str(), sizeof(_pending.password) - 1);
    _pending.ssid[sizeof(_pending.ssid) - 1]         = '\0';
    _pending.password[sizeof(_pending.password) - 1] = '\0';

    _hasCredentials = true;
    _server.send(200, "text/plain", "OK");
}

void Esp32CaptivePortal::redirectToRoot() {
    _server.sendHeader("Location", "http://192.168.4.1/", true);
    _server.send(302, "text/plain", "");
}

} // namespace hal
