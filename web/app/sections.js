window.AppSections=(function(){
  const {$}=window.AppUi;

  const titles={
    wifi:'Wi‑Fi',
    tracking:'Tracking output',
    device:'Device & battery',
    sound:'Sound & light',
    advanced:'Advanced'
  };

  let current='wifi';
  let soundSupported=true;
  let userNavigated=false;
  let startupSectionChosen=false;

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
      const key=btn.getAttribute('data-section-nav')||'wifi';
      btn.classList.toggle('is-active',current===key);
      btn.setAttribute('aria-selected',current===key?'true':'false');
      if(key==='sound'){
        btn.style.display=soundSupported?'block':'none';
        btn.classList.toggle('is-disabled',!soundSupported);
        btn.setAttribute('aria-disabled',(!soundSupported)?'true':'false');
      }
    });
  }

  function setSection(next){
    if(next==='sound'&&!soundSupported)next='wifi';
    current=titles[next]?next:'wifi';
    const title=$('sectionTitle');
    if(title)title.textContent=titles[current]||titles.wifi;

    sectionCards().forEach(card=>{
      let show=(cardSection(card)===current);
      if(show&&cardSection(card)==='sound'&&!soundSupported){
        show=false;
      }
      card.style.display=show?'block':'none';
    });
    updateNavState();
  }

  function applyStatus(j){
    if(!startupSectionChosen&&!userNavigated){
      setSection(j.setup_ap?'wifi':'tracking');
      startupSectionChosen=true;
    }
    const sw=$('sumWifi'),st=$('sumTracking'),sd=$('sumDevice'),ss=$('sumSound'),sa=$('sumAdvanced');
    if(sw){
      if(j.setup_ap)sw.textContent='Offline AP';
      else sw.textContent=j.wifi_connected?'Connected':'Not connected';
    }
    if(st){
      const hz=j.imu_period_ms?Math.round(1000/j.imu_period_ms):'—';
      st.textContent=(j.stasis?'Paused':'~'+hz+' Hz');
    }
    if(sd){
      const pct=(j.battery_percent!=null)?(((Number(j.battery_percent)>100)?'100+%':(j.battery_percent+'%'))):'—';
      sd.textContent='Batt '+pct;
    }
    if(ss){
      soundSupported=!!(j.has_rgb||j.has_buzzer);
      ss.textContent=soundSupported?'Available':'Not available';
    }
    if(sa){
      sa.textContent='Reset';
    }
    if(current==='sound'&&!soundSupported){
      setSection('wifi');
    }else{
      updateNavState();
    }
  }

  function init(){
    document.querySelectorAll('[data-section-nav]').forEach(btn=>{
      btn.addEventListener('click',()=>{
        const next=btn.getAttribute('data-section-nav')||'wifi';
        if(next==='sound'&&!soundSupported)return;
        userNavigated=true;
        setSection(next);
      });
    });
    setSection('wifi');
  }

  return {init,setSection,applyStatus,currentSection};
})();
