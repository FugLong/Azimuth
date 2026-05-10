window.AppUpdateProgress=(function(){
  const POLL_MS=900;
  let timer=0;
  let visible=false;

  function $(id){return document.getElementById(id)}

  function setBar(pct,indeterminate){
    const fill=$('updateProgressFill');
    if(!fill)return;
    if(indeterminate){
      fill.classList.add('indeterminate');
      fill.style.width='';
    }else{
      fill.classList.remove('indeterminate');
      const p=Math.max(0,Math.min(100,Math.round(Number(pct)||0)));
      fill.style.width=p+'%';
    }
  }

  function setMeta(text){
    const el=$('updateProgressMeta');
    if(el)el.textContent=text||'';
  }

  function setTitle(text){
    const el=$('updateProgressTitle');
    if(el)el.textContent=text||'Installing firmware…';
  }

  function show(initial){
    const banner=$('updateProgressBanner');
    if(!banner)return;
    visible=true;
    banner.style.display='block';
    setTitle('Installing firmware…');
    setMeta('Connecting…');
    setBar(0,true);
    if(initial&&initial.phase){
      apply({phase:initial.phase,progress_percent:0,written_bytes:0,total_bytes:0});
    }
    schedule();
  }

  function hide(){
    visible=false;
    const banner=$('updateProgressBanner');
    if(banner)banner.style.display='none';
    if(timer){clearTimeout(timer);timer=0}
  }

  function bytesText(written,total){
    if(!total||total<=0)return '';
    const kbW=Math.round(written/1024);
    const kbT=Math.round(total/1024);
    return kbW+' / '+kbT+' KB';
  }

  function apply(j){
    if(!j)return;
    const phase=j.phase||'idle';
    const pct=Number(j.progress_percent)||0;
    const w=Number(j.written_bytes)||0;
    const t=Number(j.total_bytes)||0;
    if(phase==='downloading'){
      setBar(pct,t<=0);
      setMeta((t>0?(pct+'%  '):'')+bytesText(w,t)+'  · streaming firmware');
      setTitle('Installing firmware…');
    }else if(phase==='connecting'){
      setBar(0,true);
      setMeta('Negotiating with the release server…');
    }else if(phase==='finalizing'){
      setBar(100,false);
      setMeta('Verifying & flipping the boot slot…');
    }else if(phase==='success'){
      setBar(100,false);
      setMeta('Done. Rebooting into the new build…');
      setTitle('Update complete');
      setTimeout(hide,4500);
    }else if(phase==='failed'){
      setBar(0,false);
      const err=(j.error&&String(j.error))||'unknown error';
      setMeta('Failed: '+err+' · device kept the old firmware.');
      setTitle('Update failed');
    }else{
      hide();
    }
  }

  async function poll(){
    timer=0;
    if(!visible)return;
    try{
      const j=await window.AppApi.getUpdateStatus();
      apply(j);
      if(j.phase==='downloading'||j.phase==='connecting'||j.phase==='finalizing'){
        schedule();
      }
    }catch(e){
      // Device often goes briefly unreachable around finalize/restart; keep
      // polling until we either get success or give up.
      schedule();
    }
  }

  function schedule(){
    if(timer)clearTimeout(timer);
    timer=setTimeout(poll,POLL_MS);
  }

  function init(){
    // Pick up an OTA already in progress if the user reloaded the page.
    if(window.AppApi&&typeof window.AppApi.getUpdateStatus==='function'){
      window.AppApi.getUpdateStatus().then(j=>{
        if(j&&(j.active||j.phase==='success'||j.phase==='failed')){
          if(j.phase==='success'||j.phase==='failed'){
            show(j);
            apply(j);
          }else{
            show(j);
          }
        }
      }).catch(()=>{});
    }
  }

  return{init,show,hide,apply};
})();
