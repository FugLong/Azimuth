window.AppState=(function(){
  return {
    uiTouched:{udp:false,mdns:false,hatire:false},
    power:{
      lastUserActivityMs:Date.now(),
      lastStatusOkMs:0,
      pollTimer:0
    }
  };
})();
