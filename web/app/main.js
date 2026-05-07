const {$,setToggle,syncRangeLabels,clamp255,setLedRgb,syncLedManualUi,fillInput,nudgeInputPaint}=window.AppUi;
const {applyShell}=window.AppViews;
const {applyOtAxesFromStatus}=window.AppStateFns;
const {onScan,onSave,onReboot,onBatteryCal,onFactoryReset}=window.AppControllers;
const uiTouched=window.AppState.uiTouched;
const monitorCfg=(window.AppConfig&&window.AppConfig.monitor)||{};
const POWER_IDLE_POLL_ACTIVE_MS=monitorCfg.idlePollActiveMs||25000;
const POWER_IDLE_POLL_HEARTBEAT_MS=monitorCfg.idlePollHeartbeatMs||60000;
const POWER_IDLE_GRACE_MS=monitorCfg.idleGraceMs||60000;
const POWER_BOOTSTRAP_RETRY_MS=monitorCfg.bootstrapRetryMs||3000;
const ACTIVITY_RESCHEDULE_MIN_MS=monitorCfg.activityRescheduleMinMs||1500;
const STALE_UI_AFTER_FAILURES=monitorCfg.staleUiAfterFailures||2;
let hydrateOk=false;
let pollFailCount=0;

function applyLiveStatus(j){
  applyShell(j);
}

