window.AppConfig=(function(){
  return {
    api:{
      mutationGuardHeader:'X-Azimuth-Request',
      mutationGuardValue:'1'
    },
    monitor:{
      /** While the tab is visible and there has been no pointer activity for `idleGraceMs`. */
      idlePollActiveMs:18000,
      /** @deprecated Kept for forks; visible idle uses `idlePollVisibleMs` instead. */
      idlePollHeartbeatMs:60000,
      /** Fresh polls while the tab is visible but input-idle (FUNC pause, banner text, etc.). */
      idlePollVisibleMs:12000,
      idleGraceMs:60000,
      bootstrapRetryMs:3000,
      activityRescheduleMinMs:1500,
      staleUiAfterFailures:2,
      /** Extra `/api/status` samples after first successful load (OTA / manifest / Wi‑Fi settle). */
      bootSyncBurstCount:6,
      bootSyncBurstGapMs:5000,
      /** Follow-up samples after the tab becomes visible again (iOS/Android task switch). */
      focusSyncBurstCount:4,
      focusSyncBurstGapMs:3500,
      /** While the device has not finished its one-shot release manifest fetch. */
      manifestPendingPollMs:5000
    }
  };
})();
