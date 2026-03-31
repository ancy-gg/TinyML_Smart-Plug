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
const overviewMeta = el("overviewMeta");
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
const densityButtons = Array.from(document.querySelectorAll("[data-density]"));
const modeButtons = Array.from(document.querySelectorAll("[data-mode]"));
const densityToggle = el("densityToggle");
const modeToggle = el("modeToggle");
const densityLabelLeft = el("densityLabelLeft");
const densityLabelRight = el("densityLabelRight");
const modeLabelLeft = el("modeLabelLeft");
const modeLabelRight = el("modeLabelRight");
const alertLabelLeft = el("alertLabelLeft");
const alertLabelRight = el("alertLabelRight");

const DISPLAY_TZ = "Asia/Manila";
const STALE_MS = 30000;
const HISTORY_LIMIT = 5000;
const MAX_RENDER_ROWS = 180;

const OTA_REPO_RAW_BASE = "https://raw.githubusercontent.com/ancy-gg/TinyML_Smart-Plug/main/firmware/";
const OTA_DEFAULT_BIN   = "firmware.bin";

const UI_OVERLOAD_WARN_A = 10;
const UI_SHORT_CIRCUIT_A = 20;
const UI_NORMAL_V_MIN = 207;
const UI_NORMAL_V_MAX = 253;
const UI_UNDERVOLTAGE_MIN_V = 170;
const UI_UNDERVOLTAGE_V = 207;
const UI_OVERVOLTAGE_V = 253;
const UI_OVERVOLTAGE_DELAY_V = 250;
const UI_OVERVOLTAGE_FAST_V = 265;
const UI_MAINS_ABSENT_V = 20;
const UI_TEMP_COLD_ABNORMAL_C = 10;
const UI_TEMP_WARM_C = 60;
const UI_TEMP_HOT_C = 70;
const UI_ACTIVE_CURRENT_A = 0.08;
const UI_ACTIVE_POWER_VA = 15;

let lastSeenMs = 0;
let lastReceiptMs = 0;
let historyCache = [];
let currentFilteredHistory = [];
let syntheticDisconnectActive = false;
let latestHistoryRecord = null;
let lastLiveData = null;
let lastAlertStatus = null;
let lastNotifiedAt = 0;
let previousPowerCondition = "UNKNOWN";
let previousFresh = false;
let previousEffectiveStatus = "UNKNOWN";

const LS_ALERT = "tsp_alert_enabled";
const LS_SOUND = "tsp_sound_enabled";
const LS_INSTALL_DISMISS = "tsp_install_help_dismissed";
const LS_DENSITY = "tsp_density";
const LS_MODE = "tsp_mode";
const NOTIFIABLE_STATUS_SET = new Set(["DEVICE_DISCONNECTED", "UNPLUGGED", "OVERLOAD", "SUSTAINED_OVERLOAD", "HEATING", "ARCING", "OVERVOLTAGE", "UNDERVOLTAGE"]);

const isIOS = /iphone|ipad|ipod/i.test(navigator.userAgent || "");
const isStandalone = window.matchMedia?.("(display-mode: standalone)")?.matches || window.navigator.standalone === true;
if (isIOS) document.body.classList.add("ios");
if (isStandalone) document.body.classList.add("standalone");

function loadString(k, def="") {
  const v = localStorage.getItem(k);
  return v === null ? def : String(v);
}
function saveString(k, v) { localStorage.setItem(k, String(v)); }

function applyViewPrefs() {
  const mode = loadString(LS_MODE, "consumer");
  document.body.classList.add("density-compact");
  document.body.classList.remove("density-comfortable");
  document.body.classList.toggle("mode-admin", mode === "admin");
  densityButtons.forEach((btn) => btn.classList.toggle("is-active", btn.getAttribute("data-density") === "compact"));
  modeButtons.forEach((btn) => btn.classList.toggle("is-active", btn.getAttribute("data-mode") === mode));
  if (densityToggle) densityToggle.checked = false;
  if (modeToggle) modeToggle.checked = mode === "admin";
  densityLabelLeft?.classList.remove("is-active");
  densityLabelRight?.classList.remove("is-active");
  modeLabelLeft?.classList.toggle("is-active", mode === "consumer");
  modeLabelRight?.classList.toggle("is-active", mode === "admin");
}

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

