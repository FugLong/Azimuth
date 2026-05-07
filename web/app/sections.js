window.AppSections=(function(){
  const {$}=window.AppUi;

  const titles={
    home:'Settings',
    wifi:'Wi‑Fi',
    tracking:'Tracking output',
    device:'Device & battery',
    sound:'Sound & light',
    advanced:'Advanced'
  };

  let current='home';
  let soundSupported=true;

  function sectionCards(){
    return Array.from(document.querySelectorAll('.section-card'));
  }

  function cardSection(card){
    return card.getAttribute('data-section')||'';
  }

  function sectionButtons(){
    return Array.from(document.querySelectorAll('[data-section-nav]'));
  }

  function currentSection(){
    return current;
  }

  function updateNavState(){
    sectionButtons().forEach(btn=>{
      const key=btn.getAttribute('data-section-nav')||'home';
      btn.classList.toggle('is-active',current===key);
      if(key==='sound'){
        btn.style.display=soundSupported?'block':'none';
        btn.classList.toggle('is-disabled',!soundSupported);
        btn.setAttribute('aria-disabled',(!soundSupported)?'true':'false');
      }
    });
  }

  function setSection(next){
    if(next==='sound'&&!soundSupported)next='home';
    current=titles[next]?next:'home';
    const sectionPage=$('sectionPage');
    const title=$('sectionTitle');
    const homeNav=$('cardHomeNav');
    const homeStatus=$('cardHomeStatus');
    if(title)title.textContent=titles[current]||titles.home;
    if(sectionPage)sectionPage.style.display=(current==='home')?'none':'block';
    if(homeNav)homeNav.style.display=(current==='home')?'block':'none';
    if(homeStatus)homeStatus.style.display=(current==='home')?'block':'none';

    sectionCards().forEach(card=>{
      let show=(current==='home')?false:(cardSection(card)===current);
      if(show&&cardSection(card)==='sound'&&!soundSupported){
        show=false;
      }
      card.style.display=show?'block':'none';
    });
    updateNavState();
  }

  function applyStatus(j){
    const sw=$('sumWifi'),st=$('sumTracking'),sd=$('sumDevice'),ss=$('sumSound'),sa=$('sumAdvanced');
    if(sw){
      if(j.setup_ap)sw.textContent='Offline mode (direct AP)';
      else sw.textContent=(j.wifi_connected?'Connected':'Not connected')+(j.ip?(' · '+j.ip):'');
    }
    if(st){
      const hz=j.imu_period_ms?Math.round(1000/j.imu_period_ms):'—';
      st.textContent=(j.stasis?'Paused':'~'+hz+' Hz')+' · '+((j.udp_enabled===false)?'UDP off':(j.ot_target_ok?'UDP ok':'UDP pending'));
    }
    if(sd){
      const pct=(j.battery_percent!=null)?(((Number(j.battery_percent)>100)?'100+%':(j.battery_percent+'%'))):'—';
      sd.textContent='FW '+(j.fw_version||'—')+' · Batt '+pct;
    }
    if(ss){
      soundSupported=!!(j.has_rgb||j.has_buzzer);
      ss.textContent=soundSupported?'Board supports configurable I/O':'No configurable sound/light on this board';
    }
    if(sa){
      sa.textContent='Factory reset and recovery options';
    }
    if(current==='sound'&&!soundSupported){
      setSection('home');
    }else{
      updateNavState();
    }
  }

  function init(){
    const back=$('btnSectionBack');
    if(back)back.onclick=()=>setSection('home');
    document.querySelectorAll('[data-section-nav]').forEach(btn=>{
      btn.addEventListener('click',()=>{
        const next=btn.getAttribute('data-section-nav')||'home';
        if(next==='sound'&&!soundSupported)return;
        setSection(next);
      });
    });
    setSection('home');
  }

  return {init,setSection,applyStatus,currentSection};
})();
