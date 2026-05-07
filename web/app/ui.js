window.AppUi=(function(){
  const $=id=>document.getElementById(id);

  function setMsg(t,cls){
    const m=$('msg');
    m.textContent=t||'';
    m.className=cls||'';
  }

  function setToggle(id,on){
    $(id).classList.toggle('on',on);
  }

  function updateSoundLightCard(j){
    const card=$('cardSoundLight'),hint=$('soundLightHint');
    const rr=$('rgbBrightnessRow'),br=$('buzzerVolumeRow'),lm=$('ledModeRow');
    if(!card)return;
    const hasRgb=!!j.has_rgb,hasBz=!!j.has_buzzer;
    const show=hasRgb||hasBz;
    const inSoundSection=window.AppSections&&typeof window.AppSections.currentSection==='function'
      ?window.AppSections.currentSection()==='sound'
      :true;
    card.style.display=(show&&inSoundSection)?'block':'none';
    if(hint)hint.style.display=show?'block':'none';
    if(rr)rr.style.display=hasRgb?'block':'none';
    if(lm)lm.style.display=hasRgb?'block':'none';
    if(br)br.style.display=hasBz?'block':'none';
  }

  function syncRangeLabels(){
    const rb=$('rgbBrightness'),rv=$('rgbBrightnessVal');
    const bb=$('buzzerVolume'),bv=$('buzzerVolumeVal');
    if(rb&&rv)rv.textContent=rb.value+'%';
    if(bb&&bv)bv.textContent=bb.value+'%';
  }

  function clamp255(x){
    const n=Math.round(Number(x));
    if(!Number.isFinite(n))return 0;
    return Math.max(0,Math.min(255,n));
  }

  function setLedRgb(r,g,b){
    const er=$('ledR'),eg=$('ledG'),eb=$('ledB');
    if(er)er.value=String(clamp255(r));
    if(eg)eg.value=String(clamp255(g));
    if(eb)eb.value=String(clamp255(b));
    syncLedManualUi();
  }

  function syncLedManualUi(){
    const row=$('ledManualRow'),lm=$('ledMode'),hint=$('ledModeHint');
    const sw=$('ledSwatch'),rv=$('ledRVal'),gv=$('ledGVal'),bv=$('ledBVal');
    const er=$('ledR'),eg=$('ledG'),eb=$('ledB');
    if(!lm)return;
    const mode=parseInt(lm.value,10);
    const manual=mode===3;
    if(row)row.style.display=manual?'block':'none';
    const r=er?clamp255(er.value):0,g=eg?clamp255(eg.value):0,b=eb?clamp255(eb.value):0;
    if(rv)rv.textContent=String(r);
    if(gv)gv.textContent=String(g);
    if(bv)bv.textContent=String(b);
    if(sw)sw.style.background='rgb('+r+','+g+','+b+')';
    if(hint){
      if(mode===0)hint.textContent='Smooth rainbow. Overrides (thermal, low battery, setup Wi‑Fi, pause) still win on the device.';
      else if(mode===1)hint.textContent='Slower rainbow. Same override rules as other modes.';
      else if(mode===2)hint.textContent='Green accent while the IMU reports OK; dim when waiting for data.';
      else hint.textContent='Solid color from the sliders below (after Save). RGB brightness above still scales output. System warnings override this LED.';
    }
  }

  function fillInput(el,v){
    if(!el)return;
    const s=v==null?'':String(v);
    el.value=s;
    el.setAttribute('value',s);
    el.defaultValue=s;
  }

  function nudgeInputPaint(){
    requestAnimationFrame(()=>{
      ['ssid','hostname','otHost','otPort','otSrc0','otSrc1','otSrc2','imuPeriod','wifiTx','rgbBrightness','ledMode','ledR','ledG','ledB','buzzerVolume','batteryCapacity'].forEach(id=>{
        const el=$(id);
        if(!el)return;
        el.style.transform='translateZ(1px)';
        void el.offsetHeight;
        el.style.transform='';
      });
    });
  }

  return {
    $,
    setMsg,
    setToggle,
    updateSoundLightCard,
    syncRangeLabels,
    clamp255,
    setLedRgb,
    syncLedManualUi,
    fillInput,
    nudgeInputPaint,
  };
})();
