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
  --tx:#eef4fa;--muted:#8b9cb3;--acc:#3d9ee5;--acc-dim:rgba(61,158,229,.15);
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
.wrap{max-width:34rem;margin:0 auto;padding:clamp(1rem,4vw,2rem) clamp(1rem,3vw,1.25rem) 2.5rem}
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
#buzzerVolumeRow{margin-top:1.15rem}
.hint{font-size:.75rem;color:var(--muted);margin:-.25rem 0 .85rem;line-height:1.45}
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
@media(min-width:480px){
  .row-actions{flex-wrap:nowrap}
  .row-actions .btn{flex:1}
}
</style>
</head>
<body>
<div class="wrap">
<header class="brand">
<div class="logo-wrap"><svg viewBox="0 0 470 480" fill="currentColor" role="img" aria-labelledby="azLogoTitle"><title id="azLogoTitle">Azimuth</title> <path d="M0 0 C3.96 3.28 6.12 6.68 8.31 11.29 C8.61 11.9 8.61 11.9 10.11 15.04 C10.73 16.36 11.36 17.69 12 19.06 C12.33 19.76 12.33 19.76 14.01 23.27 C18.68 33.11 23.25 42.98 27.8 52.87 C32.56 63.15 37.52 73.33 42.5 83.5 C47.53 93.77 52.56 104.04 57.34 114.43 C58.39 116.7 59.47 118.96 60.57 121.21 C63.31 126.86 65.38 132.16 66.84 138.27 C70.59 151.83 70.59 151.83 77 156 C89.34 158.71 102.12 157.56 114.61 156.55 C123.18 155.87 131.44 156.2 140 157 C140 157.99 140 158.98 140 160 C134.01 160.55 128.02 161.1 122.03 161.65 C119.99 161.84 117.96 162.03 115.93 162.22 C104.29 163.29 92.67 164.31 81 165 C81.88 167.95 81.88 167.95 84 171 C87.51 171.6 90.35 171.79 93.87 171.73 C94.86 171.74 95.85 171.75 96.88 171.76 C100.06 171.79 103.25 171.77 106.44 171.75 C110.58 171.75 114.73 171.77 118.87 171.8 C119.88 171.8 120.9 171.81 121.94 171.81 C131.71 171.9 141.31 172.82 151 174 C151 174.33 151 174.66 151 175 C144.4 175 137.8 175 131 175 C132.65 175.99 134.3 176.98 136 178 C137.83 181.43 138.25 183.12 137.19 186.88 C134.08 191.31 130.16 194.57 126 198 C129.3 197.67 132.6 197.34 136 197 C132.38 199.71 129.87 200.97 125.7 202.48 C124.5 202.92 123.29 203.36 122.05 203.82 C120.8 204.27 119.54 204.72 118.25 205.19 C116.98 205.65 115.71 206.11 114.4 206.59 C111.27 207.73 108.14 208.87 105 210 C131.18 204.99 159.34 197.39 182 183 C182.99 183 183.98 183 185 183 C141.55 216.92 54.45 245 -1 245 C-0.44 241.52 0.15 238.04 0.75 234.56 C0.91 233.57 1.06 232.58 1.23 231.55 C1.73 228.69 1.73 228.69 3 224 C9.4 222.4 14.32 222.81 20.52 224.97 C21.71 225.37 22.89 225.78 24.12 226.2 C25.34 226.63 26.56 227.06 27.81 227.5 C30.26 228.35 32.7 229.19 35.14 230.03 C35.68 230.22 35.68 230.22 38.41 231.17 C41 232 41 232 43 232 C38.38 227.94 33.75 223.91 29 220 C32.19 217.88 34.27 217.36 37.98 216.59 C39.21 216.33 40.44 216.06 41.71 215.79 C43.04 215.51 44.38 215.23 45.75 214.94 C118.29 199.04 118.29 199.04 131 184 C131 183.34 131 182.68 131 182 C118.31 174.19 102.11 176.73 88 178 C90.31 183.28 92.62 188.56 95 194 C89.86 195.71 84.77 197.07 79.5 198.28 C78.59 198.49 77.67 198.7 76.73 198.92 C74.8 199.37 72.87 199.81 70.94 200.26 C68.04 200.93 65.13 201.6 62.22 202.28 C57.64 203.34 53.05 204.39 48.47 205.44 C46.19 205.96 43.92 206.49 41.64 207.02 C40.4 207.3 39.16 207.59 37.88 207.88 C37.35 208 37.35 208 34.68 208.62 C32 209 32 209 28 208 C24.68 204.31 22.99 201 23 196 C27.29 190.39 32.48 186.35 38 182 C39.35 180.68 40.69 179.35 42 178 C40.92 178.36 39.84 178.73 38.73 179.1 C34.73 180.45 30.73 181.79 26.73 183.13 C25 183.71 23.26 184.29 21.53 184.87 C19.05 185.71 16.56 186.54 14.07 187.38 C12.57 187.88 11.07 188.38 9.53 188.9 C6 190 6 190 4 190 C2.89 185.85 2 182.31 2 178 C5.63 177.67 9.26 177.34 13 177 C13 176.34 13 175.68 13 175 C11.68 175.33 10.36 175.66 9 176 C6.33 176.04 3.67 176.04 1 176 C0.34 172.7 -0.32 169.4 -1 166 C1.64 165.67 4.28 165.34 7 165 C-0.3 148.11 -0.3 148.11 -4 141 C-6.4 144.2 -8.01 146.83 -9.7 150.39 C-10.19 151.41 -10.67 152.43 -11.17 153.48 C-11.67 154.54 -12.17 155.6 -12.69 156.69 C-12.94 157.22 -12.94 157.22 -14.24 159.93 C-18 167.86 -18 167.86 -18 169 C-15.03 168.67 -12.06 168.34 -9 168 C-9.88 176.75 -9.88 176.75 -11 179 C-15.3 179.93 -19.64 180.41 -24 181 C-19.71 180.67 -15.42 180.34 -11 180 C-11.66 183.3 -12.32 186.6 -13 190 C-21.66 187.72 -30.17 185.08 -38.69 182.31 C-39.97 181.9 -41.26 181.48 -42.58 181.05 C-45.72 180.04 -48.86 179.02 -52 178 C-45.4 184.08 -38.79 190.13 -32 196 C-33.03 198.44 -34.08 200.88 -35.12 203.31 C-35.71 204.67 -36.29 206.03 -36.88 207.43 C-37.06 207.79 -37.06 207.79 -37.93 209.61 C-39 212 -39 212 -39.84 214.87 C-41.15 218.4 -42.25 220.4 -45 223 C-51.85 225.27 -58.78 225.84 -65.94 226.5 C-67.96 226.72 -69.99 226.95 -72.02 227.18 C-77.06 227.74 -82.11 228.25 -87.16 228.73 C-91.4 229.13 -95.63 229.58 -99.87 230.02 C-106.91 230.74 -113.95 231.4 -121 232 C-120.22 227.68 -118.83 224.17 -116.94 220.19 C-116.39 219.03 -115.84 217.86 -115.28 216.67 C-115.07 216.23 -115.07 216.23 -114 214 C-115.04 214.42 -116.07 214.84 -117.14 215.28 C-118.5 215.83 -119.85 216.37 -121.25 216.94 C-122.6 217.48 -123.94 218.03 -125.33 218.59 C-129 220 -132.06 221 -136 221 C-139.4 222.2 -142.76 223.44 -146.12 224.75 C-147.04 225.1 -147.95 225.45 -148.89 225.8 C-154.68 228.03 -160.35 230.44 -166 233 C-166.99 231.68 -167.98 230.36 -169 229 C-160.55 224.43 -151.87 221.41 -142.75 218.44 C-142.04 218.2 -142.04 218.2 -138.47 217.02 C-131.76 214.8 -125.09 212.77 -118.23 211.08 C-113.71 209.82 -112.31 209.35 -109.1 205.77 C-108.61 204.61 -108.13 203.45 -107.62 202.25 C-107.1 201.08 -106.58 199.91 -106.04 198.7 C-105.7 197.81 -105.35 196.92 -105 196 C-105.76 196.25 -106.51 196.51 -107.29 196.77 C-110.73 197.9 -114.18 199.01 -117.62 200.12 C-118.81 200.52 -120 200.92 -121.23 201.33 C-122.38 201.7 -123.54 202.07 -124.73 202.45 C-125.78 202.79 -126.84 203.14 -127.93 203.49 C-131.56 204.09 -133.65 203.41 -137 202 C-133.42 200.21 -130.14 199.02 -126.34 197.84 C-125.05 197.44 -123.76 197.04 -122.44 196.62 C-121.1 196.21 -119.76 195.8 -118.38 195.38 C-117.03 194.95 -115.68 194.53 -114.29 194.1 C-104.26 191 -104.26 191 -102 191 C-101.75 190.17 -101.49 189.34 -101.23 188.48 C-99.86 184.59 -98.15 181.01 -96.32 177.3 C-95.96 176.57 -95.6 175.83 -95.23 175.07 C-94.46 173.49 -93.68 171.92 -92.9 170.34 C-90.83 166.14 -88.78 161.94 -86.73 157.74 C-86.32 156.9 -85.91 156.06 -85.49 155.2 C-82.07 148.17 -78.76 141.1 -75.5 134 C-70.12 122.28 -64.49 110.68 -58.82 99.1 C-53.94 89.11 -49.14 79.1 -44.5 69 C-39.37 57.85 -34.06 46.79 -28.69 35.75 C-28.31 34.98 -27.94 34.21 -27.55 33.42 C-25.75 29.72 -23.94 26.02 -22.13 22.33 C-20.67 19.36 -19.22 16.39 -17.78 13.4 C-17.08 11.96 -16.37 10.51 -15.64 9.02 C-15.33 8.36 -15.33 8.36 -13.74 5.07 C-10.35 -0.91 -6.44 -0.61 0 0 Z " transform="translate(241,5)"/> <path d="M0 0 C8.11 6.66 14.4 14.15 15.12 25 C17.26 30.46 18.74 30.56 24.06 32.62 C25.37 33.05 26.67 33.48 28.02 33.93 C32.67 35.63 36.31 37.81 39.81 41.31 C39.62 43.81 39.62 43.81 38.81 46.31 C35.33 48.63 34.32 48.48 30.26 48.23 C29.17 48.18 28.08 48.14 26.96 48.1 C24.66 47.99 22.37 47.86 20.08 47.71 C11.83 47.44 11.83 47.44 8.84 50 C7.37 52.02 6.08 54.17 4.81 56.31 C-4.84 64.76 -15.66 66.59 -28.19 65.31 C-40.23 62.13 -47.6 55.23 -53.19 44.31 C-54.91 40.59 -55.19 38.98 -55.19 34.69 C-55.52 33.24 -55.85 31.8 -56.19 30.31 C-60.62 28.56 -60.62 28.56 -63.35 27.98 C-69.21 26.6 -76.78 23.5 -80.19 18.31 C-80.19 15.69 -80.19 15.69 -79.19 13.31 C-75.13 11.28 -74.13 11.13 -69.86 11.47 C-52.95 12.43 -52.95 12.43 -48.03 9.3 C-45.94 6.69 -45.94 6.69 -44.13 3.93 C-34.26 -9.39 -12.52 -8.04 0 0 Z M-70.19 17.31 C-67.31 20.19 -65.8 21.09 -62.12 22.5 C-61.25 22.84 -60.37 23.19 -59.46 23.54 C-57.19 24.31 -57.19 24.31 -55.19 24.31 C-54.86 22.33 -54.53 20.35 -54.19 18.31 C-59.47 17.98 -64.75 17.65 -70.19 17.31 Z M14.81 36.31 C14.48 37.96 14.15 39.61 13.81 41.31 C19.2 42.45 24.33 42.39 29.81 42.31 C24.92 38.65 20.99 36.31 14.81 36.31 Z " transform="translate(133.19,46.69)"/> <path d="M0 0 C2 2 2 2 2.19 5 C2.46 5.99 2.72 6.98 3 8 C6.5 9.25 6.5 9.25 10 10 C10.33 10.66 10.66 11.32 11 12 C9.91 12.35 8.81 12.7 7.69 13.06 C3.2 15.42 2.51 17.28 1 22 C-2 18 -2 18 -3 15 C-7.06 13.31 -7.06 13.31 -11 12 C-10 10 -10 10 -6.62 8.81 C-2.13 6.57 -1.5 4.67 0 0 Z " transform="translate(311,62)"/> <path d="M0 0 C7.11 5.69 9.94 12.02 10.36 21 C10.05 29.4 7.48 35.8 0.79 41.06 C-6.92 45.88 -13.18 46.36 -22.21 45.56 C-30.48 42.76 -35.37 37.3 -39.14 29.62 C-41.39 21.04 -40.33 14.26 -36.21 6.56 C-27.07 -4.86 -12.52 -8.17 0 0 Z " transform="translate(369.21,88.44)"/> <path d="M0 0 C3 4 3 4 5 8 C8.62 9.75 8.62 9.75 12 11 C9 13 9 13 6.44 13.19 C5.63 13.46 4.83 13.72 4 14 C2.81 17.5 2.81 17.5 2 21 C1.34 21.33 0.68 21.66 0 22 C-0.35 20.91 -0.7 19.81 -1.06 18.69 C-3.42 14.2 -5.28 13.51 -10 12 C-9.67 11.34 -9.34 10.68 -9 10 C-8.05 9.79 -7.1 9.59 -6.12 9.38 C-3 8 -3 8 -1.12 3.88 C-0.75 2.6 -0.38 1.32 0 0 Z " transform="translate(53,93)"/> <path d="M0 0 C5.36 2.53 7.34 5.35 9 11 C9 16.42 6.77 19.23 3 23 C-1.15 25.51 -4.37 25.16 -9 24 C-13.83 20.89 -15.74 17.98 -16.5 12.19 C-15.86 6.81 -13.81 4.74 -10 1 C-6.65 -0.68 -3.69 -0.27 0 0 Z " transform="translate(84,133)"/> <path d="M0 0 C1.32 0.33 2.64 0.66 4 1 C4 1.99 4 2.98 4 4 C5.65 4 7.3 4 9 4 C9 7.67 9 11.33 9 15 C7.35 14.67 5.7 14.34 4 14 C3.67 15.32 3.34 16.64 3 18 C1.68 17.67 0.36 17.34 -1 17 C-1 16.01 -1 15.02 -1 14 C-2.65 14.33 -4.3 14.66 -6 15 C-6.33 14.01 -6.66 13.02 -7 12 C-6.34 11.01 -5.68 10.02 -5 9 C-5.66 7.68 -6.32 6.36 -7 5 C-6 4 -6 4 -1 4 C-0.67 2.68 -0.34 1.36 0 0 Z " transform="translate(133,136)"/> <path d="M0 0 C0.99 0.33 1.98 0.66 3 1 C3 1.99 3 2.98 3 4 C4.32 3.67 5.64 3.34 7 3 C7 4.32 7 5.64 7 7 C7.3 8.67 7.63 10.34 8 12 C7.2 12.1 6.39 12.21 5.56 12.31 C5.14 12.43 5.14 12.43 3 13 C2.67 13.99 2.34 14.98 2 16 C1.01 16 0.02 16 -1 16 C-1.33 15.34 -1.66 14.68 -2 14 C-3.32 13.67 -4.64 13.34 -6 13 C-6 10.03 -6 7.06 -6 4 C-4.68 3.67 -3.36 3.34 -2 3 C-1.34 2.01 -0.68 1.02 0 0 Z " transform="translate(388,155)"/> <path d="M0 0 C1.75 3.51 2.54 5.98 3.29 9.75 C3.51 10.86 3.73 11.96 3.95 13.09 C4.17 14.24 4.4 15.38 4.62 16.56 C4.74 17.14 4.74 17.14 5.32 20.04 C7 28.58 7 28.58 7 32 C7.4 31.86 7.4 31.86 9.43 31.16 C13.05 29.91 16.68 28.67 20.31 27.44 C21.58 27 22.84 26.56 24.14 26.11 C25.35 25.7 26.56 25.29 27.8 24.87 C28.92 24.49 30.03 24.11 31.18 23.71 C34 23 34 23 37 24 C34.63 26.23 32.25 28.46 29.88 30.69 C29.21 31.31 28.54 31.94 27.86 32.58 C24.01 36.19 20.09 39.67 16 43 C18.23 47.47 21.12 50.15 24.75 53.44 C25.96 54.55 27.17 55.66 28.42 56.81 C31.56 59.61 34.75 62.33 38 65 C34.84 66.58 32.72 65.06 29.46 63.91 C28.16 63.46 26.86 63 25.52 62.53 C24.15 62.05 22.78 61.56 21.38 61.06 C20 60.58 18.63 60.1 17.21 59.6 C13.8 58.41 10.4 57.21 7 56 C6.89 57.24 6.78 58.47 6.66 59.75 C6.01 65.61 4.73 71.31 3.44 77.06 C3.32 77.59 3.32 77.59 2.73 80.26 C2.16 82.84 1.58 85.42 1 88 C-0.71 84.58 -1.53 82.17 -2.29 78.5 C-2.51 77.45 -2.73 76.4 -2.95 75.31 C-3.17 74.22 -3.4 73.13 -3.62 72 C-3.74 71.45 -3.74 71.45 -4.32 68.69 C-5.21 64.35 -6 60.45 -6 56 C-6.43 56.15 -6.43 56.15 -8.63 56.93 C-12.54 58.32 -16.46 59.69 -20.38 61.06 C-21.74 61.55 -23.11 62.03 -24.52 62.53 C-25.82 62.99 -27.12 63.44 -28.46 63.91 C-29.67 64.33 -30.87 64.76 -32.11 65.2 C-35 66 -35 66 -37 65 C-30.08 58.57 -23.13 52.19 -16 46 C-15.34 45.34 -14.68 44.68 -14 44 C-28.6 30.37 -28.6 30.37 -36 24 C-32.14 22.71 -30.63 23.56 -26.8 24.87 C-25.59 25.28 -24.38 25.69 -23.14 26.11 C-21.88 26.55 -20.61 26.99 -19.31 27.44 C-18.04 27.87 -16.76 28.31 -15.45 28.75 C-12.3 29.83 -9.15 30.91 -6 32 C-5.88 31.18 -5.76 30.36 -5.64 29.52 C-4.59 22.52 -3.37 15.61 -1.88 8.69 C-1.69 7.84 -1.51 6.99 -1.32 6.12 C-0.89 4.08 -0.44 2.04 0 0 Z " transform="translate(236,166)"/> <path d="M0 0 C2 3 2 3 3 7 C6.06 8.25 6.06 8.25 9 9 C7 10.33 5 11.67 3 13 C1.81 15.62 1.81 15.62 1 18 C-1 17 -1 17 -1.75 14.56 C-3 12 -3 12 -6.12 10.69 C-6.6 10.57 -6.6 10.57 -9 10 C-8 8 -8 8 -5.62 7.25 C-2.11 5.57 -1.45 3.52 0 0 Z " transform="translate(98,201)"/> <path d="M0 0 C0 0.66 0 1.32 0 2 C-2.82 3.3 -5.62 4.48 -8.5 5.62 C-38.09 17.75 -38.09 17.75 -45 26 C-45.75 29.24 -45.75 29.24 -46 32 C-40.93 34.76 -36.77 36.52 -31 37 C-31.33 34.69 -31.66 32.38 -32 30 C-31.3 30.29 -30.6 30.58 -29.88 30.88 C-20.78 34.43 -12.31 35.17 -2.56 35.12 C-2.09 35.12 -2.09 35.12 0.33 35.12 C8.91 35.07 17.44 34.64 26 34 C21.71 33.01 17.42 32.02 13 31 C17.1 29.97 18.95 30.25 23 31 C55.2 33.02 87.09 26.82 118.72 21.64 C119.78 21.47 120.85 21.29 121.94 21.12 C123.96 20.78 125.98 20.45 127.99 20.12 C134.78 19 134.78 19 137 19 C137.7 21.58 138.38 24.16 139.06 26.75 C139.45 28.19 139.83 29.63 140.22 31.11 C141 35 141 35 140 39 C136.73 39.94 136.73 39.94 132.26 40.56 C131.45 40.68 130.64 40.8 129.8 40.92 C127.1 41.3 124.39 41.65 121.69 42 C119.8 42.26 117.91 42.51 116.02 42.77 C87.9 46.52 59.37 49.57 31 50 C29.89 50.02 28.79 50.04 27.64 50.06 C-37.87 51.13 -37.87 51.13 -52 37 C-54.35 32.31 -54.41 30.98 -53 26 C-44.95 13.82 -29.83 8.71 -16.81 3.38 C-15.73 2.92 -14.64 2.47 -13.52 2.01 C-12.49 1.6 -11.46 1.19 -10.39 0.77 C-9.48 0.41 -8.56 0.04 -7.61 -0.34 C-4.68 -1.08 -2.88 -0.81 0 0 Z " transform="translate(92,213)"/> <path d="M0 0 C1.97 2.95 1.97 2.95 3.83 7.02 C4.52 8.48 5.21 9.95 5.92 11.46 C6.6 12.96 7.29 14.46 8 16 C9.27 18.6 10.54 21.2 11.82 23.8 C13.11 26.45 14.4 29.1 15.69 31.75 C16.38 33.18 17.08 34.61 17.79 36.09 C22.61 46.05 27.3 56.06 31.92 66.11 C36.14 75.24 40.52 84.3 44.94 93.34 C56.54 117.05 56.54 117.05 61.62 127.88 C61.85 128.35 61.85 128.35 63 130.73 C65.29 135.65 66.73 139.56 66 145 C63.03 148.91 60.85 150.49 56 151 C53.55 150.48 53.55 150.48 50.95 149.55 C49.96 149.2 48.96 148.86 47.94 148.5 C46.87 148.11 45.79 147.72 44.69 147.31 C43.57 146.91 42.44 146.51 41.29 146.1 C38.84 145.22 36.39 144.33 33.95 143.44 C31.23 142.45 28.51 141.46 25.78 140.49 C21.47 138.95 17.17 137.38 12.88 135.81 C11.52 135.33 10.16 134.84 8.76 134.34 C1.56 131.67 -4.16 129 -10 124 C-11.15 123.05 -12.31 122.1 -13.5 121.12 C-16.75 117.77 -18.6 114.1 -20.64 109.91 C-21.35 108.48 -22.05 107.06 -22.77 105.58 C-23.14 104.83 -23.5 104.08 -23.88 103.3 C-24.99 101.03 -26.1 98.76 -27.22 96.49 C-29.55 91.75 -31.87 87 -34.19 82.25 C-34.6 81.41 -35.01 80.56 -35.43 79.69 C-42.35 65.51 -49.07 51.25 -55.65 36.91 C-56.94 34.14 -58.25 31.37 -59.57 28.61 C-59.95 27.81 -60.34 27.01 -60.73 26.18 C-61.72 24.13 -62.71 22.08 -63.71 20.03 C-65 17 -65 17 -65 14 C-64.31 13.84 -63.61 13.68 -62.9 13.51 C-55.7 11.84 -48.5 10.17 -41.29 8.5 C-38.61 7.88 -35.92 7.25 -33.23 6.63 C-29.37 5.73 -25.5 4.84 -21.64 3.94 C-21.04 3.8 -21.04 3.8 -17.99 3.1 C-17.43 2.97 -17.43 2.97 -14.59 2.31 C-13.61 2.08 -12.62 1.85 -11.61 1.61 C-7.49 0.64 -4.3 0 0 0 Z " transform="translate(352,233)"/> <path d="M0 0 C2.7 1.8 4.54 3.08 6 6 C7.98 6.73 9.98 7.4 12 8 C10.56 12.33 8.12 12.35 4 14 C3.15 14.58 2.31 15.15 1.44 15.75 C-1.82 17.42 -3.54 17.05 -7 16 C-7.99 15.01 -8.98 14.02 -10 13 C-11.22 12.48 -12.43 11.97 -13.69 11.44 C-14.78 10.96 -15.87 10.49 -17 10 C-17 9.34 -17 8.68 -17 8 C-16.53 7.97 -16.53 7.97 -14.12 7.81 C-11 7 -11 7 -9.81 4.56 C-9.68 4.14 -9.68 4.14 -9 2 C-5.49 -0.34 -4.14 -0.5 0 0 Z M-14 8 C-14.33 8.66 -14.66 9.32 -15 10 C-13.68 10 -12.36 10 -11 10 C-11 9.34 -11 8.68 -11 8 C-11.99 8 -12.98 8 -14 8 Z M6 8 C6 8.66 6 9.32 6 10 C7.32 9.67 8.64 9.34 10 9 C8.68 8.67 7.36 8.34 6 8 Z " transform="translate(404,249)"/> <path d="M0 0 C5.52 4.62 8.36 8.87 9.06 16.06 C8.49 22.42 6.32 26.58 1.88 31.12 C-4.2 35.17 -9.21 35.89 -16.31 34.25 C-22.33 31.24 -25.89 28.6 -28.31 22.25 C-29.47 14.25 -28.66 8.62 -23.31 2.25 C-15.79 -2.89 -8.37 -4.42 0 0 Z " transform="translate(265.31,269.75)"/> <path d="M0 0 C-1.92 6.09 -4.15 11.69 -6.99 17.43 C-7.37 18.2 -7.76 18.98 -8.15 19.78 C-9.39 22.29 -10.63 24.8 -11.88 27.31 C-13.6 30.81 -15.32 34.31 -17.04 37.81 C-17.46 38.68 -17.89 39.54 -18.33 40.44 C-22.66 49.29 -26.79 58.23 -30.81 67.23 C-34.46 75.25 -38.51 82.9 -44.75 89.25 C-45.32 89.85 -45.89 90.46 -46.48 91.08 C-53.53 96.46 -62.23 99.39 -70.48 102.39 C-71.54 102.78 -72.6 103.17 -73.69 103.57 C-75.92 104.39 -78.14 105.2 -80.36 106.01 C-83.76 107.25 -87.15 108.5 -90.54 109.76 C-92.71 110.55 -94.88 111.35 -97.05 112.14 C-97.55 112.33 -97.55 112.33 -100.11 113.27 C-112.24 117.68 -112.24 117.68 -117.25 115.75 C-120.66 113.58 -121.83 112.08 -122.46 108.07 C-121.78 102.07 -119.24 97.27 -116.62 91.88 C-116.05 90.67 -115.48 89.46 -114.89 88.21 C-112.94 84.13 -110.97 80.07 -109 76 C-108.28 74.52 -107.57 73.05 -106.83 71.52 C-103.79 65.27 -100.75 59.02 -97.69 52.78 C-91.33 39.75 -85.17 26.62 -79.13 13.44 C-78.13 11.27 -77.07 9.13 -76 7 C-72.2 6.31 -68.64 5.87 -64.81 5.59 C-63.68 5.49 -62.54 5.38 -61.37 5.28 C-57.73 4.96 -54.08 4.67 -50.44 4.38 C-46.85 4.07 -43.27 3.76 -39.69 3.45 C-37.36 3.25 -35.03 3.05 -32.7 2.86 C-24.89 2.19 -17.12 1.37 -9.34 0.35 C-6.2 0.02 -3.16 -0.07 0 0 Z " transform="translate(177,267)"/> <path d="M0 0 C2 3 2 3 2.19 5.56 C2.46 6.37 2.72 7.17 3 8 C6.5 9.19 6.5 9.19 10 10 C10.33 10.66 10.66 11.32 11 12 C8.09 13.26 6.2 14 3 14 C2.88 14.76 2.75 15.53 2.62 16.31 C2 19 2 19 0 22 C-3 16.25 -3 16.25 -3 14 C-5.64 13.34 -8.28 12.68 -11 12 C-10.67 11.34 -10.34 10.68 -10 10 C-9.07 9.79 -8.14 9.59 -7.19 9.38 C-2.63 7.41 -1.86 4.44 0 0 Z " transform="translate(427,282)"/> <path d="M0 0 C3 3 3 3 3.44 5.94 C2.9 9.72 2.14 10.85 -1 13 C-4.67 13.61 -6.29 13.49 -9.38 11.38 C-11.5 8.27 -11.69 6.69 -11 3 C-7.47 -1.04 -5.07 -1.65 0 0 Z " transform="translate(70,287)"/> <path d="M0 0 C0.66 0.33 1.32 0.66 2 1 C2.21 2.13 2.41 3.27 2.62 4.44 C4 8 4 8 8.12 9.44 C9.4 9.62 10.68 9.81 12 10 C11 12 11 12 7.62 13.19 C3.13 15.43 2.5 17.33 1 22 C-1 19 -1 19 -1.19 16.44 C-1.46 15.63 -1.72 14.83 -2 14 C-5.5 12.81 -5.5 12.81 -9 12 C-9.33 11.34 -9.66 10.68 -10 10 C-8.7 9.77 -7.4 9.55 -6.06 9.31 C-4.72 8.88 -3.38 8.45 -2 8 C-1.06 5.56 -1.06 5.56 -1 3 C-0.67 2.01 -0.34 1.02 0 0 Z " transform="translate(169,331)"/> <path d="M0 0 C2.94 -0.2 2.94 -0.2 6.56 -0.12 C7.76 -0.11 8.96 -0.09 10.19 -0.07 C10.65 -0.06 10.65 -0.06 13 0 C15.21 6.74 17.41 13.48 19.62 20.22 C20.36 22.51 21.11 24.8 21.86 27.08 C33 61.1 33 61.1 33 64 C28.71 64 24.42 64 20 64 C16 53.38 16 53.38 16 50 C9.4 50 2.8 50 -4 50 C-4.66 52.97 -5.32 55.94 -6 59 C-6.74 62.04 -6.74 62.04 -8 64 C-10.82 64.49 -10.82 64.49 -14.12 64.31 C-15.22 64.27 -16.32 64.22 -17.45 64.18 C-18.29 64.12 -19.13 64.06 -20 64 C-18.51 56.69 -16.42 49.69 -14.12 42.6 C-13.75 41.44 -13.38 40.28 -12.99 39.09 C-12.21 36.65 -11.43 34.22 -10.64 31.79 C-9.43 28.05 -8.23 24.31 -7.03 20.58 C-6.27 18.21 -5.51 15.84 -4.75 13.47 C-4.39 12.35 -4.03 11.23 -3.65 10.08 C-3.32 9.04 -2.99 8.01 -2.64 6.95 C-2.35 6.04 -2.05 5.13 -1.75 4.19 C-1 2 -1 2 0 0 Z M5 19 C4.01 22.46 4.01 22.46 -1 40 C3.62 40 8.24 40 13 40 C12.38 37.44 11.75 34.87 11.12 32.31 C10.78 30.89 10.43 29.46 10.07 27.99 C8.42 21.83 8.42 21.83 7 19 C6.34 19 5.68 19 5 19 Z " transform="translate(57,394)"/> <path d="M0 0 C14.52 0 29.04 0 44 0 C44 10.05 43.84 12.85 38.31 20.06 C37.73 20.86 37.15 21.66 36.56 22.48 C35.34 24.16 34.12 25.82 32.88 27.48 C31 29.99 29.17 32.53 27.35 35.09 C23.29 40.77 19.19 46.41 15 52 C24.57 52.33 34.14 52.66 44 53 C44 56.63 44 60.26 44 64 C29.15 64 14.3 64 -1 64 C-1 50.25 -1 50.25 5.25 42.81 C6.53 41.18 7.82 39.55 9.1 37.92 C9.75 37.11 10.4 36.3 11.07 35.46 C14.1 31.6 16.89 27.59 19.69 23.56 C22.41 19.67 25.15 15.8 28 12 C23.38 11.84 23.38 11.84 0 11 C0 7.37 0 3.74 0 0 Z " transform="translate(97,394)"/> <path d="M0 0 C14.85 0 29.7 0 45 0 C45 3.63 45 7.26 45 11 C39.72 11 34.44 11 29 11 C29 24.86 29 38.72 29 53 C34.28 53 39.56 53 45 53 C45 56.63 45 60.26 45 64 C30.15 64 15.3 64 0 64 C0 60.37 0 56.74 0 53 C5.28 53 10.56 53 16 53 C16 39.14 16 25.28 16 11 C10.72 11 5.44 11 0 11 C0 7.37 0 3.74 0 0 Z " transform="translate(151,394)"/> <path d="M0 0 C13 0 13 0 16.42 3.46 C17.25 4.93 18.02 6.42 18.75 7.94 C19.16 8.69 19.57 9.45 19.99 10.22 C21.03 12.13 22.02 14.06 23 16 C23.33 16.33 23.66 16.66 24 17 C24.71 15.86 25.42 14.71 26.15 13.54 C27.08 12.04 28.01 10.55 28.94 9.06 C29.41 8.31 29.87 7.55 30.36 6.78 C33.89 1.11 33.89 1.11 35 0 C38.96 0 42.92 0 47 0 C47 21.12 47 42.24 47 64 C43.04 64 39.08 64 35 64 C34.84 57.07 34.84 57.07 34 22 C31.03 26.95 28.06 31.9 25 37 C24.01 37 23.02 37 22 37 C20.35 34.36 20.35 34.36 12 21 C12 35.19 12 49.38 12 64 C8.04 64 4.08 64 0 64 C0 42.88 0 21.76 0 0 Z " transform="translate(205,394)"/> <path d="M0 0 C3.96 0 7.92 0 12 0 C12.02 1.33 12.04 2.65 12.06 4.02 C12.14 8.96 12.23 13.9 12.32 18.84 C12.36 20.98 12.39 23.11 12.42 25.25 C12.47 28.32 12.53 31.4 12.59 34.47 C12.6 35.42 12.61 36.37 12.62 37.35 C12.74 42.52 13.07 47.17 15 52 C19.3 53.43 23.53 53.53 28 53 C31.61 50.54 31.92 49.32 33 45 C33.15 41.58 33.26 38.18 33.32 34.77 C33.34 33.77 33.36 32.78 33.38 31.76 C33.44 28.59 33.5 25.42 33.56 22.25 C33.61 20.1 33.65 17.95 33.69 15.8 C33.8 10.53 33.9 5.27 34 0 C35.81 -0.03 37.62 -0.05 39.44 -0.06 C40.45 -0.07 41.46 -0.09 42.5 -0.1 C45 0 45 0 46 1 C46.11 3.99 46.16 6.95 46.17 9.93 C46.17 10.39 46.17 10.39 46.18 12.68 C46.18 14.61 46.19 16.54 46.19 18.47 C46.19 21.39 46.21 24.32 46.22 27.24 C46.23 29.12 46.23 30.99 46.23 32.87 C46.24 33.74 46.24 34.6 46.25 35.49 C46.22 45.45 45.18 54.22 37.56 61.25 C30.02 66.4 20.67 66.44 11.97 64.23 C5.14 61.32 2.39 56.86 0 50 C-0.61 46 -0.59 42.12 -0.49 38.09 C-0.48 36.99 -0.47 35.89 -0.47 34.77 C-0.44 31.3 -0.38 27.84 -0.31 24.38 C-0.29 22.01 -0.26 19.65 -0.24 17.29 C-0.19 11.52 -0.1 5.76 0 0 Z " transform="translate(261,394)"/> <path d="M0 0 C16.5 0 33 0 50 0 C50 6.93 50 13.86 50 21 C46.7 21 43.4 21 40 21 C40 17.37 40 13.74 40 10 C37.03 10 34.06 10 31 10 C31 24.52 31 39.04 31 54 C34.3 54 37.6 54 41 54 C41 57.3 41 60.6 41 64 C30.11 64 19.22 64 8 64 C8 60.7 8 57.4 8 54 C11.63 54 15.26 54 19 54 C19 39.48 19 24.96 19 10 C16.03 10 13.06 10 10 10 C10 13.63 10 17.26 10 21 C6.7 21 3.4 21 0 21 C0 14.07 0 7.14 0 0 Z " transform="translate(314,394)"/> <path d="M0 0 C3.96 0 7.92 0 12 0 C12 8.58 12 17.16 12 26 C18.93 26 25.86 26 33 26 C33 17.42 33 8.84 33 0 C37.29 0 41.58 0 46 0 C46 21.12 46 42.24 46 64 C41.71 64 37.42 64 33 64 C33 55.09 33 46.18 33 37 C29.7 37.16 29.7 37.16 13 38 C12.67 46.58 12.34 55.16 12 64 C8.04 64 4.08 64 0 64 C0 42.88 0 21.76 0 0 Z " transform="translate(371,394)"/> </svg></div>
<p class="sub" id="subLine">Tracker settings</p>
</header>
<p class="banner" id="setupBanner">Join <strong>Azimuth‑Setup</strong> (open network). This page may open automatically; if not, use <strong id="portalUrl">http://192.168.4.1/</strong> on <strong>port 80</strong>. <strong>azimuth.local</strong> works only on your home Wi‑Fi after setup.</p>
<p class="banner banner-update" id="updateBanner" role="status">New firmware <strong id="updateBannerLatest">—</strong> is available (this device: <strong id="updateBannerCur">—</strong>). <a href="#" id="updateBannerLink" target="_blank" rel="noopener">Open USB installer</a></p>

