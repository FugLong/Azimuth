window.AppControllers=(function(){
  const {$,setMsg,clamp255}=window.AppUi;
  const {collectOtAxes}=window.AppStateFns;

  function apiFailed(r,j,fallbackMsg){
    if(r.ok)return false;
    setMsg(j.error||fallbackMsg,'err');
    return true;
  }

  async function onScan(){
    const btn=$('btnScan');
    btn.classList.add('is-loading');
    btn.disabled=true;
    setMsg('Scanning… (tracking may hitch briefly)','');
    $('scanList').style.display='none';
    try{
      const j=await window.AppApi.scanNetworks();
      const box=$('scanList');while(box.firstChild)box.removeChild(box.firstChild);
      (j.networks||[]).forEach(n=>{
        const d=document.createElement('div');
        const t=document.createElement('span');t.textContent=n.ssid||'(hidden)';
        const rssi=document.createElement('span');rssi.className='rssi';rssi.textContent=(n.rssi!=null?n.rssi+' dBm':'');
        d.appendChild(t);d.appendChild(rssi);
        d.onclick=()=>{$('ssid').value=n.ssid||'';box.style.display='none'};
        box.appendChild(d);
      });
      box.style.display='block';
      setMsg('','');
    }catch(e){setMsg('Scan failed','err')}
    finally{
      btn.classList.remove('is-loading');
      btn.disabled=false;
    }
  }

  async function onSave(hydrateForm){
    setMsg('Saving…','');
    const otAxes=collectOtAxes();
    if(!otAxes)return;
    const body={
      ssid:$('ssid').value.trim(),
      ot_host:$('otHost').value.trim(),
      ot_port:parseInt($('otPort').value,10)||4242,
      ot_axes:otAxes,
      udp_enabled:$('udpToggle').classList.contains('on'),
      hatire_usb:$('hatireToggle').classList.contains('on'),
      mdns_on:$('mdnsToggle').classList.contains('on'),
      hostname:$('hostname').value.trim().toLowerCase(),
      imu_period_ms:parseInt($('imuPeriod').value,10)||10,
      wifi_tx:parseInt($('wifiTx').value,10),
      battery_capacity_mah:parseInt($('batteryCapacity').value,10),
      rgb_brightness:parseInt($('rgbBrightness').value,10),
      buzzer_volume:parseInt($('buzzerVolume').value,10)
    };
    if(body.rgb_brightness<0||body.rgb_brightness>100||Number.isNaN(body.rgb_brightness))body.rgb_brightness=25;
    if(body.buzzer_volume<0||body.buzzer_volume>100||Number.isNaN(body.buzzer_volume))body.buzzer_volume=25;
    if(body.battery_capacity_mah<100||body.battery_capacity_mah>5000||Number.isNaN(body.battery_capacity_mah))body.battery_capacity_mah=800;
    if(![0,1,2].includes(body.wifi_tx))body.wifi_tx=1;
    const lmEl=$('ledMode');
    if(lmEl){
      body.led_mode=parseInt(lmEl.value,10);
      if(![0,1,2,3].includes(body.led_mode))body.led_mode=0;
    }
    const lrEl=$('ledR'),lgEl=$('ledG'),lbEl=$('ledB');
    if(lrEl&&lgEl&&lbEl){
      body.led_r=clamp255(lrEl.value);
      body.led_g=clamp255(lgEl.value);
      body.led_b=clamp255(lbEl.value);
    }
    const pw=$('pass').value;
    if(pw.length)body.password=pw;
    try{
      const {response:r,json:j}=await window.AppApi.postConfig(body);
      if(apiFailed(r,j,'Save failed'))return;
      setMsg(j.restarting?'Saved — rebooting…':'Saved.','ok');
      if(!j.restarting)hydrateForm();
    }catch(e){setMsg('Network error','err')}
  }

  async function onReboot(){
    if(!confirm('Reboot device?'))return;
    try{
      const {response:r,json:j}=await window.AppApi.postReboot();
      if(apiFailed(r,j,'Reboot failed'))return;
      setMsg('Reboot sent…','ok');
    }catch(e){setMsg('Network error','err')}
  }

  async function onBatteryCal(hydrateForm){
    setMsg('Calibrating battery at 4.2V (about 3s)…','');
    try{
      const {response:r,json:j}=await window.AppApi.postConfig({battery_calibrate_4v2:true});
      if(apiFailed(r,j,'Calibration failed'))return;
      if(j.battery_calibrated){
        const off=(j.battery_cal_offset_mv!=null)?j.battery_cal_offset_mv:0;
        const sign=off>0?'+':'';
        setMsg('Battery calibrated ('+sign+off+' mV offset).','ok');
      }else{
        setMsg('Battery calibrated.','ok');
      }
      hydrateForm();
    }catch(e){setMsg('Network error','err')}
  }

  async function onFactoryReset(){
    if(!confirm('Reset all settings to factory defaults and reboot? You’ll need Azimuth‑Tracker (Offline Mode) or a USB flash to configure Wi‑Fi again.'))return;
    try{
      const {response:r,json:j}=await window.AppApi.postFactoryReset();
      if(apiFailed(r,j,'Factory reset failed'))return;
      setMsg('Resetting…','ok');
    }catch(e){setMsg('Request failed','err')}
  }

  return {onScan,onSave,onReboot,onBatteryCal,onFactoryReset};
})();
