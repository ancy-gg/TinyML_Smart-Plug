const firebaseConfig = {
  apiKey: "AIzaSyAmJlZZszyWPJFgIkTAAl_TbIySys1nvEw",
  authDomain: "tinyml-smart-plug.firebaseapp.com",
  databaseURL: "https://tinyml-smart-plug-default-rtdb.asia-southeast1.firebasedatabase.app",
  projectId: "tinyml-smart-plug",
  storageBucket: "tinyml-smart-plug.firebasestorage.app",
  messagingSenderId: "476671598143",
  appId: "1:476671598143:web:c99b4f1100988784f9628a",
  measurementId: "G-XR8LM91VLN"
};

firebase.initializeApp(firebaseConfig);
const db = firebase.database();

const el = (id) => document.getElementById(id);

const statusBadge = el("statusBadge");
const lastUpdateText = el("lastUpdateText");
const freshnessText = el("freshnessText");
const deviceIpLine = el("deviceIpLine");
const deviceIpText = el("deviceIpText");
const deviceMdnsText = el("deviceMdnsText");

const overviewHealthBadge = el("overviewHealthBadge");
const overviewPrimary = el("overviewPrimary");
const overviewSecondary = el("overviewSecondary");
const ovConnectivity = el("ovConnectivity");
const ovConnectivitySub = el("ovConnectivitySub");
const ovPowerCondition = el("ovPowerCondition");
const ovPowerSub = el("ovPowerSub");
const ovProtection = el("ovProtection");
const ovProtectionSub = el("ovProtectionSub");
const ovFirmware = el("ovFirmware");
const ovFirmwareSub = el("ovFirmwareSub");
const ovLastEvent = el("ovLastEvent");
const ovLastEventSub = el("ovLastEventSub");

const installHelp = el("installHelp");
const installCopy = el("installCopy");
const btnDismissInstall = el("btnDismissInstall");
const btnRefreshNow = el("btnRefreshNow");
const liveStateHints = Array.from(document.querySelectorAll("[data-live-state-hint]"));

const vVal   = el("vVal");
const iVal   = el("iVal");
const pVal   = el("pVal");
const tVal   = el("tVal");
const vHint  = el("vHint");
const iHint  = el("iHint");
const pHint  = el("pHint");
const tHint  = el("tHint");

const cycleNmseVal = el("cycleNmseVal");
const zcvVal       = el("zcvVal");
const zcDwellVal   = el("zcDwellVal");
const pulseCountVal= el("pulseCountVal");
const peakFluctVal = el("peakFluctVal");
const midbandResidVal = el("midbandResidVal");
const hfEnergyVal  = el("hfEnergyVal");
const wpeEntropyVal= el("wpeEntropyVal");
const specEntropyVal = el("specEntropyVal");
const thdVal       = el("thdVal");

const alertEnable = el("alertEnable");
const soundEnable = el("soundEnable");

const historyBody = el("historyBody");
const rangeSelect = el("rangeSelect");
const historyHint = el("historyHint");
const btnDownloadCSV = el("btnDownloadCSV");
const btnClearHistory = el("btnClearHistory");
const toastEl = el("toast");

const DISPLAY_TZ = "Asia/Manila";
const STALE_MS = 15000;
const HISTORY_LIMIT = 5000;
const MAX_RENDER_ROWS = 300;

const UI_MAINS_ABSENT_V = 20;
const UI_UNDERVOLTAGE_V = 200;
const UI_OVERVOLTAGE_V = 250;
const UI_NORMAL_V_MIN = 200;
const UI_NORMAL_V_MAX = 250;
const UI_ACTIVE_CURRENT_A = 0.08;
const UI_ACTIVE_POWER_VA = 15;
const UI_TEMP_COLD_ABNORMAL_C = 10;
const UI_TEMP_WARM_C = 55;
const UI_TEMP_HOT_C = 70;
const OVERLOAD_WARN_A = 10;
const SHORT_CIRCUIT_TRIP_A = 20;

const OTA_REPO_RAW_BASE = "https://raw.githubusercontent.com/ancy-gg/TinyML_Smart-Plug/main/firmware/";
const OTA_DEFAULT_BIN   = "firmware.bin";

let lastSeenMs = 0;
let lastReceiptMs = 0;
let historyCache = [];
let currentFilteredHistory = [];
let localHistoryEvents = [];
let syntheticDisconnectActive = false;
let latestHistoryRecord = null;
let lastSyntheticHistoryKey = "";
let lastEffectiveHistoryStatus = "";
let lastLiveData = null;
let lastAlertStatus = null;
let lastNotifiedAt = 0;
let previousPowerCondition = "UNKNOWN";
let previousFresh = false;

const LS_ALERT = "tsp_alert_enabled";
const LS_SOUND = "tsp_sound_enabled";
const LS_INSTALL_DISMISS = "tsp_install_help_dismissed";

const isIOS = /iphone|ipad|ipod/i.test(navigator.userAgent || "");
const isStandalone = window.matchMedia?.("(display-mode: standalone)")?.matches || window.navigator.standalone === true;
if (isIOS) document.body.classList.add("ios");
if (isStandalone) document.body.classList.add("standalone");

function buildRepoFirmwareUrl(binName) {
  const name = (binName || OTA_DEFAULT_BIN).trim();
  return OTA_REPO_RAW_BASE + encodeURIComponent(name);
}
function isLikelyVersion(v) {
  return typeof v === "string" && v.trim().length >= 3;
}
function isHttpsUrl(u) {
  try {
    const url = new URL(u);
    return url.protocol === "https:";
  } catch {
    return false;
  }
}

function toast(msg, kind = "ok") {
  if (!toastEl) return;
  toastEl.textContent = msg;
  toastEl.className = `toast toast-${kind} show`;
  clearTimeout(toastEl._t);
  toastEl._t = setTimeout(() => {
    toastEl.className = `toast toast-${kind}`;
  }, 2600);
}

function toFixedOrDash(x, digits = 2) {
  if (x === null || x === undefined || x === "" || Number.isNaN(Number(x))) return "—";
  return Number(x).toFixed(digits);
}

function formatEpochMsTZ(ms, tz = DISPLAY_TZ) {
  if (!ms || ms <= 0) return "—";
  const d = new Date(ms);
  const parts = new Intl.DateTimeFormat("en-CA", {
    timeZone: tz,
    year: "numeric",
    month: "2-digit",
    day: "2-digit",
    hour: "2-digit",
    minute: "2-digit",
    second: "2-digit",
    hour12: false
  }).formatToParts(d);

  const get = (type) => parts.find(p => p.type === type)?.value ?? "00";
  const yyyy = get("year");
  const MM   = get("month");
  const dd   = get("day");
  const HH   = get("hour");
  const mm   = get("minute");
  const ss   = get("second");
  const mmm  = String(d.getMilliseconds()).padStart(3, "0");

  return `${yyyy}-${MM}-${dd} ${HH}:${mm}:${ss}.${mmm}`;
}

function parseTsIsoToEpoch(tsIso) {
  if (!tsIso || typeof tsIso !== "string") return 0;
  const m = tsIso.match(/^(\d{4})-(\d{2})-(\d{2})[ T](\d{2}):(\d{2}):(\d{2})(?:\.(\d{1,3}))?/);
  if (!m) return 0;
  const [_, Y, Mo, D, H, Mi, S, Ms] = m;
  const ms = Number((Ms || "0").padEnd(3, "0"));
  return new Date(Number(Y), Number(Mo) - 1, Number(D), Number(H), Number(Mi), Number(S), ms).getTime();
}

function getRecordEpochMs(r) {
  if (!r || typeof r !== "object") return 0;
  if (typeof r.server_ts === "number" && r.server_ts > 0) return r.server_ts;
  if (typeof r.ts_epoch_ms === "number" && r.ts_epoch_ms > 0) return r.ts_epoch_ms;
  if (typeof r.ts_iso === "string" && r.ts_iso.trim()) return parseTsIsoToEpoch(r.ts_iso.trim());
  if (typeof r.epoch_ms === "number" && r.epoch_ms > 0) return r.epoch_ms;
  return 0;
}

