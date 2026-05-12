window.AppViews=(function(){
  const {$,setToggle,updateSoundLightCard}=window.AppUi;
  const uiTouched=window.AppState.uiTouched;

  function heroWifiTier(rssi){
    if(rssi==null)return 0;
    const n=Number(rssi);
    if(!Number.isFinite(n)||n===0)return 0;
    if(n>=-50)return 4;
    if(n>=-65)return 3;
    if(n>=-75)return 2;
    if(n>=-85)return 1;
    return 0;
  }

  function wifiTxWeight(profile){
    if(profile===0)return 0.74;
    if(profile===2)return 1;
    return 0.92;
  }

  function normalizedWifiTx(j){
    const x=j.wifi_tx;
    if(x===0||x===1||x===2)return x;
    return 1;
  }

  function heroWifiTierWeighted(rssi,wifiTx){
    const base=heroWifiTier(rssi);
    const w=wifiTxWeight(wifiTx);
    const t=Math.round(base*w);
    return Math.max(0,Math.min(4,t));
  }

  function heroWifiLabel(tier,rssi,wifiTx){
    const tail=(rssi!=null?' · '+rssi+' dBm':'');
    if(tier<=0)return(rssi!=null&&Number(rssi)!==0)?('Weak'+tail):'No link';
    const labels=['','Fair','Good','Very good','Excellent'];
    return labels[tier]+tail;
  }

  function setHeroWifiBars(barEl,tier){
    if(!barEl)return;
    barEl.querySelectorAll('span').forEach((sp,i)=>{sp.classList.toggle('on',i<tier);});
  }

  function applyHero(j){
    const ap=!!j.setup_ap;
    const hb=$('heroBatt'),hts=$('heroBattSub'),htt=$('heroTemp'),htemps=$('heroTempSub');
    const hwb=$('heroWifiBars'),hws=$('heroWifiSub'),htr=$('heroTrack'),htrk=$('heroTrackSub');
    const bp=(j.battery_percent!=null)?Number(j.battery_percent):null;
    const bm=(j.battery_mv!=null)?Number(j.battery_mv):null;
    const bstate=(j.battery_charge_state||j.battery_state||'').toLowerCase();
    if(hb){
      if(bstate==='absent'||(bm!=null&&bm<2000)){
        hb.textContent='—';
        if(hts)hts.textContent='No battery / switch off';
      }else if(bp!=null&&!Number.isNaN(bp)){
        const p=Math.round(bp);
        hb.textContent=(p>100)?'100+%':(p+'%');
        if(hts){
          hts.textContent=(bstate==='charging')?'Charging':'Battery';
        }
      }else if(bm!=null&&!Number.isNaN(bm)){
        hb.textContent=Math.round(bm)+' mV';
        if(hts)hts.textContent='Battery';
      }else{
        hb.textContent='—';
        if(hts)hts.textContent='Battery unavailable';
      }
    }
    const tc=j.chip_temp_c;
    if(htt){
      if(tc!=null&&!Number.isNaN(Number(tc))){htt.textContent=Number(tc).toFixed(1)+' °C'}
      else{htt.textContent='—'}
      const ts=j.thermal_state||'ok';
      const tile=$('heroTileTemp');
      if(tile){
        tile.classList.toggle('hero-tile-warn',ts==='warn');
        tile.classList.toggle('hero-tile-err',ts==='emergency'||ts==='thermal_hold');
      }
      if(htemps){
        if(ts==='warn')htemps.textContent='Warm';
        else if(ts==='emergency'||ts==='thermal_hold')htemps.textContent='Protection active';
        else htemps.textContent='Normal';
      }
    }
    const txProf=normalizedWifiTx(j);
    const tier=ap?0:heroWifiTierWeighted(j.rssi,txProf);
    setHeroWifiBars(hwb,tier);
    if(hws){
      if(ap)hws.textContent='Offline mode · direct AP';
      else hws.textContent=heroWifiLabel(tier,j.rssi,txProf);
    }
    if(htr){
      if(j.thermal_hold){
        htr.textContent='Wi‑Fi off';
        if(htrk)htrk.textContent='Cooling — USB tracking may still run';
      }else if(j.stasis){
        htr.textContent='Paused';
        if(htrk)htrk.textContent='Low power';
      }else{
        const im=j.imu_period_ms;
        const hz=im?Math.round(1000/im):'—';
        htr.textContent='~'+hz+' Hz';
        if(htrk)htrk.textContent=im?'Update rate':'';
      }
    }
  }

  function applyShell(j){
    const ap=!!j.setup_ap;
    window.AppState.lastStatus=j;
    applyHero(j);
    if(window.AppPoseMascot&&typeof window.AppPoseMascot.applyStatus==='function'){
      window.AppPoseMascot.applyStatus(j);
    }
    const ota=j.fw_ota||{};
    const otaBusy=!!(ota.active||ota.phase==='connecting'||ota.phase==='downloading'||ota.phase==='finalizing');
    const ub=$('updateBanner');
    if(ub){
      // While an OTA is running (or just finished) the dedicated progress card
      // is the source of truth — hide the "new firmware available" warning so
      // the user isn't tempted to click Install again mid-flash.
      if(!ap&&!otaBusy&&j.fw_update_available&&j.fw_latest_version){
        ub.style.display='block';
        $('updateBannerLatest').textContent=j.fw_latest_version;
        $('updateBannerCur').textContent=j.fw_version||'—';
        const lk=$('updateBannerLink');
        if(lk&&j.fw_flasher_url){lk.href=j.fw_flasher_url}
      }else{ub.style.display='none'}
    }
    if(j.fw_ota&&window.AppUpdateProgress){
      const o=j.fw_ota;
      if(o.active){
        if(typeof window.AppUpdateProgress.show==='function'){
          window.AppUpdateProgress.show(o);
        }
        if(typeof window.AppUpdateProgress.apply==='function'){
          window.AppUpdateProgress.apply(o);
        }
      }else if((o.phase==='success'||o.phase==='failed')&&typeof window.AppUpdateProgress.apply==='function'){
        window.AppUpdateProgress.apply(o);
      }else if(typeof window.AppUpdateProgress.hide==='function'){
        // OTA returned to idle (usually after reboot): clear any stale progress UI.
        window.AppUpdateProgress.hide();
      }
    }
    const manualBtn=$('btnUpdateManualWifi');
    const manualSubEl=$('updateManualSub');
    const hasNewer=!!(j.fw_update_available&&j.fw_latest_version);
    const blocked=ap||!j.wifi_connected||j.thermal_hold||otaBusy;
    const updateTitle=blocked
      ?(otaBusy?'Wireless update in progress…'
        :(ap?'Join your Wi‑Fi to enable wireless updates'
          :(j.thermal_hold?'Cooling — try again after a power cycle':'Wi‑Fi not connected')))
      :(hasNewer?('Install firmware '+j.fw_latest_version+' from the release server')
                :('Force re‑pull firmware '+(j.fw_version||'?')+' from the release server'));
    if(manualBtn){
      manualBtn.disabled=!!blocked;
      manualBtn.title=updateTitle;
    }
    if(manualSubEl){
      manualSubEl.textContent=otaBusy
        ?'Wireless update is running — see the progress card above.'
        :(hasNewer
          ?('Manual OTA will install '+j.fw_latest_version+' from the release server and reboot.')
          :('Force re‑pull '+(j.fw_version||'the current build')+' from the release server (no version check).'));
    }
    const bannerBtn=$('btnUpdateBannerWifi');
    if(bannerBtn){
      bannerBtn.disabled=!!blocked;
    }
    $('subLine').textContent=ap?'Offline mode · direct AP access':'On your network · idle until you use this page';
    const hz=j.imu_period_ms?Math.round(1000/j.imu_period_ms):'—';
    const cip=j.http_client_ip;
    const rssi=ap?'':(j.rssi!=null?j.rssi+' dBm':'—');
    const line1=ap?('AP · board '+ (j.ip||'—')):('LAN · board '+(j.ip||'—')+' · RSSI '+rssi);
    const line1b=' · up '+Math.round(j.uptime_ms/1000)+'s · heap '+j.heap_free;
    let udpSummary=j.udp_enabled===false?'UDP off':(j.ot_target_ok?'UDP ok':'UDP pending');
    if(j.stasis)udpSummary='Paused · '+udpSummary;
    if(j.ot_target_ok&&j.ot_resolved_ip)udpSummary+=' → '+j.ot_resolved_ip;
    const line2='FW '+(j.fw_version||'?')+' · ~'+hz+' Hz · '+ (j.hostname||'azimuth')+' · '+udpSummary+' · STA '+(j.wifi_connected?'on':'off');
    const bmv=(j.battery_mv!=null)?(j.battery_mv+' mV'):'—';
    const braw=(j.battery_raw_mv!=null)?(j.battery_raw_mv+' mV pin'):'—';
    const bpct=(j.battery_percent!=null)?(((Number(j.battery_percent)>100)?'100+%':(j.battery_percent+'%'))):'—';
    const bmah=(j.battery_remaining_mah!=null)?(j.battery_remaining_mah+' mAh'):'—';
    const bstate=j.battery_charge_state||j.battery_state||'—';
    const line3='Batt '+bpct+' ('+bmv+', '+bmah+') · raw '+braw+' · '+bstate;
    $('stats').textContent=line1+line1b+'\n'+line2+'\n'+line3;
    if($('homeStats'))$('homeStats').textContent=line1+line1b+'\n'+line2+'\n'+line3;
    if($('fwVer'))$('fwVer').textContent=j.fw_version||'—';
    if($('homeFwVer'))$('homeFwVer').textContent=j.fw_version||'—';
    if($('battState')){
      const st=(j.battery_charge_state||j.battery_state||'').toLowerCase();
      if(st==='absent'){$('battState').textContent='No battery / switch off'}
      else if(j.battery_percent!=null){
        const pct=(Number(j.battery_percent)>100)?'100+%':(j.battery_percent+'%');
        const s=(st==='charging')?' · charging':'';
        $('battState').textContent=(pct+s);
      }else if(j.battery_mv!=null){$('battState').textContent=(j.battery_mv+' mV')}
      else{$('battState').textContent=(j.battery_state||'unavailable')}
    }
    if($('homeBattState')){
      const st=(j.battery_charge_state||j.battery_state||'').toLowerCase();
      if(st==='absent'){$('homeBattState').textContent='No battery / switch off'}
      else if(j.battery_percent!=null){
        const pct=(Number(j.battery_percent)>100)?'100+%':(j.battery_percent+'%');
        const s=(st==='charging')?' · charging':'';
        $('homeBattState').textContent=(pct+s);
      }else if(j.battery_mv!=null){$('homeBattState').textContent=(j.battery_mv+' mV')}
      else{$('homeBattState').textContent=(j.battery_state||'unavailable')}
    }
    const bc=$('batteryCapacity');
    if(bc&&j.battery_capacity_mah!=null){bc.value=String(j.battery_capacity_mah)}
    const bcv=$('batteryCalOffsetVal');
    if(bcv){
      const off=(j.battery_cal_offset_mv!=null)?Number(j.battery_cal_offset_mv):0;
      const sign=off>0?'+':'';
      bcv.textContent=sign+off+' mV';
    }
    if(!uiTouched.udp)setToggle('udpToggle',!!j.udp_enabled);
    if(!uiTouched.mdns)setToggle('mdnsToggle',!!j.mdns_on);
    if(!uiTouched.imuDyn&&$('imuDynamicToggle'))setToggle('imuDynamicToggle',!!j.imu_dynamic);
    if(!uiTouched.hatire)setToggle('hatireToggle',j.hatire_usb!==false);
    updateSoundLightCard(j);
    const box=$('clientIpBox'),val=$('clientIpVal');
    if(box&&val){
      if(cip&&cip!=='0.0.0.0'){box.style.display='block';val.textContent=cip;}
      else{box.style.display='none'}
    }
    const dh=$('otDynHint');
    if(dh){
      const hostSet=!!(j.ot_host&&String(j.ot_host).trim().length);
      const hasRes=j.ot_resolved_ip&&String(j.ot_resolved_ip).length;
      dh.style.display='none';
      dh.textContent='';
      dh.classList.remove('warn');
      if(j.udp_enabled&&hostSet){
        if(hasRes&&j.ot_target_ok&&j.ot_using_dns){
          dh.style.display='block';
          dh.textContent='→ '+j.ot_resolved_ip+' (DNS)';
        }else if(!j.ot_target_ok||!hasRes){
          dh.style.display='block';
          dh.classList.add('warn');
          dh.textContent=j.wifi_connected?'Cannot resolve this hostname. Check spelling; avoid .local unless your router supports it.':'Join Wi‑Fi before hostnames can resolve.';
        }
      }
    }
    if(window.AppSections&&typeof window.AppSections.applyStatus==='function'){
      window.AppSections.applyStatus(j);
    }
  }

  return {applyShell};
})();
