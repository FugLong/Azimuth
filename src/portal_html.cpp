#include <Arduino.h>
#include "portal_html.h"

const char kPortalIndexHtml[] PROGMEM = R"AZPORTAL(<!DOCTYPE html>
<html lang="en">
<head>
<meta charset="utf-8"/>
<meta name="viewport" content="width=device-width,initial-scale=1,maximum-scale=1"/>
<meta name="color-scheme" content="dark"/>
<title>Azimuth</title>
<style>
:root{
  --bg:#0a0e14;--bg2:#121a24;--card:rgba(26,35,50,.92);--bd:rgba(61,158,229,.22);
  --tx:#eef4fa;--tx-soft:#d8e2ee;--muted:#8b9cb3;--acc:#3d9ee5;--acc-dim:rgba(61,158,229,.15);
  --ok:#4ade80;--shadow:0 8px 32px rgba(0,0,0,.35);
}
*{box-sizing:border-box}
html{-webkit-text-size-adjust:100%}
body{
  margin:0;min-height:100dvh;
  font:16px/1.5 system-ui,-apple-system,BlinkMacSystemFont,"Segoe UI",sans-serif;
  color:var(--tx);
  background:var(--bg);
  background-image:radial-gradient(ellipse 100% 60% at 50% -15%,#1a4a6e 0%,transparent 55%),radial-gradient(ellipse 80% 50% at 100% 100%,rgba(61,158,229,.08),transparent);
}
.wrap{max-width:34rem;margin:0 auto;padding:clamp(1rem,4vw,2rem) clamp(1rem,3vw,1.25rem) 7.2rem}
.brand{display:flex;flex-direction:column;align-items:center;text-align:center;margin-bottom:1.1rem}
.logo-wrap{width:min(92vw,232px);max-width:100%;margin:0 auto;color:#e8eef5;filter:drop-shadow(0 3px 20px rgba(61,158,229,.2));flex-shrink:0}
.logo-wrap svg{display:block;width:100%;height:auto;vertical-align:top;shape-rendering:geometricPrecision}
.logo-wrap .portal-logo-img{display:block;width:100%;height:auto;vertical-align:top}
.sub{color:var(--muted);font-size:.8125rem;margin:.5rem 0 0;max-width:22rem;line-height:1.45}
.banner{
  display:none;background:var(--acc-dim);border:1px solid var(--bd);border-radius:12px;
  padding:.875rem 1rem;font-size:.8125rem;line-height:1.5;color:var(--tx);margin:0 0 1.25rem;text-align:left
}
.banner.banner-update{
  display:none;background:rgba(251,191,36,.1);border-color:rgba(251,191,36,.38)
}
.banner.banner-update a{color:var(--acc);font-weight:600;text-decoration:none}
.banner.banner-update a:hover{text-decoration:underline}
.banner.banner-update #btnUpdateBannerWifi{margin-right:.4rem}
.banner.banner-update-progress{
  display:none;background:rgba(61,158,229,.1);border-color:rgba(61,158,229,.45);
  padding:.95rem 1.05rem
}
.banner.banner-update-progress strong{font-weight:700;color:var(--tx)}
.update-progress-bar{
  height:.6rem;background:var(--bg2);border:1px solid var(--bd);border-radius:999px;
  overflow:hidden;margin:.5rem 0 .35rem
}
.update-progress-bar span{
  display:block;height:100%;width:0;
  background:linear-gradient(90deg,#4eb0f2,var(--acc));
  transition:width .3s ease
}
.update-progress-bar span.indeterminate{
  width:38%;background:linear-gradient(90deg,transparent,var(--acc),transparent);
  animation:updateProgressPulse 1.4s linear infinite
}
@keyframes updateProgressPulse{
  0%{transform:translateX(-100%)}100%{transform:translateX(280%)}
}
.update-progress-meta{
  font-size:.78rem;color:var(--tx-soft);font-variant-numeric:tabular-nums
}
.update-progress-hint{
  margin:.45rem 0 0;font-size:.72rem;color:var(--muted);line-height:1.5
}
.update-card-row{
  align-items:center;justify-content:space-between;gap:.65rem;
  margin:0 0 .85rem;padding:.65rem .75rem;
  border:1px solid var(--bd);border-radius:11px;background:var(--bg2)
}
.update-card-row .btn{flex-shrink:0}
.update-card-label{font-size:.85rem;font-weight:600;color:var(--tx)}
.update-card-sub{font-size:.72rem;color:var(--muted);margin-top:.2rem;line-height:1.45}
.card{
  background:var(--card);border:1px solid var(--bd);border-radius:14px;padding:1.1rem 1.15rem;margin-bottom:1rem;
  box-shadow:var(--shadow)
}
.card>h2,.card>.hd{font-size:.7rem;font-weight:700;text-transform:uppercase;letter-spacing:.08em;color:var(--acc);margin:0 0 .75rem}
label{display:block;font-size:.75rem;font-weight:500;color:var(--muted);margin:.85rem 0 .35rem}
label:first-of-type{margin-top:0}
input[type=text],input[type=password],input[type=number]{
  width:100%;padding:.65rem .75rem;border-radius:10px;border:1px solid var(--bd);
  background-color:#121a24;color:#eef4fa;font-size:16px;line-height:1.35;
  -webkit-appearance:none;appearance:none;
  -webkit-text-fill-color:#eef4fa;caret-color:#eef4fa;opacity:1;
  transform:translateZ(0);-webkit-transform:translateZ(0)
}
input:-webkit-autofill,input:-webkit-autofill:hover,input:-webkit-autofill:focus{
  -webkit-text-fill-color:#eef4fa;
  -webkit-box-shadow:0 0 0 1000px #121a24 inset;box-shadow:0 0 0 1000px #121a24 inset;
  transition:background-color 99999s ease-out
}
input:focus{outline:none;border-color:var(--acc);box-shadow:0 0 0 3px var(--acc-dim)}
select{
  width:100%;padding:.65rem .75rem;border-radius:10px;border:1px solid var(--bd);
  background-color:#121a24;color:#eef4fa;font-size:16px;line-height:1.35;
  -webkit-appearance:none;appearance:none
}
select:focus{outline:none;border-color:var(--acc);box-shadow:0 0 0 3px var(--acc-dim)}
input[type=range]{
  width:100%;max-width:100%;height:2rem;margin:.35rem 0 0;-webkit-appearance:none;appearance:none;
  background:transparent;accent-color:var(--acc)
}
input[type=range]::-webkit-slider-runnable-track{
  height:6px;border-radius:999px;background:var(--bg2);border:1px solid var(--bd)
}
input[type=range]::-webkit-slider-thumb{
  -webkit-appearance:none;appearance:none;width:1.15rem;height:1.15rem;margin-top:-.4rem;border-radius:50%;
  background:linear-gradient(165deg,#4eb0f2,var(--acc));border:none;box-shadow:0 1px 6px rgba(0,0,0,.35);cursor:pointer
}
.sound-ctl{margin:0;padding:0}
.sound-ctl-head{
  display:flex;align-items:baseline;flex-wrap:wrap;gap:.4rem .55rem;margin:0 0 .5rem;line-height:1.35
}
.sound-ctl-label{margin:0;font-size:.9375rem;font-weight:600;color:var(--tx)}
.sound-ctl-pct{
  margin:0;font-size:1rem;font-weight:700;color:var(--acc);font-variant-numeric:tabular-nums;white-space:nowrap
}
.sound-ctl input[type=range]{margin:.15rem 0 0;width:100%}
.sound-ctl-hint{margin:.55rem 0 0;font-size:.75rem;line-height:1.45;color:var(--muted)}
#ledManualRow{margin-top:.85rem}
.led-swatch{
  width:100%;height:2.75rem;border-radius:10px;border:1px solid var(--bd);
  background:#1e2a3a;box-shadow:inset 0 1px 0 rgba(255,255,255,.06)
}
.led-quick{display:flex;flex-wrap:wrap;align-items:center;gap:.45rem;margin:.65rem 0 0}
.led-quick span{font-size:.7rem;font-weight:600;color:var(--muted);text-transform:uppercase;letter-spacing:.06em;margin-right:.2rem}
.led-quick .btn{min-height:40px;padding:.45rem .7rem;font-size:.8125rem;font-weight:600}
.led-chan-val{font-variant-numeric:tabular-nums;color:var(--acc);font-weight:700}
#buzzerVolumeRow{margin-top:1.15rem}
.hint{font-size:.75rem;color:var(--muted);margin:-.25rem 0 .85rem;line-height:1.45}
.battery-cal-meta{
  margin:.6rem 0 .45rem;
  padding:.55rem .65rem;
  border:1px solid var(--bd);
  border-radius:10px;
  background:var(--bg2);
}
.battery-cal-actions{margin:.3rem 0 .6rem}
.battery-cal-actions .btn{width:100%}
.battery-cal-hint{margin:.15rem 0 .95rem}
code{font-family:ui-monospace,SFMono-Regular,monospace;font-size:.85em;color:var(--acc)}
.row{display:flex;gap:.6rem;align-items:center;flex-wrap:wrap;justify-content:space-between}
.row-tight{justify-content:flex-start}
.btn{
  cursor:pointer;border:none;border-radius:10px;padding:.7rem 1.1rem;font-weight:600;font-size:16px;
  min-height:44px;touch-action:manipulation;-webkit-tap-highlight-color:transparent
}
.btn-primary{background:linear-gradient(165deg,#4eb0f2,var(--acc));color:#051018}
.btn-sec{background:var(--bg2);color:var(--tx);border:1px solid var(--bd)}
.card-danger{
  border-color:rgba(220,90,90,.2);
  background:rgba(18,14,16,.75);
  box-shadow:none;
}
.danger-details{margin:0}
.danger-details + .danger-details{
  margin-top:.95rem;padding-top:.95rem;border-top:1px solid var(--bd)
}
.danger-details>summary{
  cursor:pointer;list-style:none;font-size:.8125rem;font-weight:500;color:var(--muted);
  padding:.15rem 0;line-height:1.45
}
.danger-details>summary::-webkit-details-marker{display:none}
.danger-details>summary:focus-visible{outline:2px solid var(--acc);outline-offset:2px;border-radius:6px}
.danger-details[open]>summary{color:var(--tx-soft);margin-bottom:.2rem}
.danger-panel{
  margin-top:.55rem;padding-top:.65rem;border-top:1px solid rgba(248,113,113,.14)
}
.danger-panel .hint{margin:0 0 .65rem}
.btn-danger-soft{
  background:var(--bg2);color:#e8a0a0;border:1px solid rgba(248,113,113,.35);
  font-size:.875rem;font-weight:600;padding:.55rem 1rem;min-height:42px;width:auto;align-self:flex-start
}
.btn-danger-soft:active{opacity:.92}
.toggle{width:3.25rem;height:1.75rem;border-radius:999px;background:var(--bg2);border:1px solid var(--bd);position:relative;cursor:pointer;padding:0;flex-shrink:0}
.toggle.on{background:linear-gradient(165deg,#2a6090,var(--acc));border-color:transparent}
.toggle::after{content:'';position:absolute;width:1.35rem;height:1.35rem;border-radius:50%;background:#fff;top:50%;left:.2rem;transform:translateY(-50%);transition:left .18s ease;box-shadow:0 1px 4px rgba(0,0,0,.3)}
.toggle.on::after{left:1.55rem}
#msg{margin-top:.75rem;font-size:.8125rem;min-height:1.25em}
#msg.err{color:#f87171}#msg.ok{color:var(--ok)}
.scan-list{max-height:11rem;overflow:auto;border:1px solid var(--bd);border-radius:10px;margin-top:.6rem;display:none;-webkit-overflow-scrolling:touch}
.scan-list div{padding:.65rem .75rem;border-bottom:1px solid var(--bd);cursor:pointer;display:flex;justify-content:space-between;align-items:center;gap:.5rem;min-height:44px}
.scan-list div:active{background:var(--acc-dim)}
.scan-list div:last-child{border-bottom:none}
.scan-list .rssi{color:var(--muted);font-size:.8125rem;white-space:nowrap}
.btn.is-loading{
  opacity:.85;
  pointer-events:none;
}
.btn.is-loading::after{
  content:'';
  display:inline-block;
  width:.9rem;
  height:.9rem;
  margin-left:.5rem;
  border:2px solid rgba(238,244,250,.35);
  border-top-color:var(--tx);
  border-radius:50%;
  vertical-align:-0.12rem;
  animation:spin .8s linear infinite;
}
@keyframes spin{to{transform:rotate(360deg)}}
pre.stats{font-size:.72rem;color:var(--muted);white-space:pre-wrap;margin:.75rem 0 0;line-height:1.55;font-family:ui-monospace,SFMono-Regular,monospace;word-break:break-word}
.muted-hint{font-weight:400;color:var(--muted)}
.callout{
  margin:.65rem 0 0;padding:.8rem .9rem;border-radius:11px;background:var(--bg2);
  border:1px solid var(--bd)
}
.callout-hd{
  font-size:.65rem;font-weight:700;text-transform:uppercase;letter-spacing:.07em;color:var(--acc);
  margin:0 0 .55rem
}
.callout-main{display:flex;flex-wrap:wrap;align-items:center;gap:.5rem .75rem}
.ip-readout{
  font-family:ui-monospace,SFMono-Regular,monospace;font-size:.875rem;font-weight:500;color:var(--tx);
  letter-spacing:0;line-height:1.35;flex:1;min-width:9ch;word-break:break-all
}
.btn-sm{
  padding:.45rem .9rem;min-height:40px;font-size:.8125rem;font-weight:600;border-radius:9px;
  flex-shrink:0
}
.callout-note{margin:.55rem 0 0;font-size:.72rem;color:var(--muted);line-height:1.5;max-width:24rem}
.hint.ot-dyn-hint{margin:.4rem 0 .65rem;font-family:ui-monospace,SFMono-Regular,monospace;font-size:.72rem}
.hint.ot-dyn-hint:not(.warn){color:var(--muted)}
.hint.ot-dyn-hint.warn{color:#f87171;font-family:inherit;font-size:.75rem}
.hero-status{margin:0 0 1.1rem}
.hero-grid{display:grid;grid-template-columns:repeat(2,1fr);gap:.65rem}
@media(min-width:400px){
  .hero-grid{grid-template-columns:repeat(4,1fr);gap:.7rem}
}
.hero-tile{
  background:var(--card);border:1px solid var(--bd);border-radius:14px;padding:.8rem .85rem;
  box-shadow:var(--shadow);min-height:5.25rem;display:flex;flex-direction:column;justify-content:flex-start
}
.hero-tile-label{
  font-size:.62rem;font-weight:700;text-transform:uppercase;letter-spacing:.07em;color:var(--acc);
  margin:0 0 .4rem
}
.hero-tile-value{font-size:1.02rem;font-weight:600;color:var(--tx);line-height:1.2;word-break:break-word}
.hero-tile-sub{font-size:.7rem;color:var(--muted);margin:.4rem 0 0;line-height:1.35}
.hero-tile-warn .hero-tile-value{color:#fbbf24}
.hero-tile-err .hero-tile-value{color:#f87171}
.hero-wifi-bars{
  display:flex;align-items:flex-end;gap:3px;min-height:1.5rem;margin:.1rem 0 0
}
.hero-wifi-bars span{
  display:block;width:5px;border-radius:2px;background:var(--bg2);border:1px solid var(--bd);
  align-self:flex-end;transition:background .15s,border-color .15s
}
.hero-wifi-bars span.on{background:linear-gradient(180deg,#6ec9ff,var(--acc));border-color:transparent}
.hero-wifi-bars .b1{height:6px}.hero-wifi-bars .b2{height:10px}.hero-wifi-bars .b3{height:14px}.hero-wifi-bars .b4{height:18px}
.imu-health{
  margin:0 0 1rem;
  display:flex;
  align-items:center;
  gap:.45rem;
  padding:.5rem .65rem;
  border-radius:10px;
  border:1px solid var(--bd);
  background:rgba(16,27,39,.76);
  font-size:.74rem;
}
.imu-dot{
  width:.55rem;height:.55rem;border-radius:999px;
  background:#7a8ca5;box-shadow:0 0 0 1px rgba(255,255,255,.12) inset;
}
.imu-health.live .imu-dot{background:#4ade80;box-shadow:0 0 8px rgba(74,222,128,.6)}
.imu-health.warn .imu-dot{background:#fbbf24;box-shadow:0 0 8px rgba(251,191,36,.55)}
.imu-health.err .imu-dot{background:#f87171;box-shadow:0 0 8px rgba(248,113,113,.55)}
.imu-label{color:var(--muted);text-transform:uppercase;letter-spacing:.05em;font-weight:700}
.imu-state{color:var(--tx-soft);font-weight:600}
.imu-toggle{
  margin-left:auto;
  border:1px solid var(--bd);
  background:rgba(15,26,38,.92);
  color:#cfe0f1;
  border-radius:999px;
  font-size:.68rem;
  font-weight:700;
  letter-spacing:.04em;
  padding:.28rem .62rem;
  line-height:1;
  cursor:pointer;
}
.imu-toggle[aria-expanded="true"]{
  border-color:rgba(61,158,229,.42);
  color:#9fd2f6;
}
.imu-hero{
  margin:0 0 1rem;
  background:var(--card);
  border:1px solid var(--bd);
  border-radius:14px;
  padding:.85rem .9rem;
  box-shadow:var(--shadow);
}
.pose-preview .pose-preview-head{margin:0 0 .45rem}
.att-hud-badge{
  font-size:.66rem;
  font-weight:700;
  letter-spacing:.06em;
  text-transform:uppercase;
  border:1px solid rgba(61,158,229,.28);
  padding:.18rem .5rem;
  border-radius:999px;
  color:#c8d7e7;
  background:rgba(12,22,34,.82);
}
.att-hud-badge.live{color:#79f2ad;border-color:rgba(74,222,128,.45)}
.att-hud-badge.warn{color:#ffd37a;border-color:rgba(251,191,36,.46)}
.att-hud-badge.err{color:#ff9a9a;border-color:rgba(248,113,113,.46)}
.att-hud-stage{
  display:flex;justify-content:center;align-items:center;
  margin:.1rem 0 .5rem;
}
.att-hud-svg{
  width:min(100%,220px);height:auto;display:block;
  filter:drop-shadow(0 8px 16px rgba(0,0,0,.35));
}
.att-hud-readout{
  display:grid;
  grid-template-columns:repeat(3,minmax(0,1fr));
  gap:.5rem;
}
.att-hud-readout div{
  border:1px solid var(--bd);
  border-radius:10px;
  background:var(--bg2);
  padding:.45rem .5rem;
}
.att-hud-readout span{
  display:block;
  font-size:.64rem;
  text-transform:uppercase;
  letter-spacing:.06em;
  color:var(--muted);
}
.att-hud-readout strong{
  display:block;
  margin-top:.12rem;
  font-size:.9rem;
  color:var(--tx);
}
.settings-workspace{
  margin-bottom:1.15rem;
  border:1px solid rgba(61,158,229,.28);
  border-radius:16px;
  background:linear-gradient(180deg,rgba(10,16,24,.98),rgba(12,18,26,.94));
  box-shadow:0 12px 28px rgba(0,0,0,.34);
  overflow:hidden;
}
.settings-head{
  padding:.72rem .8rem .75rem;
  border-bottom:1px solid rgba(61,158,229,.2);
  background:linear-gradient(180deg,rgba(25,48,72,.72),rgba(17,31,47,.82));
}
.settings-head-top{
  display:flex;
  align-items:flex-end;
  justify-content:space-between;
  gap:.8rem;
  margin:0 0 .68rem;
}
.page-shell-title{
  font-size:.86rem;font-weight:700;letter-spacing:.08em;text-transform:uppercase;color:#9fd2f6
}
.section-shell-title{color:#9fd2f6}
.settings-subline{
  margin:0;
  font-size:.73rem;
  color:#b8c9db;
  letter-spacing:.04em;
  text-transform:uppercase;
  font-weight:650;
}
.section-tabs{
  display:grid;
  grid-template-columns:repeat(2,minmax(0,1fr));
  gap:.5rem;
}
.section-tab{
  width:100%;
  text-align:left;
  background:rgba(16,28,41,.86);
  border:1px solid rgba(61,158,229,.2);
  border-radius:12px;
  color:var(--tx);
  padding:.6rem .66rem;
  cursor:pointer;
  transition:border-color .12s ease,background .12s ease,transform .12s ease;
}
.section-tab:hover{border-color:rgba(61,158,229,.35);background:rgba(21,33,47,.96)}
.section-tab:active{transform:translateY(1px)}
.section-tab span{display:block;font-weight:650;line-height:1.25}
.section-tab small{display:block;margin-top:.17rem;color:var(--muted);font-size:.68rem;line-height:1.3}
.section-tab.is-active{
  border-color:var(--acc);
  box-shadow:0 0 0 1px rgba(61,158,229,.32) inset;
  background:rgba(23,39,56,.95);
}
.section-tab[aria-selected="true"] small{color:#b9cde1}
.section-page-body{
  padding:.85rem .8rem .5rem;
}
.section-page-body .section-card{
  margin:0 0 .75rem;
  background:rgba(18,27,38,.92);
}
.section-page-body .section-card:last-child{margin-bottom:0}
.section-tab.is-disabled{opacity:.55;cursor:not-allowed}
@media(min-width:540px){
  .section-tabs{grid-template-columns:repeat(5,minmax(0,1fr))}
}
@media(min-width:480px){
  .row-actions{flex-wrap:nowrap}
  .row-actions .btn{flex:1}
}
.action-bar-wrap{
  position:fixed;left:0;right:0;bottom:0;z-index:40;
  padding:.72rem .75rem calc(.28rem + env(safe-area-inset-bottom));
  background:linear-gradient(180deg,rgba(10,14,20,0),rgba(10,14,20,.94) 35%);
  backdrop-filter:saturate(120%) blur(2px);
}
.action-bar{
  max-width:34rem;margin:0 auto;padding:.44rem .62rem .14rem;
  border:1px solid rgba(61,158,229,.28);border-radius:14px;
  background:rgba(12,18,26,.96);
  box-shadow:0 10px 20px rgba(0,0,0,.35);
}
.action-bar #msg{
  margin-top:.2rem;
  min-height:.2rem;
}
</style>
</head>
<body>
<div class="wrap">
<header class="brand">
<div class="logo-wrap"><img class="portal-logo-img" src="data:image/webp;base64,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" alt="Azimuth" decoding="async" fetchpriority="high"/></div>
<p class="sub" id="subLine">Tracker settings</p>
</header>
<section class="hero-status" id="heroStatus" aria-label="Device status">
<div class="hero-grid">
<div class="hero-tile" id="heroTileBatt">
<div class="hero-tile-label">Battery</div>
<div class="hero-tile-value" id="heroBatt">—</div>
<div class="hero-tile-sub" id="heroBattSub"></div>
</div>
<div class="hero-tile" id="heroTileTemp">
<div class="hero-tile-label">Temperature</div>
<div class="hero-tile-value" id="heroTemp">—</div>
<div class="hero-tile-sub" id="heroTempSub">Normal</div>
</div>
<div class="hero-tile" id="heroTileWifi">
<div class="hero-tile-label">Connection</div>
<div class="hero-wifi-bars" id="heroWifiBars" aria-hidden="true"><span class="b1"></span><span class="b2"></span><span class="b3"></span><span class="b4"></span></div>
<div class="hero-tile-sub" id="heroWifiSub">—</div>
</div>
<div class="hero-tile" id="heroTileTrack">
<div class="hero-tile-label">Tracking rate</div>
<div class="hero-tile-value" id="heroTrack">—</div>
<div class="hero-tile-sub" id="heroTrackSub"></div>
</div>
</div>
</section>
<div class="imu-health" id="imuHealth" role="status" aria-live="polite">
<span class="imu-dot" id="imuDot" aria-hidden="true"></span>
<span class="imu-label">IMU</span>
<span class="imu-state" id="imuState">Checking…</span>
<button type="button" class="imu-toggle" id="imuPreviewToggle" aria-expanded="false" aria-controls="cardPosePreview">Preview</button>
</div>
<section class="imu-hero pose-preview" id="cardPosePreview" aria-live="polite" style="display:none">
<div class="row pose-preview-head">
<div class="hd" style="margin:0">Motion preview</div>
<span class="att-hud-badge" id="attHudBadge">—</span>
</div>
<div class="att-hud-stage">
<svg viewBox="-64 -64 128 128" class="att-hud-svg" aria-label="IMU attitude preview">
<defs>
<clipPath id="attHudClip"><circle cx="0" cy="0" r="48"/></clipPath>
</defs>
<g clip-path="url(#attHudClip)">
<g id="adiHorizon">
<rect x="-80" y="-80" width="160" height="80" fill="#2b5f93"></rect>
<rect x="-80" y="0" width="160" height="80" fill="#473a26"></rect>
<line x1="-80" y1="0" x2="80" y2="0" stroke="#ffffff" stroke-width="2" stroke-opacity=".9"></line>
</g>
</g>
<circle cx="0" cy="0" r="48" fill="none" stroke="rgba(238,244,250,.28)" stroke-width="2"></circle>
<line x1="-14" y1="0" x2="14" y2="0" stroke="#9fd2f6" stroke-width="2"></line>
<line x1="0" y1="-14" x2="0" y2="14" stroke="#9fd2f6" stroke-width="2" stroke-opacity=".65"></line>
</svg>
</div>
<div class="att-hud-readout">
<div><span>Yaw</span><strong id="attHudYaw">—</strong></div>
<div><span>Pitch</span><strong id="attHudPitch">—</strong></div>
<div><span>Roll</span><strong id="attHudRoll">—</strong></div>
</div>
<p class="hint" id="attHudMeta" style="margin:.5rem 0 0">Waiting for pose…</p>
</section>
<p class="banner banner-update" id="updateBanner" role="status">New firmware <strong id="updateBannerLatest">—</strong> is available (this device: <strong id="updateBannerCur">—</strong>). <button type="button" class="btn btn-sec btn-sm" id="btnUpdateBannerWifi">Install over Wi‑Fi</button> <a href="#" id="updateBannerLink" target="_blank" rel="noopener">Open USB installer</a></p>
<div class="banner banner-update-progress" id="updateProgressBanner" role="status" aria-live="polite" style="display:none">
<div><strong id="updateProgressTitle">Installing firmware…</strong></div>
<div class="update-progress-bar" aria-hidden="true"><span id="updateProgressFill"></span></div>
<div class="update-progress-meta" id="updateProgressMeta">Connecting…</div>
<p class="update-progress-hint">Don’t close this page or unplug the device. The board reboots into the new firmware automatically when the download completes.</p>
</div>
<section class="settings-workspace" id="settingsWorkspace" aria-live="polite">
<div class="settings-head">
<div class="settings-head-top">
<div class="page-shell-title section-shell-title">Settings</div>
<p class="settings-subline" id="sectionTitle">Wi‑Fi</p>
</div>
<div class="section-tabs" role="tablist" aria-label="Settings sections">
<button type="button" class="section-tab" role="tab" aria-selected="true" data-section-nav="wifi"><span>Wi‑Fi</span><small id="sumWifi">Network</small></button>
<button type="button" class="section-tab" role="tab" aria-selected="false" data-section-nav="tracking"><span>Tracking</span><small id="sumTracking">Output</small></button>
<button type="button" class="section-tab" role="tab" aria-selected="false" data-section-nav="device"><span>Device</span><small id="sumDevice">Power</small></button>
<button type="button" class="section-tab" role="tab" aria-selected="false" data-section-nav="sound"><span>Sound & light</span><small id="sumSound">Buzzer/RGB</small></button>
<button type="button" class="section-tab" role="tab" aria-selected="false" data-section-nav="advanced"><span>Advanced</span><small id="sumAdvanced">Reset</small></button>
</div>
</div>
<div class="section-page-body" id="sectionPageBody">

<div class="card section-card" id="cardWifi" data-section="wifi">
<div class="hd">Wi‑Fi</div>
<p class="hint">Home network credentials. Saving a <strong>new</strong> SSID or password reboots the device.</p>
<label for="ssid">Network name (SSID)</label>
<input type="text" id="ssid" name="ssid" autocomplete="username" autocapitalize="none" spellcheck="false"/>
<label for="pass">Password <span class="muted-hint">· leave blank to keep saved</span></label>
<input type="password" id="pass" name="password" autocomplete="current-password"/>
<div class="row row-tight" style="margin-top:.85rem">
<button type="button" class="btn btn-sec" id="btnScan">Scan networks</button>
</div>
<div class="scan-list" id="scanList"></div>
</div>

<div class="card section-card" id="cardLan" data-section="wifi">
<div class="hd">LAN & discovery</div>
<p class="hint">mDNS advertises <code>http://&lt;hostname&gt;.local:8080</code>. Changing hostname or mDNS requires a reboot.</p>
<div class="row" style="margin-bottom:.75rem">
<span>mDNS (Bonjour)</span>
<button type="button" class="toggle" id="mdnsToggle" aria-label="Toggle mDNS"></button>
</div>
<label for="hostname">Device hostname</label>
<input type="text" id="hostname" autocomplete="off" autocapitalize="none" spellcheck="false" maxlength="24" placeholder="azimuth"/>
</div>

<div class="card section-card" id="cardTrackRadio" data-section="tracking">
<div class="hd">Tracking & radio</div>
<p class="hint">Faster IMU reports reduce latency and increase USB/Wi‑Fi load. The portal is tuned for low steady load; saves and reboots still go through immediately.</p>
<div class="row" style="margin-bottom:.75rem">
<span>Variable IMU rate</span>
<button type="button" class="toggle" id="imuDynamicToggle" aria-label="Toggle variable IMU rate"></button>
</div>
<p class="hint" style="margin-top:0;margin-bottom:.65rem">When on, the device lowers IMU and UDP rate while your head is calm (saves battery). The slow end is usually <strong>~20–35 ms</strong> depending on your peak setting — not a hard jump to 25 Hz. The interval below is the <strong>peak</strong> rate when variable is on, or a fixed rate when off.</p>
<label for="imuPeriod">IMU report interval</label>
<select id="imuPeriod" aria-label="IMU period">
<option value="5">200 Hz (5 ms) — lowest latency</option>
<option value="10" selected>100 Hz (10 ms) — default</option>
<option value="20">50 Hz (20 ms)</option>
<option value="40">25 Hz (40 ms) — cooler / lower power</option>
</select>
<label for="wifiTx">Wi‑Fi TX power</label>
<select id="wifiTx" aria-label="WiFi TX power">
<option value="0">Low (~2 dBm) — coolest</option>
<option value="1" selected>Balanced (~8.5 dBm) — default</option>
<option value="2">High (~19.5 dBm) — weak AP / long range</option>
</select>
<p class="hint" style="margin-top:.75rem;margin-bottom:0">Changing IMU interval reboots the device when variable rate is <strong>off</strong>. With variable rate on, interval changes apply without a reboot.</p>
</div>

<div class="card section-card" id="cardTrackingOutput" data-section="tracking">
<div class="hd">OpenTrack (PC)</div>
<p class="hint">Your PC’s <strong>IPv4</strong> or a hostname your <strong>router’s DNS</strong> knows. <code>.local</code> / mDNS usually <strong>fail</strong> from the board—prefer an address or a DHCP hostname.</p>
<div class="row" style="margin-bottom:.6rem">
<span>USB Hatire output</span>
<button type="button" class="toggle" id="hatireToggle" aria-label="Toggle Hatire USB"></button>
</div>
<div class="row" style="margin-bottom:.75rem">
<span>UDP to PC</span>
<button type="button" class="toggle" id="udpToggle" aria-label="Toggle UDP"></button>
</div>
<label for="otHost">UDP address</label>
<input type="text" id="otHost" autocomplete="off" autocapitalize="none" spellcheck="false" placeholder="e.g. 192.168.1.42"/>
<p id="otDynHint" class="hint ot-dyn-hint" style="display:none" role="status"></p>
<div class="callout" id="clientIpBox" style="display:none">
<div class="callout-hd">This browser</div>
<div class="callout-main">
<span class="ip-readout" id="clientIpVal" aria-live="polite">—</span>
<button type="button" class="btn btn-sec btn-sm" id="btnUseClientIp">Fill address</button>
</div>
<p class="callout-note">Same machine as OpenTrack: your PC’s IPv4 address as the board sees it on the LAN.</p>
</div>
<label for="otPort">UDP port</label>
<input type="number" id="otPort" min="1" max="65535" inputmode="numeric"/>
<p class="hint" style="margin-top:.85rem">Axis mapping applies to <strong>USB Hatire</strong> and <strong>UDP</strong>. Assign yaw, pitch, and roll <strong>once each</strong> to Rot 0–2. With the <strong>default</strong> map here, OpenTrack Hatire uses <strong>Yaw→0, Roll→1, Pitch→2</strong> (see README).</p>
<div class="axis-slot" style="margin-top:.5rem">
<label for="otSrc0">Rot 0</label>
<select id="otSrc0" aria-label="Rot 0 source axis">
<option value="0">Yaw</option>
<option value="1">Pitch</option>
<option value="2">Roll</option>
</select>
<div class="row" style="margin:.45rem 0 .65rem"><span>Invert</span><button type="button" class="toggle" id="otInv0" aria-label="Invert Rot 0"></button></div>
</div>
<div class="axis-slot">
<label for="otSrc1">Rot 1</label>
<select id="otSrc1" aria-label="Rot 1 source axis">
<option value="0">Yaw</option>
<option value="1">Pitch</option>
<option value="2">Roll</option>
</select>
<div class="row" style="margin:.45rem 0 .65rem"><span>Invert</span><button type="button" class="toggle" id="otInv1" aria-label="Invert Rot 1"></button></div>
</div>
<div class="axis-slot">
<label for="otSrc2">Rot 2</label>
<select id="otSrc2" aria-label="Rot 2 source axis">
<option value="0">Yaw</option>
<option value="1">Pitch</option>
<option value="2">Roll</option>
</select>
<div class="row" style="margin:.45rem 0 .65rem"><span>Invert</span><button type="button" class="toggle" id="otInv2" aria-label="Invert Rot 2"></button></div>
</div>
</div>

<div class="card section-card" id="cardSoundLight" style="display:none" data-section="sound">
<div class="hd">Sound & light</div>
<p class="hint" id="soundLightHint" style="display:none">Stored in flash — use <strong>Save</strong> with the rest of the page.</p>
<div id="rgbBrightnessRow" style="display:none">
<div class="sound-ctl">
<div class="sound-ctl-head">
<label class="sound-ctl-label" for="rgbBrightness" id="rgbBrightnessLabel">RGB LED brightness</label>
<span class="sound-ctl-pct" id="rgbBrightnessVal" aria-live="polite">25%</span>
</div>
<input type="range" id="rgbBrightness" name="rgb_brightness" min="0" max="100" step="1" value="25" aria-valuemin="0" aria-valuemax="100" aria-labelledby="rgbBrightnessLabel"/>
</div>
</div>
<div id="ledModeRow" style="display:none">
<label for="ledMode" class="sound-ctl-label">LED mode</label>
<select id="ledMode" name="led_mode" aria-label="LED ambient mode">
<option value="0">Rainbow</option>
<option value="1">Rainbow (slow)</option>
<option value="2">Status (tracks IMU — green when OK)</option>
<option value="3">Manual RGB</option>
</select>
<p class="hint" id="ledModeHint" style="margin-top:.45rem;margin-bottom:0">Overrides (thermal, low battery, setup Wi‑Fi, pause) still take priority on the device.</p>
<div id="ledManualRow" style="display:none">
<p class="hint" style="margin:.55rem 0 .45rem">Preview matches raw R/G/B before device brightness scaling. Save to apply on the board.</p>
<div class="led-swatch" id="ledSwatch" title="RGB preview"></div>
<div class="sound-ctl" style="margin-top:.75rem">
<div class="sound-ctl-head">
<label class="sound-ctl-label" for="ledR">Red</label>
<span class="sound-ctl-pct led-chan-val" id="ledRVal" aria-live="polite">128</span>
</div>
<input type="range" id="ledR" min="0" max="255" step="1" value="80" aria-valuemin="0" aria-valuemax="255" aria-labelledby="ledR"/>
</div>
<div class="sound-ctl" style="margin-top:.65rem">
<div class="sound-ctl-head">
<label class="sound-ctl-label" for="ledG">Green</label>
<span class="sound-ctl-pct led-chan-val" id="ledGVal" aria-live="polite">128</span>
</div>
<input type="range" id="ledG" min="0" max="255" step="1" value="140" aria-valuemin="0" aria-valuemax="255" aria-labelledby="ledG"/>
</div>
<div class="sound-ctl" style="margin-top:.65rem">
<div class="sound-ctl-head">
<label class="sound-ctl-label" for="ledB">Blue</label>
<span class="sound-ctl-pct led-chan-val" id="ledBVal" aria-live="polite">128</span>
</div>
<input type="range" id="ledB" min="0" max="255" step="1" value="255" aria-valuemin="0" aria-valuemax="255" aria-labelledby="ledB"/>
</div>
<div class="led-quick">
<span>Quick</span>
<button type="button" class="btn btn-sec" data-led-rgb="0,0,0" aria-label="Preset LED off">Off</button>
<button type="button" class="btn btn-sec" data-led-rgb="255,255,255" aria-label="Preset white">White</button>
<button type="button" class="btn btn-sec" data-led-rgb="255,80,64" aria-label="Preset warm">Warm</button>
<button type="button" class="btn btn-sec" data-led-rgb="64,180,255" aria-label="Preset cool">Cool</button>
<button type="button" class="btn btn-sec" data-led-rgb="80,255,120" aria-label="Preset green">Green</button>
<button type="button" class="btn btn-sec" data-led-rgb="180,64,255" aria-label="Preset purple">Purple</button>
</div>
</div>
</div>
<div id="buzzerVolumeRow" style="display:none">
<div class="sound-ctl">
<div class="sound-ctl-head">
<label class="sound-ctl-label" for="buzzerVolume" id="buzzerVolumeLabel">Buzzer volume</label>
<span class="sound-ctl-pct" id="buzzerVolumeVal" aria-live="polite">25%</span>
</div>
<input type="range" id="buzzerVolume" name="buzzer_volume" min="0" max="100" step="1" value="25" aria-valuemin="0" aria-valuemax="100" aria-labelledby="buzzerVolumeLabel"/>
<p class="hint sound-ctl-hint">0% is mute. The scale is curved so you get useful steps in the middle, not only near 0. Save to persist.</p>
</div>
</div>
</div>

<div class="card section-card" id="cardDevice" data-section="device">
<div class="hd">Device</div>
<p class="sub" style="margin:0 0 .75rem">Firmware <strong id="fwVer">—</strong> · Battery: <strong id="battState">stub</strong>.</p>
<label for="batteryCapacity">Battery capacity (mAh)</label>
<input id="batteryCapacity" type="number" min="100" max="5000" step="50" value="800" aria-label="Battery capacity mAh">
<div class="row battery-cal-meta"><span>Battery cal offset</span><strong id="batteryCalOffsetVal">0 mV</strong></div>
<div class="row battery-cal-actions"><button type="button" class="btn btn-sec" id="btnBatteryCal">Calibrate at 4.2V (USB charging)</button></div>
<p class="hint battery-cal-hint">Runs a ~3 second averaging pass using raw ADC, then stores a single absolute offset.</p>
<pre class="stats" id="stats"></pre>
</div>

<div class="card card-danger section-card" id="cardAdvanced" data-section="advanced">
<div class="hd">Advanced</div>
<details class="danger-details">
<summary>Manual firmware reinstall…</summary>
<div class="danger-panel">
<p class="hint" id="updateManualSub">Force re‑pulls the release firmware over Wi‑Fi and reboots. Use this for testing OTA or recovering a suspect local flash.</p>
<button type="button" class="btn btn-sec" id="btnUpdateManualWifi">Reinstall firmware over Wi‑Fi</button>
</div>
</details>
<details class="danger-details">
<summary>Reset to factory defaults…</summary>
<div class="danger-panel">
<p class="hint">Clears Wi‑Fi, OpenTrack, hostname, and other saved options on this board. You’ll set up again through <strong>Azimuth‑Setup</strong> or by reflashing.</p>
<button type="button" class="btn btn-danger-soft" id="btnFactory">Reset all settings</button>
</div>
</details>
</div>
</div>
</section>
</div>
<div class="action-bar-wrap" role="region" aria-label="Global actions">
<div class="action-bar">
<div class="row row-actions">
<button type="button" class="btn btn-primary" id="btnSave">Save</button>
<button type="button" class="btn btn-sec" id="btnReboot">Reboot</button>
</div>
<div id="msg"></div>
</div>
</div>
<script>
window.AppState=(function(){
  return {
    uiTouched:{udp:false,mdns:false,hatire:false,imuDyn:false},
    power:{
      lastUserActivityMs:Date.now(),
      lastStatusOkMs:0,
      pollTimer:0
    },
    lastStatus:null
  };
})();
</script>
<script>
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
</script>
<script>
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
  async function getPose(){
    const {response:r,json:j}=await requestJson('/api/pose');
    if(!r.ok)throw new Error(j.error||('Pose failed ('+r.status+')'));
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
  async function postUpdate(){
    return requestJson('/api/update',{method:'POST',headers:MUTATION_HEADERS});
  }
  async function getUpdateStatus(){
    const {response:r,json:j}=await requestJson('/api/update_status');
    if(!r.ok)throw new Error(j.error||('Update status failed ('+r.status+')'));
    return j;
  }
  return {getStatus,getPose,scanNetworks,postConfig,postReboot,postFactoryReset,postUpdate,getUpdateStatus};
})();
</script>
<script>
window.AppUi=(function(){
  const $=id=>document.getElementById(id);

  function setMsg(t,cls){
    const m=$('msg');
    m.textContent=t||'';
    m.className=cls||'';
  }

  function setToggle(id,on){
    $(id).classList.toggle('on',on);
  }

  function updateSoundLightCard(j){
    const card=$('cardSoundLight'),hint=$('soundLightHint');
    const rr=$('rgbBrightnessRow'),br=$('buzzerVolumeRow'),lm=$('ledModeRow');
    if(!card)return;
    const hasRgb=!!j.has_rgb,hasBz=!!j.has_buzzer;
    const show=hasRgb||hasBz;
    const inSoundSection=window.AppSections&&typeof window.AppSections.currentSection==='function'
      ?window.AppSections.currentSection()==='sound'
      :true;
    card.style.display=(show&&inSoundSection)?'block':'none';
    if(hint)hint.style.display=show?'block':'none';
    if(rr)rr.style.display=hasRgb?'block':'none';
    if(lm)lm.style.display=hasRgb?'block':'none';
    if(br)br.style.display=hasBz?'block':'none';
  }

  function syncRangeLabels(){
    const rb=$('rgbBrightness'),rv=$('rgbBrightnessVal');
    const bb=$('buzzerVolume'),bv=$('buzzerVolumeVal');
    if(rb&&rv)rv.textContent=rb.value+'%';
    if(bb&&bv)bv.textContent=bb.value+'%';
  }

  function clamp255(x){
    const n=Math.round(Number(x));
    if(!Number.isFinite(n))return 0;
    return Math.max(0,Math.min(255,n));
  }

  function setLedRgb(r,g,b){
    const er=$('ledR'),eg=$('ledG'),eb=$('ledB');
    if(er)er.value=String(clamp255(r));
    if(eg)eg.value=String(clamp255(g));
    if(eb)eb.value=String(clamp255(b));
    syncLedManualUi();
  }

  function syncLedManualUi(){
    const row=$('ledManualRow'),lm=$('ledMode'),hint=$('ledModeHint');
    const sw=$('ledSwatch'),rv=$('ledRVal'),gv=$('ledGVal'),bv=$('ledBVal');
    const er=$('ledR'),eg=$('ledG'),eb=$('ledB');
    if(!lm)return;
    const mode=parseInt(lm.value,10);
    const manual=mode===3;
    if(row)row.style.display=manual?'block':'none';
    const r=er?clamp255(er.value):0,g=eg?clamp255(eg.value):0,b=eb?clamp255(eb.value):0;
    if(rv)rv.textContent=String(r);
    if(gv)gv.textContent=String(g);
    if(bv)bv.textContent=String(b);
    if(sw)sw.style.background='rgb('+r+','+g+','+b+')';
    if(hint){
      if(mode===0)hint.textContent='Smooth rainbow. Overrides (thermal, low battery, setup Wi‑Fi, pause) still win on the device.';
      else if(mode===1)hint.textContent='Slower rainbow. Same override rules as other modes.';
      else if(mode===2)hint.textContent='Green accent while the IMU reports OK; dim when waiting for data.';
      else hint.textContent='Solid color from the sliders below (after Save). RGB brightness above still scales output. System warnings override this LED.';
    }
  }

  function fillInput(el,v){
    if(!el)return;
    const s=v==null?'':String(v);
    el.value=s;
    el.setAttribute('value',s);
    el.defaultValue=s;
  }

  function nudgeInputPaint(){
    requestAnimationFrame(()=>{
      ['ssid','hostname','otHost','otPort','otSrc0','otSrc1','otSrc2','imuPeriod','wifiTx','rgbBrightness','ledMode','ledR','ledG','ledB','buzzerVolume','batteryCapacity'].forEach(id=>{
        const el=$(id);
        if(!el)return;
        el.style.transform='translateZ(1px)';
        void el.offsetHeight;
        el.style.transform='';
      });
    });
  }

  return {
    $,
    setMsg,
    setToggle,
    updateSoundLightCard,
    syncRangeLabels,
    clamp255,
    setLedRgb,
    syncLedManualUi,
    fillInput,
    nudgeInputPaint,
  };
})();
</script>
<script>
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
</script>
<script>
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
      const dyn=!!j.imu_dynamic;
      st.textContent=(j.stasis?'Paused':(dyn?('Var ~'+hz):('~'+hz+' Hz')));
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
</script>
<script>
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
    const dyn=!!j.imu_dynamic;
    const parts=[dyn?('Var peak ~'+hz+' Hz'):('~'+hz+' Hz')];
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
</script>
<script>
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
</script>
<script>
window.AppControllers=(function(){
  const {$,setMsg,clamp255}=window.AppUi;
  const {collectOtAxes}=window.AppStateFns;

  function apiFailed(r,j,fallbackMsg){
    if(r.ok)return false;
    setMsg(j.error||fallbackMsg,'err');
    return true;
  }

  async function onScan(){
    const btn=$('btnScan');
    btn.classList.add('is-loading');
    btn.disabled=true;
    setMsg('Scanning… (tracking may hitch briefly)','');
    $('scanList').style.display='none';
    try{
      const j=await window.AppApi.scanNetworks();
      const box=$('scanList');while(box.firstChild)box.removeChild(box.firstChild);
      (j.networks||[]).forEach(n=>{
        const d=document.createElement('div');
        const t=document.createElement('span');t.textContent=n.ssid||'(hidden)';
        const rssi=document.createElement('span');rssi.className='rssi';rssi.textContent=(n.rssi!=null?n.rssi+' dBm':'');
        d.appendChild(t);d.appendChild(rssi);
        d.onclick=()=>{$('ssid').value=n.ssid||'';box.style.display='none'};
        box.appendChild(d);
      });
      box.style.display='block';
      setMsg('','');
    }catch(e){setMsg('Scan failed','err')}
    finally{
      btn.classList.remove('is-loading');
      btn.disabled=false;
    }
  }

  async function onSave(hydrateForm){
    setMsg('Saving…','');
    const otAxes=collectOtAxes();
    if(!otAxes)return;
    const body={
      ssid:$('ssid').value.trim(),
      ot_host:$('otHost').value.trim(),
      ot_port:parseInt($('otPort').value,10)||4242,
      ot_axes:otAxes,
      udp_enabled:$('udpToggle').classList.contains('on'),
      hatire_usb:$('hatireToggle').classList.contains('on'),
      mdns_on:$('mdnsToggle').classList.contains('on'),
      hostname:$('hostname').value.trim().toLowerCase(),
      imu_period_ms:parseInt($('imuPeriod').value,10)||10,
      imu_dynamic:$('imuDynamicToggle')?$('imuDynamicToggle').classList.contains('on'):false,
      wifi_tx:parseInt($('wifiTx').value,10),
      battery_capacity_mah:parseInt($('batteryCapacity').value,10),
      rgb_brightness:parseInt($('rgbBrightness').value,10),
      buzzer_volume:parseInt($('buzzerVolume').value,10)
    };
    if(body.rgb_brightness<0||body.rgb_brightness>100||Number.isNaN(body.rgb_brightness))body.rgb_brightness=25;
    if(body.buzzer_volume<0||body.buzzer_volume>100||Number.isNaN(body.buzzer_volume))body.buzzer_volume=25;
    if(body.battery_capacity_mah<100||body.battery_capacity_mah>5000||Number.isNaN(body.battery_capacity_mah))body.battery_capacity_mah=800;
    if(![0,1,2].includes(body.wifi_tx))body.wifi_tx=1;
    const lmEl=$('ledMode');
    if(lmEl){
      body.led_mode=parseInt(lmEl.value,10);
      if(![0,1,2,3].includes(body.led_mode))body.led_mode=0;
    }
    const lrEl=$('ledR'),lgEl=$('ledG'),lbEl=$('ledB');
    if(lrEl&&lgEl&&lbEl){
      body.led_r=clamp255(lrEl.value);
      body.led_g=clamp255(lgEl.value);
      body.led_b=clamp255(lbEl.value);
    }
    const pw=$('pass').value;
    if(pw.length)body.password=pw;
    try{
      const {response:r,json:j}=await window.AppApi.postConfig(body);
      if(apiFailed(r,j,'Save failed'))return;
      setMsg(j.restarting?'Saved — rebooting…':'Saved.','ok');
      if(!j.restarting)hydrateForm();
    }catch(e){setMsg('Network error','err')}
  }

  async function onReboot(){
    if(!confirm('Reboot device?'))return;
    try{
      const {response:r,json:j}=await window.AppApi.postReboot();
      if(apiFailed(r,j,'Reboot failed'))return;
      setMsg('Reboot sent…','ok');
    }catch(e){setMsg('Network error','err')}
  }

  async function onBatteryCal(hydrateForm){
    setMsg('Calibrating battery at 4.2V (about 3s)…','');
    try{
      const {response:r,json:j}=await window.AppApi.postConfig({battery_calibrate_4v2:true});
      if(apiFailed(r,j,'Calibration failed'))return;
      if(j.battery_calibrated){
        const off=(j.battery_cal_offset_mv!=null)?j.battery_cal_offset_mv:0;
        const sign=off>0?'+':'';
        setMsg('Battery calibrated ('+sign+off+' mV offset).','ok');
      }else{
        setMsg('Battery calibrated.','ok');
      }
      hydrateForm();
    }catch(e){setMsg('Network error','err')}
  }

  async function onFactoryReset(){
    if(!confirm('Reset all settings to factory defaults and reboot? You’ll need Azimuth‑Tracker (Offline Mode) or a USB flash to configure Wi‑Fi again.'))return;
    try{
      const {response:r,json:j}=await window.AppApi.postFactoryReset();
      if(apiFailed(r,j,'Factory reset failed'))return;
      setMsg('Resetting…','ok');
    }catch(e){setMsg('Request failed','err')}
  }

  /**
   * Map the device's begin-update result string to a friendly message and a
   * boolean for whether we should start polling progress.
   */
  function describeBeginResult(result){
    switch(result){
      case'started':return{ok:true,msg:'Update started. Don’t close this page or unplug the device.'};
      case'already_active':return{ok:true,msg:'Update already in progress.'};
      case'no_network':return{ok:false,msg:'Can’t update: device isn’t connected to your Wi‑Fi.'};
      case'offline_ap':return{ok:false,msg:'Can’t update from the Offline‑Mode AP. Join your Wi‑Fi first.'};
      case'thermal_hold':return{ok:false,msg:'Can’t update during thermal hold. Let the device cool, then power-cycle.'};
      case'battery_critical':return{ok:false,msg:'Battery too low (≤15%). Plug into USB to charge first.'};
      case'partition_unavailable':return{ok:false,msg:'No spare OTA slot available on this build.'};
      case'http_error':return{ok:false,msg:'Couldn’t reach the release server. Try again in a moment.'};
      case'chip_busy':return{ok:false,msg:'Chip is busy. Try again in a few seconds.'};
      default:return{ok:false,msg:'Update failed: '+(result||'unknown')};
    }
  }

  function lockUpdateButtons(reason){
    const ids=['btnUpdateManualWifi','btnUpdateBannerWifi'];
    ids.forEach(id=>{
      const el=document.getElementById(id);
      if(el){
        el.disabled=true;
        if(reason)el.title=reason;
      }
    });
    const banner=document.getElementById('updateBanner');
    if(banner)banner.style.display='none';
  }

  async function onUpdateNow(){
    const last=(window.AppState&&window.AppState.lastStatus)||{};
    const cur=last.fw_version||'?';
    const latest=last.fw_latest_version||'';
    const hasNewer=!!(last.fw_update_available&&latest);
    const prompt=hasNewer
      ?('Install firmware '+latest+' over Wi‑Fi? (currently '+cur+'). The device will reboot when the download completes.')
      :('Re‑install firmware '+cur+' over Wi‑Fi? The device pulls the same build from the release server again and reboots — useful for forcing a known-good flash. Continue?');
    if(!confirm(prompt))return;
    // Lock the buttons immediately so a double-click or "did it click?" retry
    // doesn't hit /api/update twice. Banner is also hidden right away — the
    // progress card takes over as the source of truth.
    lockUpdateButtons('Wireless update starting…');
    setMsg('Asking the device to fetch firmware…','');
    try{
      const {response:r,json:j}=await window.AppApi.postUpdate();
      const desc=describeBeginResult((j&&j.result)||'');
      if(!desc.ok&&!r.ok){
        setMsg(desc.msg,'err');
        // Re-enable so the user can retry; the next /api/status poll will
        // reset the disabled state correctly anyway.
        const ids=['btnUpdateManualWifi','btnUpdateBannerWifi'];
        ids.forEach(id=>{const el=document.getElementById(id);if(el)el.disabled=false});
        return;
      }
      setMsg(desc.msg,desc.ok?'ok':'err');
      if(window.AppUpdateProgress&&typeof window.AppUpdateProgress.show==='function'){
        window.AppUpdateProgress.show(j);
      }
    }catch(e){setMsg('Network error','err')}
  }

  return {onScan,onSave,onReboot,onBatteryCal,onFactoryReset,onUpdateNow};
})();
</script>
<script>
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
</script>
<script>
const {$,setToggle,syncRangeLabels,clamp255,setLedRgb,syncLedManualUi,fillInput,nudgeInputPaint}=window.AppUi;
const {applyShell}=window.AppViews;
const {applyOtAxesFromStatus}=window.AppStateFns;
const {onScan,onSave,onReboot,onBatteryCal,onFactoryReset,onUpdateNow}=window.AppControllers;
const uiTouched=window.AppState.uiTouched;
const monitorCfg=(window.AppConfig&&window.AppConfig.monitor)||{};
const POWER_IDLE_POLL_ACTIVE_MS=monitorCfg.idlePollActiveMs||18000;
const POWER_IDLE_POLL_VISIBLE_MS=monitorCfg.idlePollVisibleMs||12000;
const POWER_IDLE_GRACE_MS=monitorCfg.idleGraceMs||60000;
const POWER_BOOTSTRAP_RETRY_MS=monitorCfg.bootstrapRetryMs||3000;
const ACTIVITY_RESCHEDULE_MIN_MS=monitorCfg.activityRescheduleMinMs||1500;
const STALE_UI_AFTER_FAILURES=monitorCfg.staleUiAfterFailures||2;
const BOOT_SYNC_BURST_COUNT=monitorCfg.bootSyncBurstCount||6;
const BOOT_SYNC_BURST_GAP_MS=monitorCfg.bootSyncBurstGapMs||5000;
const FOCUS_SYNC_BURST_COUNT=monitorCfg.focusSyncBurstCount||4;
const FOCUS_SYNC_BURST_GAP_MS=monitorCfg.focusSyncBurstGapMs||3500;
const MANIFEST_PENDING_POLL_MS=monitorCfg.manifestPendingPollMs||5000;
const IMU_LIVE_POLL_MS=monitorCfg.posePreviewPollMs||700;
const TAB_COORD_KEY='azimuth.portal.activeTabs.v1';
const TAB_HEARTBEAT_MS=3000;
const TAB_STALE_MS=12000;
const TAB_MAX_ACTIVE=2;
let hydrateOk=false;
let pollFailCount=0;
let posePollTimer=0;
let tabHeartbeatTimer=0;
let tabAdmitted=true;
const tabId='tab_'+Math.random().toString(36).slice(2)+'_'+Date.now().toString(36);
/** One-shot shorter delay after `/api/status` (e.g. manifest check still running on device). */
let preferNextPollMs=null;
let burstSeriesId=0;
let didBootStatusBurst=false;

function syncTrackingHeroFromPose(j){
  const htr=$('heroTrack');
  const htrk=$('heroTrackSub');
  if(!htr)return;
  if(j.thermal_hold){
    htr.textContent='Wi‑Fi off';
    if(htrk)htrk.textContent='Cooling — USB tracking may still run';
    return;
  }
  if(j.stasis){
    htr.textContent='Paused';
    if(htrk)htrk.textContent='Low power';
    return;
  }
  const im=j.imu_period_ms;
  const hz=im?Math.round(1000/im):'—';
  const dyn=!!j.imu_dynamic;
  htr.textContent=dyn?('Var · ~'+hz+' Hz peak'):('~'+hz+' Hz');
  if(htrk)htrk.textContent=dyn?'Variable IMU rate':(im?'Update rate':'');
}

function applyLiveStatus(j){
  applyShell(j);
}

async function hydrateForm(){
  const j=await window.AppApi.getStatus();
  fillInput($('ssid'),j.ssid||'');
  fillInput($('pass'),'');
  fillInput($('hostname'),j.hostname||'');
  fillInput($('otHost'),j.ot_host||'');
  fillInput($('otPort'),j.ot_port||4242);
  const p=j.imu_period_ms||10;
  $('imuPeriod').value=[5,10,20,40].includes(p)?String(p):'10';
  if(!uiTouched.imuDyn){
    const imuDynEl=$('imuDynamicToggle');
    if(imuDynEl)setToggle('imuDynamicToggle',!!j.imu_dynamic);
  }
  $('wifiTx').value=([0,1,2].includes(j.wifi_tx))?String(j.wifi_tx):'1';
  const bc=$('batteryCapacity');
  if(bc){
    const v=parseInt(j.battery_capacity_mah,10);
    bc.value=Number.isFinite(v)?String(v):'800';
  }
  const rb=$('rgbBrightness'),bv=$('buzzerVolume');
  if(rb){
    const x=j.rgb_brightness;
    rb.value=(x!=null&&x>=0&&x<=100)?String(x):'25';
    rb.setAttribute('aria-valuenow',rb.value);
  }
  if(bv){
    const x=j.buzzer_volume;
    bv.value=(x!=null&&x>=0&&x<=100)?String(x):'25';
    bv.setAttribute('aria-valuenow',bv.value);
  }
  const lm=$('ledMode');
  if(lm){
    const m=j.led_mode;
    lm.value=([0,1,2,3].includes(Number(m)))?String(m):'0';
  }
  const lr=$('ledR'),lg=$('ledG'),lb=$('ledB');
  if(lr){
    const rv=(x)=>(x!=null&&x>=0&&x<=255)?clamp255(x):null;
    const r=rv(j.led_r),g=rv(j.led_g),b=rv(j.led_b);
    if(r!=null)lr.value=String(r);
    if(lg&&g!=null)lg.value=String(g);
    if(lb&&b!=null)lb.value=String(b);
  }
  syncRangeLabels();
  syncLedManualUi();
  applyOtAxesFromStatus(j.ot_axes);
  uiTouched.udp=uiTouched.mdns=uiTouched.hatire=uiTouched.imuDyn=false;
  applyLiveStatus(j);
  nudgeInputPaint();
  hydrateOk=true;
  window.AppState.power.lastStatusOkMs=Date.now();
  clearBootstrapTimer();
  if(j.fw_update_check_done===false&&!j.setup_ap){
    preferNextPollMs=MANIFEST_PENDING_POLL_MS;
  }
  if(!didBootStatusBurst){
    didBootStatusBurst=true;
    scheduleStatusBurst({count:BOOT_SYNC_BURST_COUNT,gapMs:BOOT_SYNC_BURST_GAP_MS});
  }
}
/**
 * Extra `/api/status` polls spaced by `gapMs` (boot, tab focus, OTA reload).
 * Cancels any previous burst series so navigations do not stack timers.
 */
function scheduleStatusBurst(opts){
  const count=(opts&&opts.count)||4;
  const gapMs=(opts&&opts.gapMs)||4000;
  if(!count||gapMs<=0)return;
  const series=++burstSeriesId;
  markUserActivity(true);
  let fired=0;
  function tick(){
    if(series!==burstSeriesId)return;
    if(fired>=count)return;
    fired++;
    pollOnly().finally(()=>{
      if(series!==burstSeriesId)return;
      if(fired<count)setTimeout(tick,gapMs);
    });
  }
  setTimeout(tick,gapMs);
}
async function pollOnly(){
  try{
    const j=await window.AppApi.getStatus();
    applyLiveStatus(j);
    hydrateOk=true;
    pollFailCount=0;
    window.AppState.power.lastStatusOkMs=Date.now();
    clearBootstrapTimer();
    if(j.fw_update_check_done===false&&!j.setup_ap){
      preferNextPollMs=MANIFEST_PENDING_POLL_MS;
    }
  }catch(e){
    pollFailCount++;
    if(pollFailCount>=STALE_UI_AFTER_FAILURES){
      const s=$('subLine');
      if(s){
        const msSinceOk=Date.now()-(window.AppState.power.lastStatusOkMs||0);
        const age=(window.AppState.power.lastStatusOkMs>0)?Math.max(1,Math.round(msSinceOk/1000)):'?';
        s.textContent='Connection lost — retrying (last update '+age+'s ago)…';
      }
    }
  }
}
let bootstrapTimer=0;
let lastUserActivityMs=window.AppState.power.lastUserActivityMs;
let pollTimer=window.AppState.power.pollTimer;
function markUserActivity(force){
  const now=Date.now();
  if(!force&&(now-lastUserActivityMs)<ACTIVITY_RESCHEDULE_MIN_MS)return;
  lastUserActivityMs=now;
  window.AppState.power.lastUserActivityMs=lastUserActivityMs;
  schedulePowerAwarePoll();
}
function readTabRegistry(){
  try{
    const raw=localStorage.getItem(TAB_COORD_KEY);
    if(!raw)return {};
    const o=JSON.parse(raw);
    return(o&&typeof o==='object')?o:{};
  }catch(e){return {}}
}
function writeTabRegistry(reg){
  try{localStorage.setItem(TAB_COORD_KEY,JSON.stringify(reg));}catch(e){}
}
function pruneTabRegistry(reg,now){
  Object.keys(reg).forEach(id=>{
    const rec=reg[id];
    if(!rec||typeof rec!=='object'||!Number.isFinite(rec.ts)||now-rec.ts>TAB_STALE_MS){
      delete reg[id];
    }
  });
}
function setTabAdmitted(next){
  if(tabAdmitted===next)return;
  tabAdmitted=next;
  if(!tabAdmitted){
    clearPollTimer();
    clearPosePollTimer();
  }else{
    markUserActivity(true);
    pollOnly();
    schedulePosePoll();
  }
}
function refreshTabAdmission(){
  const now=Date.now();
  const reg=readTabRegistry();
  pruneTabRegistry(reg,now);
  reg[tabId]={ts:now,visible:!document.hidden};
  const visibleActive=Object.entries(reg)
    .filter(([,rec])=>rec&&rec.visible===true&&Number.isFinite(rec.ts)&&now-rec.ts<=TAB_STALE_MS)
    .sort((a,b)=>b[1].ts-a[1].ts)
    .map(([id])=>id);
  const allowed=new Set(visibleActive.slice(0,TAB_MAX_ACTIVE));
  writeTabRegistry(reg);
  setTabAdmitted(!document.hidden&&allowed.has(tabId));
}
function clearTabHeartbeat(){
  if(!tabHeartbeatTimer)return;
  clearTimeout(tabHeartbeatTimer);
  tabHeartbeatTimer=0;
}
function scheduleTabHeartbeat(){
  clearTabHeartbeat();
  tabHeartbeatTimer=setTimeout(()=>{
    tabHeartbeatTimer=0;
    refreshTabAdmission();
    scheduleTabHeartbeat();
  },TAB_HEARTBEAT_MS);
}
function shouldPollNow(){
  if(document.hidden||!tabAdmitted)return false;
  return true;
}
function pollDelayMs(){
  if(!shouldPollNow())return 0;
  if(preferNextPollMs!=null){
    const v=preferNextPollMs;
    preferNextPollMs=null;
    return v;
  }
  // Visible tab: keep polling at least every `idlePollVisibleMs` even when the
  // user is not touching the screen (hardware FUNC pause, reading settings).
  if((Date.now()-lastUserActivityMs)>POWER_IDLE_GRACE_MS){
    return POWER_IDLE_POLL_VISIBLE_MS;
  }
  return POWER_IDLE_POLL_ACTIVE_MS;
}
function clearPollTimer(){
  if(pollTimer){
    clearTimeout(pollTimer);
    pollTimer=0;
    window.AppState.power.pollTimer=0;
  }
}
function clearBootstrapTimer(){
  if(bootstrapTimer){
    clearTimeout(bootstrapTimer);
    bootstrapTimer=0;
  }
}
function clearPosePollTimer(){
  if(!posePollTimer)return;
  clearTimeout(posePollTimer);
  posePollTimer=0;
}
function schedulePosePoll(){
  clearPosePollTimer();
  if(document.hidden||!tabAdmitted)return;
  if(!(window.AppPoseMascot&&typeof window.AppPoseMascot.isPreviewActive==='function'&&window.AppPoseMascot.isPreviewActive())){
    return;
  }
  posePollTimer=setTimeout(async()=>{
    posePollTimer=0;
    if(document.hidden){
      schedulePosePoll();
      return;
    }
    try{
      const j=await window.AppApi.getPose();
      if(window.AppPoseMascot&&typeof window.AppPoseMascot.applyPose==='function'){
        window.AppPoseMascot.applyPose(j);
      }
      syncTrackingHeroFromPose(j);
    }catch(e){}
    schedulePosePoll();
  },IMU_LIVE_POLL_MS);
}
function scheduleBootstrapRetry(){
  clearBootstrapTimer();
  if(hydrateOk||document.hidden)return;
  bootstrapTimer=setTimeout(async()=>{
    if(hydrateOk||document.hidden)return;
    await pollOnly();
    scheduleBootstrapRetry();
  },POWER_BOOTSTRAP_RETRY_MS);
}
function schedulePowerAwarePoll(){
  clearPollTimer();
  const d=pollDelayMs();
  if(!d)return;
  pollTimer=setTimeout(async()=>{
    pollTimer=0;
    if(shouldPollNow())await pollOnly();
    schedulePowerAwarePoll();
  },d);
  window.AppState.power.pollTimer=pollTimer;
}
$('udpToggle').onclick=()=>{uiTouched.udp=true;setToggle('udpToggle',!$('udpToggle').classList.contains('on'))};
$('mdnsToggle').onclick=()=>{uiTouched.mdns=true;setToggle('mdnsToggle',!$('mdnsToggle').classList.contains('on'))};
const imuDynToggleEl=$('imuDynamicToggle');
if(imuDynToggleEl){
  imuDynToggleEl.onclick=()=>{
    uiTouched.imuDyn=true;
    setToggle('imuDynamicToggle',!$('imuDynamicToggle').classList.contains('on'));
  };
}
$('hatireToggle').onclick=()=>{uiTouched.hatire=true;setToggle('hatireToggle',!$('hatireToggle').classList.contains('on'))};
[0,1,2].forEach(i=>{const id='otInv'+i;$(id).onclick=()=>setToggle(id,!$(id).classList.contains('on'));});
const _rb=$('rgbBrightness'),_bv=$('buzzerVolume');
if(_rb)_rb.addEventListener('input',syncRangeLabels);
if(_bv)_bv.addEventListener('input',syncRangeLabels);
['ledR','ledG','ledB'].forEach(id=>{const el=$(id);if(el)el.addEventListener('input',syncLedManualUi);});
const _lm=$('ledMode');
if(_lm)_lm.addEventListener('change',syncLedManualUi);
document.querySelectorAll('[data-led-rgb]').forEach(btn=>{
  btn.addEventListener('click',()=>{
    const raw=btn.getAttribute('data-led-rgb')||'';
    const p=raw.split(',').map(x=>clamp255(x.trim()));
    if(p.length!==3)return;
    const lm=$('ledMode');
    if(lm)lm.value='3';
    setLedRgb(p[0],p[1],p[2]);
  });
});
$('btnUseClientIp').onclick=()=>{
  const v=$('clientIpVal')&&$('clientIpVal').textContent;
  if(v&&v!=='—'){fillInput($('otHost'),v)}
};
$('btnScan').onclick=()=>onScan();
$('btnSave').onclick=()=>onSave(hydrateForm);
$('btnReboot').onclick=()=>onReboot();
$('btnBatteryCal').onclick=()=>onBatteryCal(hydrateForm);
$('btnFactory').onclick=()=>onFactoryReset();
const _btnUpdateManualWifi=$('btnUpdateManualWifi');
if(_btnUpdateManualWifi)_btnUpdateManualWifi.onclick=()=>onUpdateNow();
const _btnUpdateBannerWifi=$('btnUpdateBannerWifi');
if(_btnUpdateBannerWifi)_btnUpdateBannerWifi.onclick=()=>onUpdateNow();
if(window.AppUpdateProgress&&typeof window.AppUpdateProgress.init==='function'){
  window.AppUpdateProgress.init();
}
if(window.AppSections&&typeof window.AppSections.init==='function'){
  window.AppSections.init();
}
if(window.AppPoseMascot&&typeof window.AppPoseMascot.init==='function'){
  window.AppPoseMascot.init();
}
refreshTabAdmission();
scheduleTabHeartbeat();
window.addEventListener('azimuth:pose-preview-active',()=>{
  schedulePosePoll();
  if(window.AppPoseMascot&&typeof window.AppPoseMascot.isPreviewActive==='function'&&window.AppPoseMascot.isPreviewActive()){
    pollOnly();
  }
});
hydrateForm().catch(()=>{
  // Keep trying quickly until the first successful status arrives.
  scheduleBootstrapRetry();
});
['pointerdown','pointermove','keydown','touchstart','scroll'].forEach(ev=>{
  const force=(ev==='pointerdown'||ev==='keydown'||ev==='touchstart');
  window.addEventListener(ev,()=>markUserActivity(force),{passive:true});
});
document.addEventListener('visibilitychange',()=>{
  refreshTabAdmission();
  if(!document.hidden){
    markUserActivity(true);
    if(tabAdmitted){
      pollOnly();
      scheduleStatusBurst({count:FOCUS_SYNC_BURST_COUNT,gapMs:FOCUS_SYNC_BURST_GAP_MS});
      schedulePosePoll();
    }
    if(!hydrateOk)scheduleBootstrapRetry();
  }else{
    clearPollTimer();
    clearPosePollTimer();
    clearBootstrapTimer();
    burstSeriesId++;
  }
});
window.addEventListener('pageshow',e=>{
  if(!e.persisted)return;
  refreshTabAdmission();
  markUserActivity(true);
  if(tabAdmitted){
    pollOnly();
    scheduleStatusBurst({count:FOCUS_SYNC_BURST_COUNT,gapMs:FOCUS_SYNC_BURST_GAP_MS});
    schedulePosePoll();
  }
});
window.addEventListener('online',()=>{
  markUserActivity(true);
  if(tabAdmitted&&!document.hidden){
    pollOnly();
    scheduleStatusBurst({count:2,gapMs:2500});
  }
});
window.addEventListener('storage',e=>{
  if(e.key===TAB_COORD_KEY)refreshTabAdmission();
});
window.addEventListener('beforeunload',()=>{
  const reg=readTabRegistry();
  delete reg[tabId];
  writeTabRegistry(reg);
  clearTabHeartbeat();
});
schedulePowerAwarePoll();
scheduleBootstrapRetry();
</script>
</body>
</html>
)AZPORTAL";