async function hydrateForm(){
  const j=await window.AppApi.getStatus();
  fillInput($('ssid'),j.ssid||'');
  fillInput($('pass'),'');
  fillInput($('hostname'),j.hostname||'');
  fillInput($('otHost'),j.ot_host||'');
  fillInput($('otPort'),j.ot_port||4242);
  const p=j.imu_period_ms||10;
  $('imuPeriod').value=[5,10,20,40].includes(p)?String(p):'10';
  $('wifiTx').value=([0,1,2].includes(j.wifi_tx))?String(j.wifi_tx):'1';
  const bc=$('batteryCapacity');
  if(bc){
    const v=parseInt(j.battery_capacity_mah,10);
    bc.value=Number.isFinite(v)?String(v):'800';
  }
  const rb=$('rgbBrightness'),bv=$('buzzerVolume');
  if(rb){
    const x=j.rgb_brightness;
    rb.value=(x!=null&&x>=0&&x<=100)?String(x):'25';
    rb.setAttribute('aria-valuenow',rb.value);
  }
  if(bv){
    const x=j.buzzer_volume;
    bv.value=(x!=null&&x>=0&&x<=100)?String(x):'25';
    bv.setAttribute('aria-valuenow',bv.value);
  }
  const lm=$('ledMode');
  if(lm){
    const m=j.led_mode;
    lm.value=([0,1,2,3].includes(Number(m)))?String(m):'0';
  }
  const lr=$('ledR'),lg=$('ledG'),lb=$('ledB');
  if(lr){
    const rv=(x)=>(x!=null&&x>=0&&x<=255)?clamp255(x):null;
    const r=rv(j.led_r),g=rv(j.led_g),b=rv(j.led_b);
    if(r!=null)lr.value=String(r);
    if(lg&&g!=null)lg.value=String(g);
    if(lb&&b!=null)lb.value=String(b);
  }
  syncRangeLabels();
  syncLedManualUi();
  applyOtAxesFromStatus(j.ot_axes);
  uiTouched.udp=uiTouched.mdns=uiTouched.hatire=false;
  applyLiveStatus(j);
  nudgeInputPaint();
  hydrateOk=true;
  window.AppState.power.lastStatusOkMs=Date.now();
  clearBootstrapTimer();
}
async function pollOnly(){
  try{
    applyLiveStatus(await window.AppApi.getStatus());
    hydrateOk=true;
    pollFailCount=0;
    window.AppState.power.lastStatusOkMs=Date.now();
    clearBootstrapTimer();
  }catch(e){
    pollFailCount++;
    if(pollFailCount>=STALE_UI_AFTER_FAILURES){
      const s=$('subLine');
      if(s){
        const msSinceOk=Date.now()-(window.AppState.power.lastStatusOkMs||0);
        const age=(window.AppState.power.lastStatusOkMs>0)?Math.max(1,Math.round(msSinceOk/1000)):'?';
        s.textContent='Connection lost — retrying (last update '+age+'s ago)…';
      }
    }
  }
}
let bootstrapTimer=0;
let lastUserActivityMs=window.AppState.power.lastUserActivityMs;
let pollTimer=window.AppState.power.pollTimer;
function markUserActivity(force){
  const now=Date.now();
  if(!force&&(now-lastUserActivityMs)<ACTIVITY_RESCHEDULE_MIN_MS)return;
  lastUserActivityMs=now;
  window.AppState.power.lastUserActivityMs=lastUserActivityMs;
  schedulePowerAwarePoll();
}
function shouldPollNow(){
  if(document.hidden)return false;
  return true;
}
function pollDelayMs(){
  if(!shouldPollNow())return 0;
  return (Date.now()-lastUserActivityMs)<=POWER_IDLE_GRACE_MS
    ?POWER_IDLE_POLL_ACTIVE_MS
    :POWER_IDLE_POLL_HEARTBEAT_MS;
}
function clearPollTimer(){
  if(pollTimer){
    clearTimeout(pollTimer);
    pollTimer=0;
    window.AppState.power.pollTimer=0;
  }
}
function clearBootstrapTimer(){
  if(bootstrapTimer){
    clearTimeout(bootstrapTimer);
    bootstrapTimer=0;
  }
}
function scheduleBootstrapRetry(){
  clearBootstrapTimer();
  if(hydrateOk||document.hidden)return;
  bootstrapTimer=setTimeout(async()=>{
    if(hydrateOk||document.hidden)return;
    await pollOnly();
    scheduleBootstrapRetry();
  },POWER_BOOTSTRAP_RETRY_MS);
}
function schedulePowerAwarePoll(){
  clearPollTimer();
  const d=pollDelayMs();
  if(!d)return;
  pollTimer=setTimeout(async()=>{
    pollTimer=0;
    if(shouldPollNow())await pollOnly();
    schedulePowerAwarePoll();
  },d);
  window.AppState.power.pollTimer=pollTimer;
}
$('udpToggle').onclick=()=>{uiTouched.udp=true;setToggle('udpToggle',!$('udpToggle').classList.contains('on'))};
$('mdnsToggle').onclick=()=>{uiTouched.mdns=true;setToggle('mdnsToggle',!$('mdnsToggle').classList.contains('on'))};
$('hatireToggle').onclick=()=>{uiTouched.hatire=true;setToggle('hatireToggle',!$('hatireToggle').classList.contains('on'))};
[0,1,2].forEach(i=>{const id='otInv'+i;$(id).onclick=()=>setToggle(id,!$(id).classList.contains('on'));});
const _rb=$('rgbBrightness'),_bv=$('buzzerVolume');
if(_rb)_rb.addEventListener('input',syncRangeLabels);
if(_bv)_bv.addEventListener('input',syncRangeLabels);
['ledR','ledG','ledB'].forEach(id=>{const el=$(id);if(el)el.addEventListener('input',syncLedManualUi);});
const _lm=$('ledMode');
if(_lm)_lm.addEventListener('change',syncLedManualUi);
document.querySelectorAll('[data-led-rgb]').forEach(btn=>{
  btn.addEventListener('click',()=>{
    const raw=btn.getAttribute('data-led-rgb')||'';
    const p=raw.split(',').map(x=>clamp255(x.trim()));
    if(p.length!==3)return;
    const lm=$('ledMode');
    if(lm)lm.value='3';
    setLedRgb(p[0],p[1],p[2]);
  });
});
$('btnUseClientIp').onclick=()=>{
  const v=$('clientIpVal')&&$('clientIpVal').textContent;
  if(v&&v!=='—'){fillInput($('otHost'),v)}
};
$('btnScan').onclick=()=>onScan();
$('btnSave').onclick=()=>onSave(hydrateForm);
$('btnReboot').onclick=()=>onReboot();
$('btnBatteryCal').onclick=()=>onBatteryCal(hydrateForm);
$('btnFactory').onclick=()=>onFactoryReset();
if(window.AppSections&&typeof window.AppSections.init==='function'){
  window.AppSections.init();
}
hydrateForm().catch(()=>{
  // Keep trying quickly until the first successful status arrives.
  scheduleBootstrapRetry();
});
['pointerdown','pointermove','keydown','touchstart','scroll'].forEach(ev=>{
  const force=(ev==='pointerdown'||ev==='keydown'||ev==='touchstart');
  window.addEventListener(ev,()=>markUserActivity(force),{passive:true});
});
document.addEventListener('visibilitychange',()=>{
  if(!document.hidden){
    markUserActivity(true);
    pollOnly();
    if(!hydrateOk)scheduleBootstrapRetry();
  }else{
    clearPollTimer();
    clearBootstrapTimer();
  }
});
schedulePowerAwarePoll();
scheduleBootstrapRetry();
