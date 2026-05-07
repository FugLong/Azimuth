window.AppApi=(function(){
  const cfg=(window.AppConfig&&window.AppConfig.api)||{};
  const MUTATION_GUARD_HEADER=cfg.mutationGuardHeader||'X-Azimuth-Request';
  const MUTATION_GUARD_VALUE=cfg.mutationGuardValue||'1';
  const MUTATION_HEADERS={};
  MUTATION_HEADERS[MUTATION_GUARD_HEADER]=MUTATION_GUARD_VALUE;

  async function parseJsonResponse(r){
    let j={};
    try{
      j=await r.json();
    }catch(e){
      j={error:r.ok?'Invalid JSON response':'Request failed'};
    }
    return {response:r,json:j};
  }

  async function requestJson(url,opts){
    const r=await fetch(url,opts);
    return parseJsonResponse(r);
  }

  async function getStatus(){
    const {response:r,json:j}=await requestJson('/api/status');
    if(!r.ok)throw new Error(j.error||('Status failed ('+r.status+')'));
    return j;
  }
  async function scanNetworks(){
    const {response:r,json:j}=await requestJson('/api/scan');
    if(!r.ok)throw new Error(j.error||('Scan failed ('+r.status+')'));
    return j;
  }
  async function postConfig(body){
    return requestJson('/api/config',{
      method:'POST',
      headers:Object.assign({'Content-Type':'application/json'},MUTATION_HEADERS),
      body:JSON.stringify(body)
    });
  }
  async function postReboot(){
    return requestJson('/api/reboot',{method:'POST',headers:MUTATION_HEADERS});
  }
  async function postFactoryReset(){
    return requestJson('/api/factory_reset',{method:'POST',headers:MUTATION_HEADERS});
  }
  return {getStatus,scanNetworks,postConfig,postReboot,postFactoryReset};
})();