function loadBool(k, def=false){
  const v = localStorage.getItem(k);
  if (v === null) return def;
  return v === "1";
}
function saveBool(k, v){ localStorage.setItem(k, v ? "1" : "0"); }

function initCollapsibles() {
  document.querySelectorAll(".collapse-toggle").forEach((btn) => {
    const targetId = btn.getAttribute("data-collapse-target");
    const body = targetId ? document.getElementById(targetId) : null;
    if (!body) return;

    const setState = (collapsed) => {
      body.classList.toggle("is-collapsed", collapsed);
      body.hidden = collapsed;
      btn.dataset.collapsed = collapsed ? "true" : "false";
      btn.textContent = collapsed ? "Show" : "Hide";
      btn.setAttribute("aria-expanded", collapsed ? "false" : "true");
      body.setAttribute("aria-hidden", collapsed ? "true" : "false");
    };

    setState(body.classList.contains("is-collapsed") || body.hidden);

    btn.addEventListener("click", () => {
      const collapsed = btn.dataset.collapsed !== "true";
      setState(collapsed);
    });
  });
}

if (alertEnable) alertEnable.checked = loadBool(LS_ALERT, true);
if (soundEnable) soundEnable.checked = loadBool(LS_SOUND, false);
alertEnable?.addEventListener("change", () => saveBool(LS_ALERT, alertEnable.checked));
soundEnable?.addEventListener("change", () => saveBool(LS_SOUND, soundEnable.checked));

function playBeep(){
  try{
    const ctx = new (window.AudioContext || window.webkitAudioContext)();
    const o = ctx.createOscillator();
    const g = ctx.createGain();
    o.type = "sine";
    o.frequency.value = 880;
    g.gain.value = 0.08;
    o.connect(g);
    g.connect(ctx.destination);
    o.start();
    setTimeout(() => { o.stop(); ctx.close(); }, 180);
  }catch(e){}
}

async function showFaultNotification(title, body){
  if (!(alertEnable?.checked ?? true)) return;
  if (!("Notification" in window)) return;
  if (Notification.permission === "default") {
    try { await Notification.requestPermission(); } catch {}
  }
  if (Notification.permission !== "granted") return;

  if ("serviceWorker" in navigator) {
    const reg = await navigator.serviceWorker.ready.catch(()=>null);
    if (reg) {
      reg.showNotification(title, {
        body,
        icon: "icons/icon-192.png",
        badge: "icons/icon-192.png",
        tag: "tsp-fault",
        renotify: true
      });
      return;
    }
  }
  new Notification(title, { body, icon: "icons/icon-192.png" });
}

function classifyStatus(s) {
  const u = (s || "").toUpperCase().trim();
  if (!u) return "NORMAL";
  if (u === "DEVICE DISCONNECTED") return "DEVICE_DISCONNECTED";
  if (u === "DEVICE UNPLUGGED" || u === "UNPLUGGED") return "UNPLUGGED";
  if (u === "DEVICE ON") return "DEVICE_ON";
  if (u === "DEVICE PLUGGED IN") return "DEVICE_PLUGGED_IN";
  if (u === "DEVICE ONLINE") return "DEVICE_ONLINE";
  if (u === "FIRMWARE UPDATED") return "FIRMWARE_UPDATED";
  if (u.includes("DISCON")) return "DEVICE_DISCONNECTED";
  if (u.includes("UNPLUG")) return "UNPLUGGED";
  if (u.includes("SHORT")) return "SHORT_CIRCUIT";
  if (u.includes("ARC")) return "ARCING";
  if (u.includes("HEAT")) return "HEATING";
  if (u.includes("OVERVOLT")) return "OVERVOLTAGE";
  if (u.includes("UNDERVOLT")) return "UNDERVOLTAGE";
  if (u === "OVERLOAD" || u.includes("OVERLOAD")) return "OVERLOAD";
  if (u.includes("SAFE")) return "SAFE_MODE";
  if (u.includes("OTA")) return "OTA_UPDATING";
  if (u.includes("CONFIG")) return "CONFIG_PORTAL";
  if (u.includes("CONNECTING")) return "WIFI_CONNECTING";
  if (u.includes("STARTUP")) return "STARTUP_STABILIZING";
  if (u.includes("WARN")) return "WARNING";
  if (u.includes("NORM")) return "NORMAL";
  return u.replace(/\s+/g, "_");
}

function prettyStatus(kind) {
  const k = classifyStatus(kind);
  return k.replaceAll("_", " ");
}

function statusBadgeHTML(kind) {
  switch (kind) {
    case "OVERLOAD": return `OVERLOAD <span class="emoji blink">⬆️</span>`;
    case "SHORT_CIRCUIT": return `SHORT CIRCUIT`;
    case "HEATING":  return `SOCKET HEATING <span class="emoji flicker">🔥</span>`;
    case "ARCING":   return `ARC FAULT <span class="emoji zap">⚡</span>`;
    case "OVERVOLTAGE": return `OVERVOLTAGE`;
    case "UNDERVOLTAGE": return `UNDERVOLTAGE`;
    case "UNPLUGGED": return `UNPLUGGED`;
    case "DEVICE_DISCONNECTED": return `DISCONNECTED`;
    case "DEVICE_ON": return `DEVICE ON`;
    case "DEVICE_ONLINE": return `ONLINE`;
    case "DEVICE_PLUGGED_IN": return `PLUGGED IN`;
    case "FIRMWARE_UPDATED": return `FIRMWARE UPDATED`;
    case "SAFE_MODE": return `SAFE MODE`;
    case "CONFIG_PORTAL": return `WIFI SETUP`;
    case "WIFI_CONNECTING": return `WIFI CONNECTING`;
    case "STARTUP_STABILIZING": return `STARTUP STABILIZING`;
    case "OTA_UPDATING": return `OTA UPDATING`;
    case "NORMAL": return `MONITORING`;
    default: return prettyStatus(kind);
  }
}

function setTopStatus(kind) {
  if (!statusBadge) return;
  const k = classifyStatus(kind);
  statusBadge.className = "status";

  if (k === "DEVICE_DISCONNECTED") statusBadge.classList.add("status-DISCONNECTED");
  else if (k === "UNPLUGGED") statusBadge.classList.add("status-UNPLUGGED");
  else if (k === "OVERLOAD") statusBadge.classList.add("status-OVERLOAD");
  else if (k === "SHORT_CIRCUIT") statusBadge.classList.add("status-SHORT");
  else if (k === "HEATING") statusBadge.classList.add("status-HEATING");
  else if (k === "ARCING") statusBadge.classList.add("status-ARCING");
  else if (k === "OVERVOLTAGE") statusBadge.classList.add("status-OVERVOLTAGE");
  else if (k === "UNDERVOLTAGE") statusBadge.classList.add("status-UNDERVOLTAGE");
  else if (k === "FIRMWARE_UPDATED") statusBadge.classList.add("status-FW");
  else if (k === "SAFE_MODE" || k === "CONFIG_PORTAL" || k === "WIFI_CONNECTING" || k === "STARTUP_STABILIZING" || k === "OTA_UPDATING") statusBadge.classList.add("status-WARN");
  else statusBadge.classList.add("status-OK");

  statusBadge.innerHTML = statusBadgeHTML(k);
  if (k !== "DEVICE_DISCONNECTED") {
    statusBadge.classList.remove("bump");
    void statusBadge.offsetWidth;
    statusBadge.classList.add("bump");
  }
}