<div class="card">
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

<div class="card">
<div class="hd">LAN & discovery</div>
<p class="hint">mDNS advertises <code>http://&lt;hostname&gt;.local:8080</code>. Changing hostname or mDNS requires a reboot.</p>
<div class="row" style="margin-bottom:.75rem">
<span>mDNS (Bonjour)</span>
<button type="button" class="toggle" id="mdnsToggle" aria-label="Toggle mDNS"></button>
</div>
<label for="hostname">Device hostname</label>
<input type="text" id="hostname" autocomplete="off" autocapitalize="none" spellcheck="false" maxlength="24" placeholder="azimuth"/>
</div>

<div class="card">
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

<div class="card">
<div class="hd">Tracking & radio</div>
<p class="hint">Faster IMU reports reduce latency and increase USB/Wi‑Fi load. Power profile adjusts network servicing and Wi‑Fi sleep behavior.</p>
<label for="powerProfile">Power profile</label>
<select id="powerProfile" aria-label="Power profile">
<option value="0">Performance tracking — max responsiveness</option>
<option value="1" selected>Balanced — default</option>
<option value="2">Battery saver — lowest heat / battery use</option>
</select>
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

<div class="card" id="cardSoundLight" style="display:none">
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

<div class="card">
<div class="hd">Device</div>
<p class="sub" style="margin:0 0 .75rem">Firmware <strong id="fwVer">—</strong> · Battery: <strong id="battState">stub</strong>.</p>
<div class="row row-actions">
<button type="button" class="btn btn-primary" id="btnSave">Save</button>
<button type="button" class="btn btn-sec" id="btnReboot">Reboot</button>
</div>
<div id="msg"></div>
<pre class="stats" id="stats"></pre>
</div>