function updateAlertToggleUI() {
  const enabled = !!(alertEnable?.checked);
  alertLabelLeft?.classList.toggle("is-active", !enabled);
  alertLabelRight?.classList.toggle("is-active", enabled);
}

applyViewPrefs();
densityButtons.forEach((btn) => btn.addEventListener("click", () => {
  saveString(LS_DENSITY, btn.getAttribute("data-density") || "compact");
  applyViewPrefs();
}));
modeButtons.forEach((btn) => btn.addEventListener("click", () => {
  saveString(LS_MODE, btn.getAttribute("data-mode") || "consumer");
  applyViewPrefs();
}));
densityToggle?.addEventListener("change", () => {
  saveString(LS_DENSITY, "compact");
  applyViewPrefs();
  setHeaderMenuOpen(false);
});
modeToggle?.addEventListener("change", () => {
  saveString(LS_MODE, modeToggle.checked ? "admin" : "consumer");
  applyViewPrefs();
  setHeaderMenuOpen(false);
});


const menuTrigger = el("menuTrigger");
const headerMenu = el("headerMenu");
const btnChangeWifi = el("btnChangeWifi");
const btnRevertFirmware = el("btnRevertFirmware");
const btnRelayOn = el("btnRelayOn");
const btnRelayOff = el("btnRelayOff");
const relayRocker = el("relayRocker");
const btnFaultCleared = el("btnFaultCleared");

function setHeaderMenuOpen(open) {
  if (!menuTrigger || !headerMenu) return;
  headerMenu.hidden = !open;
  menuTrigger.setAttribute("aria-expanded", open ? "true" : "false");
  document.body.classList.toggle("menu-open", open);
}

menuTrigger?.addEventListener("click", (ev) => {
  ev.stopPropagation();
  setHeaderMenuOpen(headerMenu?.hidden ?? true);
});

headerMenu?.addEventListener("click", (ev) => ev.stopPropagation());
document.addEventListener("click", () => setHeaderMenuOpen(false));
window.addEventListener("resize", () => setHeaderMenuOpen(false));
document.addEventListener("keydown", (ev) => {
  if (ev.key === "Escape") setHeaderMenuOpen(false);
});

densityButtons.forEach((btn) => btn.addEventListener("click", () => setHeaderMenuOpen(false)));
modeButtons.forEach((btn) => btn.addEventListener("click", () => setHeaderMenuOpen(false)));

async function sendControlPulse(buttonEl, busyText, okText, errText, payload) {
  setHeaderMenuOpen(false);
  const oldText = buttonEl?.textContent || "";
  if (buttonEl) {
    buttonEl.disabled = true;
    buttonEl.textContent = busyText;
  }
  try {
    await db.ref("/controls").update(payload);
    toast(okText, "ok");
  } catch (e) {
    console.error(e);
    toast(errText, "err");
  } finally {
    if (buttonEl) {
      buttonEl.disabled = false;
      buttonEl.textContent = oldText;
    }
  }
}

btnChangeWifi?.addEventListener("click", async () => {
  setHeaderMenuOpen(false);

  const oldText = btnChangeWifi.textContent;
  btnChangeWifi.disabled = true;
  btnChangeWifi.textContent = "Opening...";

  try {
    const token = `portal_${Date.now()}`;
    await db.ref("/controls").update({
      open_portal_token: token,
      open_portal_requested_at: firebase.database.ServerValue.TIMESTAMP
    });
    console.log("Change WiFi requested:", token);
    toast("Hotspot request sent. AP will open for 15s.", "ok");
  } catch (e) {
    console.error("Change WiFi write failed:", e);
    toast("Failed to request WiFi change.", "err");
  } finally {
    btnChangeWifi.disabled = false;
    btnChangeWifi.textContent = oldText || "Change WiFi";
  }
});

btnRevertFirmware?.addEventListener("click", async () => {
  setHeaderMenuOpen(false);
  const confirmed = window.confirm("Reboot into the previous OTA firmware slot? Use this when the current firmware OTA is broken.");
  if (!confirmed) return;

  const oldText = btnRevertFirmware.textContent || "Revert Firmware";
  btnRevertFirmware.disabled = true;
  btnRevertFirmware.textContent = "Reverting...";

  try {
    const token = `revert_fw_${Date.now()}`;
    await db.ref("/controls").update({
      revert_fw_token: token,
      revert_fw_requested_at: firebase.database.ServerValue.TIMESTAMP
    });
    toast("Firmware revert requested. Device will reboot into the previous OTA slot if available.", "warn");
  } catch (e) {
    console.error("Firmware revert write failed:", e);
    toast("Failed to request firmware revert.", "err");
  } finally {
    btnRevertFirmware.disabled = false;
    btnRevertFirmware.textContent = oldText;
  }
});