function pillHTML(kind) {
  const k = classifyStatus(kind);
  if (k === "OVERLOAD") return `<span class="pill pill-OVERLOAD">OVERLOAD <span class="emoji blink">⬆️</span></span>`;
  if (k === "SHORT_CIRCUIT") return `<span class="pill pill-SHORT">SHORT CIRCUIT</span>`;
  if (k === "HEATING")  return `<span class="pill pill-HEATING">SOCKET HEATING <span class="emoji flicker">🔥</span></span>`;
  if (k === "ARCING")   return `<span class="pill pill-ARCING">ARC FAULT <span class="emoji zap">⚡</span></span>`;
  if (k === "OVERVOLTAGE") return `<span class="pill pill-OVERVOLTAGE">OVERVOLTAGE</span>`;
  if (k === "UNDERVOLTAGE") return `<span class="pill pill-UNDERVOLTAGE">UNDERVOLTAGE</span>`;
  if (k === "UNPLUGGED") return `<span class="pill pill-UNPLUG">DEVICE UNPLUGGED</span>`;
  if (k === "DEVICE_DISCONNECTED") return `<span class="pill pill-DIS">DEVICE DISCONNECTED</span>`;
  if (k === "DEVICE_ON") return `<span class="pill pill-OK">DEVICE ON</span>`;
  if (k === "DEVICE_PLUGGED_IN") return `<span class="pill pill-ONLINE">DEVICE PLUGGED IN</span>`;
  if (k === "DEVICE_ONLINE") return `<span class="pill pill-ONLINE">DEVICE ONLINE</span>`;
  if (k === "FIRMWARE_UPDATED") return `<span class="pill pill-FW">FIRMWARE UPDATED</span>`;
  if (k === "NORMAL") return `<span class="pill pill-OK">MONITORING</span>`;
  return `<span class="pill pill-OK">${prettyStatus(k)}</span>`;
}

function animateNumber(node) {
  if (!node) return;
  node.classList.remove("tick");
  void node.offsetWidth;
  node.classList.add("tick");
}

function setLiveZeroes() {
  const zeroMap = [
    [vVal, "0.0"], [iVal, "0.000"], [pVal, "0.0"], [tVal, "0.0"],
    [cycleNmseVal, "0.000"], [zcvVal, "0.000"], [zcDwellVal, "0.000"],
    [pulseCountVal, "0.000"], [peakFluctVal, "0.000"], [midbandResidVal, "0.000"],
    [hfEnergyVal, "0.000"], [wpeEntropyVal, "0.000"], [specEntropyVal, "0.000"], [thdVal, "0.000"]
  ];
  zeroMap.forEach(([node, text]) => { if (node) node.textContent = text; });
}

function injectSyntheticDisconnect() {
  syntheticDisconnectActive = true;
}

function liveIsFresh() {
  return !!lastSeenMs && (Date.now() - lastSeenMs) <= STALE_MS;
}

function latestLiveEpoch() {
  return getRecordEpochMs(lastLiveData || {});
}

function derivePowerConditionFromLive(data) {
  const v = Number(data?.voltage ?? 0);
  const raw = classifyStatus(data?.power_condition || data?.status || "NORMAL");
  if (raw === "UNPLUGGED" || v < UI_MAINS_ABSENT_V) return "UNPLUGGED";
  if (raw === "OVERVOLTAGE" || v > UI_OVERVOLTAGE_V) return "OVERVOLTAGE";
  if (raw === "UNDERVOLTAGE" || (v >= UI_MAINS_ABSENT_V && v < UI_UNDERVOLTAGE_V)) return "UNDERVOLTAGE";
  if (v >= UI_NORMAL_V_MIN && v <= UI_NORMAL_V_MAX) return "NORMAL";
  return "UNKNOWN";
}

function isLiveLoadActive(data) {
  const i = Number(data?.current ?? 0);
  const p = Number(data?.apparent_power ?? 0);
  return i >= UI_ACTIVE_CURRENT_A || p >= UI_ACTIVE_POWER_VA;
}

function deriveLiveStatus(data) {
  const raw = classifyStatus(data?.status || "NORMAL");
  const i = Number(data?.current ?? 0);
  if (["DEVICE_DISCONNECTED", "SHORT_CIRCUIT", "OVERLOAD", "HEATING", "ARCING", "OVERVOLTAGE", "UNDERVOLTAGE", "UNPLUGGED", "SAFE_MODE", "CONFIG_PORTAL", "WIFI_CONNECTING", "STARTUP_STABILIZING", "OTA_UPDATING", "FIRMWARE_UPDATED"].includes(raw)) return raw;
  if (i > SHORT_CIRCUIT_TRIP_A) return "SHORT_CIRCUIT";
  if (i >= OVERLOAD_WARN_A) return "OVERLOAD";
  const power = derivePowerConditionFromLive(data);
  if (power === "OVERVOLTAGE") return "OVERVOLTAGE";
  if (power === "UNDERVOLTAGE") return "UNDERVOLTAGE";
  if (power === "UNPLUGGED") return "UNPLUGGED";
  return "NORMAL";
}

function setLiveUnavailable() {
  const unavailableMap = [
    vVal, iVal, pVal, tVal,
    cycleNmseVal, zcvVal, zcDwellVal, pulseCountVal, peakFluctVal, midbandResidVal,
    hfEnergyVal, wpeEntropyVal, specEntropyVal, thdVal
  ];
  unavailableMap.forEach((node) => { if (node) node.textContent = "—"; });
}

function effectiveStatusKind() {
  if (liveIsFresh() && lastLiveData) return deriveLiveStatus(lastLiveData);
  return "DEVICE_DISCONNECTED";
}

function effectiveTimestamp() {
  const liveEpoch = latestLiveEpoch();
  if (liveEpoch > 0) return liveEpoch;
  return getRecordEpochMs(latestHistoryRecord || {});
}

function effectivePowerCondition() {
  if (liveIsFresh() && lastLiveData) return derivePowerConditionFromLive(lastLiveData);
  return "DEVICE_DISCONNECTED";
}

function updateFreshnessText() {
  if (!freshnessText) return;
  if (!lastSeenMs) {
    freshnessText.textContent = "Waiting for device…";
    return;
  }
  const ageSec = Math.max(0, Math.round((Date.now() - lastSeenMs) / 1000));
  freshnessText.textContent = liveIsFresh() ? `Live • ${ageSec}s ago` : `Disconnected • ${ageSec}s ago`;
}

function applyMetricHints(data) {
  if (!liveIsFresh()) {
    if (vHint) vHint.textContent = "Disconnected. No fresh voltage data.";
    if (iHint) iHint.textContent = "Disconnected. No fresh current data.";
    if (pHint) pHint.textContent = "Disconnected. No fresh power data.";
    if (tHint) tHint.textContent = "Disconnected. No fresh temperature data.";
    return;
  }

  const i = Number(data?.current ?? 0);
  const p = Number(data?.apparent_power ?? 0);
  const t = Number(data?.temp ?? 0);
  const powerKind = effectivePowerCondition();

  if (vHint) {
    if (powerKind === "UNPLUGGED") vHint.textContent = "Outlet input missing.";
    else if (powerKind === "UNDERVOLTAGE") vHint.textContent = "Below 200 V.";
    else if (powerKind === "OVERVOLTAGE") vHint.textContent = "Above 250 V.";
    else if (powerKind === "NORMAL") vHint.textContent = "Within 200–250 V.";
    else vHint.textContent = "Checking voltage.";
  }

  if (iHint) {
    if (powerKind === "UNPLUGGED") iHint.textContent = "No load while unplugged.";
    else if (i > SHORT_CIRCUIT_TRIP_A) iHint.textContent = "Short-circuit trip zone.";
    else if (i >= OVERLOAD_WARN_A) iHint.textContent = "Overload alarm zone.";
    else if (isLiveLoadActive(data)) iHint.textContent = "Load is active.";
    else iHint.textContent = "Idle load.";
  }

  if (pHint) {
    if (powerKind === "UNPLUGGED") pHint.textContent = "No apparent power.";
    else if (p >= 1500) pHint.textContent = "Heavy apparent demand.";
    else if (p >= 300) pHint.textContent = "Active appliance demand.";
    else if (p >= UI_ACTIVE_POWER_VA) pHint.textContent = "Light demand.";
    else pHint.textContent = "Near zero demand.";
  }

  if (tHint) {
    if (t >= UI_TEMP_HOT_C) tHint.textContent = "Trip zone: socket heating.";
    else if (t >= UI_TEMP_WARM_C) tHint.textContent = "Socket warming.";
    else if (t < UI_TEMP_COLD_ABNORMAL_C) tHint.textContent = "Cold environment.";
    else tHint.textContent = "Thermal state normal.";
  }
}