<div class="card card-danger">
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
<script>
const $=id=>document.getElementById(id);
function setMsg(t,cls){const m=$('msg');m.textContent=t||'';m.className=cls||''}
function setToggle(id,on){$(id).classList.toggle('on',on)}
const uiTouched={udp:false,mdns:false,hatire:false};
function updateSoundLightCard(j){
  const card=$('cardSoundLight'),hint=$('soundLightHint');
  const rr=$('rgbBrightnessRow'),br=$('buzzerVolumeRow');
  if(!card)return;
  const hasRgb=!!j.has_rgb,hasBz=!!j.has_buzzer;
  const show=hasRgb||hasBz;
  card.style.display=show?'block':'none';
  if(hint)hint.style.display=show?'block':'none';
  if(rr)rr.style.display=hasRgb?'block':'none';
  if(br)br.style.display=hasBz?'block':'none';
}
function syncRangeLabels(){
  const rb=$('rgbBrightness'),rv=$('rgbBrightnessVal');
  const bb=$('buzzerVolume'),bv=$('buzzerVolumeVal');
  if(rb&&rv)rv.textContent=rb.value+'%';
  if(bb&&bv)bv.textContent=bb.value+'%';
}
function applyShell(j){
  const ap=!!j.setup_ap;
  $('setupBanner').style.display=ap?'block':'none';
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
  $('subLine').textContent=ap?'Provisioning · join your Wi‑Fi below':'On your network · idle until you use this page';
  if(ap&&j.portal_url)$('portalUrl').textContent=j.portal_url;
  const hz=j.imu_period_ms?Math.round(1000/j.imu_period_ms):'—';
  const cip=j.http_client_ip;
  const rssi=ap?'':(j.rssi!=null?j.rssi+' dBm':'—');
  const line1=ap?('AP · board '+ (j.ip||'—')):('LAN · board '+(j.ip||'—')+' · RSSI '+rssi);
  const line1b=' · up '+Math.round(j.uptime_ms/1000)+'s · heap '+j.heap_free;
  let udpSummary=j.udp_enabled===false?'UDP off':(j.ot_target_ok?'UDP ok':'UDP pending');
  if(j.ot_target_ok&&j.ot_resolved_ip)udpSummary+=' → '+j.ot_resolved_ip;
  const line2='FW '+(j.fw_version||'?')+' · ~'+hz+' Hz · '+ (j.hostname||'azimuth')+' · '+udpSummary+' · STA '+(j.wifi_connected?'on':'off');
  $('stats').textContent=line1+line1b+'\n'+line2;
  if($('fwVer'))$('fwVer').textContent=j.fw_version||'—';
  if($('battState')){
    if(j.battery_mv!=null){$('battState').textContent=(j.battery_mv+' mV')}
    else{$('battState').textContent=(j.battery_state||'stub')}
  }
  if(!uiTouched.udp)setToggle('udpToggle',!!j.udp_enabled);
  if(!uiTouched.mdns)setToggle('mdnsToggle',!!j.mdns_on);
  if(!uiTouched.hatire)setToggle('hatireToggle',j.hatire_usb!==false);
  updateSoundLightCard(j);
  let showClientIp=false;
  const box=$('clientIpBox'),val=$('clientIpVal');
  if(box&&val){
    if(cip&&cip!=='0.0.0.0'){showClientIp=true;box.style.display='block';val.textContent=cip;}
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
}
function fillInput(el,v){
  if(!el)return;
  const s=v==null?'':String(v);
  el.value=s;
  el.setAttribute('value',s);
  el.defaultValue=s;
}
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
function nudgeInputPaint(){
  requestAnimationFrame(()=>{
    ['ssid','hostname','otHost','otPort','otSrc0','otSrc1','otSrc2','powerProfile','imuPeriod','wifiTx','rgbBrightness','buzzerVolume'].forEach(id=>{
      const el=$(id);
      if(!el)return;
      el.style.transform='translateZ(1px)';
      void el.offsetHeight;
      el.style.transform='';
    });
  });
}
async function hydrateForm(){
  const r=await fetch('/api/status');const j=await r.json();
  fillInput($('ssid'),j.ssid||'');
  fillInput($('pass'),'');
  fillInput($('hostname'),j.hostname||'');
  fillInput($('otHost'),j.ot_host||'');
  fillInput($('otPort'),j.ot_port||4242);
  const p=j.imu_period_ms||10;
  $('imuPeriod').value=[5,10,20,40].includes(p)?String(p):'10';
  $('wifiTx').value=([0,1,2].includes(j.wifi_tx))?String(j.wifi_tx):'1';
  $('powerProfile').value=([0,1,2].includes(j.power_profile))?String(j.power_profile):'1';
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
  syncRangeLabels();
  applyOtAxesFromStatus(j.ot_axes);
  uiTouched.udp=uiTouched.mdns=uiTouched.hatire=false;
  applyShell(j);
  nudgeInputPaint();
}
async function pollOnly(){
  try{const r=await fetch('/api/status');applyShell(await r.json());}catch(e){}
}
$('udpToggle').onclick=()=>{uiTouched.udp=true;setToggle('udpToggle',!$('udpToggle').classList.contains('on'))};
$('mdnsToggle').onclick=()=>{uiTouched.mdns=true;setToggle('mdnsToggle',!$('mdnsToggle').classList.contains('on'))};
$('hatireToggle').onclick=()=>{uiTouched.hatire=true;setToggle('hatireToggle',!$('hatireToggle').classList.contains('on'))};
[0,1,2].forEach(i=>{const id='otInv'+i;$(id).onclick=()=>setToggle(id,!$(id).classList.contains('on'));});
const _rb=$('rgbBrightness'),_bv=$('buzzerVolume');
if(_rb)_rb.addEventListener('input',syncRangeLabels);
if(_bv)_bv.addEventListener('input',syncRangeLabels);
$('btnUseClientIp').onclick=()=>{
  const v=$('clientIpVal')&&$('clientIpVal').textContent;
  if(v&&v!=='—'){fillInput($('otHost'),v)}
};
$('btnScan').onclick=async()=>{
  setMsg('Scanning… (tracking may hitch briefly)','');
  $('scanList').style.display='none';
  try{
    const r=await fetch('/api/scan');const j=await r.json();
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
};
$('btnSave').onclick=async()=>{
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
    power_profile:parseInt($('powerProfile').value,10),
    imu_period_ms:parseInt($('imuPeriod').value,10)||10,
    wifi_tx:parseInt($('wifiTx').value,10),
    rgb_brightness:parseInt($('rgbBrightness').value,10),
    buzzer_volume:parseInt($('buzzerVolume').value,10)
  };
  if(body.rgb_brightness<0||body.rgb_brightness>100||Number.isNaN(body.rgb_brightness))body.rgb_brightness=25;
  if(body.buzzer_volume<0||body.buzzer_volume>100||Number.isNaN(body.buzzer_volume))body.buzzer_volume=25;
  if(![0,1,2].includes(body.power_profile))body.power_profile=1;
  if(![0,1,2].includes(body.wifi_tx))body.wifi_tx=1;
  const pw=$('pass').value;
  if(pw.length)body.password=pw;
  try{
    const r=await fetch('/api/config',{method:'POST',headers:{'Content-Type':'application/json'},body:JSON.stringify(body)});
    const j=await r.json();
    if(!r.ok){setMsg(j.error||'Save failed','err');return}
    setMsg(j.restarting?'Saved — rebooting…':'Saved.','ok');
    if(!j.restarting)hydrateForm();
  }catch(e){setMsg('Network error','err')}
};
$('btnReboot').onclick=async()=>{
  if(!confirm('Reboot device?'))return;
  await fetch('/api/reboot',{method:'POST'});
  setMsg('Reboot sent…','ok');
};
$('btnFactory').onclick=async()=>{
  if(!confirm('Reset all settings to factory defaults and reboot? You’ll need Azimuth‑Setup or a USB flash to configure Wi‑Fi again.'))return;
  try{
    await fetch('/api/factory_reset',{method:'POST'});
    setMsg('Resetting…','ok');
  }catch(e){setMsg('Request failed','err')}
};
hydrateForm();
setInterval(()=>{if(!document.hidden)pollOnly();},15000);
</script>
</body>
</html>
)AZPORTAL";
