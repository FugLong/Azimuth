/**
 * Attitude HUD: artificial horizon + numeric yaw/pitch/roll + tracking summary.
 * Same /api/status payload — no extra HTTP. Angles snap to each sample (honest for slow poll).
 */
window.AppPoseMascot=(function(){
  let horizon=null;
  let badge=null;
  let yawEl=null;
  let pitchEl=null;
  let rollEl=null;
  let metaEl=null;
  let healthEl=null;
  let healthStateEl=null;
  let previewCardEl=null;
  let toggleEl=null;
  let expanded=false;

  function wrap180(deg){
    let x=deg;
    while(x>180)x-=360;
    while(x<-180)x+=360;
    return x;
  }

  function fmtDeg(v){
    if(!Number.isFinite(v))return'—';
    const w=wrap180(v);
    const s=(w>=0?'+':'')+w.toFixed(1)+'°';
    return s;
  }

  function parseTri(j){
    const y=j.pose_yaw_deg;
    // UI convention expects pitch/roll opposite of transport naming.
    const p=j.pose_roll_deg;
    const r=j.pose_pitch_deg;
    if(y==null&&p==null&&r==null)return null;
    const ny=Number(y);
    const np=Number(p);
    const nr=Number(r);
    if(!Number.isFinite(ny)||!Number.isFinite(np)||!Number.isFinite(nr))return null;
    return{y:ny,p:np,r:nr};
  }

  function setBadge(text,cls){
    if(!badge)return;
    badge.textContent=text;
    badge.classList.remove('live','warn','err');
    if(cls)badge.classList.add(cls);
  }

  function setHealth(text,cls){
    if(!healthEl||!healthStateEl)return;
    healthStateEl.textContent=text;
    healthEl.classList.remove('live','warn','err');
    if(cls)healthEl.classList.add(cls);
  }

  function isPreviewActive(){ return expanded; }

  function setExpanded(on){
    expanded=!!on;
    if(previewCardEl)previewCardEl.style.display=expanded?'block':'none';
    if(toggleEl)toggleEl.setAttribute('aria-expanded',expanded?'true':'false');
    window.dispatchEvent(new CustomEvent('azimuth:pose-preview-active',{detail:{active:expanded}}));
  }

  function buildMeta(j){
    const ap=!!j.setup_ap;
    if(ap)return'Offline AP · pose N/A on this page';
    const hz=j.imu_period_ms?Math.round(1000/j.imu_period_ms):'—';
    const parts=['~'+hz+' Hz'];
    if(j.thermal_hold){
      parts.push('Wi‑Fi off (thermal)');
    }else if(j.stasis){
      parts.push('Pose paused (FUNC)');
    }else if(j.udp_enabled===false){
      parts.push('UDP off');
    }else if(!j.ot_target_ok){
      parts.push('UDP target pending');
    }else{
      parts.push('UDP path armed');
    }
    if(j.ot_resolved_ip&&String(j.ot_resolved_ip).length){
      parts.push(String(j.ot_resolved_ip));
    }
    return parts.join(' · ');
  }

  function applyStatus(j){
    horizon=horizon||document.getElementById('adiHorizon');
    badge=badge||document.getElementById('attHudBadge');
    yawEl=yawEl||document.getElementById('attHudYaw');
    pitchEl=pitchEl||document.getElementById('attHudPitch');
    rollEl=rollEl||document.getElementById('attHudRoll');
    metaEl=metaEl||document.getElementById('attHudMeta');
    healthEl=healthEl||document.getElementById('imuHealth');
    healthStateEl=healthStateEl||document.getElementById('imuState');
    previewCardEl=previewCardEl||document.getElementById('cardPosePreview');
    if(!horizon)return;

    const ap=!!j.setup_ap;
    const tri=parseTri(j);
    if(metaEl)metaEl.textContent=buildMeta(j);

    if(ap||!tri){
      if(horizon)horizon.setAttribute('transform','rotate(0) translate(0 0)');
      if(yawEl)yawEl.textContent='—';
      if(pitchEl)pitchEl.textContent='—';
      if(rollEl)rollEl.textContent='—';
      if(ap){
        setBadge('SETUP','warn');
        setHealth('Offline AP','warn');
        return;
      }
      if(j.thermal_hold){
        setBadge('COOLING','err');
        setHealth('Cooling hold','err');
        return;
      }
      if(j.stasis){
        setBadge('PAUSED','warn');
        setHealth('Paused','warn');
        return;
      }
      if(j.udp_enabled===false){
        setBadge('UDP OFF','warn');
        setHealth('IMU status only','warn');
        return;
      }
      if(j.ot_target_ok){
        setBadge('LIVE','live');
        setHealth('Live (UDP)','live');
      }else{
        setBadge('NO TARGET','warn');
        setHealth('Target pending','warn');
      }
      if(metaEl){
        metaEl.textContent='Waiting for fresh pose samples…';
      }
      return;
    }

    const pitchPx=Math.max(-40,Math.min(40,-(tri.p/55)*38));
    if(horizon)horizon.setAttribute('transform','rotate('+(-tri.r)+') translate(0 '+pitchPx+')');
    if(yawEl)yawEl.textContent=fmtDeg(tri.y);
    if(pitchEl)pitchEl.textContent=fmtDeg(tri.p);
    if(rollEl)rollEl.textContent=fmtDeg(tri.r);

    if(j.thermal_hold){
      setBadge('COOLING','err');
      setHealth('Cooling hold','err');
    }else if(j.stasis){
      setBadge('PAUSED','warn');
      setHealth('Paused','warn');
    }else if(j.udp_enabled===false){
      setBadge('UDP OFF','warn');
      setHealth('IMU live · UDP off','warn');
    }else if(!j.ot_target_ok){
      setBadge('NO TARGET','warn');
      setHealth('IMU live · target pending','warn');
    }else{
      setBadge('LIVE','live');
      setHealth('Live','live');
    }
  }

  function applyPose(j){
    applyStatus(j);
  }

  function init(){
    horizon=document.getElementById('adiHorizon');
    badge=document.getElementById('attHudBadge');
    yawEl=document.getElementById('attHudYaw');
    pitchEl=document.getElementById('attHudPitch');
    rollEl=document.getElementById('attHudRoll');
    metaEl=document.getElementById('attHudMeta');
    healthEl=document.getElementById('imuHealth');
    healthStateEl=document.getElementById('imuState');
    previewCardEl=document.getElementById('cardPosePreview');
    toggleEl=document.getElementById('imuPreviewToggle');
    if(toggleEl){
      toggleEl.addEventListener('click',()=>setExpanded(!expanded));
    }
    setExpanded(false);
  }

  return {init,applyStatus,applyPose,isPreviewActive};
})();
