window.AppConfig=(function(){
  return {
    api:{
      mutationGuardHeader:'X-Azimuth-Request',
      mutationGuardValue:'1'
    },
    monitor:{
      idlePollActiveMs:25000,
      idlePollHeartbeatMs:60000,
      idleGraceMs:60000,
      bootstrapRetryMs:3000,
      activityRescheduleMinMs:1500,
      staleUiAfterFailures:2
    }
  };
})();
