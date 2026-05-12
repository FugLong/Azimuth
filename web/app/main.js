const {$,setToggle,syncRangeLabels,clamp255,setLedRgb,syncLedManualUi,fillInput,nudgeInputPaint}=window.AppUi;
const {applyShell}=window.AppViews;
const {applyOtAxesFromStatus}=window.AppStateFns;
const {onScan,onSave,onReboot,onBatteryCal,onFactoryReset,onUpdateNow}=window.AppControllers;
const uiTouched=window.AppState.uiTouched;
const monitorCfg=(window.AppConfig&&window.AppConfig.monitor)||{};
const POWER_IDLE_POLL_ACTIVE_MS=monitorCfg.idlePollActiveMs||18000;
const POWER_IDLE_POLL_VISIBLE_MS=monitorCfg.idlePollVisibleMs||12000;
const POWER_IDLE_GRACE_MS=monitorCfg.idleGraceMs||60000;
const POWER_BOOTSTRAP_RETRY_MS=monitorCfg.bootstrapRetryMs||3000;
const ACTIVITY_RESCHEDULE_MIN_MS=monitorCfg.activityRescheduleMinMs||1500;
const STALE_UI_AFTER_FAILURES=monitorCfg.staleUiAfterFailures||2;
const BOOT_SYNC_BURST_COUNT=monitorCfg.bootSyncBurstCount||6;
const BOOT_SYNC_BURST_GAP_MS=monitorCfg.bootSyncBurstGapMs||5000;
const FOCUS_SYNC_BURST_COUNT=monitorCfg.focusSyncBurstCount||4;
const FOCUS_SYNC_BURST_GAP_MS=monitorCfg.focusSyncBurstGapMs||3500;
const MANIFEST_PENDING_POLL_MS=monitorCfg.manifestPendingPollMs||5000;
const IMU_LIVE_POLL_MS=monitorCfg.posePreviewPollMs||700;
const TAB_COORD_KEY='azimuth.portal.activeTabs.v1';
const TAB_HEARTBEAT_MS=3000;
const TAB_STALE_MS=12000;
const TAB_MAX_ACTIVE=2;
let hydrateOk=false;
let pollFailCount=0;
let posePollTimer=0;
let tabHeartbeatTimer=0;
let tabAdmitted=true;
const tabId='tab_'+Math.random().toString(36).slice(2)+'_'+Date.now().toString(36);
/** One-shot shorter delay after `/api/status` (e.g. manifest check still running on device). */
let preferNextPollMs=null;
let burstSeriesId=0;
let didBootStatusBurst=false;

function syncTrackingHeroFromPose(j){
  const htr=$('heroTrack');
  const htrk=$('heroTrackSub');
  if(!htr)return;
  if(j.thermal_hold){
    htr.textContent='Wi‑Fi off';
    if(htrk)htrk.textContent='Cooling — USB tracking may still run';
    return;
  }
  if(j.stasis){
    htr.textContent='Paused';
    if(htrk)htrk.textContent='Low power';
    return;
  }
  const im=j.imu_period_ms;
  const hz=im?Math.round(1000/im):'—';
  const dyn=!!j.imu_dynamic;
  htr.textContent=dyn?('Var · ~'+hz+' Hz peak'):('~'+hz+' Hz');
  if(htrk)htrk.textContent=dyn?'Variable IMU rate':(im?'Update rate':'');
}

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
  if(!uiTouched.imuDyn){
    const imuDynEl=$('imuDynamicToggle');
    if(imuDynEl)setToggle('imuDynamicToggle',!!j.imu_dynamic);
  }
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
  uiTouched.udp=uiTouched.mdns=uiTouched.hatire=uiTouched.imuDyn=false;
  applyLiveStatus(j);
  nudgeInputPaint();
  hydrateOk=true;
  window.AppState.power.lastStatusOkMs=Date.now();
  clearBootstrapTimer();
  if(j.fw_update_check_done===false&&!j.setup_ap){
    preferNextPollMs=MANIFEST_PENDING_POLL_MS;
  }
  if(!didBootStatusBurst){
    didBootStatusBurst=true;
    scheduleStatusBurst({count:BOOT_SYNC_BURST_COUNT,gapMs:BOOT_SYNC_BURST_GAP_MS});
  }
}
/**
 * Extra `/api/status` polls spaced by `gapMs` (boot, tab focus, OTA reload).
 * Cancels any previous burst series so navigations do not stack timers.
 */
