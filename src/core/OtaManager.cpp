#include "core/OtaManager.h"
#include "events/EventType.h"
#include <Arduino.h>
// Update.hasError() is accessed only through IOta::httpHasError() — no direct Update.h here

// Payload structs for OTA events
struct OtaProgressPayload { uint8_t pct; };

static const char UPDATE_HTML[] = R"html(
<!DOCTYPE html><html><head>
<meta charset="utf-8">
<meta name="viewport" content="width=device-width,initial-scale=1">
<title>Atualizar Firmware</title>
<style>
:root{--bg:#f0f2f5;--card:#fff;--text:#1a1a2e;--sub:#666;--bdr:#ddd;--btn:#0066cc;--bh:#0052a3;--ok-bg:#d4edda;--ok-c:#155724;--er-bg:#f8d7da;--er-c:#721c24}
html.dark{--bg:#0f1117;--card:#1e2130;--text:#e0e6f0;--sub:#8892a4;--bdr:#2e3548;--btn:#4d8ef0;--bh:#3a7ae0;--ok-bg:#1a3a25;--ok-c:#6ee08a;--er-bg:#3a1a1a;--er-c:#f08080}
*{box-sizing:border-box}
body{font-family:sans-serif;max-width:440px;margin:0 auto;padding:1em;background:var(--bg);color:var(--text)}
.hdr{display:flex;justify-content:space-between;align-items:center;margin-bottom:.4em}
h2{margin:0;font-size:1.2em}
.thm{background:none;border:1px solid var(--sub);border-radius:20px;padding:.3em .85em;cursor:pointer;color:var(--text);font-size:.82em}
.back{color:var(--btn);text-decoration:none;font-size:.9em}
.card{background:var(--card);border-radius:10px;padding:1.2em;margin:.7em 0;box-shadow:0 1px 4px rgba(0,0,0,.1)}
p{color:var(--sub);font-size:.9em;margin:.2em 0 .8em}
.drop{border:2px dashed var(--bdr);border-radius:8px;padding:2em 1em;text-align:center;cursor:pointer;transition:border-color .2s;color:var(--sub)}
.drop.over,.drop:hover{border-color:var(--btn)}
.drop input{display:none}
.fname{margin:.6em 0;font-size:.9em;color:var(--text);word-break:break-all}
.btn{width:100%;padding:.7em;background:var(--btn);color:#fff;border:none;border-radius:6px;font-size:1em;cursor:pointer;margin-top:.6em;transition:background .15s}
.btn:disabled{background:var(--bdr);cursor:not-allowed}
.btn:not(:disabled):active{background:var(--bh)}
.bar-wrap{background:var(--bdr);border-radius:6px;height:10px;margin-top:.8em;overflow:hidden;display:none}
.bar{height:100%;width:0;background:var(--btn);border-radius:6px;transition:width .3s}
.pct{text-align:center;font-size:.85em;color:var(--sub);margin:.3em 0}
.msg{padding:.75em;border-radius:6px;text-align:center;margin-top:.6em;font-weight:bold;display:none}
.ok{background:var(--ok-bg);color:var(--ok-c)}
.err{background:var(--er-bg);color:var(--er-c)}
.info{font-size:.8em;color:var(--sub);margin-top:.6em;text-align:center}
</style></head><body>
<div class="hdr">
  <a class="back" href="/config">&#8592; Configurações</a>
  <h2>Atualizar Firmware</h2>
  <button class="thm" id="thm" onclick="tog()"></button>
</div>
<div class="card">
  <p>Selecione o arquivo <strong>.bin</strong> gerado pelo PlatformIO (<code>.pio/build/esp32-c3-devkitm-1/firmware.bin</code>)</p>
  <div class="drop" id="drop" onclick="document.getElementById('fi').click()"
       ondragover="ev(event,true)" ondragleave="ev(event,false)" ondrop="drop(event)">
    <input type="file" id="fi" accept=".bin" onchange="pick(this.files)">
    &#128190; Clique ou arraste o firmware.bin aqui
  </div>
  <div class="fname" id="fname"></div>
  <button class="btn" id="ubtn" disabled onclick="upload()">Enviar firmware</button>
  <div class="bar-wrap" id="bw"><div class="bar" id="bar"></div></div>
  <div class="pct" id="pct"></div>
  <div class="msg" id="msg"></div>
  <p class="info">O dispositivo reiniciará automaticamente após a gravação.</p>
</div>
<script>
var D=document,H=D.documentElement,file=null;
function applyDark(d){H.classList.toggle('dark',d);D.getElementById('thm').textContent=d?'Claro':'Escuro'}
function tog(){var d=!H.classList.contains('dark');localStorage.setItem('t',d?'1':'0');applyDark(d)}
(function(){var s=localStorage.getItem('t');applyDark(s!=null?s==='1':window.matchMedia('(prefers-color-scheme:dark)').matches)})();
function ev(e,on){e.preventDefault();D.getElementById('drop').classList.toggle('over',on)}
function drop(e){e.preventDefault();D.getElementById('drop').classList.remove('over');pick(e.dataTransfer.files)}
function pick(files){if(!files||!files.length)return;file=files[0];D.getElementById('fname').textContent=file.name+' ('+Math.round(file.size/1024)+' KB)';D.getElementById('ubtn').disabled=false}
function setMsg(ok,txt){var m=D.getElementById('msg');m.className='msg '+(ok?'ok':'err');m.textContent=txt;m.style.display='block'}
function upload(){
  if(!file)return;
  var fd=new FormData();fd.append('firmware',file);
  var xhr=new XMLHttpRequest();
  xhr.open('POST','/update');
  D.getElementById('bw').style.display='block';
  D.getElementById('ubtn').disabled=true;
  xhr.upload.onprogress=function(e){
    if(!e.lengthComputable)return;
    var p=Math.round(e.loaded/e.total*100);
    D.getElementById('bar').style.width=p+'%';
    D.getElementById('pct').textContent=p+'%';
  };
  xhr.onload=function(){
    if(xhr.status===200){setMsg(true,'Firmware gravado! Reiniciando...')}
    else{setMsg(false,'Erro: '+xhr.responseText);D.getElementById('ubtn').disabled=false}
  };
  xhr.onerror=function(){setMsg(false,'Falha na conexão.');D.getElementById('ubtn').disabled=false};
  xhr.send(fd);
}
</script></body></html>
)html";

namespace core {

OtaManager::OtaManager(hal::IOta& ota, hal::IWebServer& server,
                        hal::IDevice& device, events::EventBus& eventBus)
    : _ota(ota), _server(server), _device(device), _eventBus(eventBus) {}

void OtaManager::begin() {
    _eventBus.subscribe(events::EventType::WifiConnected,      this);
    _eventBus.subscribe(events::EventType::WifiDisconnected,   this);
    _eventBus.subscribe(events::EventType::WifiConfigRequired, this);
}

void OtaManager::loop() {
    if (_networkActive) _ota.handleNetwork();

    if (_pendingRestart && millis() >= _restartAt) {
        _pendingRestart = false;
        _device.restart();
    }
}

void OtaManager::onEvent(const events::Event& event) {
    switch (event.type) {
        case events::EventType::WifiConnected:
            startOta(static_cast<const char*>(event.payload));
            break;
        case events::EventType::WifiDisconnected:
        case events::EventType::WifiConfigRequired:
            _networkActive = false;
            break;
        default:
            break;
    }
}

void OtaManager::startOta(const char* ip) {
    _ota.beginNetwork("balancac3",
        [this]() {
            _eventBus.publish({events::EventType::OtaStarted});
        },
        [this](uint32_t done, uint32_t total) {
            static OtaProgressPayload p;
            p.pct = total > 0 ? static_cast<uint8_t>(done * 100 / total) : 0;
            if (p.pct != _lastProgressPct) {
                _lastProgressPct = p.pct;
                _eventBus.publish({events::EventType::OtaProgress, &p});
            }
        },
        [this]() {
            _eventBus.publish({events::EventType::OtaSuccess});
        },
        [this](const char* error) {
            _eventBus.publish({events::EventType::OtaError, error});
        }
    );

    if (!_routesRegistered) {
        registerHttpRoute();
        _routesRegistered = true;
    }
    _networkActive = true;
}

void OtaManager::registerHttpRoute() {
    // Order matters. IWebServer::on() registers as HTTP_ANY, so it also matches
    // POST; the framework picks the FIRST handler whose canHandle() passes. With
    // the page registered first, POST /update landed on a handler that has no
    // upload function, so the firmware bytes were parsed and discarded while the
    // browser still got a 200. Registering the POST-only upload handler first
    // makes GET fall through to the page and POST reach the flasher.
    _server.onUpload("/update",
        [this]() { handleUploadCompletion(); },
        [this](hal::UploadStatus status, const uint8_t* data, size_t len) {
            handleUploadChunk(status, data, len);
        }
    );

    _server.on("/update", [this]() {
        _server.send(200, "text/html", UPDATE_HTML);
    });
}

void OtaManager::handleUploadCompletion() {
    if (_ota.httpHasError()) {
        _server.send(500, "text/plain", _ota.httpError());
        _eventBus.publish({events::EventType::OtaError, _ota.httpError()});
    } else {
        _server.send(200, "text/plain", "OK");
        _eventBus.publish({events::EventType::OtaSuccess});
        _pendingRestart = true;
        _restartAt = millis() + 800;
    }
}

void OtaManager::handleUploadChunk(hal::UploadStatus status, const uint8_t* data, size_t len) {
    switch (status) {
        case hal::UploadStatus::Start:
            _lastProgressPct = 0;
            _ota.httpBegin(len);
            _eventBus.publish({events::EventType::OtaStarted});
            break;
        case hal::UploadStatus::Write:
            _ota.httpWrite(data, len);
            break;
        case hal::UploadStatus::End:
            _ota.httpEnd();
            break;
        case hal::UploadStatus::Abort:
            _eventBus.publish({events::EventType::OtaError, "Upload aborted"});
            break;
    }
}

} // namespace core