function setMiniBadge(kind, text) {
  if (!overviewHealthBadge) return;
  overviewHealthBadge.className = "mini-badge";
  if (kind === "fault") overviewHealthBadge.classList.add("mini-badge-FAULT");
  else if (kind === "warn") overviewHealthBadge.classList.add("mini-badge-WARN");
  else overviewHealthBadge.classList.add("mini-badge-OK");
  overviewHealthBadge.textContent = text;
}

function renderOverview() {
  const fresh = liveIsFresh();
  const status = effectiveStatusKind();
  const live = lastLiveData || {};
  const powerCondition = effectivePowerCondition();
  const ip = (live.ip || "").toString().trim();
  const mdns = (live.mdns || "tinyml-smart-plug.local").toString();
  const lastEventStatus = latestHistoryRecord ? prettyStatus(latestHistoryRecord.status || "") : "—";
  const lastEventTime = latestHistoryRecord ? formatEpochMsTZ(getRecordEpochMs(latestHistoryRecord)) : "—";
  const loadActive = fresh && isLiveLoadActive(live);
  const ageSec = lastSeenMs ? Math.max(0, Math.round((Date.now() - lastSeenMs) / 1000)) : null;

  if (overviewPrimary) {
    if (!fresh) overviewPrimary.textContent = "Device disconnected";
    else if (status === "UNPLUGGED") overviewPrimary.textContent = "Device unplugged";
    else if (status === "NORMAL") overviewPrimary.textContent = loadActive ? "System healthy" : "System idle";
    else if (status === "OVERLOAD") overviewPrimary.textContent = "Overload warning";
    else if (status === "SHORT_CIRCUIT") overviewPrimary.textContent = "Short circuit detected";
    else if (status === "HEATING") overviewPrimary.textContent = "Socket heating detected";
    else if (status === "ARCING") overviewPrimary.textContent = "Arc fault detected";
    else if (status === "UNDERVOLTAGE") overviewPrimary.textContent = "Undervoltage detected";
    else if (status === "OVERVOLTAGE") overviewPrimary.textContent = "Overvoltage detected";
    else if (status === "STARTUP_STABILIZING") overviewPrimary.textContent = "Startup stabilizing";
    else if (status === "WIFI_CONNECTING") overviewPrimary.textContent = "Connecting to Wi-Fi";
    else if (status === "CONFIG_PORTAL") overviewPrimary.textContent = "Wi-Fi setup mode";
    else if (status === "OTA_UPDATING") overviewPrimary.textContent = "OTA update in progress";
    else overviewPrimary.textContent = prettyStatus(status);
  }

  if (overviewSecondary) {
    if (!fresh) overviewSecondary.textContent = "No fresh live packet has reached the dashboard for 15 seconds, so the current state is treated as disconnected.";
    else if (status === "UNPLUGGED") overviewSecondary.textContent = "The smart plug is reachable, but the input line is absent or near zero.";
    else if (status === "NORMAL" && !loadActive) overviewSecondary.textContent = "Input is healthy and the connected appliance appears idle.";
    else if (status === "NORMAL" && loadActive) overviewSecondary.textContent = "Input is healthy and the connected appliance is drawing load.";
    else if (status === "OVERLOAD") overviewSecondary.textContent = "Measured current has reached the overload alarm threshold of 10 A.";
    else if (status === "SHORT_CIRCUIT") overviewSecondary.textContent = "Measured current has exceeded the short-circuit threshold of 20 A.";
    else if (status === "HEATING") overviewSecondary.textContent = "Socket temperature has reached the heating trip threshold of 70 °C.";
    else if (status === "ARCING") overviewSecondary.textContent = "The AI model is detecting a waveform pattern consistent with arcing.";
    else if (status === "UNDERVOLTAGE") overviewSecondary.textContent = "Input voltage is below the 200 V protection threshold.";
    else if (status === "OVERVOLTAGE") overviewSecondary.textContent = "Input voltage is above the 250 V protection threshold.";
    else overviewSecondary.textContent = "The dashboard prioritizes fresh live data so stale packets are not mistaken for a current condition.";
  }

  if (ovConnectivity) ovConnectivity.textContent = fresh ? (classifyStatus(live.status || "") === "CONFIG_PORTAL" ? "Setup Mode" : "Online") : "Disconnected";
  if (ovConnectivitySub) {
    if (!lastSeenMs) ovConnectivitySub.textContent = "Waiting for the first live packet.";
    else if (fresh) {
      const pieces = [`Fresh packet ${ageSec}s ago`];
      if (ip) pieces.push(ip);
      if (mdns) pieces.push(mdns);
      ovConnectivitySub.textContent = pieces.join(" • ");
    } else {
      ovConnectivitySub.textContent = `No fresh live packet for ${ageSec}s.`;
    }
  }

  if (ovPowerCondition) ovPowerCondition.textContent = powerCondition === "DEVICE_DISCONNECTED" ? "Unknown" : prettyStatus(powerCondition === "UNKNOWN" ? status : powerCondition);
  if (ovPowerSub) {
    if (powerCondition === "UNPLUGGED") ovPowerSub.textContent = "Input line is missing or below the presence threshold.";
    else if (powerCondition === "UNDERVOLTAGE") ovPowerSub.textContent = "Below the 200 V trip threshold.";
    else if (powerCondition === "OVERVOLTAGE") ovPowerSub.textContent = "Above the 250 V trip threshold.";
    else if (powerCondition === "DEVICE_DISCONNECTED") ovPowerSub.textContent = "Waiting for fresh live data.";
    else if (powerCondition === "NORMAL") ovPowerSub.textContent = "Within the expected 200–250 V window.";
    else ovPowerSub.textContent = "Checking line condition.";
  }

  if (ovProtection) ovProtection.textContent = status === "NORMAL" ? "Monitoring" : prettyStatus(status);
  if (ovProtectionSub) {
    if (status === "ARCING") ovProtectionSub.textContent = "AI model-positive arc event based on waveform features.";
    else if (status === "HEATING") ovProtectionSub.textContent = "Socket temperature is at or above 70 °C.";
    else if (status === "SHORT_CIRCUIT") ovProtectionSub.textContent = "Measured current is above 20 A.";
    else if (status === "OVERLOAD") ovProtectionSub.textContent = "Measured current is at or above 10 A.";
    else if (status === "UNDERVOLTAGE") ovProtectionSub.textContent = "Voltage is below 200 V.";
    else if (status === "OVERVOLTAGE") ovProtectionSub.textContent = "Voltage is above 250 V.";
    else if (status === "UNPLUGGED") ovProtectionSub.textContent = "No mains input detected.";
    else if (status === "DEVICE_DISCONNECTED") ovProtectionSub.textContent = "No fresh live data from the device.";
    else ovProtectionSub.textContent = "No active protection trip from the latest live data.";
  }

  if (ovLastEvent) ovLastEvent.textContent = lastEventStatus;
  if (ovLastEventSub) ovLastEventSub.textContent = lastEventStatus === "—" ? "No history event yet." : `${lastEventTime}`;

  if (["HEATING", "ARCING", "SHORT_CIRCUIT"].includes(status)) setMiniBadge("fault", "Critical");
  else if (["OVERLOAD", "UNDERVOLTAGE", "OVERVOLTAGE", "UNPLUGGED", "WIFI_CONNECTING", "STARTUP_STABILIZING", "CONFIG_PORTAL", "OTA_UPDATING", "DEVICE_DISCONNECTED"].includes(status)) setMiniBadge("warn", fresh ? "Attention" : "Offline");
  else if (fresh) setMiniBadge("ok", loadActive ? "Active" : "Idle");
  else setMiniBadge("warn", "Offline");

  liveStateHints.forEach((node) => { node.textContent = fresh ? "LIVE" : "DISCONNECTED"; });
  if (deviceMdnsText) deviceMdnsText.textContent = mdns || "tinyml-smart-plug.local";
}