btnRelayOn?.addEventListener("click", async () => {
  await sendControlPulse(
    btnRelayOn,
    "Sending...",
    "Relay ON pulse sent.",
    "Failed to send Relay ON.",
    {
      relay_on_token: `relay_on_${Date.now()}`,
      relay_on_requested_at: firebase.database.ServerValue.TIMESTAMP
    }
  );
});

btnRelayOff?.addEventListener("click", async () => {
  await sendControlPulse(
    btnRelayOff,
    "Sending...",
    "Relay OFF pulse sent.",
    "Failed to send Relay OFF.",
    {
      relay_off_token: `relay_off_${Date.now()}`,
      relay_off_requested_at: firebase.database.ServerValue.TIMESTAMP
    }
  );
});

btnFaultCleared?.addEventListener("click", async () => {
  const oldText = btnFaultCleared.textContent || "";
  btnFaultCleared.disabled = true;
  btnFaultCleared.textContent = "Clearing...";
  try {
    await db.ref("/controls").update({
      fault_clear_token: `fault_clear_${Date.now()}`,
      fault_clear_requested_at: firebase.database.ServerValue.TIMESTAMP
    });
    toast("Fault clear requested.", "ok");
  } catch (e) {
    console.error(e);
    toast("Failed to clear fault.", "err");
  } finally {
    btnFaultCleared.disabled = false;
    btnFaultCleared.textContent = oldText || "Clear Fault";
  }
});

if (alertEnable) alertEnable.checked = loadBool(LS_ALERT, true);
if (soundEnable) soundEnable.checked = loadBool(LS_SOUND, false);
updateAlertToggleUI();
alertEnable?.addEventListener("change", async () => {
  saveBool(LS_ALERT, alertEnable.checked);
  updateAlertToggleUI();
  if (alertEnable.checked && "Notification" in window && Notification.permission === "default") {
    try { await Notification.requestPermission(); } catch {}
  }
  setHeaderMenuOpen(false);
});
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

  const tagSafe = String(body || title || "status")
    .toLowerCase()
    .replace(/[^a-z0-9]+/g, "-")
    .replace(/^-+|-+$/g, "") || "status";

  if ("serviceWorker" in navigator) {
    const reg = await navigator.serviceWorker.ready.catch(()=>null);
    if (reg) {
      reg.showNotification(title, {
        body,
        icon: "icons/icon-192.png",
        badge: "icons/icon-192.png",
        tag: `tsp-${tagSafe}`,
        renotify: true
      });
      return;
    }
  }
  new Notification(title, { body, icon: "icons/icon-192.png", tag: `tsp-${tagSafe}` });
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
  if (u.includes("SUSTAINED") && u.includes("OVERLOAD")) return "SUSTAINED_OVERLOAD";
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
    case "SUSTAINED_OVERLOAD": return `SUSTAINED OVERLOAD <span class="emoji blink">⬆️</span>`;
    case "HEATING":  return `HEATING <span class="emoji flicker">🔥</span>`;
    case "ARCING":   return `ARCING <span class="emoji zap">⚡</span>`;
    case "OVERVOLTAGE": return `OVERVOLTAGE`;
    case "UNDERVOLTAGE": return `UNDERVOLTAGE`;
    case "UNPLUGGED": return `UNPLUGGED`;
    case "DEVICE_DISCONNECTED": return `DISCONNECTED`;
    case "DEVICE_ON": return `DEVICE ON`;
    case "DEVICE_ONLINE": return `ONLINE`;
    case "DEVICE_PLUGGED_IN": return `PLUGGED IN`;
    case "FIRMWARE_UPDATED": return `FIRMWARE UPDATED`;
    case "SAFE_MODE": return `SAFE MODE`;
    case "CONFIG_PORTAL": return `CONFIG PORTAL`;
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
  else if (k === "OVERLOAD" || k === "SUSTAINED_OVERLOAD") statusBadge.classList.add("status-OVERLOAD");
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
  if (k === "SUSTAINED_OVERLOAD") return `<span class="pill pill-OVERLOAD">SUSTAINED OVERLOAD <span class="emoji blink">⬆️</span></span>`;
  if (k === "HEATING")  return `<span class="pill pill-HEATING">HEATING <span class="emoji flicker">🔥</span></span>`;
  if (k === "ARCING")   return `<span class="pill pill-ARCING">ARCING <span class="emoji zap">⚡</span></span>`;
  if (k === "OVERVOLTAGE") return `<span class="pill pill-OVERVOLTAGE">OVERVOLTAGE</span>`;
  if (k === "UNDERVOLTAGE") return `<span class="pill pill-UNDERVOLTAGE">UNDERVOLTAGE</span>`;
  if (k === "UNPLUGGED") return `<span class="pill pill-UNPLUG">DEVICE UNPLUGGED</span>`;
  if (k === "DEVICE_DISCONNECTED") return `<span class="pill pill-DIS">DEVICE DISCONNECTED</span>`;
  if (k === "DEVICE_ON") return `<span class="pill pill-OK">DEVICE ON</span>`;
  if (k === "DEVICE_PLUGGED_IN") return `<span class="pill pill-ONLINE">DEVICE PLUGGED IN</span>`;
  if (k === "DEVICE_ONLINE") return `<span class="pill pill-ONLINE">DEVICE ONLINE</span>`;
  if (k === "FIRMWARE_UPDATED") return `<span class="pill pill-FW">FIRMWARE UPDATED</span>`;
  if (k === "NORMAL") return `<span class="pill pill-OK">NORMAL</span>`;
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
  return !!lastReceiptMs && (Date.now() - lastReceiptMs) <= STALE_MS;
}

