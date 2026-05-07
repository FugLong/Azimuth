window.AppStateFns=(function(){
  const {$,setToggle,setMsg}=window.AppUi;

  function otAxesDefault(){
    return[{src:0,inv:false},{src:2,inv:false},{src:1,inv:true}];
  }

  function applyOtAxesFromStatus(ax){
    const d=Array.isArray(ax)&&ax.length===3?ax:otAxesDefault();
    for(let i=0;i<3;i++){
      const o=d[i]||{src:0,inv:false};
      const s=([0,1,2].includes(o.src))?o.src:0;
      $('otSrc'+i).value=String(s);
      setToggle('otInv'+i,!!o.inv);
    }
  }

  function collectOtAxes(){
    const s=[0,1,2].map(i=>parseInt($('otSrc'+i).value,10));
    if(new Set(s).size!==3){
      setMsg('Use yaw, pitch, and roll exactly once across Rot 0–2.','err');
      return null;
    }
    return s.map((src,i)=>({src,inv:$('otInv'+i).classList.contains('on')}));
  }

  return {otAxesDefault,applyOtAxesFromStatus,collectOtAxes};
})();