function renderTopState() {
  const status = effectiveStatusKind();
  setTopStatus(status);
  const ts = effectiveTimestamp();
  if (lastUpdateText) lastUpdateText.textContent = ts ? formatEpochMsTZ(ts) : "—";
  updateFreshnessText();
  renderOverview();
}

function transitionNotice(newLive) {
  const nowFresh = liveIsFresh();
  const powerCondition = effectivePowerCondition();
  const statusStr = deriveLiveStatus(newLive || {});
  const statusForHistory = statusStr === "NORMAL" ? "DEVICE_ON" : statusStr;
  const cameOnline = nowFresh && !previousFresh;
  const wentOffline = !nowFresh && previousFresh;
  const powerRestored = previousPowerCondition === "UNPLUGGED" && powerCondition !== "UNPLUGGED" && nowFresh;

  if (cameOnline) toast("Device reconnected.", "info");
  if (wentOffline) toast("Device disconnected after 15s without fresh data.", "warn");
  if (powerRestored) toast("Power restored. Device plugged back in detected.", "ok");

  if (cameOnline) recordEffectiveHistoryTransition(statusForHistory, newLive || {}, latestLiveEpoch() || Date.now());
  else if (nowFresh) recordEffectiveHistoryTransition(statusForHistory, newLive || {}, latestLiveEpoch() || Date.now());

  previousFresh = nowFresh;
  previousPowerCondition = powerCondition;

  const notifySet = new Set(["ARCING", "HEATING", "SHORT_CIRCUIT", "OVERLOAD", "UNDERVOLTAGE", "OVERVOLTAGE", "UNPLUGGED"]);
  const isInteresting = notifySet.has(statusStr);
  const wasInteresting = notifySet.has(classifyStatus(lastAlertStatus || ""));
  const shouldNotify = isInteresting && (!wasInteresting || statusStr !== classifyStatus(lastAlertStatus || ""));

  if (shouldNotify && (Date.now() - lastNotifiedAt) > 4000) {
    const body = `Status: ${prettyStatus(statusStr)}
V=${toFixedOrDash(newLive?.voltage,1)}V  I=${toFixedOrDash(newLive?.current,2)}A  T=${toFixedOrDash(newLive?.temp,1)}°C`;
    showFaultNotification("TinyML Smart Plug Alert", body);
    if ((soundEnable?.checked ?? false)) playBeep();
    lastNotifiedAt = Date.now();
  }
  lastAlertStatus = statusStr;
}

function getRangeBounds(rangeKey) {
  const now = Date.now();
  const todayStr = new Intl.DateTimeFormat("en-CA", {
    timeZone: DISPLAY_TZ,
    year: "numeric",
    month: "2-digit",
    day: "2-digit"
  }).format(new Date(now));
  const todayMidnight = new Date(`${todayStr}T00:00:00`).getTime();
  if (rangeKey === "today") return { start: todayMidnight, end: todayMidnight + 86400000 };
  if (rangeKey === "yesterday") return { start: todayMidnight - 86400000, end: todayMidnight };
  if (rangeKey === "30d") return { start: now - 30 * 86400000, end: now + 1 };
  return { start: now - 7 * 86400000, end: now + 1 };
}

function rangeLabel(key) {
  if (key === "today") return "Today";
  if (key === "yesterday") return "Yesterday";
  if (key === "30d") return "Last 30 days";
  return "Last 7 days";
}

function makeHistoryEvent(kind, payload = {}, epoch = Date.now()) {
  const src = payload || {};
  return {
    status: classifyStatus(kind),
    voltage: Number(src.voltage ?? lastLiveData?.voltage ?? 0),
    current: Number(src.current ?? lastLiveData?.current ?? 0),
    temp: Number(src.temp ?? lastLiveData?.temp ?? 0),
    apparent_power: Number(src.apparent_power ?? lastLiveData?.apparent_power ?? 0),
    cycle_nmse: src.cycle_nmse ?? lastLiveData?.cycle_nmse ?? null,
    zcv: src.zcv ?? lastLiveData?.zcv ?? null,
    zc_dwell_ratio: src.zc_dwell_ratio ?? lastLiveData?.zc_dwell_ratio ?? null,
    pulse_count_per_cycle: src.pulse_count_per_cycle ?? lastLiveData?.pulse_count_per_cycle ?? null,
    peak_fluct_cv: src.peak_fluct_cv ?? lastLiveData?.peak_fluct_cv ?? null,
    midband_residual_rms: src.midband_residual_rms ?? lastLiveData?.midband_residual_rms ?? null,
    hf_band_energy_ratio: src.hf_band_energy_ratio ?? lastLiveData?.hf_band_energy_ratio ?? null,
    wpe_entropy: src.wpe_entropy ?? lastLiveData?.wpe_entropy ?? null,
    spec_entropy: src.spec_entropy ?? lastLiveData?.spec_entropy ?? null,
    thd_i: src.thd_i ?? lastLiveData?.thd_i ?? null,
    server_ts: epoch,
    ts_epoch_ms: epoch,
    _local: true
  };
}

function pushLocalHistoryEvent(kind, payload = {}, epoch = Date.now()) {
  const status = classifyStatus(kind);
  const bucket = Math.floor(epoch / 1000);
  const key = `${status}|${bucket}`;
  if (lastSyntheticHistoryKey === key) return;
  lastSyntheticHistoryKey = key;
  localHistoryEvents.push(makeHistoryEvent(status, payload, epoch));
  if (localHistoryEvents.length > 120) localHistoryEvents = localHistoryEvents.slice(-120);
}

function getMergedHistory() {
  return [...historyCache, ...localHistoryEvents];
}

function refreshLatestHistoryRecord() {
  const merged = getMergedHistory();
  latestHistoryRecord = merged.reduce((best, rec) => {
    if (!best) return rec;
    return getRecordEpochMs(rec) > getRecordEpochMs(best) ? rec : best;
  }, null);
}

function recordEffectiveHistoryTransition(kind, payload = {}, epoch = Date.now()) {
  const normalized = classifyStatus(kind);
  if (!normalized || normalized === lastEffectiveHistoryStatus) return;
  const loggable = new Set(["DEVICE_ON","DEVICE_ONLINE","DEVICE_DISCONNECTED","UNPLUGGED","OVERLOAD","SHORT_CIRCUIT","HEATING","ARCING","UNDERVOLTAGE","OVERVOLTAGE","CONFIG_PORTAL","WIFI_CONNECTING","STARTUP_STABILIZING","OTA_UPDATING","FIRMWARE_UPDATED"]);
  if (!loggable.has(normalized)) return;

  const latestStatus = classifyStatus(latestHistoryRecord?.status || "");
  const latestEpoch = getRecordEpochMs(latestHistoryRecord || {});
  if (latestStatus === normalized && latestEpoch > 0 && Math.abs(epoch - latestEpoch) < 20000) {
    lastEffectiveHistoryStatus = normalized;
    return;
  }

  lastEffectiveHistoryStatus = normalized;
  pushLocalHistoryEvent(normalized, payload, epoch);
  refreshLatestHistoryRecord();
  applyHistoryFilter();
}

function applyHistoryFilter() {
  const key = rangeSelect?.value || "7d";
  const { start, end } = getRangeBounds(key);

  const filtered = getMergedHistory().filter(r => {
    const epoch = getRecordEpochMs(r);
    return epoch && epoch >= start && epoch < end;
  });

  currentFilteredHistory = filtered.slice();
  if (historyHint) historyHint.textContent = `Showing: ${rangeLabel(key)} (${filtered.length})`;

  if (!filtered.length) {
    historyBody.innerHTML = `<tr><td colspan="8" class="muted">No history in this range.</td></tr>`;
    return;
  }

  filtered.sort((a, b) => getRecordEpochMs(b) - getRecordEpochMs(a));
  const rows = filtered.slice(0, MAX_RENDER_ROWS).map((r, idx) => {
    const epoch = getRecordEpochMs(r);
    const timeStr = formatEpochMsTZ(epoch);
    return `
      <tr class="row-in" style="animation-delay:${Math.min(idx, 10) * 25}ms">
        <td class="mono">${timeStr}</td>
        <td>${pillHTML(r.status)}</td>
        <td class="mono">${toFixedOrDash(r.voltage, 1)}</td>
        <td class="mono">${toFixedOrDash(r.current, 3)}</td>
        <td class="mono">${toFixedOrDash(r.temp, 1)}</td>
        <td class="mono">${toFixedOrDash(r.cycle_nmse, 3)}</td>
        <td class="mono">${toFixedOrDash(r.zcv, 3)}</td>
        <td class="mono">${toFixedOrDash(r.thd_i, 3)}</td>
      </tr>
    `;
  }).join("");
  historyBody.innerHTML = rows;
}
rangeSelect?.addEventListener("change", applyHistoryFilter);

