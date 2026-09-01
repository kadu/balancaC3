#include "core/WebApp.h"
#include "core/ScaleManager.h"
#include "core/RecipeStorage.h"
#include "core/TimerManager.h"
#include "core/RecipeManager.h"
#include "events/EventType.h"
#include "config.h"
#include "build_info.h"
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
.timer{font-size:1.9em;font-weight:bold;color:var(--sub);margin:.05em 0 .35em;line-height:1;transition:color .2s}
.timer.run{color:var(--btn)}
.tico{font-size:.6em;margin-right:.3em;vertical-align:.1em}
.tsep{height:1px;background:var(--bdr);margin:0 0 .5em}
.unit{font-size:.4em;font-weight:normal;color:var(--sub);margin-left:.15em}
.raw{font-size:.8em;color:var(--sub);margin:.2em 0}
.warn{background:var(--warnbg);color:var(--warn);border-radius:6px;padding:.5em .8em;font-size:.85em;margin:.5em 0}
.dot{display:inline-block;width:8px;height:8px;border-radius:50%;background:#22c55e;margin-right:.4em}
.dot.off{background:#ef4444}
p{color:var(--sub);margin:.3em 0}
a.btn{display:inline-block;margin-top:.8em;padding:.6em 1.4em;background:var(--btn);color:#fff;border-radius:6px;text-decoration:none;font-size:.95em}
a.btn:active{background:var(--bh)}
.btns{display:flex;gap:.8em}
.btns>div{flex:1}
.pb{width:100%;padding:1em .3em;background:var(--card);color:var(--text);border:2px solid var(--btn);border-radius:10px;font-family:inherit;cursor:pointer;display:flex;flex-direction:column;gap:.15em;align-items:center;user-select:none;-webkit-user-select:none;-webkit-tap-highlight-color:transparent;touch-action:manipulation;transition:background .08s,transform .08s,border-color .08s}
.pb.hold{background:var(--btn);color:#fff;transform:scale(.97)}
.pb.long{background:var(--warn);border-color:var(--warn);color:#fff}
.pb.flash{animation:fl .45s ease-out}
@keyframes fl{0%{background:var(--btn);border-color:var(--btn);color:#fff;transform:scale(.93)}100%{background:var(--card);border-color:var(--btn);color:var(--text);transform:scale(1)}}
.pbn{font-size:1.02em;font-weight:bold}
.pbl{font-size:.74em;opacity:.75}
.hint{font-size:.7em;color:var(--sub);margin-top:.45em;line-height:1.4}
.lft{text-align:left}
.rttl{font-size:.8em;color:var(--sub);margin-bottom:.55em}
.rbar{display:flex;gap:.5em;align-items:center}
select{flex:1;min-width:0;padding:.55em .6em;border:1px solid var(--bdr);border-radius:6px;background:var(--card);color:var(--text);font-family:inherit;font-size:.92em}
.rgo{padding:.55em 1.1em;background:var(--btn);color:#fff;border:none;border-radius:6px;font-family:inherit;font-size:.92em;cursor:pointer;white-space:nowrap}
.rgo.stop{background:#ef4444}
.rtot{font-size:.8em;color:var(--sub);margin:.7em 0 .1em;display:flex;justify-content:space-between}
.steps{list-style:none;margin:.5em 0 0;padding:0}
.stp{border-left:3px solid var(--bdr);padding:.5em .1em .5em .7em;margin:0 0 .35em;opacity:.55;transition:opacity .2s}
.stp.next{opacity:.85}
.stp.cur{opacity:1;border-left-color:var(--btn);background:rgba(128,128,128,.09);border-radius:0 6px 6px 0}
.stp.done{opacity:.4}
.stp.done .sname{text-decoration:line-through}
.srow{display:flex;justify-content:space-between;align-items:baseline;gap:.5em}
.sname{font-size:.95em;font-weight:600}
.stp.cur .sname{color:var(--btn)}
.sdur{font-size:.8em;color:var(--sub);white-space:nowrap}
.swtr{font-size:.78em;color:var(--sub);margin-top:.15em}
.bar{height:7px;background:var(--bdr);border-radius:4px;overflow:hidden;margin:.5em 0 .3em}
.bar>i{display:block;height:100%;width:0;background:var(--btn);border-radius:4px;transition:width .25s linear}
.bar.wait>i{background:var(--warn);width:100%;animation:pl 1.4s ease-in-out infinite}
@keyframes pl{0%,100%{opacity:.25}50%{opacity:.8}}
.slive{display:flex;justify-content:space-between;font-size:.78em;color:var(--sub);font-variant-numeric:tabular-nums}
.stp.cur .slive b{color:var(--text);font-weight:600}
.rmsg{font-size:.8em;color:var(--sub);margin-top:.5em;min-height:1.1em}
.ver{text-align:right;font-size:.68em;color:var(--sub);opacity:.7;margin:.4em .3em 1.4em;font-variant-numeric:tabular-nums}
</style></head><body>
<div class="hdr"><h1>BalancaC3</h1><div style="display:flex;align-items:center;gap:.5em"><a href="/config" style="display:flex;align-items:center;gap:.3em;color:var(--text);text-decoration:none;font-size:.88em;border:1px solid var(--sub);border-radius:20px;padding:.3em .75em"><span style="font-size:1.1em">&#9881;</span>Configurações</a><button class="thm" id="thm" onclick="tog()"></button></div></div>
<div class="card">
  <div style="font-size:.8em;color:var(--sub);margin-bottom:.4em">
    <span class="dot" id="dot"></span><span id="sensor-status">aguardando...</span>
  </div>
  <div class="timer" id="timer"><span class="tico" id="tico">&#9208;</span><span id="tval">0:00</span></div>
  <div class="tsep"></div>
  <div class="weight" id="weight-val">--<span class="unit" id="weight-unit">g</span></div>
  <div class="raw" id="raw-val"></div>
  <div class="warn" id="cal-warn" style="display:none">Sensor sem calibracao — valore raw apenas. Acesse Configurações para calibrar.</div>
  <div style="margin-top:.8em"><button onclick="doTare()" style="padding:.6em 1.6em;background:var(--btn);color:#fff;border:none;border-radius:6px;font-size:.95em;cursor:pointer">Tarar</button></div>
  <div id="tare-msg" style="font-size:.82em;color:var(--sub);margin-top:.4em;min-height:1.2em"></div>
</div>
<div class="card lft">
  <div class="rttl">Receita — escolha e inicie o preparo</div>
  <div class="rbar">
    <select id="rsel"><option value="0">carregando...</option></select>
    <button class="rgo" id="rgo" onclick="rgoClick()">Iniciar</button>
  </div>
  <div class="rmsg" id="rmsg"></div>
  <div class="rtot" id="rtot" style="display:none"><span id="rtotL"></span><span id="rtotR"></span></div>
  <ol class="steps" id="rsteps"></ol>
</div>
<div class="card">
  <div style="font-size:.8em;color:var(--sub);margin-bottom:.7em">Controles — toque rapido ou segure</div>
  <div class="btns">
    <div>
      <button class="pb" id="pb1"><span class="pbn">Botão 1</span><span class="pbl">Timer</span></button>
      <div class="hint">toque: iniciar/pausar<br>segurar: zerar</div>
    </div>
    <div>
      <button class="pb" id="pb2"><span class="pbn">Botão 2</span><span class="pbl">Tara</span></button>
      <div class="hint">toque: tarar<br>segurar: receitas</div>
    </div>
  </div>
</div>
<div class="ver" id="ver"></div>
<script>
var H=document.documentElement,D=document;
function applyDark(d){H.classList.toggle('dark',d);D.getElementById('thm').textContent=d?'Claro':'Escuro'}
function tog(){var d=!H.classList.contains('dark');localStorage.setItem('t',d?'1':'0');applyDark(d)}
(function(){var s=localStorage.getItem('t');applyDark(s!=null?s==='1':window.matchMedia('(prefers-color-scheme:dark)').matches)})();
var prevBtn=null;
function flash(id){var e=D.getElementById(id);e.classList.remove('flash');void e.offsetWidth;e.classList.add('flash')}
(function poll(){
  fetch('/scale/weight').then(function(r){return r.json()}).then(function(d){
    if(prevBtn){
      if(d.b1c!==prevBtn.b1c||d.b1l!==prevBtn.b1l)flash('pb1');
      if(d.b2c!==prevBtn.b2c||d.b2l!==prevBtn.b2l)flash('pb2');
    }
    prevBtn={b1c:d.b1c,b1l:d.b1l,b2c:d.b2c,b2l:d.b2l};
    var ts=d.tsec|0,tm=(ts/60)|0,tsx=ts%60;
    D.getElementById('tval').textContent=tm+':'+(tsx<10?'0':'')+tsx;
    D.getElementById('tico').innerHTML=d.trun?'&#9654;':'&#9208;';
    D.getElementById('timer').classList.toggle('run',!!d.trun);
    updRecipe(d);
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
/* ── receita ───────────────────────────────────────────────────────────── */
var rSteps=null,rTitle='',rLoadedId=0,rKey='';
function fmt(s){s=s|0;var m=(s/60)|0,x=s%60;return m+':'+(x<10?'0':'')+x}
function esc(t){var e=D.createElement('div');e.textContent=t==null?'':t;return e.innerHTML}
function msg(t){var m=D.getElementById('rmsg');m.textContent=t;
  if(t)setTimeout(function(){if(m.textContent===t)m.textContent=''},3000)}
(function loadRecipes(){
  fetch('/recipes').then(function(r){return r.json()}).then(function(a){
    var s=D.getElementById('rsel');s.innerHTML='';
    if(!a||!a.length){s.innerHTML='<option value="0">nenhuma receita salva</option>';return}
    a.forEach(function(r){
      var o=D.createElement('option');o.value=r.id;
      o.textContent=r.title+(r.waterTotal?' — '+r.waterTotal+'ml':'');
      s.appendChild(o);
    });
  }).catch(function(){});
})();
function rgoClick(){
  var stop=D.getElementById('rgo').classList.contains('stop');
  var id=stop?0:(D.getElementById('rsel').value|0);
  if(!stop&&!id){msg('Selecione uma receita.');return}
  fetch('/recipe/start?id='+id).then(function(r){if(!r.ok)msg('Falha ao iniciar a receita.')})
    .catch(function(){msg('Falha ao iniciar a receita.')});
}
// Steps are static once the recipe starts, so fetch them once and keep them.
function ensureRecipe(id){
  if(rLoadedId===id)return;
  rLoadedId=id;rSteps=null;
  fetch('/recipe?id='+id).then(function(r){return r.json()}).then(function(o){
    rTitle=o.title||'Receita';rSteps=o.steps||[];rKey='';
  }).catch(function(){rLoadedId=0});
}
// Full list is always rendered — past steps struck through, current one expanded
// with its live bar, upcoming ones legible so you can see what comes next.
function renderSteps(cur){
  var ol=D.getElementById('rsteps');ol.innerHTML='';
  if(!rSteps)return;
  var cum=0;
  rSteps.forEach(function(s,i){
    cum+=(s.water|0);
    var li=D.createElement('li');
    li.className='stp '+(i<cur?'done':i===cur?'cur':i===cur+1?'next':'');
    var h='<div class="srow"><span class="sname">'+(i+1)+'. '+esc(s.type||'Passo')+'</span>'+
          '<span class="sdur">'+fmt(s.duration|0)+'</span></div>'+
          '<div class="swtr">'+((s.water|0)?'+'+(s.water|0)+' ml':'sem agua')+
          ' → alvo '+cum+' ml</div>';
    if(i===cur)h+='<div class="bar" id="rbar"><i id="rbari"></i></div>'+
                  '<div class="slive"><span id="relw"></span><span id="rremw"></span></div>';
    li.innerHTML=h;ol.appendChild(li);
  });
}
function updRecipe(d){
  var go=D.getElementById('rgo'),tot=D.getElementById('rtot'),sel=D.getElementById('rsel');
  if(!d.ract){
    go.textContent='Iniciar';go.classList.remove('stop');sel.disabled=false;
    if(rLoadedId){rLoadedId=0;rSteps=null;rKey='';
      D.getElementById('rsteps').innerHTML='';tot.style.display='none';msg('Receita encerrada.')}
    return;
  }
  ensureRecipe(d.rid);
  go.textContent='Encerrar';go.classList.add('stop');sel.disabled=true;
  var key=d.rid+'|'+d.rstep+'|'+(rSteps?rSteps.length:0);
  if(key!==rKey){rKey=key;renderSteps(d.rstep)}
  tot.style.display='flex';
  D.getElementById('rtotL').textContent=(rTitle||'Receita')+' — passo '+(d.rstep+1)+'/'+d.rn;
  D.getElementById('rtotR').textContent='total '+fmt(d.rtot);
  var bar=D.getElementById('rbar');if(!bar)return;
  var dur=(rSteps&&rSteps[d.rstep])?(rSteps[d.rstep].duration|0):0;
  if(!d.rrun){
    bar.className='bar wait';D.getElementById('rbari').style.width='100%';
    D.getElementById('relw').innerHTML='<b>aguardando agua...</b>';
    D.getElementById('rremw').textContent='inicia ao detectar peso';
  }else{
    bar.className='bar';
    D.getElementById('rbari').style.width=(dur?Math.min(100,d.rel*100/dur):0)+'%';
    D.getElementById('relw').innerHTML='decorrido <b>'+fmt(d.rel)+'</b>';
    D.getElementById('rremw').innerHTML=dur?('faltam <b>'+fmt(d.rrem)+'</b>'):'sem limite';
  }
}
/* ── botoes ────────────────────────────────────────────────────────────── */
var LONG_MS=700,hold={};
[1,2].forEach(function(n){
  var e=D.getElementById('pb'+n);
  e.addEventListener('contextmenu',function(ev){ev.preventDefault()});
  e.addEventListener('pointerdown',function(ev){
    ev.preventDefault();
    if(e.setPointerCapture)e.setPointerCapture(ev.pointerId);
    e.classList.remove('flash');e.classList.add('hold');
    hold[n]={lg:false,t:setTimeout(function(){if(hold[n]){hold[n].lg=true;e.classList.add('long')}},LONG_MS)};
  });
  e.addEventListener('pointerup',function(){
    var h=hold[n];if(!h)return;hold[n]=null;clearTimeout(h.t);
    e.classList.remove('hold','long');
    fetch('/button?n='+n+'&a='+(h.lg?'long':'click')).catch(function(){});
  });
  e.addEventListener('pointercancel',function(){
    var h=hold[n];if(!h)return;hold[n]=null;clearTimeout(h.t);
    e.classList.remove('hold','long');
  });
});
fetch('/build').then(function(r){return r.text()}).then(function(t){
  document.getElementById('ver').textContent=t;
}).catch(function(){});
</script>
</body></html>
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
.ver{text-align:right;font-size:.68em;color:var(--sub);opacity:.7;margin:.4em .3em 1.4em;font-variant-numeric:tabular-nums}
</style></head><body>
<div class="hdr">
  <a class="back" href="/">&#8592; Voltar</a>
  <h2>Configurações</h2>
  <button class="thm" id="thm" onclick="tog()"></button>
</div>

<div class="tabs">
  <button class="tab active" onclick="showTab('receitas',this)">&#9749; Receitas</button>
  <button class="tab" onclick="showTab('balanca',this)">&#9878; Balança</button>
  <button class="tab" onclick="showTab('leds',this)">&#128161; LEDs</button>
  <button class="tab" onclick="showTab('wifi',this)">&#128246; WiFi</button>
  <button class="tab" onclick="showTab('device',this)">&#9881; Dispositivo</button>
</div>

<!-- Receitas -->
<div id="pane-receitas" class="pane active">
  <div id="recipe-list-view">
    <div class="card" id="recipe-list-card">
      <p class="hint" id="recipe-empty" style="display:none">Nenhuma receita cadastrada.</p>
      <div id="recipe-items"></div>
      <button class="btn" onclick="showRecipeForm(null)" style="margin-top:.5em">+ Nova Receita</button>
    </div>
  </div>
  <div id="recipe-form-view" style="display:none">
    <div class="card">
      <p style="font-size:.78em;color:var(--sub);text-transform:uppercase;letter-spacing:.05em;margin:0 0 .7em;font-weight:600" id="recipe-form-title">Nova Receita</p>
      <input type="hidden" id="rf-id" value="0">
      <label>Título</label>
      <input id="rf-title" type="text" placeholder="Ex: V60 Clássico">
      <label>Tamanho da moagem</label>
      <input id="rf-grind" type="text" placeholder="Ex: Médio-fino">
      <label>Água total (ml)</label>
      <input id="rf-water" type="number" min="1" max="2000" placeholder="300" oninput="updateWaterLeft()">
      <label>Café (g)</label>
      <input id="rf-coffee" type="number" min="1" max="200" placeholder="20">
      <label>Temperatura (°C)</label>
      <input id="rf-heat" type="number" min="60" max="100" placeholder="93">
    </div>
    <div class="card">
      <p style="font-size:.78em;color:var(--sub);text-transform:uppercase;letter-spacing:.05em;margin:0 0 .4em;font-weight:600">Preparo</p>
      <p class="hint" style="display:flex;justify-content:space-between"><span>Água restante: <strong id="water-left">--</strong></span><span>Tempo total: <strong id="total-time">0:00</strong></span></p>
      <div id="steps-list"></div>
      <button class="btn" onclick="addStep()" style="background:var(--bdr);color:var(--text);margin-top:.4em">+ Adicionar etapa</button>
    </div>
    <div style="display:flex;gap:.5em;margin-top:.5em">
      <button class="btn" onclick="saveRecipe()" style="flex:2">Salvar receita</button>
      <button class="btn" onclick="cancelRecipeForm()" style="flex:1;background:var(--bdr);color:var(--text)">Cancelar</button>
    </div>
    <div class="msg" id="recipe-msg"></div>
  </div>
</div>

<!-- Balança -->
<div id="pane-balanca" class="pane">
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

<div class="ver" id="ver"></div>
<script>
var D=document,H=D.documentElement;
function applyDark(d){H.classList.toggle('dark',d);D.getElementById('thm').textContent=d?'Claro':'Escuro'}
function tog(){var d=!H.classList.contains('dark');localStorage.setItem('t',d?'1':'0');applyDark(d)}
(function(){var s=localStorage.getItem('t');applyDark(s!=null?s==='1':window.matchMedia('(prefers-color-scheme:dark)').matches)})();

// ── Recipes ──────────────────────────────────────────────────────────
var STEP_TYPES=['Despejo','Flor','Aguardar','Redemoinho','Mexa','Personalizado'];
var _steps=[];

function fmtTime(s){var m=Math.floor(s/60);return m+':'+(s%60<10?'0':'')+s%60;}

function loadRecipeList(){
  fetch('/recipes').then(function(r){return r.json()}).then(function(list){
    var el=D.getElementById('recipe-items');
    var empty=D.getElementById('recipe-empty');
    el.innerHTML='';
    if(!list||!list.length){empty.style.display='block';return}
    empty.style.display='none';
    list.forEach(function(r){
      var div=D.createElement('div');
      div.style.cssText='display:flex;justify-content:space-between;align-items:center;padding:.5em .2em;border-bottom:1px solid var(--bdr)';
      var meta='';
      if(r.waterTotal) meta+='<span style="font-size:.78em;color:var(--sub);margin-right:.5em">'+r.waterTotal+'ml</span>';
      if(r.totalSecs)  meta+='<span style="font-size:.78em;color:var(--sub);margin-right:.5em">'+fmtTime(r.totalSecs)+'</span>';
      div.innerHTML='<div><span style="font-size:.95em">'+r.title+'</span><br>'+meta+'</div>'
        +'<div style="display:flex;gap:.4em">'
        +'<button onclick="editRecipe('+r.id+')" style="background:none;border:1px solid var(--btn);color:var(--btn);border-radius:5px;padding:.2em .6em;cursor:pointer;font-size:.82em">Editar</button>'
        +'<button onclick="deleteRecipe('+r.id+',this)" style="background:none;border:1px solid var(--dan);color:var(--dan);border-radius:5px;padding:.2em .6em;cursor:pointer;font-size:.82em">&#128465;</button>'
        +'</div>';
      el.appendChild(div);
    });
  }).catch(function(){});
}

function showRecipeForm(recipe){
  D.getElementById('recipe-list-view').style.display='none';
  D.getElementById('recipe-form-view').style.display='block';
  _steps=[];
  D.getElementById('steps-list').innerHTML='';
  if(recipe){
    D.getElementById('recipe-form-title').textContent='Editar Receita';
    D.getElementById('rf-id').value=recipe.id||0;
    D.getElementById('rf-title').value=recipe.title||'';
    D.getElementById('rf-grind').value=recipe.grind||'';
    D.getElementById('rf-water').value=recipe.waterTotal||'';
    D.getElementById('rf-coffee').value=recipe.coffee||'';
    D.getElementById('rf-heat').value=recipe.heat||'';
    (recipe.steps||[]).forEach(function(s){addStepRow(s)});
  } else {
    D.getElementById('recipe-form-title').textContent='Nova Receita';
    D.getElementById('rf-id').value=0;
    ['rf-title','rf-grind','rf-water','rf-coffee','rf-heat'].forEach(function(id){D.getElementById(id).value=''});
  }
  updateWaterLeft();
}

function cancelRecipeForm(){
  D.getElementById('recipe-form-view').style.display='none';
  D.getElementById('recipe-list-view').style.display='block';
}

function editRecipe(id){
  fetch('/recipe?id='+id).then(function(r){return r.json()}).then(function(recipe){
    showRecipeForm(recipe);
  }).catch(function(){});
}

function deleteRecipe(id,btn){
  if(!confirm('Apagar esta receita?')) return;
  fetch('/recipe?id='+id+'\x26_method=DELETE',{method:'POST'})
  .then(function(){loadRecipeList()}).catch(function(){});
}

function updateWaterLeft(){
  var total=parseFloat(D.getElementById('rf-water').value)||0;
  var used=_steps.reduce(function(s,st){return s+(parseFloat(st.water)||0)},0);
  var left=total-used;
  D.getElementById('water-left').textContent=left.toFixed(1)+'ml';
  var totalSecs=_steps.reduce(function(s,st){return s+(parseInt(st.duration)||0)},0);
  D.getElementById('total-time').textContent=fmtTime(totalSecs);
}

function addStep(){
  // Suggest remaining water; zero for Aguardar
  var total=parseFloat(D.getElementById('rf-water').value)||0;
  var used=_steps.reduce(function(s,st){return s+(parseFloat(st.water)||0)},0);
  var left=Math.max(0,total-used);
  addStepRow({type:'Despejo',water:left>0?left:'',duration:'',detail:''});
}

function addStepRow(s){
  var idx=_steps.length;
  _steps.push(s);
  var div=D.createElement('div');
  div.id='step-'+idx;
  div.style.cssText='border:1px solid var(--bdr);border-radius:7px;padding:.7em;margin:.4em 0;background:var(--bg)';
  var opts=STEP_TYPES.map(function(t){return'<option'+(t===s.type?' selected':'')+'>'+t+'</option>'}).join('');
  div.innerHTML='<div style="display:flex;justify-content:space-between;align-items:center;margin-bottom:.4em">'
    +'<strong style="font-size:.85em">Etapa '+(idx+1)+'</strong>'
    +'<button onclick="removeStep('+idx+')" style="background:none;border:none;color:var(--dan);cursor:pointer;font-size:1.1em">&#10005;</button></div>'
    +'<select id="st-type-'+idx+'" onchange="stUpd('+idx+')" style="width:100%;padding:.45em;border:1px solid var(--bdr);border-radius:5px;background:var(--inp);color:var(--text);margin-bottom:.3em">'+opts+'</select>'
    +'<div style="display:flex;gap:.4em;margin-bottom:.3em">'
    +'<div style="flex:1"><label style="font-size:.78em;color:var(--sub)">Água (ml)</label>'
    +'<input id="st-water-'+idx+'" type="number" min="0" value="'+(s.water||'')+'" oninput="stUpd('+idx+');updateWaterLeft()" style="width:100%;padding:.45em;border:1px solid var(--bdr);border-radius:5px;background:var(--inp);color:var(--text)"></div>'
    +'<div style="flex:1"><label style="font-size:.78em;color:var(--sub)">Duração (s)</label>'
    +'<input id="st-dur-'+idx+'" type="number" min="0" value="'+(s.duration||'')+'" oninput="stUpd('+idx+')" style="width:100%;padding:.45em;border:1px solid var(--bdr);border-radius:5px;background:var(--inp);color:var(--text)"></div></div>'
    +'<label style="font-size:.78em;color:var(--sub)">Detalhe</label>'
    +'<input id="st-detail-'+idx+'" type="text" value="'+(s.detail||'')+'" oninput="stUpd('+idx+')" placeholder="Opcional" style="width:100%;padding:.45em;border:1px solid var(--bdr);border-radius:5px;background:var(--inp);color:var(--text)">';
  D.getElementById('steps-list').appendChild(div);
  stUpd(idx);
  updateWaterLeft();
}

function stUpd(idx){
  var type=D.getElementById('st-type-'+idx).value;
  var waterEl=D.getElementById('st-water-'+idx);
  if(type==='Aguardar'){waterEl.value='0';waterEl.disabled=true;}
  else waterEl.disabled=false;
  _steps[idx]={
    type:type,
    water:type==='Aguardar'?0:(parseFloat(waterEl.value)||0),
    duration:parseInt(D.getElementById('st-dur-'+idx).value)||0,
    detail:D.getElementById('st-detail-'+idx).value
  };
  updateWaterLeft();
}

function removeStep(idx){
  _steps.splice(idx,1);
  // Rebuild all steps
  var saved=_steps.slice();
  _steps=[];
  D.getElementById('steps-list').innerHTML='';
  saved.forEach(function(s){addStepRow(s)});
  updateWaterLeft();
}

function saveRecipe(){
  var id=parseInt(D.getElementById('rf-id').value)||0;
  var recipe={
    id:id||undefined,
    title:D.getElementById('rf-title').value.trim(),
    grind:D.getElementById('rf-grind').value.trim(),
    waterTotal:parseFloat(D.getElementById('rf-water').value)||0,
    coffee:parseFloat(D.getElementById('rf-coffee').value)||0,
    heat:parseInt(D.getElementById('rf-heat').value)||0,
    steps:_steps.map(function(s,i){
      stUpd(i);return _steps[i];
    })
  };
  if(!recipe.title){D.getElementById('rf-title').focus();return}
  // Validate all water is used
  var totalW=recipe.waterTotal;
  var usedW=recipe.steps.reduce(function(s,st){return s+(parseFloat(st.water)||0)},0);
  if(totalW>0 && Math.abs(totalW-usedW)>0.5){
    showMsg('recipe-msg',false,'Agua nao distribuida completamente ('+usedW.toFixed(1)+'ml de '+totalW+'ml usada)');
    return;
  }
  // Store totalSecs in recipe
  recipe.totalSecs=recipe.steps.reduce(function(s,st){return s+(parseInt(st.duration)||0)},0);
  fetch('/recipe',{method:'POST',headers:{'Content-Type':'application/json'},body:JSON.stringify(recipe)})
  .then(function(r){return r.json()}).then(function(){
    showMsg('recipe-msg',true,'Receita salva!');
    setTimeout(function(){cancelRecipeForm();loadRecipeList();},1200);
  }).catch(function(){showMsg('recipe-msg',false,'Erro ao salvar.')});
}

// Load recipes when tab is opened
var recipesLoaded=false;
function showTab(id,btn){
  D.querySelectorAll('.pane').forEach(function(p){p.classList.remove('active')});
  D.querySelectorAll('.tab').forEach(function(b){b.classList.remove('active')});
  D.getElementById('pane-'+id).classList.add('active');
  btn.classList.add('active');
  localStorage.setItem('tab',id);
  if(id==='wifi' && !wifiLoaded) loadWifi();
  if(id==='receitas') loadRecipeList();
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
fetch('/build').then(function(r){return r.text()}).then(function(t){
  document.getElementById('ver').textContent=t;
}).catch(function(){});
</script>
</body></html>
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
    _eventBus.subscribe(events::EventType::Button1Pressed,     this);
    _eventBus.subscribe(events::EventType::Button1LongPressed, this);
    _eventBus.subscribe(events::EventType::Button2Pressed,     this);
    _eventBus.subscribe(events::EventType::Button2LongPressed, this);
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
        // Mirror physical button activity to the web UI. Fires for web-originated
        // presses too, so both sources give identical on-screen feedback.
        case events::EventType::Button1Pressed:     ++_b1Clicks; break;
        case events::EventType::Button1LongPressed: ++_b1Longs;  break;
        case events::EventType::Button2Pressed:     ++_b2Clicks; break;
        case events::EventType::Button2LongPressed: ++_b2Longs;  break;
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
    _server.on("/button",         [this]() { handleButtonAction(); });
    _server.on("/recipe/start",   [this]() { handleRecipeStart(); });
    _server.on("/build",          [this]() { handleBuildInfo(); });
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
    _server.on("/recipes",        [this]() { handleRecipeList(); });
    _server.on("/recipe",         [this]() {
        String method = _server.arg("_method");
        if (method == "DELETE") handleRecipeDelete();
        else if (!_server.arg("id").isEmpty() && _server.arg("title").isEmpty()) handleRecipeGet();
        else handleRecipeSave();
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

// Served as its own endpoint rather than substituted into the HTML: the config
// page is ~30 KB, and copying it on the heap just to swap a placeholder would
// cost tens of KB per request with WiFi up. The pages stay static literals.
void WebApp::handleBuildInfo() {
    char buf[96];
    snprintf(buf, sizeof(buf), "v%s \xC2\xB7 %s \xC2\xB7 %s",
             FIRMWARE_VERSION, BUILD_STAMP, BUILD_GIT_HASH);
    _server.send(200, "text/plain; charset=utf-8", buf);
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
    uint32_t tsec      = _timer ? _timer->elapsedSeconds() : 0;
    bool     trun      = _timer ? _timer->isRunning()      : false;

    bool     ract  = _recipeMgr && _recipeMgr->isRecipeActive();
    uint16_t rid   = ract ? _recipeMgr->activeRecipeId()    : 0;
    uint8_t  rstep = ract ? _recipeMgr->currentStepIndex()  : 0;
    uint8_t  rn    = ract ? _recipeMgr->stepCount()         : 0;
    uint32_t rel   = ract ? _recipeMgr->stepElapsedSecs()   : 0;
    uint32_t rrem  = ract ? _recipeMgr->stepRemainingSecs() : 0;
    uint32_t rtot  = ract ? _recipeMgr->totalElapsedSecs()  : 0;
    bool     rrun  = ract && _recipeMgr->isStepRunning();

    char    buf[320];
    snprintf(buf, sizeof(buf),
             "{\"grams\":%.2f,\"raw\":%ld,\"calibrated\":%s,\"ready\":%s,"
             "\"b1c\":%u,\"b1l\":%u,\"b2c\":%u,\"b2l\":%u,"
             "\"tsec\":%lu,\"trun\":%s,"
             "\"ract\":%s,\"rid\":%u,\"rstep\":%u,\"rn\":%u,"
             "\"rel\":%lu,\"rrem\":%lu,\"rtot\":%lu,\"rrun\":%s}",
             grams, (long)raw,
             calibrated ? "true" : "false",
             ready      ? "true" : "false",
             _b1Clicks, _b1Longs, _b2Clicks, _b2Longs,
             (unsigned long)tsec, trun ? "true" : "false",
             ract ? "true" : "false", rid, rstep, rn,
             (unsigned long)rel, (unsigned long)rrem, (unsigned long)rtot,
             rrun ? "true" : "false");
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

void WebApp::handleRecipeList() {
    if (!_recipes) { _server.send(200, "application/json", "[]"); return; }
    String list = _recipes->listRecipes();
    _server.send(200, "application/json", list.c_str());
}

void WebApp::handleRecipeGet() {
    if (!_recipes) { _server.send(404, "text/plain", "not found"); return; }
    uint16_t id = static_cast<uint16_t>(_server.arg("id").toInt());
    String json = _recipes->loadRecipe(id);
    if (json.isEmpty()) _server.send(404, "text/plain", "not found");
    else                _server.send(200, "application/json", json.c_str());
}

void WebApp::handleRecipeSave() {
    if (!_recipes) { _server.send(500, "text/plain", "no storage"); return; }
    // Body comes as plain JSON via POST
    String body = _server.arg("plain");
    if (body.isEmpty()) { _server.send(400, "text/plain", "empty body"); return; }
    uint16_t id = _recipes->saveRecipe(body.c_str());
    if (id == 0) { _server.send(500, "text/plain", "save failed"); return; }
    char buf[32];
    snprintf(buf, sizeof(buf), "{\"id\":%u}", id);
    _server.send(200, "application/json", buf);
}

// Start (or cancel, with id=0) a recipe from the web. Drives the very same
// RecipeManager state machine the device menu drives.
void WebApp::handleRecipeStart() {
    if (!_recipeMgr) { _server.send(503, "text/plain", "no recipe manager"); return; }
    String idStr = _server.arg("id");
    if (idStr.isEmpty()) { _server.send(400, "text/plain", "missing id"); return; }
    uint16_t id = static_cast<uint16_t>(idStr.toInt());
    if (!_recipeMgr->startRecipe(id)) { _server.send(404, "text/plain", "not found"); return; }
    _server.send(200, "text/plain", "OK");
}

// Emit the same event pair a physical button produces: Down gives the LED flash
// and buzzer feedback, then Pressed/LongPressed triggers the actual action.
void WebApp::handleButtonAction() {
    String n = _server.arg("n");
    String a = _server.arg("a");

    bool isLong = (a == "long");
    if (!isLong && a != "click") { _server.send(400, "text/plain", "bad action"); return; }

    if (n == "1") {
        _eventBus.publish({events::EventType::Button1Down});
        _eventBus.publish({isLong ? events::EventType::Button1LongPressed
                                  : events::EventType::Button1Pressed});
    } else if (n == "2") {
        _eventBus.publish({events::EventType::Button2Down});
        _eventBus.publish({isLong ? events::EventType::Button2LongPressed
                                  : events::EventType::Button2Pressed});
    } else {
        _server.send(400, "text/plain", "bad button");
        return;
    }

    _server.send(200, "text/plain", "OK");
}

void WebApp::handleRecipeDelete() {
    if (!_recipes) { _server.send(500, "text/plain", "no storage"); return; }
    uint16_t id = static_cast<uint16_t>(_server.arg("id").toInt());
    if (_recipes->deleteRecipe(id)) _server.send(200, "text/plain", "OK");
    else                            _server.send(404, "text/plain", "not found");
}

} // namespace core