function scheduleStatusBurst(opts){
  const count=(opts&&opts.count)||4;
  const gapMs=(opts&&opts.gapMs)||4000;
  if(!count||gapMs<=0)return;
  const series=++burstSeriesId;
  markUserActivity(true);
  let fired=0;
  function tick(){
    if(series!==burstSeriesId)return;
    if(fired>=count)return;
    fired++;
    pollOnly().finally(()=>{
      if(series!==burstSeriesId)return;
      if(fired<count)setTimeout(tick,gapMs);
    });
  }
  setTimeout(tick,gapMs);
}
async function pollOnly(){
  try{
    const j=await window.AppApi.getStatus();
    applyLiveStatus(j);
    hydrateOk=true;
    pollFailCount=0;
    window.AppState.power.lastStatusOkMs=Date.now();
    clearBootstrapTimer();
    if(j.fw_update_check_done===false&&!j.setup_ap){
      preferNextPollMs=MANIFEST_PENDING_POLL_MS;
    }
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
function readTabRegistry(){
  try{
    const raw=localStorage.getItem(TAB_COORD_KEY);
    if(!raw)return {};
    const o=JSON.parse(raw);
    return(o&&typeof o==='object')?o:{};
  }catch(e){return {}}
}
function writeTabRegistry(reg){
  try{localStorage.setItem(TAB_COORD_KEY,JSON.stringify(reg));}catch(e){}
}
function pruneTabRegistry(reg,now){
  Object.keys(reg).forEach(id=>{
    const rec=reg[id];
    if(!rec||typeof rec!=='object'||!Number.isFinite(rec.ts)||now-rec.ts>TAB_STALE_MS){
      delete reg[id];
    }
  });
}
function setTabAdmitted(next){
  if(tabAdmitted===next)return;
  tabAdmitted=next;
  if(!tabAdmitted){
    clearPollTimer();
    clearPosePollTimer();
  }else{
    markUserActivity(true);
    pollOnly();
    schedulePosePoll();
  }
}
function refreshTabAdmission(){
  const now=Date.now();
  const reg=readTabRegistry();
  pruneTabRegistry(reg,now);
  reg[tabId]={ts:now,visible:!document.hidden};
  const visibleActive=Object.entries(reg)
    .filter(([,rec])=>rec&&rec.visible===true&&Number.isFinite(rec.ts)&&now-rec.ts<=TAB_STALE_MS)
    .sort((a,b)=>b[1].ts-a[1].ts)
    .map(([id])=>id);
  const allowed=new Set(visibleActive.slice(0,TAB_MAX_ACTIVE));
  writeTabRegistry(reg);
  setTabAdmitted(!document.hidden&&allowed.has(tabId));
}
function clearTabHeartbeat(){
  if(!tabHeartbeatTimer)return;
  clearTimeout(tabHeartbeatTimer);
  tabHeartbeatTimer=0;
}
function scheduleTabHeartbeat(){
  clearTabHeartbeat();
  tabHeartbeatTimer=setTimeout(()=>{
    tabHeartbeatTimer=0;
    refreshTabAdmission();
    scheduleTabHeartbeat();
  },TAB_HEARTBEAT_MS);
}
function shouldPollNow(){
  if(document.hidden||!tabAdmitted)return false;
  return true;
}
function pollDelayMs(){
  if(!shouldPollNow())return 0;
  if(preferNextPollMs!=null){
    const v=preferNextPollMs;
    preferNextPollMs=null;
    return v;
  }
  // Visible tab: keep polling at least every `idlePollVisibleMs` even when the
  // user is not touching the screen (hardware FUNC pause, reading settings).
  if((Date.now()-lastUserActivityMs)>POWER_IDLE_GRACE_MS){
    return POWER_IDLE_POLL_VISIBLE_MS;
  }
  return POWER_IDLE_POLL_ACTIVE_MS;
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
function clearPosePollTimer(){
  if(!posePollTimer)return;
  clearTimeout(posePollTimer);
  posePollTimer=0;
}
function schedulePosePoll(){
  clearPosePollTimer();
  if(document.hidden||!tabAdmitted)return;
  if(!(window.AppPoseMascot&&typeof window.AppPoseMascot.isPreviewActive==='function'&&window.AppPoseMascot.isPreviewActive())){
    return;
  }
  posePollTimer=setTimeout(async()=>{
    posePollTimer=0;
    if(document.hidden){
      schedulePosePoll();
      return;
    }
    try{
      const j=await window.AppApi.getPose();
      if(window.AppPoseMascot&&typeof window.AppPoseMascot.applyPose==='function'){
        window.AppPoseMascot.applyPose(j);
      }
      syncTrackingHeroFromPose(j);
    }catch(e){}
    schedulePosePoll();
  },IMU_LIVE_POLL_MS);
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
const imuDynToggleEl=$('imuDynamicToggle');
if(imuDynToggleEl){
  imuDynToggleEl.onclick=()=>{
    uiTouched.imuDyn=true;
    setToggle('imuDynamicToggle',!$('imuDynamicToggle').classList.contains('on'));
  };
}
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
const _btnUpdateManualWifi=$('btnUpdateManualWifi');
if(_btnUpdateManualWifi)_btnUpdateManualWifi.onclick=()=>onUpdateNow();
const _btnUpdateBannerWifi=$('btnUpdateBannerWifi');
if(_btnUpdateBannerWifi)_btnUpdateBannerWifi.onclick=()=>onUpdateNow();
if(window.AppUpdateProgress&&typeof window.AppUpdateProgress.init==='function'){
  window.AppUpdateProgress.init();
}
if(window.AppSections&&typeof window.AppSections.init==='function'){
  window.AppSections.init();
}
if(window.AppPoseMascot&&typeof window.AppPoseMascot.init==='function'){
  window.AppPoseMascot.init();
}
refreshTabAdmission();
scheduleTabHeartbeat();
window.addEventListener('azimuth:pose-preview-active',()=>{
  schedulePosePoll();
  if(window.AppPoseMascot&&typeof window.AppPoseMascot.isPreviewActive==='function'&&window.AppPoseMascot.isPreviewActive()){
    pollOnly();
  }
});
hydrateForm().catch(()=>{
  // Keep trying quickly until the first successful status arrives.
  scheduleBootstrapRetry();
});
['pointerdown','pointermove','keydown','touchstart','scroll'].forEach(ev=>{
  const force=(ev==='pointerdown'||ev==='keydown'||ev==='touchstart');
  window.addEventListener(ev,()=>markUserActivity(force),{passive:true});
});
document.addEventListener('visibilitychange',()=>{
  refreshTabAdmission();
  if(!document.hidden){
    markUserActivity(true);
    if(tabAdmitted){
      pollOnly();
      scheduleStatusBurst({count:FOCUS_SYNC_BURST_COUNT,gapMs:FOCUS_SYNC_BURST_GAP_MS});
      schedulePosePoll();
    }
    if(!hydrateOk)scheduleBootstrapRetry();
  }else{
    clearPollTimer();
    clearPosePollTimer();
    clearBootstrapTimer();
    burstSeriesId++;
  }
});
window.addEventListener('pageshow',e=>{
  if(!e.persisted)return;
  refreshTabAdmission();
  markUserActivity(true);
  if(tabAdmitted){
    pollOnly();
    scheduleStatusBurst({count:FOCUS_SYNC_BURST_COUNT,gapMs:FOCUS_SYNC_BURST_GAP_MS});
    schedulePosePoll();
  }
});
window.addEventListener('online',()=>{
  markUserActivity(true);
  if(tabAdmitted&&!document.hidden){
    pollOnly();
    scheduleStatusBurst({count:2,gapMs:2500});
  }
});
window.addEventListener('storage',e=>{
  if(e.key===TAB_COORD_KEY)refreshTabAdmission();
});
window.addEventListener('beforeunload',()=>{
  const reg=readTabRegistry();
  delete reg[tabId];
  writeTabRegistry(reg);
  clearTabHeartbeat();
});
schedulePowerAwarePoll();
scheduleBootstrapRetry();