function csvEscape(v) {
  const s = String(v ?? "");
  return /[",\n]/.test(s) ? `"${s.replace(/"/g, '""')}"` : s;
}

function downloadTextFile(filename, text) {
  const blob = new Blob([text], { type: "text/csv;charset=utf-8" });
  const url = URL.createObjectURL(blob);
  const a = document.createElement("a");
  a.href = url;
  a.download = filename;
  document.body.appendChild(a);
  a.click();
  a.remove();
  URL.revokeObjectURL(url);
}

btnDownloadCSV?.addEventListener("click", () => {
  if (!currentFilteredHistory.length) {
    toast("No data to export in this range.", "err");
    return;
  }

  const rows = currentFilteredHistory
    .slice()
    .sort((a, b) => getRecordEpochMs(a) - getRecordEpochMs(b));

  const header = ["timestamp","epoch_ms","status","voltage","current","temp","cycle_nmse","zcv","zc_dwell_ratio","pulse_count_per_cycle","peak_fluct_cv","midband_residual_rms","hf_band_energy_ratio","wpe_entropy","spec_entropy","thd_i"];
  const lines = [header.join(",")];

  for (const r of rows) {
    const epoch = getRecordEpochMs(r);
    const ts = formatEpochMsTZ(epoch);
    lines.push([
      csvEscape(ts),
      csvEscape(epoch),
      csvEscape(r.status ?? ""),
      csvEscape(r.voltage ?? ""),
      csvEscape(r.current ?? ""),
      csvEscape(r.temp ?? ""),
      csvEscape(r.cycle_nmse ?? ""),
      csvEscape(r.zcv ?? ""),
      csvEscape(r.zc_dwell_ratio ?? ""),
      csvEscape(r.pulse_count_per_cycle ?? ""),
      csvEscape(r.peak_fluct_cv ?? ""),
      csvEscape(r.midband_residual_rms ?? ""),
      csvEscape(r.hf_band_energy_ratio ?? ""),
      csvEscape(r.wpe_entropy ?? ""),
      csvEscape(r.spec_entropy ?? ""),
      csvEscape(r.thd_i ?? "")
    ].join(","));
  }

  const key = rangeSelect?.value || "range";
  downloadTextFile(`TSP_faults_${key}_${new Date().toISOString().slice(0,10)}.csv`, lines.join("\n"));
  toast("CSV downloaded.", "ok");
});

btnClearHistory?.addEventListener("click", async () => {
  const ok = confirm("Clear ALL fault history in Firebase? This cannot be undone.");
  if (!ok) return;
  btnClearHistory.disabled = true;
  btnClearHistory.textContent = "Clearing...";
  try {
    await db.ref("history").remove();
    toast("History cleared.", "ok");
  } catch (e) {
    console.error(e);
    toast("Failed to clear history (rules may block write).", "err");
  } finally {
    btnClearHistory.disabled = false;
    btnClearHistory.textContent = "Clear";
  }
});

function updateInstallHelp() {
  if (!installHelp) return;
  if (loadBool(LS_INSTALL_DISMISS, false)) {
    installHelp.classList.add("hidden");
    return;
  }
  const showIOSHelp = isIOS && !isStandalone;
  const showGeneric = !isIOS && !isStandalone;
  if (!(showIOSHelp || showGeneric)) {
    installHelp.classList.add("hidden");
    return;
  }
  if (installCopy) {
    if (showIOSHelp) installCopy.textContent = "On iPhone or iPad, tap Share, then Add to Home Screen for the best app-like experience.";
    else installCopy.textContent = "Install this dashboard for a faster launch, a cleaner full-screen layout, and offline shell support.";
  }
  installHelp.classList.remove("hidden");
}
btnDismissInstall?.addEventListener("click", () => {
  saveBool(LS_INSTALL_DISMISS, true);
  installHelp?.classList.add("hidden");
});
btnRefreshNow?.addEventListener("click", () => window.location.reload());
updateInstallHelp();

function updateLiveDom(data) {
  const v   = toFixedOrDash(data.voltage, 1);
  const i   = toFixedOrDash(data.current, 3);
  const p   = toFixedOrDash(data.apparent_power, 1);
  const t   = toFixedOrDash(data.temp, 1);
  const cn  = toFixedOrDash(data.cycle_nmse, 3);
  const z   = toFixedOrDash(data.zcv, 3);
  const zd  = toFixedOrDash(data.zc_dwell_ratio, 3);
  const pc  = toFixedOrDash(data.pulse_count_per_cycle, 3);
  const pf  = toFixedOrDash(data.peak_fluct_cv, 3);
  const mr  = toFixedOrDash(data.midband_residual_rms, 3);
  const hf  = toFixedOrDash(data.hf_band_energy_ratio, 3);
  const wpe = toFixedOrDash(data.wpe_entropy, 3);
  const se  = toFixedOrDash(data.spec_entropy, 3);
  const th  = toFixedOrDash(data.thd_i, 3);

  if (vVal && vVal.textContent !== v) { vVal.textContent = v; animateNumber(vVal); }
  if (iVal && iVal.textContent !== i) { iVal.textContent = i; animateNumber(iVal); }
  if (pVal && pVal.textContent !== p) { pVal.textContent = p; animateNumber(pVal); }
  if (tVal && tVal.textContent !== t) { tVal.textContent = t; animateNumber(tVal); }
  if (cycleNmseVal && cycleNmseVal.textContent !== cn) { cycleNmseVal.textContent = cn; animateNumber(cycleNmseVal); }
  if (zcvVal && zcvVal.textContent !== z) { zcvVal.textContent = z; animateNumber(zcvVal); }
  if (zcDwellVal && zcDwellVal.textContent !== zd) { zcDwellVal.textContent = zd; animateNumber(zcDwellVal); }
  if (pulseCountVal && pulseCountVal.textContent !== pc) { pulseCountVal.textContent = pc; animateNumber(pulseCountVal); }
  if (peakFluctVal && peakFluctVal.textContent !== pf) { peakFluctVal.textContent = pf; animateNumber(peakFluctVal); }
  if (midbandResidVal && midbandResidVal.textContent !== mr) { midbandResidVal.textContent = mr; animateNumber(midbandResidVal); }
  if (hfEnergyVal && hfEnergyVal.textContent !== hf) { hfEnergyVal.textContent = hf; animateNumber(hfEnergyVal); }
  if (wpeEntropyVal && wpeEntropyVal.textContent !== wpe) { wpeEntropyVal.textContent = wpe; animateNumber(wpeEntropyVal); }
  if (specEntropyVal && specEntropyVal.textContent !== se) { specEntropyVal.textContent = se; animateNumber(specEntropyVal); }
  if (thdVal && thdVal.textContent !== th) { thdVal.textContent = th; animateNumber(thdVal); }

  applyMetricHints(data);
}

db.ref("live_data").on("value", (snap) => {
  const data = snap.val();
  if (!data) return;

  syntheticDisconnectActive = false;
  lastReceiptMs = Date.now();
  lastSeenMs = getRecordEpochMs(data) || lastReceiptMs;
  lastLiveData = data;

  const wifi = !!data.wifi_connected;
  const ip = (data.ip || "").toString().trim();
  if (deviceIpLine && deviceIpText) {
    if (wifi && ip) {
      deviceIpLine.style.display = "";
      deviceIpText.textContent = ip;
    } else {
      deviceIpLine.style.display = "none";
      deviceIpText.textContent = "—";
    }
  }
  if (deviceMdnsText) deviceMdnsText.textContent = (data.mdns || "tinyml-smart-plug.local").toString();

  updateLiveDom(data);
  renderTopState();
  transitionNotice(data);
}, (err) => {
  console.error(err);
  renderTopState();
});

db.ref("history")
  .orderByChild("server_ts")
  .limitToLast(HISTORY_LIMIT)
  .on("value", (snap) => {
    const obj = snap.val();
    if (!obj) {
      historyCache = [];
      refreshLatestHistoryRecord();
      applyHistoryFilter();
      renderOverview();
      renderTopState();
      return;
    }

    historyCache = Object.values(obj);
    refreshLatestHistoryRecord();

    applyHistoryFilter();
    renderOverview();
    if (!liveIsFresh()) renderTopState();
  });

setInterval(() => {
  if (!lastSeenMs) {
    renderTopState();
    return;
  }
  if ((Date.now() - lastSeenMs) > STALE_MS) {
    if (deviceIpLine) deviceIpLine.style.display = "none";
    setLiveUnavailable();
    applyMetricHints(lastLiveData || {});
    const latestStatus = classifyStatus(latestHistoryRecord?.status || "");
    if (previousFresh || latestStatus !== "DEVICE_DISCONNECTED") {
      recordEffectiveHistoryTransition("DEVICE_DISCONNECTED", lastLiveData || {}, Date.now());
      previousFresh = false;
    }
  }
  renderTopState();
}, 1000);

document.addEventListener("visibilitychange", () => {
  if (document.visibilityState === "visible") renderTopState();
});

(function initOta() {
  const otaCurVer = el("otaCurVer");
  const otaCurUrl = el("otaCurUrl");
  const otaDesiredVer = el("otaDesiredVer");
  const otaBinName = el("otaBinName");
  const otaFirmwareUrl = el("otaFirmwareUrl");
  const btnFillRepoUrl = el("btnFillRepoUrl");
  const btnPublishOta = el("btnPublishOta");
  if (!btnPublishOta || !otaDesiredVer) return;

  db.ref("ota").on("value", (snap) => {
    const v = snap.val() || {};
    if (otaCurVer) otaCurVer.textContent = (v.desired_version || "—").toString();
    if (otaCurUrl) otaCurUrl.textContent = (v.firmware_url || "—").toString();
  });

  btnFillRepoUrl?.addEventListener("click", () => {
    const url = buildRepoFirmwareUrl(otaBinName?.value || OTA_DEFAULT_BIN);
    if (otaFirmwareUrl) otaFirmwareUrl.value = url;
    toast("Repo firmware URL filled.", "ok");
  });

  btnPublishOta.addEventListener("click", async () => {
    const desired = (otaDesiredVer.value || "").trim();
    let url = (otaFirmwareUrl?.value || "").trim();
    if (!url) url = buildRepoFirmwareUrl(otaBinName?.value || OTA_DEFAULT_BIN);
    if (!isLikelyVersion(desired)) {
      toast("Enter a valid desired version (e.g., TSP-v0.1.1).", "err");
      return;
    }
    if (!isHttpsUrl(url)) {
      toast("Firmware URL must be a valid HTTPS URL.", "err");
      return;
    }

    btnPublishOta.disabled = true;
    const oldText = btnPublishOta.textContent;
    btnPublishOta.textContent = "Publishing...";

    try {
      await db.ref("ota").update({
        desired_version: desired,
        firmware_url: url,
        published_at: firebase.database.ServerValue.TIMESTAMP
      });
      toast("OTA published. Devices will pull on next check.", "ok");
    } catch (e) {
      console.error(e);
      toast("Publish failed (Firebase rules may block writes).", "err");
    } finally {
      btnPublishOta.disabled = false;
      btnPublishOta.textContent = oldText || "Update";
    }
  });
})();

const mlLogEnable = el("mlLogEnable");
const mlLogDur = el("mlLogDur");
const mlLoadType = el("mlLoadType");
const mlLabelOverride = el("mlLabelOverride");
const btnDownloadSessionMl = el("btnDownloadSessionMl");
const btnDownloadAllMl = el("btnDownloadAllMl");
const btnClearMlLogs = el("btnClearMlLogs");
const btnUploadCsv = el("btnUploadCsv");
const mlCsvUpload = el("mlCsvUpload");
const mlLogStatus = el("mlLogStatus");
const mlSessionBody = el("mlSessionBody");
let currentSessionId = "";

function downloadTextFileGeneric(filename, text, mime="text/csv;charset=utf-8") {
  const blob = new Blob([text], { type: mime });
  const url = URL.createObjectURL(blob);
  const a = document.createElement("a");
  a.href = url;
  a.download = filename;
  document.body.appendChild(a);
  a.click();
  a.remove();
  URL.revokeObjectURL(url);
}

function makeSessionId() { return "sess_" + Date.now(); }
function labelText(v) {
  if (String(v) === "1") return "ARC";
  if (String(v) === "0") return "NORMAL";
  return "AUTO";
}
function durationText(meta) {
  const st = Number(meta?.start_ms || 0);
  const en = Number(meta?.end_ms || 0);
  if (!st) return "—";
  const durMs = (en > st ? en : Date.now()) - st;
  return `${Math.max(0, Math.round(durMs / 1000))}s`;
}

async function fetchSessionCsv(sessionId) {
  const snap = await db.ref(`ml_logs/${sessionId}`).get();
  if (!snap.exists()) return "";
  const chunksObj = snap.val() || {};
  const keys = Object.keys(chunksObj).sort((a,b) => (chunksObj[a]?.created_at || 0) - (chunksObj[b]?.created_at || 0));
  let header = "";
  let rows = [];
  for (const k of keys) {
    const csv = chunksObj[k]?.csv || "";
    if (!csv) continue;
    const lines = csv.split("\n").filter(x => x.trim().length);
    if (lines.length === 0) continue;
    if (!header) {
      header = lines[0];
      rows.push(...lines.slice(1));
    } else {
      const startIdx = (lines[0].trim() === header.trim()) ? 1 : 0;
      rows.push(...lines.slice(startIdx));
    }
  }
  if (!header) return "";
  return header + "\n" + rows.join("\n") + "\n";
}

function sessionFilename(meta, sessionId) {
  const start = meta?.start_ms || 0;
  const end = meta?.end_ms || 0;
  const startStr = start ? formatEpochMsTZ(start).replace(/[ :.]/g,"-") : "START";
  const endStr   = end ? formatEpochMsTZ(end).replace(/[ :.]/g,"-") : "OPEN";
  const load = (meta?.load_type || "unknown").toString().replace(/[^a-zA-Z0-9_-]/g,"_");
  return `TSP_ML_${startStr}__${endStr}__${load}__${sessionId}.csv`;
}

if (mlLogEnable) {
  db.ref("ml_log").on("value", (s) => {
    const v = s.val() || {};
    mlLogEnable.checked = !!v.enabled;
    if (typeof v.duration_s === "number" && mlLogDur) mlLogDur.value = String(v.duration_s);
    if (mlLoadType && v.load_type) mlLoadType.value = v.load_type;
    if (mlLabelOverride && (v.label_override !== undefined)) mlLabelOverride.value = String(v.label_override);
    if (v.session_id) currentSessionId = v.session_id;
  });

  mlLogEnable.addEventListener("change", async () => {
    const enabled = !!mlLogEnable.checked;
    const dur = parseInt(mlLogDur?.value || "10", 10);
    const load = (mlLoadType?.value || "unknown").trim() || "unknown";
    const labelOv = parseInt(mlLabelOverride?.value || "-1", 10);

    if (enabled) {
      const sid = makeSessionId();
      currentSessionId = sid;
      await db.ref("ml_log").update({ enabled: true, duration_s: dur, session_id: sid, load_type: load, label_override: labelOv });
      await db.ref(`ml_sessions/${sid}`).set({
        start_ms: firebase.database.ServerValue.TIMESTAMP,
        end_ms: null,
        load_type: load,
        duration_s: dur,
        label_override: labelOv
      });
      if (mlLogStatus) mlLogStatus.textContent = `Logging enabled. Session: ${sid}`;
      toast("Logger enabled (session created).", "ok");
    } else {
      const sid = currentSessionId;
      await db.ref("ml_log").update({ enabled: false });
      if (sid) await db.ref(`ml_sessions/${sid}`).update({ end_ms: firebase.database.ServerValue.TIMESTAMP });
      if (mlLogStatus) mlLogStatus.textContent = `Logging disabled. Session closed: ${sid || "—"}`;
      toast("Logger disabled.", "ok");
    }
  });
}

mlLogDur?.addEventListener("change", async () => {
  const dur = parseInt(mlLogDur.value || "10", 10);
  await db.ref("ml_log").update({ duration_s: dur });
  toast("Duration updated.", "ok");
});
mlLoadType?.addEventListener("change", async () => {
  await db.ref("ml_log").update({ load_type: (mlLoadType.value || "unknown").trim() || "unknown" });
});
mlLabelOverride?.addEventListener("change", async () => {
  const v = parseInt(mlLabelOverride.value || "-1", 10);
  await db.ref("ml_log").update({ label_override: v });
});

if (mlSessionBody) {
  db.ref("ml_sessions").limitToLast(50).on("value", (s) => {
    const obj = s.val() || {};
    const ids = Object.keys(obj).sort((a,b) => (obj[b]?.start_ms||0) - (obj[a]?.start_ms||0));
    mlSessionBody.innerHTML = ids.map((sid) => {
      const meta = obj[sid] || {};
      const st = meta.start_ms ? formatEpochMsTZ(meta.start_ms) : "—";
      const en = meta.end_ms ? formatEpochMsTZ(meta.end_ms) : "—";
      const load = meta.load_type || "unknown";
      const lab  = labelText(meta.label_override);
      return `
        <tr>
          <td class="mono">${sid}</td>
          <td class="mono">${st}</td>
          <td class="mono">${en}</td>
          <td class="mono">${durationText(meta)}</td>
          <td class="mono">${String(load)}</td>
          <td class="mono">${lab}</td>
          <td class="session-actions">
            <button class="btn btn-small" data-view-sid="${sid}">View</button>
            <button class="btn btn-small" data-sid="${sid}">Download</button>
            <button class="btn btn-small btn-danger" data-del-sid="${sid}">Delete</button>
          </td>
        </tr>
      `;
    }).join("");

    mlSessionBody.querySelectorAll("button[data-sid]").forEach(btn => {
      btn.addEventListener("click", async () => {
        const sid = btn.getAttribute("data-sid");
        const meta = obj[sid] || {};
        const csv = await fetchSessionCsv(sid);
        if (!csv) { toast("No data for this session yet.", "err"); return; }
        downloadTextFileGeneric(sessionFilename(meta, sid), csv);
        if (mlLogStatus) mlLogStatus.textContent = `Downloaded session: ${sid}`;
        toast("Session CSV downloaded.", "ok");
      });
    });

    mlSessionBody.querySelectorAll("button[data-view-sid]").forEach(btn => {
      btn.addEventListener("click", () => {
        const sid = btn.getAttribute("data-view-sid");
        const meta = obj[sid] || {};
        if (typeof window.openSessionViewer === "function") window.openSessionViewer(sid, meta);
        else window.location.href = `session.html?sid=${encodeURIComponent(sid)}`;
      });
    });

    mlSessionBody.querySelectorAll("button[data-del-sid]").forEach(btn => {
      btn.addEventListener("click", async () => {
        const sid = btn.getAttribute("data-del-sid");
        if (!sid) return;
        if (!confirm(`Delete session ${sid}? This will remove both metadata and CSV chunks.`)) return;
        try {
          await db.ref(`ml_logs/${sid}`).remove();
          await db.ref(`ml_sessions/${sid}`).remove();
          toast("Session deleted.", "ok");
        } catch (e) {
          console.error(e);
          toast("Failed to delete this session.", "err");
        }
      });
    });
  });
}

btnDownloadSessionMl?.addEventListener("click", async () => {
  const sid = currentSessionId;
  if (!sid) { toast("No active session_id.", "err"); return; }
  const metaSnap = await db.ref(`ml_sessions/${sid}`).get();
  const meta = metaSnap.exists() ? metaSnap.val() : {};
  const csv = await fetchSessionCsv(sid);
  if (!csv) { toast("No session logs yet.", "err"); return; }
  downloadTextFileGeneric(sessionFilename(meta, sid), csv);
  if (mlLogStatus) mlLogStatus.textContent = `Downloaded session: ${sid}`;
  toast("Session CSV downloaded.", "ok");
});

btnDownloadAllMl?.addEventListener("click", async () => {
  const metaSnap = await db.ref("ml_sessions").get();
  const sessions = metaSnap.exists() ? metaSnap.val() : {};
  const ids = Object.keys(sessions || {});
  if (ids.length === 0) { toast("No sessions yet.", "err"); return; }
  ids.sort((a,b) => (sessions[a]?.start_ms||0) - (sessions[b]?.start_ms||0));
  let header = "";
  let rows = [];
  for (const sid of ids) {
    const csv = await fetchSessionCsv(sid);
    if (!csv) continue;
    const lines = csv.split("\n").filter(x => x.trim().length);
    if (lines.length === 0) continue;
    if (!header) {
      header = lines[0];
      rows.push(...lines.slice(1));
    } else {
      const startIdx = (lines[0].trim() === header.trim()) ? 1 : 0;
      rows.push(...lines.slice(startIdx));
    }
  }
  if (!header || rows.length === 0) { toast("No session rows found.", "err"); return; }
  const ts = new Date().toISOString().slice(0,19).replace(/[:T]/g,"-");
  downloadTextFileGeneric(`TSP_ML_ALL_${ts}.csv`, header + "\n" + rows.join("\n") + "\n");
  if (mlLogStatus) mlLogStatus.textContent = `Downloaded ALL sessions (${rows.length} rows)`;
  toast("All sessions downloaded.", "ok");
});

btnClearMlLogs?.addEventListener("click", async () => {
  if (!confirm("Delete ALL ML logs and sessions? This cannot be undone.")) return;
  try {
    await db.ref("ml_log").update({ enabled: false });
    await db.ref("ml_logs").remove();
    await db.ref("ml_sessions").remove();
    if (mlLogStatus) mlLogStatus.textContent = "ML logs cleared.";
    toast("ML logs cleared.", "ok");
  } catch (e) {
    console.error(e);
    toast("Failed to clear ML logs (rules may block).", "err");
  }
});

btnUploadCsv?.addEventListener("click", () => mlCsvUpload?.click());
mlCsvUpload?.addEventListener("change", async (ev) => {
  const file = ev.target?.files?.[0];
  if (!file) return;
  try {
    const text = await file.text();
    if (!text || text.indexOf(",") < 0) throw new Error("Invalid CSV");
    const meta = { load_type: "uploaded_csv", duration_s: "—", start_ms: Date.now(), end_ms: Date.now() };
    if (typeof window.openSessionViewerFromCsv === "function") {
      window.openSessionViewerFromCsv(file.name, text, meta);
      toast("CSV loaded into plot viewer.", "ok");
    } else {
      toast("CSV viewer is not ready yet.", "err");
    }
  } catch (e) {
    console.error(e);
    toast("Failed to load CSV.", "err");
  } finally {
    if (mlCsvUpload) mlCsvUpload.value = "";
  }
});

initCollapsibles();

if ("serviceWorker" in navigator) {
  navigator.serviceWorker.register("sw.js").then((reg) => {
    reg.update();
    let refreshing = false;
    navigator.serviceWorker.addEventListener("controllerchange", () => {
      if (refreshing) return;
      refreshing = true;
      window.location.reload();
    });
    document.addEventListener("visibilitychange", () => {
      if (document.visibilityState === "visible") reg.update();
    });
  }).catch(console.error);
}

renderTopState();
