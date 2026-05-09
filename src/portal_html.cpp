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
.logo-wrap{width:min(92vw,232px);margin:0 auto;color:#e8eef5;filter:drop-shadow(0 3px 20px rgba(61,158,229,.2))}
.logo-wrap svg{display:block;width:100%;height:auto}
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
<div class="logo-wrap"><svg viewBox="0 0 679.66 768.31" fill="currentColor" role="img" aria-labelledby="azLogoTitle"><title id="azLogoTitle">Azimuth</title> <path fill-rule="evenodd" d="M337.12 0L332.32 0.99L328.01 3.29L324.46 6.67L321.96 10.89L316.52 22.29L311.07 33.7L305.62 45.1L300.17 56.5L294.12 69.16L288.08 81.82L282.03 94.47L275.99 107.13L250.23 161.06L224.47 214.99L198.7 268.91L172.93 322.83L158.06 327.49L143.08 332L128.09 336.52L113.18 341.24L113.65 341.8L114.91 342.27L116.4 342.72L117.54 343.19L119.11 343.76L120.68 344.43L122.26 344.8L123.86 344.52L134.92 341.17L146.02 337.94L157.17 334.87L168.36 331.98L167.4 334.56L166.11 337.27L164.72 340.01L163.48 342.69L161.5 347.02L159.5 351.08L156.64 354.16L152.08 355.57L128.52 362.54L104.99 369.77L81.8 377.89L59.21 387.55L58.6 388.14L59.23 388.37L60.37 388.45L61.3 388.64L62.96 389.58L64.16 391.06L64.8 392.87L64.78 394.78L79.22 387.5L95.58 380.4L111.3 374.48L123.8 370.74L116.95 375.17L109.91 380.36L104.81 386.7L103.64 390.42L103.76 394.57L104.08 395.31L104.46 395.8L104.98 395.94L105.77 395.64L105.72 390.31L107.84 385.57L111.62 381.36L116.57 377.64L128 371.49L138.15 366.76L142.2 365.1L146.31 363.53L150.48 362.16L154.65 361.09L150.99 368.9L147.14 376.77L143.42 384.61L140.11 392.27L154.11 390.91L168.11 389.53L182.1 388.08L196.08 386.53L207.24 385.2L218.38 383.81L229.52 382.36L240.65 380.84L247.04 379.94L253.43 379.03L259.81 378.09L266.19 377.14L269.2 376.73L271.11 375.22L272.43 372.93L273.66 370.16L278.3 360.85L282.63 350.99L287.21 341.44L292.59 333.08L284.94 325.74L276.83 318.52L268.59 311.34L260.54 304.11L255.54 299.69L255.36 299.14L256.32 299.33L260.35 300.94L265.07 302.41L279.7 307.23L294.34 312.16L308.97 317.01L323.58 321.57L324.47 317.35L325.48 312.98L326.32 308.68L326.67 304.69L320.75 305.67L314.84 306.69L308.93 307.74L303.03 308.83L303.4 308.06L303.76 307.25L304.11 306.44L304.49 305.67L310.28 304.77L316.13 303.94L321.94 303.07L327.63 302L330.32 289.67L330.76 284.52L326.15 284.37L313.66 287.03L319.47 274.17L325.66 261.25L331.96 248.3L338.09 235.38L343.36 245.99L348.52 256.82L353.67 267.7L358.85 278.49L347.91 280.32L345.51 281.25L344.63 282.62L345.75 287.97L348.05 298.95L352.79 298.55L357.66 297.85L362.57 297.05L367.42 296.38L367.76 297.06L368.1 297.79L368.4 298.51L368.62 299.17L363.77 299.73L358.92 300.27L354.07 300.85L349.24 301.51L348.95 303.74L349.49 306.45L350.28 309.3L350.74 311.93L352.01 318.68L353.4 321.19L356.5 320.7L362.88 318.46L374.97 314.46L387.07 310.44L399.16 306.43L411.26 302.42L413.79 301.6L416.34 300.73L418.87 299.89L421.35 299.15L412.15 307.35L402.93 315.54L393.72 323.73L384.52 331.94L386.99 337.47L389.62 343.12L392.38 348.66L395.23 353.88L402.54 352.35L409.84 350.8L417.14 349.24L424.44 347.66L444.09 343.32L463.7 338.84L483.29 334.24L502.86 329.57L503.69 329.36L504.57 329.19L505.4 328.97L506.09 328.56L505.12 326.47L504.13 324.39L503.14 322.31L502.15 320.24L500.02 315.62L497.61 310.91L495.39 306.26L493.82 301.81L503.76 300.57L513.73 299.13L523.7 298.04L533.68 297.87L552 300.89L562.82 304.76L565.68 306.97L566.97 309.35L566.81 311.87L565.29 314.51L558.61 320.11L547.76 326.01L516.92 338.15L479.44 349.8L442.01 359.86L393.99 370.75L400.83 376.87L407.7 382.96L414.57 389.06L421.4 395.18L407.38 390.59L393.29 385.91L379.19 381.21L365.15 376.54L354.76 378.5L350.92 380.93L349.84 386.3L347.71 397.09L344.83 410.41L344.57 413.44L345.43 414.67L351.46 413.66L364.84 411.19L395.66 405.11L426.34 398.32L456.84 390.76L487.11 382.35L513.23 374.49L550.96 361.41L592.92 344.82L631.72 326.42L647.62 317.06L659.96 307.89L667.81 299.1L669.76 294.93L670.24 290.93L669.14 287.14L666.34 283.57L661.72 280.27L655.17 277.25L635.82 272.16L607.35 268.53L608.19 269.26L609.03 270.04L609.85 270.84L610.63 271.64L631.14 273.45L647.69 276.84L659.63 281.7L663.67 284.65L666.3 287.93L667.46 291.52L667.06 295.42L665.02 299.61L661.25 304.08L648.22 313.79L627.32 324.44L599.05 335.37L570.13 344.47L540.8 352.24L511.29 359.21L527.64 353.29L544.31 347.15L561 340.75L577.39 334.03L576.22 333.59L574.8 333.07L573.38 332.55L572.21 332.12L568.55 333.33L564.84 334.53L561.13 335.71L557.45 336.85L552.67 338.22L553.66 337.14L565.31 329.22L572.16 323.12L577.15 316.12L578.35 312.39L578.37 308.57L576.98 304.72L573.93 300.88L571.58 299.05L568.97 297.6L566.24 296.39L563.5 295.28L568.07 295.19L572.69 295.18L577.32 295.24L581.93 295.34L602.07 297.44L609.48 299.68L614.28 302.71L615.82 306.56L613.49 311.23L606.65 316.74L594.65 323.1L592.07 324.31L590.02 325.12L589.8 325.89L592.73 327.02L606.45 319.31L614.71 312.76L618.08 307.27L617.18 302.76L612.59 299.13L604.9 296.29L582.59 292.6L555.02 290.95L526.91 290.59L488.08 290.8L484.07 282.6L483.6 280.69L484.28 279.74L489.38 279.2L500.03 277.93L520.25 275.65L540.5 273.67L560.79 272.16L581.11 271.32L583.06 269.62L583.84 268.62L583.32 267.6L581.35 265.84L555.05 264.8L528.72 264.4L502.4 264.69L476.1 265.71L451.72 214.67L423.59 155.76L395.57 97.11L371.53 46.83L367.05 37.46L362.56 28.09L358.08 18.71L353.59 9.34L350.69 5.28L346.73 2.26L342.09 0.44L337.12 0M578.58 52.03L577.78 52.47L577.08 53.1L576.54 53.85L576.21 54.67L573.58 55.01L571.8 56.59L571.27 58.86L572.4 61.28L571.25 63.62L571.81 65.95L573.63 67.54L576.23 67.68L577.54 69.84L579.83 70.55L582.14 69.85L583.52 67.73L586.14 67.52L587.94 65.88L588.47 63.55L587.25 61.24L588.46 58.93L587.94 56.59L586.15 54.96L583.54 54.77L582.87 53.3L581.68 52.32L580.18 51.88L578.58 52.03M125.37 61.35L110.42 64.3L96.89 70.94L85.3 80.63L76.14 92.71L64.56 91.39L50.58 89.82L37.53 90.65L32.41 92.79L28.78 96.51L29.21 101.68L32.32 106.19L37.43 110.11L43.86 113.51L57.91 118.99L69 123.13L69.62 131.11L71.3 138.71L77.48 152.57L86.85 164.29L98.72 173.44L112.36 179.58L127.09 182.3L142.21 181.15L149.7 179L157.01 175.73L164.79 171.01L171.75 165.18L177.78 158.39L182.77 150.78L192.78 151.99L202.87 153.06L212.96 153.44L223 152.56L226.59 151.36L229.01 149.44L230.61 144.28L228.46 138.75L223.22 134.52L215.84 130.58L208.21 127.14L200.42 124.06L192.56 121.19L191.12 120.73L190.33 119.86L190.01 118.66L189.95 117.17L187.99 105.36L183.65 94.39L177.24 84.54L169.09 76.07L159.51 69.25L148.81 64.34L137.33 61.62L125.37 61.35M54.86 101.23L59.16 101.61L63.45 102.03L67.74 102.52L72.02 103.12L70.28 109.88L68.68 111.7L65.11 110.25L57.45 107.26L50.67 104.33L45.59 102.05L44.81 101.29L45.8 100.86L54.86 101.23M194.33 133.38L199.7 135.43L205.04 137.58L210.28 139.93L215.38 142.58L209.48 142.49L203.56 142.18L197.65 141.68L191.79 140.99L188.96 140.71L187.71 140.12L187.66 138.72L188.43 135.99L189.06 132.85L189.9 131.85L191.47 132.28L194.33 133.38M464.54 96.11L463.28 99.5L461.91 103.19L460.54 106.89L459.28 110.32L449.07 114.01L445.62 115.81L449 117.62L459.27 121.34L462.95 131.47L464.77 134.85L466.62 131.47L470.36 121.32L480.47 117.66L483.84 115.85L480.48 114.01L470.36 110.27L468.94 106.63L467.57 102.95L466.22 99.28L464.83 95.62L464.71 95.67L464.64 95.81L464.59 95.97L464.54 96.11M532.79 134.39L522.76 136.77L514.32 140.69L507.45 145.9L502.1 152.16L498.25 159.22L495.85 166.83L494.88 174.74L495.3 182.71L497.08 190.5L500.18 197.84L504.56 204.5L510.2 210.23L517.05 214.78L525.09 217.91L534.28 219.36L544.59 218.9L553.84 216.17L561.61 212.07L567.94 206.82L572.86 200.64L576.4 193.75L578.58 186.37L579.44 178.73L579.02 171.05L577.33 163.53L574.41 156.42L570.29 149.92L565.01 144.25L558.58 139.65L551.05 136.32L542.45 134.5L532.79 134.39M26.56 155.33L25.82 157.25L25.14 159.23L24.39 161.16L23.45 162.94L19.76 164.42L14.17 166.29L10.2 168.08L9.91 168.8L11.34 169.33L14.45 170.5L17.59 171.63L20.72 172.79L23.76 174.06L27.47 184.21L29.33 187.43L31.15 183.93L34.78 173.92L44.01 170.51L48.61 168.92L46.9 167.3L37.16 163.77L34.12 161.1L32.35 157.03L31.04 152.5L29.4 148.41L28.63 149.76L27.92 151.59L27.25 153.56L26.56 155.33M70.72 216.78L62.14 220.03L56.54 225.75L53.76 232.98L53.67 240.77L56.12 248.16L60.95 254.2L68.02 257.93L77.18 258.4L85.92 255.16L91.64 249.45L94.48 242.22L94.59 234.44L92.13 227.05L87.23 221.01L80.04 217.27L70.72 216.78M164.31 222.35L162.81 223.67L162.12 225.55L161.74 227.61L161.15 229.46L155.37 228.17L152.34 229.76L152.54 233.23L156.48 237.62L154.64 239.34L152.8 241.27L151.88 243.45L152.79 245.89L154.68 247.12L156.85 247.01L159.11 246.3L161.26 245.72L161.81 248.35L162.65 251L164.23 252.82L166.96 252.99L168.75 251.82L169.63 249.92L170.07 247.71L170.57 245.65L176.31 247.02L179.41 245.41L179.26 241.89L175.25 237.58L179.29 233.23L179.45 229.75L176.35 228.16L170.56 229.49L169.98 226.7L168.98 223.99L167.21 222.25L164.31 222.35M593.57 254.93L592.33 255.91L591.75 257.37L591.4 258.94L590.84 260.3L586.35 259.48L583.77 261.23L583.68 264.39L586.69 267.81L583.66 271.3L583.78 274.46L586.43 276.19L590.99 275.37L591.57 277.75L592.67 279.87L594.43 281.11L596.97 280.84L598.22 279.84L598.88 278.43L599.26 276.86L599.67 275.37L604.2 276.19L606.88 274.48L607.04 271.33L604 267.86L606.98 264.36L606.9 261.21L604.3 259.49L599.71 260.29L599.09 257.87L597.93 255.73L596.13 254.53L593.57 254.93M332.37 298.81L330.91 305.84L329.49 312.91L328.04 319.95L326.49 326.9L312.85 322.45L299.23 317.95L285.61 313.43L271.99 308.92L282.65 318.48L293.4 328.05L304.18 337.6L314.95 347.12L304.72 356.28L293.4 366.22L282.21 376.13L272.35 385.18L284.9 381.09L297.42 376.93L309.94 372.77L322.48 368.64L323.5 368.33L324.52 367.97L325.54 367.65L326.54 367.4L326.99 369.46L327.42 371.53L327.84 373.6L328.28 375.67L329.85 383.26L331.41 390.85L332.97 398.44L334.53 406.04L336.92 417.78L338.13 421.85L339.34 418.04L341.77 406.17L343.76 396.51L345.74 386.85L347.74 377.19L349.74 367.53L351.78 367.94L353.94 368.63L356.12 369.41L358.26 370.07L369.68 373.86L381.1 377.66L392.52 381.45L403.96 385.18L394.16 376.14L382.96 366.22L371.62 356.26L361.41 347.13L371.71 337.96L382.98 327.96L394.21 318L404.41 308.91L400.2 310.28L395.98 311.68L391.76 313.08L387.54 314.48L378.09 317.59L368.62 320.8L359.16 323.92L349.74 326.76L347.73 317.09L345.74 307.41L343.75 297.74L341.76 288.06L340.87 283.77L339.99 279.47L339.09 275.18L338.17 270.92L336.68 277.87L335.23 284.85L333.8 291.83L332.37 298.81M580.9 322.18L579.73 325.14L577.48 326.88L574.62 328.01L571.66 329.13L573.46 329.87L575.49 330.63L577.54 331.36L579.4 332.05L581.37 337.43L582.38 339.21L583.39 337.42L585.32 332.07L589.85 330.41L592.37 329.49L591.69 328.5L586.64 326.64L584.53 324.34L583.36 320.66L582.4 318.85L580.9 322.18M103.44 333.2L102.59 335.49L101.72 337.85L100.85 340.22L99.99 342.52L91.5 345.6L88.68 347.15L91.51 348.68L99.98 351.74L103.01 360.08L104.57 363.01L106.13 360.33L109.2 351.85L112.24 350.63L115.35 349.49L118.46 348.36L121.52 347.18L118.48 345.93L115.36 344.83L112.27 343.68L109.31 342.33L107.79 338.83L106.24 333.93L104.76 330.95L103.44 333.2M82.41 351.84L63.72 359.04L45.18 366.83L27.45 376.09L11.16 387.72L5.03 395.4L2.87 402.53L4.02 409.02L7.83 414.77L13.65 419.7L20.83 423.72L36.65 428.72L56.32 432.17L76.14 434.44L116.05 436.14L156.09 435.27L195.97 433.28L225.33 430.88L254.62 427.81L283.85 424.13L313 419.91L317.87 419.15L322.75 418.39L327.62 417.61L332.5 416.83L332.45 415.7L332.21 414.45L331.9 413.18L331.67 411.95L330.5 406.17L329.31 400.4L328.12 394.62L326.94 388.84L326.5 386.67L326.18 384.94L325.25 384.05L322.95 384.4L294.2 389.26L265.41 393.93L236.58 398.3L207.68 402.25L184.23 404.59L160.53 406.1L136.96 405.29L125.34 403.55L113.9 400.66L112.26 401.79L114.1 403.1L124.37 405.9L136.89 408.27L143.89 409.41L122.82 410.8L93.63 412.23L65.77 411.33L55.29 409.28L48.68 405.76L46.17 405.58L44.13 404.4L42.74 402.45L42.19 400L40.26 404.13L40.08 408.49L41.48 412.63L44.32 416.11L38.03 415.07L31.77 413.78L25.65 412.01L19.78 409.53L15.64 406.37L14.36 402.84L15.35 399.11L18.02 395.35L25.97 388.33L33.42 383.09L47.57 374.91L62.25 367.72L77.28 361.26L92.49 355.27L95.32 354.18L95.63 353.47L94.11 352.82L91.45 351.89L89.16 350.97L86.93 350.63L84.7 350.91L82.41 351.84M529.89 385.13L502.97 391.75L475.9 398.03L448.84 404.15L421.93 410.33L435.14 438.13L448.37 465.91L461.63 493.68L474.89 521.45L495.2 564.57L503.17 579.01L511.71 590.17L522.35 599.14L536.62 607.01L556.02 614.87L582.1 623.8L592.6 627.61L603.1 631.41L613.59 635.22L624.09 639.02L631.3 639.98L638.06 637.62L643.2 632.64L645.52 625.74L642.93 614.82L638.13 604.2L632.59 593.75L627.79 583.31L618.08 562.98L608.36 542.64L598.65 522.3L588.93 501.96L579.2 481.59L569.47 461.21L559.74 440.84L550.01 420.47L546 412.07L541.99 403.68L537.98 395.28L533.98 386.88L533.36 385.49L532.56 384.74L531.44 384.63L529.89 385.13M52.68 387.36L51.67 387.74L50.79 388.39L50.15 389.26L49.84 390.3L47.19 390.43L45.32 392.05L44.75 394.4L46.02 396.72L44.79 399.1L45.3 401.46L47.1 403.1L49.74 403.32L51.18 405.5L53.51 406.2L55.82 405.42L57.19 403.19L59.8 403.15L61.62 401.47L62.14 399.04L60.85 396.77L62.15 394.46L61.63 392.09L59.8 390.44L57.14 390.27L56.54 388.92L55.49 387.91L54.16 387.35L52.68 387.36M107.5 394.62L107.37 394.96L107.09 395.71L106.81 396.46L106.68 396.8L103.7 397.88L102.81 398.46L103.86 399.07L106.72 400.23L107.17 401.24L107.54 402.53L107.97 403.63L108.6 404.07L108.98 403.07L109.32 402.03L109.71 401.03L110.23 400.12L111.32 399.64L112.86 399.14L113.99 398.66L113.87 398.21L110.37 396.5L108.97 393.98L108.42 392.68L107.5 394.62M615.1 412.54L610.73 413.7L607.17 416.22L604.4 419.65L602.4 423.56L594.16 425.67L592.49 427.15L592.31 428.72L595.87 431.59L603.86 433.15L606.2 436.54L609.29 438.95L616.7 440.87L624.12 438.93L627.21 436.52L629.56 433.15L637.72 431.52L641.11 428.6L640.78 427.02L638.99 425.54L630.64 423.52L628.55 418.77L624.92 415.12L620.27 412.92L615.1 412.54M602.48 427.4L602.66 429.59L602.03 430.19L600.66 429.9L598.65 429.47L595.66 428.43L596.65 427.5L599.59 426.79L602.48 426.39L602.48 426.54L602.48 426.89L602.48 427.24L602.48 427.4M633.9 426.84L637.58 427.95L636.99 428.86L634.1 429.62L630.85 430.26L630.71 429.44L630.81 428.42L630.96 427.34L630.92 426.35L631.67 426.44L632.42 426.56L633.17 426.69L633.9 426.84M232.58 443.07L202.01 446.71L171.4 449.93L140.74 452.65L110.03 454.82L91.49 493.43L71.27 535.7L51.14 577.86L32.85 616.15L31.61 619.04L30.89 622.09L30.76 625.21L31.29 628.31L34.13 634.02L39.01 638.09L45.05 640.01L51.39 639.29L62.11 635.41L72.83 631.53L83.55 627.66L94.28 623.78L109.07 618.33L124.06 613.23L138.78 607.56L152.77 600.38L162.95 591.64L171.43 581.41L178.58 570.04L184.79 557.89L195.88 532.61L207.69 508.35L215.54 491.94L223.38 475.52L231.23 459.11L239.07 442.69L237.84 442.38L236.11 442.57L234.24 442.91L232.58 443.07M366.56 443.68L359.34 445.32L353.22 448.09L348.19 451.81L344.23 456.3L341.32 461.39L339.44 466.9L338.7 478.49L341.86 489.66L344.86 494.65L348.78 499.01L353.6 502.57L359.3 505.14L365.87 506.55L373.28 506.64L380.45 504.97L386.53 502.18L391.52 498.46L395.46 493.98L398.35 488.9L400.21 483.4L400.94 471.85L397.79 460.71L394.81 455.73L390.91 451.38L386.12 447.82L380.45 445.24L373.93 443.8L366.56 443.68M657.57 474.28L657.15 475.43L656.21 477.97L655.27 480.51L654.84 481.67L644.67 485.41L641.31 487.26L644.7 489.09L654.81 492.75L658.53 503.02L660.32 506.43L662.12 502.99L665.84 492.75L675.02 489.34L679.66 487.76L677.98 486.13L668.24 482.6L665.41 480.34L663.78 476.77L662.61 472.74L661.16 469.1L660.28 467.54L659.37 469.11L658.46 471.97L657.57 474.28M47.38 475.45L42.54 477.6L39.57 481.12L38.33 485.45L38.66 490L40.44 494.2L43.53 497.47L47.79 499.24L53.07 498.92L57.72 496.7L60.58 493.2L61.77 488.94L61.44 484.48L59.7 480.36L56.69 477.12L52.54 475.3L47.38 475.45M407.03 531.8L406.31 532.26L405.74 532.88L405.29 533.6L404.95 534.38L403.28 534.33L401.75 534.91L400.6 536.05L400.04 537.65L400.03 538.54L400.27 539.36L400.69 540.12L401.18 540.82L400.04 543.2L400.58 545.51L402.38 547.11L405.01 547.35L406.41 549.5L408.69 550.17L410.98 549.41L412.41 547.28L413.78 547.4L415.1 547.08L416.21 546.32L416.98 545.16L417.3 543.98L417.13 542.83L416.7 541.73L416.2 540.69L417.27 538.41L416.67 536.09L414.85 534.5L412.32 534.46L411.63 532.9L410.33 531.88L408.71 531.49L407.03 531.8M225.38 549.39L224.09 552.86L222.68 556.68L221.26 560.51L219.96 564.02L209.92 567.66L206.47 569.47L209.75 571.33L219.86 575.07L223.66 585.21L225.49 588.61L227.28 585.23L230.99 575.04L241.11 571.36L244.55 569.56L241.23 567.73L231.05 563.99L229.6 560.35L228.27 556.58L226.9 552.87L225.38 549.39M17.86 710.77L13.4 724.59L8.94 738.42L4.48 752.24L0 766.06L4.82 766.04L10.66 766.02L16.49 766L21.31 765.98L23.12 760.17L25.04 754L26.95 747.84L28.73 742.08L36.07 742.08L44.95 742.08L53.83 742.08L61.17 742.08L63 747.84L65 754.01L67 760.18L68.88 765.98L74.25 765.94L80.28 766.08L86.18 766.1L91.17 765.72L82.92 740.07L73.81 711.76L64.68 683.38L56.36 657.5L51.53 657.5L45.7 657.5L39.87 657.5L35.05 657.5L32.37 665.83L26.46 684.14L20.55 702.45L17.86 710.77M46 688.95L48.74 697.84L51.44 706.74L54.12 715.64L56.83 724.54L53.09 725.01L48.89 725.02L44.57 724.9L40.45 724.95L38.78 724.93L37.09 724.93L35.41 724.91L33.79 724.81L35.07 720.35L36.4 715.89L37.75 711.43L39.08 706.98L40.41 702.82L42.11 696.18L44.02 690.44L45.01 688.95L46 688.95M102.88 666.95L102.88 668.43L102.88 671.68L102.88 674.93L102.88 676.41L113.82 676.43L127.04 676.45L140.27 676.47L151.21 676.48L139.69 692.48L125.78 711.83L111.86 731.17L100.35 747.16L100.35 751.44L100.35 756.61L100.35 761.78L100.35 766.05L117.71 766.05L138.7 766.05L159.68 766.05L177.04 766.05L177.04 761.77L177.04 756.6L177.04 751.42L177.04 747.14L164.85 747.12L151.51 747.16L138.24 747.16L126.28 747.01L138.13 730.25L151.2 711.8L164.31 693.29L176.3 676.38L176.3 672.11L176.3 666.94L176.3 661.77L176.3 657.5L159.68 657.5L139.59 657.5L119.5 657.5L102.88 657.5L102.88 658.97L102.88 662.22L102.88 665.48L102.88 666.95M193.57 666.95L193.57 668.43L193.57 671.68L193.57 674.93L193.57 676.41L199.8 676.41L207.34 676.41L214.88 676.41L221.12 676.41L221.12 692.42L221.12 711.78L221.12 731.13L221.12 747.14L214.88 747.14L207.34 747.14L199.8 747.14L193.57 747.14L193.57 751.42L193.57 756.6L193.57 761.77L193.57 766.05L210.86 766.05L231.77 766.05L252.67 766.05L269.96 766.05L269.96 761.77L269.96 756.6L269.96 751.42L269.96 747.14L263.69 747.14L256.11 747.14L248.53 747.14L242.26 747.14L242.26 731.13L242.26 711.77L242.26 692.42L242.26 676.41L248.53 676.41L256.11 676.41L263.69 676.41L269.96 676.41L269.96 672.13L269.96 666.95L269.96 661.78L269.96 657.5L252.67 657.5L231.76 657.5L210.86 657.5L193.57 657.5L193.57 658.97L193.57 662.22L193.57 665.48L193.57 666.95M284.55 711.78L284.55 720.26L284.55 738.91L284.55 757.57L284.55 766.05L289.1 766.05L294.61 766.05L300.11 766.05L304.66 766.05L304.64 748.32L304.73 728.78L304.57 709.28L303.77 691.63L304.83 691.94L306.11 693.73L307.37 695.92L308.39 697.4L311.57 702.36L315.47 708.48L319.39 714.62L322.61 719.67L323.48 719.67L324.54 719.67L325.59 719.67L326.46 719.67L329.76 714.26L333.64 707.91L337.51 701.6L340.76 696.28L341.65 694.92L342.48 693.38L343.44 692.04L344.74 691.28L344.07 709.5L343.92 728.58L343.99 747.71L343.97 766.05L348.79 766.05L354.62 766.05L360.45 766.05L365.26 766.05L365.26 741.48L365.26 711.77L365.26 682.07L365.26 657.5L360.39 657.5L354.51 657.5L348.62 657.5L343.75 657.5L339.27 664.75L334.47 672.71L329.61 680.59L325 687.57L320.35 680.6L315.45 672.7L310.59 664.73L306.09 657.5L301.21 657.5L295.32 657.5L289.43 657.5L284.55 657.5L284.55 665.98L284.55 684.64L284.55 703.29L284.55 711.78M379.12 694.61L379.2 710.59L378.82 726.99L380.73 742.79L383.4 750.16L387.67 757L394.09 762.04L401.61 765.61L409.85 767.71L418.42 768.31L426.94 767.4L435.03 764.96L442.29 760.97L448.35 755.42L454.05 744.82L456.34 733.12L456.72 720.93L456.69 708.89L456.75 697.91L456.71 683.34L456.67 668.69L456.7 657.5L452.15 657.5L446.65 657.5L441.15 657.5L436.6 657.5L436.57 670.67L436.56 686.42L436.55 702.09L436.51 715.03L436.59 726.75L434.36 738.45L431.88 743.29L428.23 746.93L423.19 748.95L416.58 748.97L410.97 747.91L406.81 745.38L403.88 741.68L401.97 737.11L400.42 726.59L400.48 716.23L400.44 703.01L400.43 686.99L400.43 670.91L400.4 657.5L395.58 657.5L389.76 657.5L383.93 657.5L379.11 657.5L379.11 663.3L379.11 676.06L379.12 688.81L379.12 694.61M468.46 675.44L468.46 678.24L468.46 684.41L468.46 690.58L468.46 693.38L472.34 693.38L477.02 693.38L481.71 693.38L485.59 693.38L485.59 689.14L485.59 684L485.59 678.87L485.59 674.62L488.96 674.62L493.03 674.62L497.11 674.62L500.48 674.62L500.48 691.44L500.48 711.78L500.48 732.11L500.48 748.93L496.47 748.93L491.62 748.93L486.77 748.93L482.76 748.93L482.76 752.81L482.76 757.49L482.76 762.18L482.76 766.05L495.53 766.05L510.98 766.05L526.42 766.05L539.19 766.05L539.19 762.18L539.19 757.49L539.19 752.8L539.19 748.93L535.22 748.93L530.41 748.93L525.6 748.93L521.62 748.93L521.62 732.11L521.62 711.77L521.62 691.44L521.62 674.62L524.96 674.62L528.99 674.62L533.03 674.62L536.36 674.62L536.36 678.87L536.36 684L536.36 689.14L536.36 693.38L540.24 693.38L544.93 693.38L549.61 693.38L553.49 693.38L553.49 685.26L553.49 675.44L553.49 665.62L553.49 657.5L534.24 657.5L510.97 657.5L487.71 657.5L468.46 657.5L468.46 660.3L468.46 666.47L468.46 672.64L468.46 675.44M565.25 711.78L565.25 720.26L565.25 738.91L565.25 757.57L565.25 766.05L570.07 766.05L575.9 766.05L581.73 766.05L586.55 766.05L586.55 755.77L586.55 743.34L586.55 730.92L586.55 720.64L594.47 720.64L604.05 720.64L613.62 720.64L621.54 720.64L621.54 730.92L621.54 743.35L621.54 755.77L621.54 766.05L626.36 766.05L632.19 766.05L638.02 766.05L642.84 766.05L642.84 741.48L642.84 711.77L642.84 682.07L642.84 657.5L638.02 657.5L632.19 657.5L626.36 657.5L621.54 657.5L621.54 667.51L621.54 679.61L621.54 691.71L621.54 701.72L613.62 701.72L604.04 701.72L594.47 701.72L586.55 701.72L586.55 691.71L586.55 679.61L586.55 667.51L586.55 657.5L581.73 657.5L575.9 657.5L570.07 657.5L565.25 657.5L565.25 665.98L565.25 684.64L565.25 703.29L565.25 711.78"/></svg></div>
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
<p class="banner banner-update" id="updateBanner" role="status">New firmware <strong id="updateBannerLatest">—</strong> is available (this device: <strong id="updateBannerCur">—</strong>). <a href="#" id="updateBannerLink" target="_blank" rel="noopener">Open USB installer</a></p>
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
<p class="hint" style="margin-top:.75rem;margin-bottom:0">Changing IMU interval reboots the device so the sensor can resync.</p>
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
    uiTouched:{udp:false,mdns:false,hatire:false},
    power:{
      lastUserActivityMs:Date.now(),
      lastStatusOkMs:0,
      pollTimer:0
    }
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
      idlePollActiveMs:25000,
      idlePollHeartbeatMs:60000,
      idleGraceMs:60000,
      bootstrapRetryMs:3000,
      activityRescheduleMinMs:1500,
      staleUiAfterFailures:2
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
  return {getStatus,getPose,scanNetworks,postConfig,postReboot,postFactoryReset};
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
    applyHero(j);
    if(window.AppPoseMascot&&typeof window.AppPoseMascot.applyStatus==='function'){
      window.AppPoseMascot.applyStatus(j);
    }
    const ub=$('updateBanner');
    if(ub){
      if(!ap&&j.fw_update_available&&j.fw_latest_version){
        ub.style.display='block';
        $('updateBannerLatest').textContent=j.fw_latest_version;
        $('updateBannerCur').textContent=j.fw_version||'—';
        const lk=$('updateBannerLink');
        if(lk&&j.fw_flasher_url){lk.href=j.fw_flasher_url}
      }else{ub.style.display='none'}
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

  return {onScan,onSave,onReboot,onBatteryCal,onFactoryReset};
})();
</script>
<script>
const {$,setToggle,syncRangeLabels,clamp255,setLedRgb,syncLedManualUi,fillInput,nudgeInputPaint}=window.AppUi;
const {applyShell}=window.AppViews;
const {applyOtAxesFromStatus}=window.AppStateFns;
const {onScan,onSave,onReboot,onBatteryCal,onFactoryReset}=window.AppControllers;
const uiTouched=window.AppState.uiTouched;
const monitorCfg=(window.AppConfig&&window.AppConfig.monitor)||{};
const POWER_IDLE_POLL_ACTIVE_MS=monitorCfg.idlePollActiveMs||25000;
const POWER_IDLE_POLL_HEARTBEAT_MS=monitorCfg.idlePollHeartbeatMs||60000;
const POWER_IDLE_GRACE_MS=monitorCfg.idleGraceMs||60000;
const POWER_BOOTSTRAP_RETRY_MS=monitorCfg.bootstrapRetryMs||3000;
const ACTIVITY_RESCHEDULE_MIN_MS=monitorCfg.activityRescheduleMinMs||1500;
const STALE_UI_AFTER_FAILURES=monitorCfg.staleUiAfterFailures||2;
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
  htr.textContent='~'+hz+' Hz';
  if(htrk)htrk.textContent=im?'Update rate':'';
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
  uiTouched.udp=uiTouched.mdns=uiTouched.hatire=false;
  applyLiveStatus(j);
  nudgeInputPaint();
  hydrateOk=true;
  window.AppState.power.lastStatusOkMs=Date.now();
  clearBootstrapTimer();
}
async function pollOnly(){
  try{
    applyLiveStatus(await window.AppApi.getStatus());
    hydrateOk=true;
    pollFailCount=0;
    window.AppState.power.lastStatusOkMs=Date.now();
    clearBootstrapTimer();
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
  return (Date.now()-lastUserActivityMs)<=POWER_IDLE_GRACE_MS
    ?POWER_IDLE_POLL_ACTIVE_MS
    :POWER_IDLE_POLL_HEARTBEAT_MS;
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
      schedulePosePoll();
    }
    if(!hydrateOk)scheduleBootstrapRetry();
  }else{
    clearPollTimer();
    clearPosePollTimer();
    clearBootstrapTimer();
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