function latestLiveEpoch() {
  return getRecordEpochMs(lastLiveData || {});
}

function derivePowerConditionFromLive(data) {
  const v = Number(data?.voltage ?? 0);
  const raw = classifyStatus(data?.power_condition || data?.status || "NORMAL");
  if (raw === "OVERVOLTAGE") return "OVERVOLTAGE";
  if (raw === "UNPLUGGED" || v < UI_MAINS_ABSENT_V) return "UNPLUGGED";
  if (raw === "UNDERVOLTAGE") return "UNDERVOLTAGE";

  // Fallback classification when the device-side power_condition is absent:
  // - deep collapses are treated as mains loss / collapse, not undervoltage
  // - staged undervoltage is only shown in the intended 170 V to 206 V band
  // - overvoltage fallback follows the normal-band ceiling, while the hint text
  //   still documents the delayed / fast trip regions used by protection logic
  if (v >= UI_UNDERVOLTAGE_MIN_V && v < UI_UNDERVOLTAGE_V) return "UNDERVOLTAGE";
  if (v > UI_NORMAL_V_MAX) return "OVERVOLTAGE";
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
  if (["DEVICE_DISCONNECTED", "OVERLOAD", "SUSTAINED_OVERLOAD", "HEATING", "ARCING", "OVERVOLTAGE", "UNDERVOLTAGE", "UNPLUGGED", "SAFE_MODE", "CONFIG_PORTAL", "WIFI_CONNECTING", "STARTUP_STABILIZING", "OTA_UPDATING", "FIRMWARE_UPDATED"].includes(raw)) return raw;
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

function formatElapsedClock(ms, includeAgo = true) {
  if (!Number.isFinite(ms) || ms < 0) ms = 0;
  const total = Math.round(ms / 1000);
  const h = String(Math.floor(total / 3600)).padStart(2, "0");
  const m = String(Math.floor((total % 3600) / 60)).padStart(2, "0");
  const s = String(total % 60).padStart(2, "0");
  return `${h}h ${m}m ${s}s${includeAgo ? " ago" : ""}`;
}

function statusToastKind(kind) {
  const k = classifyStatus(kind);
  if (["ARCING", "HEATING", "SUSTAINED_OVERLOAD", "OVERVOLTAGE"].includes(k)) return "err";
  if (["DEVICE_DISCONNECTED", "UNPLUGGED", "OVERLOAD", "UNDERVOLTAGE"].includes(k)) return "warn";
  if (["FIRMWARE_UPDATED", "DEVICE_ONLINE", "DEVICE_PLUGGED_IN"].includes(k)) return "info";
  return "ok";
}

function maybeHandleEffectiveStatusChange() {
  const next = classifyStatus(effectiveStatusKind());
  if (next === previousEffectiveStatus) return;

  const prev = previousEffectiveStatus;
  previousEffectiveStatus = next;
  if (prev === "UNKNOWN") return;

  const label = prettyStatus(next);
  if (!["NORMAL", "DEVICE_ON", "DEVICE_ONLINE", "DEVICE_PLUGGED_IN"].includes(next)) {
    toast(`Status: ${label}.`, statusToastKind(next));
  }

  const pageVisible = document.visibilityState === "visible" && (typeof document.hasFocus !== "function" || document.hasFocus());
  if (!pageVisible && NOTIFIABLE_STATUS_SET.has(next) && (Date.now() - lastNotifiedAt) > 750) {
    showFaultNotification("TinyML Smart Plug Status", label);
    if ((soundEnable?.checked ?? false)) playBeep();
    lastNotifiedAt = Date.now();
  }
}

function updateFreshnessText() {
  if (!freshnessText) return;
  if (!lastReceiptMs) {
    freshnessText.textContent = "Waiting for device…";
    return;
  }
  const age = Math.max(0, Date.now() - lastReceiptMs);
  const ageText = formatElapsedClock(age);
  if (age <= STALE_MS) freshnessText.textContent = `Live • ${ageText}`;
  else freshnessText.textContent = `Stale • ${ageText}`;
}

function applyMetricHints(data) {
  if (!liveIsFresh()) {
    if (vHint) vHint.textContent = "—";
    if (iHint) iHint.textContent = "—";
    if (pHint) pHint.textContent = "—";
    if (tHint) tHint.textContent = "—";
    return;
  }

  const i = Number(data?.current ?? 0);
  const p = Number(data?.apparent_power ?? 0);
  const t = Number(data?.temp ?? 0);
  const powerKind = effectivePowerCondition();

  if (vHint) {
    const vNow = Number(data?.voltage ?? 0);
    if (powerKind === "UNPLUGGED") vHint.textContent = "No mains.";
    else if (powerKind === "UNDERVOLTAGE") vHint.textContent = `${UI_UNDERVOLTAGE_MIN_V}-${UI_NORMAL_V_MIN - 1} V staged`;
    else if (powerKind === "OVERVOLTAGE") vHint.textContent = `>${UI_NORMAL_V_MAX} V staged`;
    else if (vNow >= UI_MAINS_ABSENT_V && vNow < UI_UNDERVOLTAGE_MIN_V) vHint.textContent = "Collapse / mains loss";
    else vHint.textContent = `${UI_NORMAL_V_MIN}-${UI_NORMAL_V_MAX} V`;
  }

  if (iHint) {
    if (powerKind === "UNPLUGGED") iHint.textContent = "0 load";
    else if (i >= UI_SHORT_CIRCUIT_A) iHint.textContent = `> ${UI_SHORT_CIRCUIT_A} A`;
    else if (i >= UI_OVERLOAD_WARN_A) iHint.textContent = `${UI_OVERLOAD_WARN_A} A alarm`;
    else if (isLiveLoadActive(data)) iHint.textContent = "Load on";
    else iHint.textContent = "Idle";
  }

  if (pHint) {
    if (powerKind === "UNPLUGGED") pHint.textContent = "0 VA";
    else if (p >= 1500) pHint.textContent = "High VA";
    else if (p >= 300) pHint.textContent = "Active";
    else if (p >= UI_ACTIVE_POWER_VA) pHint.textContent = "Light";
    else pHint.textContent = "Idle";
  }

  if (tHint) {
    if (t < UI_TEMP_COLD_ABNORMAL_C) tHint.textContent = `< ${UI_TEMP_COLD_ABNORMAL_C} °C`;
    else if (t >= UI_TEMP_HOT_C) tHint.textContent = `≥ ${UI_TEMP_HOT_C} °C trip`;
    else if (t >= UI_TEMP_WARM_C) tHint.textContent = `≥ ${UI_TEMP_WARM_C} °C warm`;
    else tHint.textContent = "Normal";
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
  const fw = (live.fw_version || "—").toString();
  const otaReady = !!live.ota_ready;
  const ip = (live.ip || "").toString().trim();
  const mdns = (live.mdns || "tinyml-smart-plug.local").toString();
  const lastEventStatus = latestHistoryRecord ? prettyStatus(latestHistoryRecord.status || "") : "—";
  const lastEventTime = latestHistoryRecord ? formatEpochMsTZ(getRecordEpochMs(latestHistoryRecord)) : "—";
  const loadActive = fresh && isLiveLoadActive(live);

  if (overviewPrimary) {
    if (!fresh) overviewPrimary.textContent = "Offline";
    else if (status === "UNPLUGGED") overviewPrimary.textContent = "Unplugged";
    else if (status === "NORMAL") overviewPrimary.textContent = "Healthy";
    else if (status === "STARTUP_STABILIZING") overviewPrimary.textContent = "Starting";
    else if (status === "WIFI_CONNECTING") overviewPrimary.textContent = "Wi‑Fi";
    else if (status === "CONFIG_PORTAL") overviewPrimary.textContent = "Wi‑Fi Setup";
    else if (status === "OTA_UPDATING") overviewPrimary.textContent = "Updating";
    else overviewPrimary.textContent = prettyStatus(status);
  }

  if (overviewSecondary) {
    if (!fresh) overviewSecondary.textContent = "No live data.";
    else if (status === "UNPLUGGED") overviewSecondary.textContent = "No mains.";
    else if (status === "NORMAL") overviewSecondary.textContent = loadActive ? "Load active" : "Idle";
    else if (status === "OVERLOAD") overviewSecondary.textContent = "Alarm";
    else if (status === "SUSTAINED_OVERLOAD") overviewSecondary.textContent = "Trip";
    else if (status === "HEATING") overviewSecondary.textContent = "Heat trip";
    else if (status === "ARCING") overviewSecondary.textContent = "Arc trip";
    else if (status === "UNDERVOLTAGE") overviewSecondary.textContent = "Low line trip";
    else if (status === "OVERVOLTAGE") overviewSecondary.textContent = "High line trip";
    else overviewSecondary.textContent = "Monitoring";
  }

  const rawLiveStatus = classifyStatus(live.status || "");
  const connectionText = !fresh
    ? "Offline"
    : (rawLiveStatus === "CONFIG_PORTAL"
        ? "Portal"
        : (rawLiveStatus === "WIFI_CONNECTING" ? "Connecting" : "Online"));

  const ageMs = lastReceiptMs ? Math.max(0, Date.now() - lastReceiptMs) : 0;
  if (overviewMeta) {
    overviewMeta.textContent = !lastReceiptMs
      ? "Waiting for device…"
      : `${fresh ? "Live" : "Stale"} • ${formatElapsedClock(ageMs)}`;
  }

  if (ovConnectivity) ovConnectivity.textContent = connectionText;
  if (ovConnectivitySub) {
    const pieces = [];
    if (lastReceiptMs) pieces.push(formatElapsedClock(ageMs));
    ovConnectivitySub.textContent = pieces.join(" • ") || "No live link.";
  }

  if (ovProtection) {
    if (!fresh || status === "DEVICE_DISCONNECTED") ovProtection.textContent = "Offline";
    else ovProtection.textContent = (status === "NORMAL") ? ((live.load_state || (loadActive ? "LOAD ON" : "LOAD OFF"))) : prettyStatus(status);
  }
  if (ovProtectionSub) {
    if (status === "OVERLOAD") ovProtectionSub.textContent = "Alarm only.";
    else if (status === "SUSTAINED_OVERLOAD") ovProtectionSub.textContent = "Relay tripped.";
    else if (["HEATING", "ARCING", "UNDERVOLTAGE", "OVERVOLTAGE"].includes(status)) ovProtectionSub.textContent = "Relay tripped.";
    else if (status === "UNPLUGGED") ovProtectionSub.textContent = "No trip.";
    else ovProtectionSub.textContent = "No active trip.";
  }

  if (ovFirmware) ovFirmware.textContent = fw;
  if (ovFirmwareSub) ovFirmwareSub.textContent = `OTA ${otaReady ? "ready" : "off"} • ${fresh ? "online" : "offline"}`;

  if (ovLastEvent) ovLastEvent.textContent = lastEventStatus;
  if (ovLastEventSub) ovLastEventSub.textContent = lastEventStatus === "—" ? "No history." : lastEventTime;

  if (status === "HEATING" || status === "ARCING" || status === "SUSTAINED_OVERLOAD") setMiniBadge("fault", "Critical");
  else if (["OVERLOAD", "UNDERVOLTAGE", "OVERVOLTAGE", "UNPLUGGED", "WIFI_CONNECTING", "STARTUP_STABILIZING", "CONFIG_PORTAL", "OTA_UPDATING", "DEVICE_DISCONNECTED"].includes(status)) setMiniBadge("warn", fresh ? "Attention" : "Offline");
  else if (fresh) setMiniBadge("ok", loadActive ? "Active" : "Idle");
  else setMiniBadge("warn", "Offline");

  liveStateHints.forEach((node) => { node.textContent = fresh ? "LIVE" : "STALE"; });
  if (deviceMdnsText) deviceMdnsText.textContent = mdns || "tinyml-smart-plug.local";
}

function renderTopState() {
  const status = effectiveStatusKind();
  setTopStatus(status);
  const ts = effectiveTimestamp();
  if (lastUpdateText) lastUpdateText.textContent = ts ? formatEpochMsTZ(ts) : "—";
  updateFreshnessText();
  renderOverview();
  updateRelayControls();
  maybeHandleEffectiveStatusChange();
}

function transitionNotice(newLive) {
  const nowFresh = liveIsFresh();
  const powerCondition = effectivePowerCondition();
  const freshChanged = nowFresh && !previousFresh;
  const powerRestored = previousPowerCondition === "UNPLUGGED" && powerCondition !== "UNPLUGGED" && nowFresh;

  if (freshChanged) toast("Device is online again.", "info");
  if (powerRestored) toast("Power restored. Device plugged back in detected.", "ok");

  previousFresh = nowFresh;
  previousPowerCondition = powerCondition;
  lastAlertStatus = classifyStatus(newLive?.status || effectiveStatusKind());
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

function formatDurationMs(ms) {
  if (!ms || ms <= 0) return "—";
  return formatElapsedClock(ms, false);
}

function historyDurationMsDesc(sorted, idx) {
  const startEpoch = getRecordEpochMs(sorted[idx]);
  if (!startEpoch) return 0;
  let endEpoch = 0;
  if (idx === 0) {
    endEpoch = effectiveTimestamp() || Date.now();
  } else {
    endEpoch = getRecordEpochMs(sorted[idx - 1]);
  }
  return (endEpoch > startEpoch) ? (endEpoch - startEpoch) : 0;
}

function relayControlsLocked() {
  const k = classifyStatus(effectiveStatusKind());
  return !liveIsFresh() || ["ARCING", "HEATING", "OVERLOAD", "SUSTAINED_OVERLOAD", "UNDERVOLTAGE", "OVERVOLTAGE", "UNPLUGGED", "DEVICE_DISCONNECTED", "SAFE_MODE", "STARTUP_STABILIZING", "OTA_UPDATING", "CONFIG_PORTAL", "WIFI_CONNECTING"].includes(k);
}

function updateRelayControls() {
  const locked = relayControlsLocked();
  relayRocker?.classList.toggle("is-disabled", locked);
  if (btnRelayOn) btnRelayOn.disabled = locked;
  if (btnRelayOff) btnRelayOff.disabled = locked;
  if (btnFaultCleared) btnFaultCleared.disabled = !liveIsFresh();
}

function applyHistoryFilter() {
  const key = rangeSelect?.value || "7d";
  const { start, end } = getRangeBounds(key);

  const filtered = historyCache.filter(r => {
    const epoch = getRecordEpochMs(r);
    return epoch && epoch >= start && epoch < end;
  });

  currentFilteredHistory = filtered.slice();
  if (historyHint) historyHint.textContent = `Showing: ${rangeLabel(key)} (${filtered.length})`;

  if (!filtered.length) {
    historyBody.innerHTML = `<tr><td colspan="6" class="muted">No history in this range.</td></tr>`;
    return;
  }

  filtered.sort((a, b) => getRecordEpochMs(b) - getRecordEpochMs(a));
  const rows = filtered.slice(0, MAX_RENDER_ROWS).map((r, idx) => {
    const epoch = getRecordEpochMs(r);
    const timeStr = formatEpochMsTZ(epoch);
    const durStr = formatDurationMs(historyDurationMsDesc(filtered, idx));
    return `
      <tr class="row-in" style="animation-delay:${Math.min(idx, 10) * 25}ms">
        <td class="mono">${timeStr}</td>
        <td>${pillHTML(r.status)}</td>
        <td class="mono">${toFixedOrDash(r.voltage, 1)}</td>
        <td class="mono">${toFixedOrDash(r.current, 3)}</td>
        <td class="mono">${toFixedOrDash(r.temp, 1)}</td>
        <td class="mono">${durStr}</td>
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

  const rows = currentFilteredHistory.slice().sort((a, b) => getRecordEpochMs(a) - getRecordEpochMs(b));
  const header = ["timestamp","epoch_ms","status","voltage","current","temp","duration_ms"];
  const lines = [header.join(",")];

  for (let i = 0; i < rows.length; i++) {
    const r = rows[i];
    const epoch = getRecordEpochMs(r);
    const ts = formatEpochMsTZ(epoch);
    const descIndex = rows.length - 1 - i;
    const sortedDesc = rows.slice().sort((a,b) => getRecordEpochMs(b) - getRecordEpochMs(a));
    const durationMs = historyDurationMsDesc(sortedDesc, descIndex);
    lines.push([
      csvEscape(ts),
      csvEscape(epoch),
      csvEscape(r.status ?? ""),
      csvEscape(r.voltage ?? ""),
      csvEscape(r.current ?? ""),
      csvEscape(r.temp ?? ""),
      csvEscape(durationMs)
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
    if (showIOSHelp) installCopy.textContent = "On iPhone or iPad, tap Share then Add to Home Screen.";
    else installCopy.textContent = "Install for a faster full-screen dashboard.";
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
  lastSeenMs = getRecordEpochMs(data) || lastSeenMs || lastReceiptMs;
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
      latestHistoryRecord = null;
      applyHistoryFilter();
      renderOverview();
      renderTopState();
      return;
    }

    historyCache = Object.values(obj);
    latestHistoryRecord = historyCache.reduce((best, rec) => {
      if (!best) return rec;
      return getRecordEpochMs(rec) > getRecordEpochMs(best) ? rec : best;
    }, null);

    applyHistoryFilter();
    renderOverview();
    if (!liveIsFresh()) renderTopState();
  });

setInterval(() => {
  if (!lastReceiptMs) {
    renderTopState();
    return;
  }
  if ((Date.now() - lastReceiptMs) > STALE_MS) {
    if (deviceIpLine) deviceIpLine.style.display = "none";
    setLiveUnavailable();
    applyMetricHints(lastLiveData || {});
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
      const currentSnap = await db.ref("ota").get();
      const current = currentSnap.val() || {};
      await db.ref("ota").update({
        desired_version: desired,
        firmware_url: url,
        previous_desired_version: (current.desired_version || "").toString(),
        previous_firmware_url: (current.firmware_url || "").toString(),
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
let lastMlLogEnabled = null;

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
    const enabledNow = !!v.enabled;
    mlLogEnable.checked = enabledNow;
    if (typeof v.duration_s === "number" && mlLogDur) mlLogDur.value = String(v.duration_s);
    if (mlLoadType && v.load_type) mlLoadType.value = v.load_type;
    if (mlLabelOverride && (v.label_override !== undefined)) mlLabelOverride.value = String(v.label_override);
    if (v.session_id) currentSessionId = v.session_id;

    if (lastMlLogEnabled === true && !enabledNow) {
      const sid = (v.last_completed_session_id || currentSessionId || "—").toString();
      if (mlLogStatus) mlLogStatus.textContent = `Logging finished by device. Session closed: ${sid}`;
      toast("Logger finished automatically.", "ok");
    }
    lastMlLogEnabled = enabledNow;
  });

  mlLogEnable.addEventListener("change", async () => {
    const enabled = !!mlLogEnable.checked;
    const dur = Math.max(1, parseInt(mlLogDur?.value || "10", 10) || 10);
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
      if (mlLogStatus) mlLogStatus.textContent = `Logging enabled. Session: ${sid} • Total ${dur}s • Upload chunk 10s`;
      toast(`Logger enabled for ${dur}s.`, "ok");
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
  const dur = Math.max(1, parseInt(mlLogDur.value || "10", 10) || 10);
  mlLogDur.value = String(dur);
  await db.ref("ml_log").update({ duration_s: dur });
  toast(`Duration updated to ${dur}s.`, "ok");
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
