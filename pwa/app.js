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

let firebaseServerOffsetMs = 0;
db.ref("/.info/serverTimeOffset").on("value", (snap) => {
  const next = Number(snap.val());
  firebaseServerOffsetMs = Number.isFinite(next) ? next : 0;
});

function nowWithServerOffsetMs() {
  return Date.now() + firebaseServerOffsetMs;
}

const el = (id) => document.getElementById(id);

const statusBadge = el("statusBadge");
const lastUpdateText = el("lastUpdateText");
const freshnessText = el("freshnessText");
const deviceIpLine = el("deviceIpLine");
const deviceIpText = el("deviceIpText");
const deviceMdnsText = el("deviceMdnsText");

const overviewHealthBadge = el("overviewHealthBadge");
const healthWifiChip = el("healthWifiChip");
const healthWifiBars = el("healthWifiBars");
const healthWifiText = el("healthWifiText");
const healthLatencyChip = el("healthLatencyChip");
const healthLatencyText = el("healthLatencyText");
const overviewPrimary = el("overviewPrimary");
const overviewSecondary = el("overviewSecondary");
const overviewMeta = el("overviewMeta");
const ovLoadFamily = el("ovLoadFamily");
const ovLoadFamilySub = el("ovLoadFamilySub");
const ovConnectivitySub = ovLoadFamilySub;
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
const btnInstallApp = el("btnInstallApp");
const btnDismissInstall = el("btnDismissInstall");
const btnRefreshNow = el("btnRefreshNow");
const liveStateHints = Array.from(document.querySelectorAll("[data-live-state-hint]"));

const vVal   = el("vVal");
const iVal   = el("iVal");
const pVal   = el("pVal");
const tNtcVal = el("tNtcVal");
const tVal   = el("tVal");
const vHint  = el("vHint");
const iHint  = el("iHint");
const pHint  = el("pHint");
const tNtcHint = el("tNtcHint");
const tHint  = el("tHint");

const irmsZscoreVal = el("irmsZscoreVal");
const deltaIrmsVal = el("deltaIrmsVal");
const halfcycleAsymVal = el("halfcycleAsymVal");
const cycleNmseVal = el("cycleNmseVal");
const deltaHfEnergyVal = el("deltaHfEnergyVal");
const vSagPctVal = el("vSagPctVal");
const midbandRatioVal = el("midbandRatioVal");

const zcvVal = el("zcvVal");
const spectralFluxVal = el("spectralFluxVal");
const peakFluctVal = el("peakFluctVal");
const residualCrestVal = el("residualCrestVal");
const thdIVal = el("thdIVal");
const hfEnergyDeltaVal = el("hfEnergyDeltaVal");
const edgeSpikeVal = el("edgeSpikeVal");

const alertEnable = el("alertEnable");
const soundEnable = el("soundEnable");

const historyBody = el("historyBody");
const historyFaultBody = el("historyFaultBody");
const historyNormalBody = el("historyNormalBody");
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
const STALE_MS = 22000;
const HISTORY_LIMIT = 1000;

const MAX_RENDER_ROWS = 180;

const OTA_RELEASE_BASE = "https://github.com/ancy-gg/TinyML_Smart-Plug/releases/download/";
const OTA_RELEASE_TAG  = "updates";
const OTA_DEFAULT_BIN  = "";

const DEVICE_FAMILY_OPTIONS = ["unknown", "resistive_linear", "inductive_motor", "rectifier_smps", "phase_angle_controlled", "brush_universal_motor", "other_mixed"];
const DEVICE_FAMILY_CODE_MAP = { unknown: -1, resistive_linear: 0, inductive_motor: 1, rectifier_smps: 2, phase_angle_controlled: 3, brush_universal_motor: 4, other_mixed: 5 };
const DIVISION_TAG_OPTIONS = ["start", "steady", "arc"];
const RUNTIME_CONTEXT_FAMILY_LABELS = {
  [-1]: "Unknown",
  0: "Resistive",
  1: "Inductive",
  2: "SMPS",
  3: "Phase Control",
  4: "Brushed Motor",
  5: "Others"
};

const UI_OVERLOAD_WARN_A = 10;
const UI_SHORT_CIRCUIT_A = 20;
const UI_NORMAL_V_MIN = 200;
const UI_NORMAL_V_MAX = 250;
const UI_UNDERVOLTAGE_MIN_V = 170;
const UI_UNDERVOLTAGE_V = 200;
const UI_OVERVOLTAGE_V = 250;
const UI_OVERVOLTAGE_DELAY_V = 250;
const UI_OVERVOLTAGE_FAST_V = 260;
const UI_MAINS_ABSENT_V = 50;
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
let latchedFaultUi = false;
let pendingDeviceCommand = null;
let lastCommandLatencyMs = null;
let lastDeviceAckTokenSeen = "";
let activeViewedSessionSid = "";

const LS_ALERT = "tsp_alert_enabled";
const LS_SOUND = "tsp_sound_enabled";
const LS_INSTALL_DISMISS = "tsp_install_help_dismissed";
const LS_DENSITY = "tsp_density";
const LS_MODE = "tsp_mode";
const NOTIFIABLE_STATUS_SET = new Set(["DEVICE_DISCONNECTED", "UNPLUGGED", "OVERLOAD", "SUSTAINED_OVERLOAD", "HEATING", "ARCING", "OVERVOLTAGE", "UNDERVOLTAGE"]);

let deferredInstallPrompt = null;

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

function normalizeOtaAssetName(desiredVersion, binName) {
  let name = (binName || "").trim();
  if (!name && isLikelyVersion(desiredVersion)) name = `${desiredVersion}.bin`;
  if (!name) name = "firmware.bin";
  if (!/\.bin$/i.test(name)) name += ".bin";
  return name;
}

function buildRepoFirmwareUrl(desiredVersion, binName) {
  const assetName = normalizeOtaAssetName(desiredVersion, binName);
  return `${OTA_RELEASE_BASE}${encodeURIComponent(OTA_RELEASE_TAG)}/${encodeURIComponent(assetName)}`;
}
function isLikelyVersion(v) {
  return typeof v === "string" && v.trim().length >= 3;
}

function safeFilenameSegment(v, fallback = "session") {
  return String(v || fallback)
    .replace(/\.[a-z0-9]+$/i, "")
    .replace(/[^a-zA-Z0-9_-]+/g, "_")
    .replace(/^_+|_+$/g, "")
    .slice(0, 64) || fallback;
}

function normalizeDeviceFamilyToken(v, fallback = "unknown") {
  const raw = String(v || "").trim().toLowerCase().replace(/[^a-z0-9]+/g, "_").replace(/^_+|_+$/g, "");
  const alias = {
    resistive: "resistive_linear", resistive_linear: "resistive_linear", heater: "resistive_linear", heating: "resistive_linear",
    inductive: "inductive_motor", motor: "inductive_motor", fan: "inductive_motor", inductive_motor: "inductive_motor",
    smps: "rectifier_smps", rectifier: "rectifier_smps", rectifier_smps: "rectifier_smps", charger: "rectifier_smps", adapter: "rectifier_smps",
    dimmer: "phase_angle_controlled", phase: "phase_angle_controlled", dimmer_phase: "phase_angle_controlled", phase_angle: "phase_angle_controlled", phase_angle_controlled: "phase_angle_controlled",
    universal: "brush_universal_motor", universal_motor: "brush_universal_motor", brush: "brush_universal_motor", brush_universal_motor: "brush_universal_motor", vacuum: "brush_universal_motor",
    mixed: "other_mixed", mixed_unknown: "other_mixed", other: "other_mixed", other_mixed: "other_mixed", unknown: "unknown"
  };
  return alias[raw] || (DEVICE_FAMILY_OPTIONS.includes(raw) ? raw : fallback);
}

function deviceFamilyCodeFromToken(v) {
  const token = normalizeDeviceFamilyToken(v, "unknown");
  return Object.prototype.hasOwnProperty.call(DEVICE_FAMILY_CODE_MAP, token) ? DEVICE_FAMILY_CODE_MAP[token] : -1;
}

function normalizeDeviceNameToken(v, fallback = "unknown_device") {
  const raw = String(v || "").trim().replace(/\.[a-z0-9]+$/i, "");
  const token = raw.toLowerCase().replace(/[^a-z0-9]+/g, "_").replace(/^_+|_+$/g, "").slice(0, 64);
  return token || fallback;
}

function normalizeDivisionTagToken(v, fallback = "steady") {
  const raw = String(v || "").trim().toLowerCase().replace(/[^a-z0-9]+/g, "_").replace(/^_+|_+$/g, "");
  const alias = { startup: "start", start: "start", steady: "steady", baseline: "steady", arc: "arc", close: "steady", closing: "steady", end: "steady", ending: "steady" };
  return alias[raw] || fallback;
}

function normalizeNotesText(v) {
  return String(v || "").replace(/\s+/g, " ").trim().slice(0, 160);
}

function pickSessionFamily(meta) {
  return normalizeDeviceFamilyToken(meta?.device_family || meta?.family || meta?.parsed_load_family || meta?.load_family || "unknown");
}

function pickSessionDevice(meta, fallback = "unknown_device") {
  const fromMeta = String(meta?.device_name || meta?.load_type || "").trim();
  if (fromMeta && fromMeta.toLowerCase() !== "uploaded_csv" && fromMeta.toLowerCase() !== "unknown") return normalizeDeviceNameToken(fromMeta, fallback);
  const sourceFile = String(meta?.source_file || "").trim();
  if (sourceFile) return normalizeDeviceNameToken(canonicalDatasetStem(sourceFile, sourceFile), fallback);
  return normalizeDeviceNameToken(fallback, fallback);
}

function structuredDatasetBase(meta, fallback = "session") {
  const family = normalizeDeviceFamilyToken(meta?.device_family || meta?.load_family || meta?.parsed_load_family || "unknown");
  const device = normalizeDeviceNameToken(meta?.device_name || meta?.load_type || canonicalDatasetStem(meta?.source_file || fallback, fallback), fallback);
  const trial = Math.max(1, parseInt(meta?.trial_number || meta?.trial || 1, 10) || 1);
  const division = normalizeDivisionTagToken(meta?.division_tag || meta?.division || "steady");
  return `${family}__${device}__trial_${trial}__${division}`;
}

function buildStructuredDatasetFilename(meta, fallback = "session") {
  return `${structuredDatasetBase(meta, fallback)}.csv`;
}

function familyTitle(v) {
  return titleizeTokenText(String(v || "").replace(/_/g, " "), "Mixed Unknown");
}

function runtimeContextFamilyLabel(code) {
  const n = Number(code);
  if (Number.isFinite(n) && Object.prototype.hasOwnProperty.call(RUNTIME_CONTEXT_FAMILY_LABELS, n)) {
    return RUNTIME_CONTEXT_FAMILY_LABELS[n];
  }
  return "Unknown";
}

function deviceTitle(v) {
  return titleizeTokenText(v, "Unknown Device");
}

function contextDisplayText(meta) {
  return `${familyTitle(pickSessionFamily(meta))} • ${deviceTitle(pickSessionDevice(meta))}`;
}

function deriveRowTimingWindow(rows) {
  const numericSeries = (field) => rows.map((row) => Number(row?.[field])).filter((v) => Number.isFinite(v));
  const epochStart = numericSeries("epoch_ms");
  const frameStart = numericSeries("frame_start_uptime_ms");
  const frameEnd = numericSeries("frame_end_uptime_ms");
  const uptime = numericSeries("uptime_ms");

  if (epochStart.length >= 2) {
    const startMs = epochStart[0];
    const endMs = epochStart[epochStart.length - 1];
    if (endMs > startMs) return { durationS: (endMs - startMs) / 1000, startMs, endMs, sourceSampleRateHz: null };
  }

  const frameSpan = buildContinuousSecondsFromField(rows, "frame_start_uptime_ms", 0.001) || buildContinuousSecondsFromField(rows, "frame_end_uptime_ms", 0.001) || buildContinuousSecondsFromField(rows, "uptime_ms", 0.001);
  const sourceFs = rows.map((row) => Number(row?.source_sample_rate_hz ?? row?.feature_frame_rate_hz ?? row?.adc_fs_hz)).find((v) => Number.isFinite(v) && v > 0) || null;
  if (frameSpan != null) return { durationS: frameSpan, startMs: null, endMs: null, sourceSampleRateHz: sourceFs };
  if (sourceFs && rows.length > 1) return { durationS: Math.max(0, (rows.length - 1) / sourceFs), startMs: null, endMs: null, sourceSampleRateHz: sourceFs };
  if (sourceFs && rows.length > 0) return { durationS: rows.length / sourceFs, startMs: null, endMs: null, sourceSampleRateHz: sourceFs };
  return { durationS: null, startMs: null, endMs: null, sourceSampleRateHz: sourceFs };
}

function titleizeTokenText(v, fallback = "Unknown") {
  const base = String(v || "")
    .replace(/\.[a-z0-9]+$/i, "")
    .replace(/[_-]+/g, " ")
    .replace(/\s+/g, " ")
    .trim();
  if (!base) return fallback;
  return base.replace(/\b\w+/g, (m) => m.charAt(0).toUpperCase() + m.slice(1).toLowerCase());
}

function formatSessionStamp(ms) {
  if (!ms) return "Unknown Time";
  return formatDisplayTimestamp(ms);
}

function sessionDisplayName(meta, sessionId) {
  return deviceTitle(pickSessionDevice(meta, sessionId || "session"));
}

function sessionSecondaryText(meta, sessionId) {
  const trial = Math.max(1, parseInt(meta?.trial_number || 1, 10) || 1);
  const division = titleizeTokenText(normalizeDivisionTagToken(meta?.division_tag || "steady"), "Steady");
  return `Trial ${trial} • ${division}`;
}

function sessionLoadText(meta) {
  return familyTitle(pickSessionFamily(meta));
}

function formatDisplayDateOnly(ms, tz = DISPLAY_TZ) {
  if (!ms || ms <= 0) return "—";
  return new Intl.DateTimeFormat("en-US", {
    timeZone: tz,
    month: "short",
    day: "2-digit",
    year: "numeric"
  }).format(new Date(ms));
}

function formatDisplayTimeOnly(ms, tz = DISPLAY_TZ) {
  if (!ms || ms <= 0) return "—";
  return new Intl.DateTimeFormat("en-US", {
    timeZone: tz,
    hour: "2-digit",
    minute: "2-digit",
    second: "2-digit",
    hour12: true
  }).format(new Date(ms));
}

function formatDisplayTimeRange(startMs, endMs, fallback = "—") {
  const startText = startMs ? formatDisplayTimeOnly(startMs) : "—";
  if (endMs && endMs > 0) return `${startText} - ${formatDisplayTimeOnly(endMs)}`;
  if (startMs) return `${startText} - Running`;
  return fallback;
}

function wifiBarsForRssi(rssi, fresh) {
  if (!fresh || !Number.isFinite(rssi)) return 0;
  if (rssi >= -55) return 4;
  if (rssi >= -67) return 3;
  if (rssi >= -75) return 2;
  if (rssi >= -85) return 1;
  return 0;
}

function formatLatencyMs(ms) {
  if (!Number.isFinite(ms) || ms < 0) return "—";
  return `${Math.round(ms)} ms`;
}

function wifiSignalQuality(bars, fresh) {
  if (!fresh || bars <= 0) return { key: "offline", label: "Offline", bars: 0 };
  if (bars >= 4) return { key: "excellent", label: "Excellent", bars: 4 };
  if (bars === 3) return { key: "strong", label: "Strong", bars: 3 };
  if (bars === 2) return { key: "weak", label: "Weak", bars: 2 };
  return { key: "poor", label: "Poor", bars: 1 };
}

function renderHealthTelemetry() {
  const fresh = liveIsFresh();
  const rssi = Number(lastLiveData?.wifi_rssi);
  const bars = wifiBarsForRssi(rssi, fresh);
  const quality = wifiSignalQuality(bars, fresh);

  if (healthWifiBars) healthWifiBars.className = `wifi-bars wifi-bars-${quality.bars} signal-${quality.key}${fresh ? " is-live" : ""}`;
  if (healthWifiText) healthWifiText.textContent = quality.label;
  if (healthWifiChip) healthWifiChip.className = `health-chip health-chip-wifi signal-${quality.key}`;

  if (healthLatencyText) healthLatencyText.textContent = formatLatencyMs(lastCommandLatencyMs);
  if (healthLatencyChip) healthLatencyChip.classList.toggle("is-offline", !Number.isFinite(lastCommandLatencyMs));
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

let holdHintTimer = null;
let holdHintAnchor = null;
function showHoldHint(target) {
  const msg = String(target?.getAttribute?.("data-hold-hint") || target?.getAttribute?.("title") || "").trim();
  if (!msg || !toastEl) return;
  holdHintAnchor = target;
  toast(msg, "info");
}
function clearHoldHint() {
  if (holdHintTimer) {
    clearTimeout(holdHintTimer);
    holdHintTimer = null;
  }
  holdHintAnchor = null;
}
document.addEventListener("pointerdown", (ev) => {
  const target = ev.target?.closest?.("[data-hold-hint]");
  if (!target) { clearHoldHint(); return; }
  clearHoldHint();
  holdHintTimer = setTimeout(() => {
    showHoldHint(target);
    holdHintTimer = null;
  }, 380);
});
document.addEventListener("pointerup", clearHoldHint);
document.addEventListener("pointercancel", clearHoldHint);
document.addEventListener("pointermove", (ev) => {
  if (!holdHintAnchor) return;
  const current = ev.target?.closest?.("[data-hold-hint]");
  if (current !== holdHintAnchor) clearHoldHint();
});

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

function formatDisplayTimestamp(ms, tz = DISPLAY_TZ) {
  if (!ms || ms <= 0) return "—";
  const d = new Date(ms);
  const datePart = new Intl.DateTimeFormat("en-US", {
    timeZone: tz,
    month: "long",
    day: "2-digit",
    year: "numeric"
  }).format(d);
  const timePart = new Intl.DateTimeFormat("en-US", {
    timeZone: tz,
    hour: "2-digit",
    minute: "2-digit",
    second: "2-digit",
    hour12: true
  }).format(d);
  return `${datePart} | ${timePart}`;
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
const relayToggle = el("relayToggle");
const btnFaultCleared = el("btnFaultCleared");
let INTERNAL_RELAY_TOGGLE_UPDATE = false;
let relayBusyReleaseTimer = null;

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

const CONTROL_PULSE_DEBOUNCE_MS = 1500;


function deriveRelayLatchedOn(data = lastLiveData) {
  if (!data || typeof data !== "object") return false;

  if (pendingDeviceCommand && Number.isFinite(pendingDeviceCommand.localSentMs)) {
    const pendingAgeMs = Date.now() - pendingDeviceCommand.localSentMs;
    if (pendingAgeMs >= 0 && pendingAgeMs <= Math.max(2600, CONTROL_PULSE_DEBOUNCE_MS + 1200)) {
      if (pendingDeviceCommand.kind === "relay_on") return true;
      if (pendingDeviceCommand.kind === "relay_off") return false;
    }
  }

  if (typeof data.relay_latched_on === "boolean") return data.relay_latched_on;
  const numeric = Number(data.relay_latched_on);
  if (Number.isFinite(numeric)) return numeric > 0.5;

  const pulseKind = String(data.last_relay_pulse_kind || data.last_control_ack_kind || "").trim().toLowerCase();
  if (pulseKind === "relay_on") return true;
  if (pulseKind === "relay_off") return false;
  return false;
}

function syncRelayRockerVisualState(data = lastLiveData) {
  const isOn = deriveRelayLatchedOn(data);
  relayRocker?.classList.toggle("is-on", isOn);
  relayRocker?.classList.toggle("is-off", !isOn);
  if (relayToggle) {
    INTERNAL_RELAY_TOGGLE_UPDATE = true;
    relayToggle.checked = isOn;
    INTERNAL_RELAY_TOGGLE_UPDATE = false;
  }
}

function setRelayBusy(isBusy) {
  if (relayBusyReleaseTimer) {
    clearTimeout(relayBusyReleaseTimer);
    relayBusyReleaseTimer = null;
  }
  relayRocker?.classList.toggle("is-busy", !!isBusy);
  if (isBusy) {
    relayBusyReleaseTimer = setTimeout(() => {
      relayBusyReleaseTimer = null;
      relayRocker?.classList.remove("is-busy");
      updateRelayControls();
    }, Math.max(1800, CONTROL_PULSE_DEBOUNCE_MS + 300));
  }
  updateRelayControls();
}

async function sendRelayPulse(nextOn) {
  if (relayControlsLocked()) {
    syncRelayRockerVisualState(lastLiveData);
    updateRelayControls();
    return;
  }
  const now = nowWithServerOffsetMs();
  if (relayToggle?._tspBusyUntil && now < relayToggle._tspBusyUntil) {
    syncRelayRockerVisualState(lastLiveData);
    return;
  }

  const token = `${nextOn ? "relay_on" : "relay_off"}_${Date.now()}`;
  const payload = nextOn
    ? {
        relay_off_token: "",
        relay_on_token: token,
        relay_on_requested_at: firebase.database.ServerValue.TIMESTAMP
      }
    : {
        relay_on_token: "",
        relay_off_token: token,
        relay_off_requested_at: firebase.database.ServerValue.TIMESTAMP
      };
  const requestPath = nextOn ? "/controls/relay_on_requested_at" : "/controls/relay_off_requested_at";

  if (relayToggle) {
    relayToggle._tspBusyUntil = now + CONTROL_PULSE_DEBOUNCE_MS;
    relayToggle.disabled = true;
  }
  setRelayBusy(true);

  try {
    await db.ref("/controls").update(payload);
    let requestMs = null;
    try {
      const reqSnap = await db.ref(requestPath).get();
      const reqNum = Number(reqSnap.val());
      if (Number.isFinite(reqNum) && reqNum > 0) requestMs = reqNum;
    } catch (readErr) {
      console.warn("Failed to read relay request timestamp", readErr);
    }
    pendingDeviceCommand = {
      kind: nextOn ? "relay_on" : "relay_off",
      token,
      requestMs,
      localSentMs: nowWithServerOffsetMs()
    };
    toast(nextOn ? "Relay ON pulse sent." : "Relay OFF pulse sent.", "ok");
  } catch (e) {
    console.error(e);
    syncRelayRockerVisualState(lastLiveData);
    setRelayBusy(false);
    toast(nextOn ? "Failed to send Relay ON." : "Failed to send Relay OFF.", "err");
  } finally {
    setTimeout(() => {
      if (relayToggle) relayToggle.disabled = relayControlsLocked();
    }, CONTROL_PULSE_DEBOUNCE_MS);
  }
}

async function sendControlPulse(buttonEl, busyText, okText, errText, payload, ackInfo = null) {
  setHeaderMenuOpen(false);
  const now = nowWithServerOffsetMs();
  if (buttonEl && buttonEl._tspBusyUntil && now < buttonEl._tspBusyUntil) return;
  const oldText = buttonEl?.textContent || "";
  if (buttonEl) {
    buttonEl._tspBusyUntil = now + CONTROL_PULSE_DEBOUNCE_MS;
    buttonEl.disabled = true;
    buttonEl.textContent = busyText;
  }
  try {
    await db.ref("/controls").update(payload);
    if (ackInfo?.kind && ackInfo?.token && ackInfo?.requestPath) {
      let requestMs = null;
      try {
        const reqSnap = await db.ref(ackInfo.requestPath).get();
        const reqNum = Number(reqSnap.val());
        if (Number.isFinite(reqNum) && reqNum > 0) requestMs = reqNum;
      } catch (readErr) {
        console.warn("Failed to read control request timestamp", readErr);
      }
      pendingDeviceCommand = {
        kind: ackInfo.kind,
        token: ackInfo.token,
        requestMs,
        localSentMs: nowWithServerOffsetMs()
      };
    }
    toast(okText, "ok");
  } catch (e) {
    console.error(e);
    toast(errText, "err");
  } finally {
    if (buttonEl) {
      setTimeout(() => {
        buttonEl.disabled = false;
        buttonEl.textContent = oldText;
      }, CONTROL_PULSE_DEBOUNCE_MS);
    }
  }
}

btnChangeWifi?.addEventListener("click", async () => {
  setHeaderMenuOpen(false);

  const now = Date.now();
  if (btnChangeWifi._tspBusyUntil && now < btnChangeWifi._tspBusyUntil) return;

  const oldText = btnChangeWifi.textContent;
  btnChangeWifi._tspBusyUntil = now + CONTROL_PULSE_DEBOUNCE_MS;
  btnChangeWifi.disabled = true;
  btnChangeWifi.textContent = "Opening...";

  try {
    const token = `portal_${Date.now()}`;
    await db.ref("/controls").update({
      open_portal_token: token,
      open_portal_requested_at: firebase.database.ServerValue.TIMESTAMP
    });
    pendingDeviceCommand = {
      kind: "open_portal",
      token,
      requestMs: Date.now(),
      localSentMs: Date.now()
    };
    console.log("Change WiFi requested:", token);
    toast("Hotspot request sent. AP will open for 15s.", "ok");
  } catch (e) {
    console.error("Change WiFi write failed:", e);
    toast("Failed to request WiFi change.", "err");
  } finally {
    setTimeout(() => {
      btnChangeWifi.disabled = false;
      btnChangeWifi.textContent = oldText || "Change WiFi";
    }, CONTROL_PULSE_DEBOUNCE_MS);
  }
});

btnRevertFirmware?.addEventListener("click", async () => {
  setHeaderMenuOpen(false);
  const now = Date.now();
  if (btnRevertFirmware._tspBusyUntil && now < btnRevertFirmware._tspBusyUntil) return;
  const confirmed = window.confirm("Reboot into the previous OTA firmware slot? Use this when the current firmware OTA is broken.");
  if (!confirmed) return;

  const oldText = btnRevertFirmware.textContent || "Revert Firmware";
  btnRevertFirmware._tspBusyUntil = now + CONTROL_PULSE_DEBOUNCE_MS;
  btnRevertFirmware.disabled = true;
  btnRevertFirmware.textContent = "Reverting...";

  try {
    const token = `revert_fw_${Date.now()}`;
    await db.ref("/controls").update({
      revert_fw_token: token,
      revert_fw_requested_at: firebase.database.ServerValue.TIMESTAMP
    });
    pendingDeviceCommand = {
      kind: "revert_fw",
      token,
      requestMs: Date.now(),
      localSentMs: Date.now()
    };
    toast("Firmware revert requested. Device will reboot into the previous OTA slot if available.", "warn");
  } catch (e) {
    console.error("Firmware revert write failed:", e);
    toast("Failed to request firmware revert.", "err");
  } finally {
    setTimeout(() => {
      btnRevertFirmware.disabled = false;
      btnRevertFirmware.textContent = oldText;
    }, CONTROL_PULSE_DEBOUNCE_MS);
  }
});

btnRelayOn?.addEventListener("click", async () => {
  await sendRelayPulse(true);
});

btnRelayOff?.addEventListener("click", async () => {
  await sendRelayPulse(false);
});

relayToggle?.addEventListener("change", async () => {
  if (INTERNAL_RELAY_TOGGLE_UPDATE) return;
  await sendRelayPulse(!!relayToggle.checked);
});

btnFaultCleared?.addEventListener("click", async () => {
  const oldText = btnFaultCleared.textContent || "";
  btnFaultCleared.disabled = true;
  btnFaultCleared.textContent = "Clearing...";
  try {
    const token = `fault_clear_${Date.now()}`;
    await db.ref("/controls").update({
      fault_clear_token: token,
      fault_clear_requested_at: firebase.database.ServerValue.TIMESTAMP
    });
    pendingDeviceCommand = {
      kind: "fault_clear",
      token,
      requestMs: Date.now(),
      localSentMs: Date.now()
    };
    latchedFaultUi = false;
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
    [vVal, "0.0"], [iVal, "0.000"], [tNtcVal, "0.0"], [tVal, "0.0"], [pVal, "0.0"],
    [irmsZscoreVal, "0.000"], [deltaIrmsVal, "0.000"], [halfcycleAsymVal, "0.000"],
    [cycleNmseVal, "0.000"], [deltaHfEnergyVal, "0.000"], [vSagPctVal, "0.00"],
    [midbandRatioVal, "0.000"], [zcvVal, "0.000"], [spectralFluxVal, "0.000"],
    [peakFluctVal, "0.000"], [residualCrestVal, "0.000"], [thdIVal, "0.000"],
    [hfEnergyDeltaVal, "0.000"], [edgeSpikeVal, "0.000"]
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
  if (raw === "UNPLUGGED") return "UNPLUGGED";
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

function isLiveLoadDetected(data) {
  if (typeof data?.load_detected === "boolean") return data.load_detected;
  const numeric = Number(data?.load_detected);
  if (Number.isFinite(numeric)) return numeric > 0.5;
  return isLiveLoadActive(data);
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
    vVal, iVal, tNtcVal, tVal, pVal,
    irmsZscoreVal, deltaIrmsVal, halfcycleAsymVal,
    cycleNmseVal, deltaHfEnergyVal, vSagPctVal,
    midbandRatioVal, zcvVal, spectralFluxVal,
    peakFluctVal, residualCrestVal, thdIVal, hfEnergyDeltaVal, edgeSpikeVal
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
    if (tNtcHint) tNtcHint.textContent = "—";
    if (tHint) tHint.textContent = "—";
    return;
  }

  const i = Number(data?.current ?? 0);
  const p = Number(data?.apparent_power ?? 0);
  const t = Number(data?.estimated_socket_temp ?? data?.temp ?? 0);
  const powerKind = effectivePowerCondition();

  if (vHint) {
    const vNow = Number(data?.voltage ?? 0);
    if (powerKind === "UNPLUGGED") vHint.textContent = "No mains.";
    else if (powerKind === "UNDERVOLTAGE") vHint.textContent = `${UI_UNDERVOLTAGE_MIN_V}-${UI_NORMAL_V_MIN - 1} V staged`;
    else if (powerKind === "OVERVOLTAGE") vHint.textContent = `>${UI_NORMAL_V_MAX} V staged`;
    else if (vNow < UI_MAINS_ABSENT_V) vHint.textContent = "0 V pending unplugged";
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

  if (tNtcHint) {
    tNtcHint.textContent = "Raw thermistor";
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
  const lastEventTime = latestHistoryRecord ? formatDisplayTimestamp(getRecordEpochMs(latestHistoryRecord)) : "—";
  const loadActive = fresh && isLiveLoadActive(live);
  const loadDetected = fresh && isLiveLoadDetected(live);
  const faultLatched = !!live.fault_latched || !!live.web_controls_locked;
  if (fresh && (["ARCING", "HEATING", "SUSTAINED_OVERLOAD", "UNDERVOLTAGE", "OVERVOLTAGE"].includes(status) || faultLatched)) latchedFaultUi = true;

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

  const contextLatched = !!live.context_latched;
  const contextAcquiring = !!live.context_acquiring;
  const runtimeFamily = runtimeContextFamilyLabel(live.context_family_code_runtime);
  const provisionalFamily = runtimeContextFamilyLabel(live.context_family_code_provisional);
  const runtimeConfidence = Number(live.context_family_confidence);
  const provisionalConfidence = Number(live.context_family_confidence_provisional);
  const loadOnSinceEpochMs = Number(live.load_on_since_epoch_ms || 0);
  const loadOnDurationMsRaw = Number(live.load_on_duration_ms || 0);
  const loadOnTimerMs = (fresh && loadDetected)
    ? (loadOnSinceEpochMs > 0
        ? Math.max(0, nowWithServerOffsetMs() - loadOnSinceEpochMs)
        : Math.max(0, loadOnDurationMsRaw))
    : 0;

  if (ovLoadFamily) {
    if (!fresh) ovLoadFamily.textContent = "Offline";
    else if (!loadDetected) ovLoadFamily.textContent = "No Load";
    else if (contextLatched) ovLoadFamily.textContent = runtimeFamily;
    else ovLoadFamily.textContent = "Identifying...";
  }
  if (ovLoadFamilySub) {
    if (!fresh) ovLoadFamilySub.textContent = "No live link.";
    else if (!loadDetected) ovLoadFamilySub.textContent = "No active load detected.";
    else if (contextLatched) {
      ovLoadFamilySub.textContent = Number.isFinite(runtimeConfidence) && runtimeConfidence > 0
        ? `Latched | ${Math.round(runtimeConfidence * 100)}% confidence`
        : "Latched from context model.";
    } else if (contextAcquiring && Number.isFinite(provisionalConfidence) && provisionalConfidence > 0 && provisionalFamily !== "Unknown") {
      ovLoadFamilySub.textContent = `Identifying | provisional ${provisionalFamily} ${Math.round(provisionalConfidence * 100)}%`;
    } else if (contextAcquiring) {
      ovLoadFamilySub.textContent = "Context model is acquiring.";
    } else {
      ovLoadFamilySub.textContent = "Waiting for context frames.";
    }
    const pieces = [ovLoadFamilySub.textContent];
    ovConnectivitySub.textContent = pieces.join(" • ") || "No live link.";
  }

  if (ovLoadFamilySub && ovConnectivitySub && loadOnTimerMs > 0) {
    ovConnectivitySub.textContent = `${ovLoadFamilySub.textContent} | On for ${formatElapsedClock(loadOnTimerMs, false)}`;
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
  renderHealthTelemetry();
}

function renderTopState() {
  const status = effectiveStatusKind();
  setTopStatus(status);
  const ts = effectiveTimestamp();
  if (lastUpdateText) lastUpdateText.textContent = ts ? formatDisplayTimestamp(ts) : "—";
  updateFreshnessText();
  renderOverview();
  renderHealthTelemetry();
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
  const liveLock = !!lastLiveData?.fault_latched || !!lastLiveData?.web_controls_locked;
  return !liveIsFresh() || liveLock || latchedFaultUi || ["ARCING", "HEATING", "OVERLOAD", "SUSTAINED_OVERLOAD", "UNDERVOLTAGE", "OVERVOLTAGE", "UNPLUGGED", "DEVICE_DISCONNECTED", "SAFE_MODE", "STARTUP_STABILIZING", "OTA_UPDATING", "CONFIG_PORTAL", "WIFI_CONNECTING"].includes(k);
}

function updateRelayControls() {
  const locked = relayControlsLocked();
  const busy = relayRocker?.classList.contains("is-busy");
  relayRocker?.classList.toggle("is-disabled", locked || !!busy);
  if (btnRelayOn) btnRelayOn.disabled = locked || !!busy;
  if (btnRelayOff) btnRelayOff.disabled = locked || !!busy;
  if (relayToggle) relayToggle.disabled = locked || !!busy;
  if (btnFaultCleared) btnFaultCleared.disabled = !liveIsFresh();
}

function applyHistoryFilter() {
  const key = rangeSelect?.value || "7d";
  const { start, end } = getRangeBounds(key);
  const filtered = historyCache.filter(r => {
    const epoch = getRecordEpochMs(r);
    return epoch && epoch >= start && epoch < end;
  }).sort((a, b) => getRecordEpochMs(b) - getRecordEpochMs(a));

  currentFilteredHistory = filtered.slice();
  if (historyHint) historyHint.textContent = `Showing: ${rangeLabel(key)} (${filtered.length})`;

  const faultSet = new Set(["ARCING","HEATING","OVERLOAD","SUSTAINED_OVERLOAD","OVERVOLTAGE","UNDERVOLTAGE","UNPLUGGED","DEVICE_DISCONNECTED","SAFE_MODE"]);
  const renderRows = (rows, colspan = 2) => rows.length ? rows.map((r, idx) => {
    const epoch = getRecordEpochMs(r);
    const timeStr = formatDisplayTimestamp(epoch);
    return `<tr class="row-in" style="animation-delay:${Math.min(idx, 10) * 25}ms"><td class="mono">${timeStr}</td><td>${pillHTML(r.status)}</td></tr>`;
  }).join("") : `<tr><td colspan="${colspan}" class="muted">No history in this range.</td></tr>`;

  if (historyFaultBody && historyNormalBody) {
    const faultRows = filtered.filter((r) => faultSet.has(classifyStatus(r.status || r.power_condition || "NORMAL"))).slice(0, MAX_RENDER_ROWS);
    const normalRows = filtered.filter((r) => !faultSet.has(classifyStatus(r.status || r.power_condition || "NORMAL"))).slice(0, MAX_RENDER_ROWS);
    historyFaultBody.innerHTML = renderRows(faultRows, 2);
    historyNormalBody.innerHTML = renderRows(normalRows, 2);
    return;
  }

  if (!filtered.length) {
    historyBody.innerHTML = `<tr><td colspan="6" class="muted">No history in this range.</td></tr>`;
    return;
  }
  historyBody.innerHTML = filtered.slice(0, MAX_RENDER_ROWS).map((r, idx) => {
    const epoch = getRecordEpochMs(r);
    const timeStr = formatDisplayTimestamp(epoch);
    const durStr = formatDurationMs(historyDurationMsDesc(filtered, idx));
    return `<tr class="row-in" style="animation-delay:${Math.min(idx, 10) * 25}ms"><td class="mono">${timeStr}</td><td>${pillHTML(r.status)}</td><td class="mono">${toFixedOrDash(r.voltage, 1)}</td><td class="mono">${toFixedOrDash(r.current, 3)}</td><td class="mono">${toFixedOrDash(r.temp, 1)}</td><td class="mono">${durStr}</td></tr>`;
  }).join("");
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

  const dismissed = loadBool(LS_INSTALL_DISMISS, false);
  const installed = isStandalone || window.matchMedia?.("(display-mode: standalone)")?.matches;
  const showIOSHelp = isIOS && !installed;
  const showGenericHelp = !isIOS && !installed;
  const canPromptInstall = !!deferredInstallPrompt && !isIOS && !installed;

  if (dismissed || installed || !(showIOSHelp || showGenericHelp)) {
    installHelp.classList.add("hidden");
    btnInstallApp?.classList.add("hidden");
    return;
  }

  if (installCopy) {
    if (showIOSHelp) installCopy.textContent = "On iPhone or iPad, tap Share then Add to Home Screen.";
    else if (canPromptInstall) installCopy.textContent = "Install this dashboard for a faster full-screen app experience.";
    else installCopy.textContent = "Use your browser menu and choose Install App to add this dashboard to your device.";
  }

  btnInstallApp?.classList.toggle("hidden", !canPromptInstall);
  installHelp.classList.remove("hidden");
}

btnInstallApp?.addEventListener("click", async () => {
  if (!deferredInstallPrompt) {
    toast("Use your browser menu and choose Install App.", "warn");
    return;
  }

  try {
    deferredInstallPrompt.prompt();
    const choice = await deferredInstallPrompt.userChoice;
    if (choice?.outcome === "accepted") {
      toast("Install prompt opened.", "ok");
    }
  } catch (e) {
    console.error(e);
    toast("Install prompt failed to open.", "err");
  } finally {
    deferredInstallPrompt = null;
    updateInstallHelp();
  }
});

btnDismissInstall?.addEventListener("click", () => {
  saveBool(LS_INSTALL_DISMISS, true);
  installHelp?.classList.add("hidden");
});
btnRefreshNow?.addEventListener("click", () => window.location.reload());
updateInstallHelp();

function formatFeatureValue(key, value) {
  const schema = window.TinyMLFeatureSchema;
  if (schema?.formatFeatureValue) return schema.formatFeatureValue(key, value, "—");
  const n = Number(value);
  if (!Number.isFinite(n)) return "—";
  return n.toFixed(3);
}

function updateLiveDom(data) {
  const v   = toFixedOrDash(data.voltage, 1);
  const i   = toFixedOrDash(data.current, 3);
  const p   = toFixedOrDash(data.apparent_power, 1);
  const tNtc = toFixedOrDash(data.temp_ntc ?? data.temp_ntc_c, 1);
  const t   = toFixedOrDash(data.estimated_socket_temp ?? data.temp, 1);

  const iz  = formatFeatureValue("abs_irms_zscore_vs_baseline", data.abs_irms_zscore_vs_baseline);
  const di  = formatFeatureValue("delta_irms_abs", data.delta_irms_abs);
  const ha  = formatFeatureValue("halfcycle_asymmetry", data.halfcycle_asymmetry);
  const dfl = formatFeatureValue("low_current_ratio", data.low_current_ratio);
  const plc = formatFeatureValue("pulse_count_per_cycle", data.pulse_count_per_cycle);
  const mlr = formatFeatureValue("max_low_current_run_ms", data.max_low_current_run_ms);
  const mrr = formatFeatureValue("midband_residual_ratio", data.midband_residual_ratio);

  const z   = formatFeatureValue("zcv", data.zcv);
  const sf  = formatFeatureValue("spectral_flux_midhf", data.spectral_flux_midhf);
  const pf  = formatFeatureValue("peak_fluct_cv", data.peak_fluct_cv);
  const rcf = formatFeatureValue("residual_crest_factor", data.residual_crest_factor);
  const thd = formatFeatureValue("thd_i", data.thd_i);
  const hfd = formatFeatureValue("hf_energy_delta", data.hf_energy_delta);
  const esr = formatFeatureValue("edge_spike_ratio", data.edge_spike_ratio);

  if (vVal && vVal.textContent !== v) { vVal.textContent = v; animateNumber(vVal); }
  if (iVal && iVal.textContent !== i) { iVal.textContent = i; animateNumber(iVal); }
  if (tNtcVal && tNtcVal.textContent !== tNtc) { tNtcVal.textContent = tNtc; animateNumber(tNtcVal); }
  if (tVal && tVal.textContent !== t) { tVal.textContent = t; animateNumber(tVal); }
  if (pVal && pVal.textContent !== p) { pVal.textContent = p; animateNumber(pVal); }

  if (irmsZscoreVal && irmsZscoreVal.textContent !== iz) { irmsZscoreVal.textContent = iz; animateNumber(irmsZscoreVal); }
  if (deltaIrmsVal && deltaIrmsVal.textContent !== di) { deltaIrmsVal.textContent = di; animateNumber(deltaIrmsVal); }
  if (halfcycleAsymVal && halfcycleAsymVal.textContent !== ha) { halfcycleAsymVal.textContent = ha; animateNumber(halfcycleAsymVal); }
  if (cycleNmseVal && cycleNmseVal.textContent !== plc) { cycleNmseVal.textContent = plc; animateNumber(cycleNmseVal); }
  if (deltaHfEnergyVal && deltaHfEnergyVal.textContent !== mlr) { deltaHfEnergyVal.textContent = mlr; animateNumber(deltaHfEnergyVal); }
  if (vSagPctVal && vSagPctVal.textContent !== dfl) { vSagPctVal.textContent = dfl; animateNumber(vSagPctVal); }
  if (midbandRatioVal && midbandRatioVal.textContent !== mrr) { midbandRatioVal.textContent = mrr; animateNumber(midbandRatioVal); }

  if (zcvVal && zcvVal.textContent !== z) { zcvVal.textContent = z; animateNumber(zcvVal); }
  if (spectralFluxVal && spectralFluxVal.textContent !== sf) { spectralFluxVal.textContent = sf; animateNumber(spectralFluxVal); }
  if (peakFluctVal && peakFluctVal.textContent !== pf) { peakFluctVal.textContent = pf; animateNumber(peakFluctVal); }
  if (residualCrestVal && residualCrestVal.textContent !== rcf) { residualCrestVal.textContent = rcf; animateNumber(residualCrestVal); }
  if (thdIVal && thdIVal.textContent !== thd) { thdIVal.textContent = thd; animateNumber(thdIVal); }
  if (hfEnergyDeltaVal && hfEnergyDeltaVal.textContent !== hfd) { hfEnergyDeltaVal.textContent = hfd; animateNumber(hfEnergyDeltaVal); }
  if (edgeSpikeVal && edgeSpikeVal.textContent !== esr) { edgeSpikeVal.textContent = esr; animateNumber(edgeSpikeVal); }

  applyMetricHints(data);
}

db.ref("live_data").on("value", (snap) => {
  const data = snap.val();
  if (!data) return;

  syntheticDisconnectActive = false;
  lastReceiptMs = Date.now();
  lastSeenMs = getRecordEpochMs(data) || lastSeenMs || lastReceiptMs;
  lastLiveData = data;
  syncRelayRockerVisualState(data);
  if (!data.relay_pulse_active && !pendingDeviceCommand) setRelayBusy(false);

  const ackToken = String(data.last_control_ack_token || "").trim();
  const ackKind = String(data.last_control_ack_kind || "").trim();
  const ackServerMs = Number(data.last_control_ack_server_ts || 0);
  if (ackToken && ackToken !== lastDeviceAckTokenSeen) {
    lastDeviceAckTokenSeen = ackToken;
    if (pendingDeviceCommand && pendingDeviceCommand.token === ackToken && (!pendingDeviceCommand.kind || pendingDeviceCommand.kind === ackKind)) {
      if (Number.isFinite(pendingDeviceCommand.requestMs) && pendingDeviceCommand.requestMs > 0 && Number.isFinite(ackServerMs) && ackServerMs > 0) {
        lastCommandLatencyMs = Math.max(0, ackServerMs - pendingDeviceCommand.requestMs);
      } else if (Number.isFinite(pendingDeviceCommand.localSentMs)) {
        lastCommandLatencyMs = Math.max(0, Date.now() - pendingDeviceCommand.localSentMs);
      }
      pendingDeviceCommand = null;
    }
    if (pendingLoggerStart && ackKind === "ml_log_enable" && pendingLoggerStart.token === ackToken && Number.isFinite(ackServerMs) && ackServerMs > 0) {
      startLoggerCountdown(ackServerMs, Number(pendingLoggerStart.durationS || mlLogDur?.value || 0));
      db.ref(`ml_sessions/${pendingLoggerStart.sid}`).update({ start_ms: ackServerMs, start_ms_device_ack: ackServerMs }).catch(() => {});
      db.ref("ml_log").update({ start_ms_device_ack: ackServerMs }).catch(() => {});
      if (mlLogStatus) mlLogStatus.textContent = `Logging active. Device acknowledged session: ${pendingLoggerStart.sid}`;
      pendingLoggerStart = null;
    }
  }
  if (ackToken && (ackKind === "relay_on" || ackKind === "relay_off")) setRelayBusy(false);

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
  const btnOtaDownload = el("btnOtaDownload");
  const otaDesiredVer = el("otaDesiredVer");
  const otaFirmwareUrl = el("otaFirmwareUrl");
  const btnFillRepoUrl = el("btnFillRepoUrl");
  const btnPublishOta = el("btnPublishOta");
  if (!btnPublishOta || !otaDesiredVer) return;

  db.ref("ota").on("value", (snap) => {
    const v = snap.val() || {};
    if (otaCurVer) otaCurVer.textContent = (v.desired_version || "—").toString();
    const url = (v.firmware_url || "").toString().trim();
    if (otaCurUrl) otaCurUrl.textContent = url || "—";
    if (btnOtaDownload) {
      if (isHttpsUrl(url)) {
        btnOtaDownload.setAttribute("href", url);
        btnOtaDownload.removeAttribute("aria-disabled");
        btnOtaDownload.classList.remove("is-disabled");
      } else {
        btnOtaDownload.setAttribute("href", "#");
        btnOtaDownload.setAttribute("aria-disabled", "true");
        btnOtaDownload.classList.add("is-disabled");
      }
    }
  });

  btnOtaDownload?.addEventListener("click", (ev) => {
    if (btnOtaDownload.getAttribute("aria-disabled") === "true") ev.preventDefault();
  });

  btnFillRepoUrl?.addEventListener("click", () => {
    const desired = (otaDesiredVer?.value || "").trim();
    const assetName = normalizeOtaAssetName(desired, OTA_DEFAULT_BIN);
    const url = buildRepoFirmwareUrl(desired, assetName);
    if (otaFirmwareUrl) otaFirmwareUrl.value = url;
    toast("OTA version prepared.", "ok");
  });

  btnPublishOta.addEventListener("click", async () => {
    const desired = (otaDesiredVer.value || "").trim();
    let url = (otaFirmwareUrl?.value || "").trim();
    if (!url) {
      const assetName = normalizeOtaAssetName(desired, OTA_DEFAULT_BIN);
      url = buildRepoFirmwareUrl(desired, assetName);
      if (otaFirmwareUrl) otaFirmwareUrl.value = url;
    }
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
      const publishTs = Date.now();
      try {
        const u = new URL(url);
        u.searchParams.set("v", desired);
        u.searchParams.set("t", String(publishTs));
        url = u.toString();
      } catch {}

      const currentSnap = await db.ref("ota").get();
      const current = currentSnap.val() || {};
      await Promise.all([
        db.ref("ota").update({
          desired_version: desired,
          firmware_url: url,
          previous_desired_version: (current.desired_version || "").toString(),
          previous_firmware_url: (current.firmware_url || "").toString(),
          published_at: firebase.database.ServerValue.TIMESTAMP
        }),
        db.ref("/controls").update({
          ota_check_token: `ota_check_${publishTs}`,
          ota_check_requested_at: firebase.database.ServerValue.TIMESTAMP
        })
      ]);
      toast("OTA published. Device asked to check now.", "ok");
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
const mlDeviceFamily = el("mlDeviceFamily");
const mlDeviceName = el("mlDeviceName");
const mlTrialNumber = el("mlTrialNumber");
const mlDivisionTag = el("mlDivisionTag");
const mlSessionNotes = el("mlSessionNotes");
const mlTrustedNormal = el("mlTrustedNormal");
const mlLabelOverride = el("mlLabelOverride");
const btnDownloadSessionMl = el("btnDownloadSessionMl");
const btnDownloadAllMl = el("btnDownloadAllMl");
const btnClearMlLogs = el("btnClearMlLogs");
const btnUploadCsv = el("btnUploadCsv");
const mlCsvUpload = el("mlCsvUpload");
const mlLogStatus = el("mlLogStatus");
const mlSessionBody = el("mlSessionBody");
const mlProcessedBody = el("mlProcessedBody");
const mlOriginalPanel = el("mlOriginalPanel");
const mlProcessedPanel = el("mlProcessedPanel");
const btnMlTabOriginal = el("btnMlTabOriginal");
const btnMlTabProcessed = el("btnMlTabProcessed");
const mlCountdown = el("mlCountdown");
let currentSessionId = "";
let lastMlLogEnabled = null;
let activeLoggerTab = "original";
let loggerCountdownTimer = null;
let loggerCountdownEndsAtMs = 0;
let pendingLoggerStart = null;

btnUploadCsv?.addEventListener("click", () => {
  if (!mlCsvUpload) return;
  mlCsvUpload.value = "";
  mlCsvUpload.click();
});

function applyActiveMlSessionRow() {
  if (!mlSessionBody) return;
  mlSessionBody.querySelectorAll("tr[data-session-row]").forEach((row) => {
    row.classList.toggle("is-active-view", !!activeViewedSessionSid && row.getAttribute("data-session-row") === activeViewedSessionSid);
  });
}

document.addEventListener("tsp-active-session-changed", (ev) => {
  activeViewedSessionSid = String(ev?.detail?.sid || "").trim();
  applyActiveMlSessionRow();
});

function setLoggerTab(tab) {
  activeLoggerTab = tab === "processed" ? "processed" : "original";
  mlOriginalPanel?.classList.toggle("hidden", activeLoggerTab !== "original");
  mlProcessedPanel?.classList.toggle("hidden", activeLoggerTab !== "processed");
  btnMlTabOriginal?.classList.toggle("is-active", activeLoggerTab === "original");
  btnMlTabProcessed?.classList.toggle("is-active", activeLoggerTab === "processed");
  if (btnDownloadAllMl) btnDownloadAllMl.textContent = activeLoggerTab === "processed" ? "Download All Processed" : "Download All Original";
}
btnMlTabOriginal?.addEventListener("click", () => setLoggerTab("original"));
btnMlTabProcessed?.addEventListener("click", () => setLoggerTab("processed"));
setLoggerTab("original");

function canonicalDatasetStem(name, fallback = "session") {
  let stem = String(name || "").replace(/\.csv$/i, "").trim();
  if (!stem) return fallback;
  stem = stem.replace(/^TSP_ML_/i, "");
  stem = stem.replace(/^upload_\d+_/i, "");
  stem = stem.replace(/^processed_\d+_/i, "");
  stem = stem.replace(/_(AUTO|ARC|NORMAL)_[0-9]{4}-[0-9]{2}-[0-9]{2}[-_0-9]*$/i, "");
  stem = stem.replace(/__[a-z]+_\d+$/i, "");
  stem = stem.replace(/(?:_|-)(?:19|20)\d{2}(?:[_-]?\d{2}){2}(?:[_-]?\d{2}){1,3}$/i, "");
  stem = stem.replace(/(?:_|-)(?:19|20)\d{2}(?:[_-]?\d{2}){1,5}$/i, "");
  stem = stem.replace(/(?:_|-)processed$/i, "");
  stem = stem.replace(/_+/g, "_").replace(/^_+|_+$/g, "");
  return stem || fallback;
}

function loggerFilename(meta, sessionId, fallback = "session") {
  const base = safeFilenameSegment(structuredDatasetBase(meta || {}, fallback), fallback);
  return `${base}.csv`;
}

function updateLoggerCountdownText() {
  if (!mlCountdown) return;
  const nowMs = nowWithServerOffsetMs();
  if (!loggerCountdownEndsAtMs || loggerCountdownEndsAtMs <= nowMs) {
    mlCountdown.textContent = "—";
    return;
  }
  mlCountdown.textContent = `${Math.max(0, Math.ceil((loggerCountdownEndsAtMs - nowMs) / 1000))}s`;
}

function startLoggerCountdown(startMs, durationS) {
  if (!Number.isFinite(startMs) || !Number.isFinite(durationS) || durationS <= 0) return;
  loggerCountdownEndsAtMs = startMs + (durationS * 1000);
  clearInterval(loggerCountdownTimer);
  loggerCountdownTimer = setInterval(() => {
    updateLoggerCountdownText();
    if (!loggerCountdownEndsAtMs || loggerCountdownEndsAtMs <= nowWithServerOffsetMs()) {
      clearInterval(loggerCountdownTimer);
      loggerCountdownTimer = null;
    }
  }, 250);
  updateLoggerCountdownText();
}

function stopLoggerCountdown() {
  loggerCountdownEndsAtMs = 0;
  clearInterval(loggerCountdownTimer);
  loggerCountdownTimer = null;
  updateLoggerCountdownText();
}

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

function compareMaybeNumberAsc(a, b) {
  const aOk = Number.isFinite(a);
  const bOk = Number.isFinite(b);
  if (aOk && bOk) return a - b;
  if (aOk) return -1;
  if (bOk) return 1;
  return 0;
}

function sortSessionCsvLines(header, lines) {
  const cleanHeader = String(header || "").trim();
  if (!cleanHeader || !Array.isArray(lines) || lines.length < 2) return lines || [];

  const cols = cleanHeader.split(",").map((name) => normalizeUploadedCsvHeader(name));
  const idxFrameStart = cols.indexOf("frame_start_uptime_ms");
  const idxUptime = cols.indexOf("uptime_ms");
  const idxFrameEnd = cols.indexOf("frame_end_uptime_ms");
  const idxFeatureEnd = cols.indexOf("feature_compute_end_uptime_ms");
  const idxEpoch = cols.indexOf("epoch_ms");
  const idxLogEnqueue = cols.indexOf("log_enqueue_uptime_ms");

  const parseNum = (parts, idx) => {
    if (idx < 0 || idx >= parts.length) return NaN;
    const v = Number(parts[idx]);
    return Number.isFinite(v) ? v : NaN;
  };

  return lines
    .map((line, rowIndex) => {
      const parts = String(line || "").split(",");
      return {
        line,
        rowIndex,
        frameStart: parseNum(parts, idxFrameStart),
        uptime: parseNum(parts, idxUptime),
        frameEnd: parseNum(parts, idxFrameEnd),
        featureEnd: parseNum(parts, idxFeatureEnd),
        epoch: parseNum(parts, idxEpoch),
        logEnqueue: parseNum(parts, idxLogEnqueue),
      };
    })
    .sort((a, b) =>
      compareMaybeNumberAsc(a.frameStart, b.frameStart) ||
      compareMaybeNumberAsc(a.uptime, b.uptime) ||
      compareMaybeNumberAsc(a.frameEnd, b.frameEnd) ||
      compareMaybeNumberAsc(a.featureEnd, b.featureEnd) ||
      compareMaybeNumberAsc(a.epoch, b.epoch) ||
      compareMaybeNumberAsc(a.logEnqueue, b.logEnqueue) ||
      (a.rowIndex - b.rowIndex))
    .map((row) => row.line);
}


function normalizeUploadedCsvHeader(name) {
  return String(name || "")
    .trim()
    .toLowerCase()
    .replace(/[^a-z0-9]+/g, "_")
    .replace(/^_+|_+$/g, "");
}

function medianPositiveDiff(values) {
  const diffs = [];
  for (let i = 1; i < values.length; i++) {
    const a = Number(values[i - 1]);
    const b = Number(values[i]);
    const d = b - a;
    if (Number.isFinite(d) && d > 0) diffs.push(d);
  }
  if (!diffs.length) return null;
  diffs.sort((a, b) => a - b);
  return diffs[Math.floor(diffs.length / 2)] || null;
}

function buildContinuousSecondsFromField(rows, field, scale = 0.001) {
  const raw = rows.map((row) => Number(row?.[field])).filter((v) => Number.isFinite(v));
  if (raw.length < 2) return null;
  const medianStep = Math.max(1e-6, ((medianPositiveDiff(raw) || 0) * scale) || 0);
  if (!(medianStep > 0)) return null;
  const gapThreshold = Math.max(medianStep * 4.0, 0.75);
  let total = 0;
  let prev = raw[0];
  for (let i = 1; i < raw.length; i++) {
    const cur = raw[i];
    let dt = (cur - prev) * scale;
    if (!Number.isFinite(dt) || dt <= 0) dt = medianStep;
    if (dt > gapThreshold) dt = medianStep;
    total += dt;
    prev = cur;
  }
  return total > 0 ? total : null;
}

function parseDatasetFilenameMeta(name) {
  const stem = canonicalDatasetStem(name, "uploaded_csv");
  const structured = stem.split("__").filter(Boolean);
  if (structured.length >= 4 && /^trial_?\d+$/i.test(structured[2])) {
    const trial = Math.max(1, parseInt(String(structured[2]).replace(/[^0-9]+/g, ""), 10) || 1);
    return {
      deviceFamily: normalizeDeviceFamilyToken(structured[0]),
      deviceName: normalizeDeviceNameToken(structured[1], structured[1]),
      loadType: normalizeDeviceNameToken(structured[1], structured[1]),
      trial,
      division: normalizeDivisionTagToken(structured[3]),
    };
  }
  const parts = stem.split(/[_\s-]+/).filter(Boolean);
  const aliases = { startup: "start", start: "start", steady: "steady", baseline: "steady", arc: "arc", close: "steady", closing: "steady", end: "steady", ending: "steady" };
  const last = String(parts[parts.length - 1] || "").toLowerCase();
  const division = aliases[last] || "";
  let trial = 1;
  let endIdx = parts.length;
  if (division) endIdx -= 1;
  if (endIdx > 0) {
    const maybeTrial = Number(String(parts[endIdx - 1]).replace(/[^0-9]+/g, ""));
    if (Number.isInteger(maybeTrial) && maybeTrial > 0) {
      trial = maybeTrial;
      endIdx -= 1;
    }
  }
  const device = safeFilenameSegment(parts.slice(0, Math.max(0, endIdx)).join("_"), "uploaded_csv");
  return { deviceFamily: "unknown", deviceName: device, loadType: device, trial, division };
}

function deriveUploadedCsvTimingMeta(csvText) {
  const clean = String(csvText || "").replace(/^\uFEFF/, "").trim();
  if (!clean) return { rowCount: 0, durationS: null, startMs: null, endMs: null, sourceSampleRateHz: null };
  const parsed = Papa.parse(clean, { header: true, dynamicTyping: true, skipEmptyLines: true, transformHeader: normalizeUploadedCsvHeader });
  const rows = (parsed?.data || []).filter((row) => row && Object.keys(row).length);
  const rowCount = rows.length;
  if (!rowCount) return { rowCount: 0, durationS: null, startMs: null, endMs: null, sourceSampleRateHz: null };
  const timing = deriveRowTimingWindow(rows);
  return { rowCount, durationS: timing.durationS, startMs: timing.startMs, endMs: timing.endMs, sourceSampleRateHz: timing.sourceSampleRateHz };
}

function makeSessionId() { return "sess_" + Date.now(); }
function labelText(v) {
  if (String(v) === "1") return "ARC";
  if (String(v) === "0") return "NORMAL";
  return "AUTO";
}
function formatDurationSeconds(seconds) {
  const sec = Number(seconds);
  if (!Number.isFinite(sec) || sec < 0) return "—";
  const rounded = Math.round(sec);
  if (Math.abs(sec - rounded) < 0.05) return `${rounded}s`;
  return `${sec.toFixed(1)}s`;
}
function uploadedCsvDurationSeconds(meta) {
  const explicit = Number(meta?.source_duration_s ?? meta?.duration_s);
  if (Number.isFinite(explicit) && explicit > 0) return explicit;
  const st = Number(meta?.start_ms || 0);
  const en = Number(meta?.end_ms || 0);
  if (st > 0 && en > st) return (en - st) / 1000;
  const rows = Number(meta?.row_count || 0);
  const sourceFs = Number(meta?.source_sample_rate_hz || 0);
  if (rows > 0 && Number.isFinite(sourceFs) && sourceFs > 0) return rows / sourceFs;
  return null;
}
function isUploadedCsvSession(meta) {
  return !!(meta?.uploaded_csv || String(meta?.load_type || "").trim().toLowerCase() === "uploaded_csv");
}
function sessionCaptureStartMs(meta) {
  const startMs = Number(meta?.capture_start_ms || meta?.first_epoch_ms || meta?.start_ms_device_ack || meta?.start_ms || meta?.requested_ms || 0);
  return Number.isFinite(startMs) && startMs > 0 ? startMs : 0;
}
function sessionCaptureEndMs(meta) {
  const endMs = Number(meta?.capture_end_ms || meta?.last_epoch_ms || meta?.end_ms || meta?.saved_at_ms || 0);
  return Number.isFinite(endMs) && endMs > 0 ? endMs : 0;
}
function sessionAcquisitionStartMs(meta) {
  const startMs = Number(meta?.start_ms_device_ack || meta?.start_ms || meta?.requested_ms || 0);
  return Number.isFinite(startMs) && startMs > 0 ? startMs : 0;
}
function sessionAcquisitionEndMs(meta) {
  const endMs = Number(meta?.end_ms || meta?.saved_at_ms || 0);
  return Number.isFinite(endMs) && endMs > 0 ? endMs : 0;
}
function sessionAcquisitionDurationSec(meta) {
  const st = sessionAcquisitionStartMs(meta);
  const en = sessionAcquisitionEndMs(meta);
  if (st > 0 && en >= st) return Math.max(0, (en - st) / 1000);
  return null;
}
function durationText(meta) {
  const measuredSec = Number(meta?.source_duration_s);
  if (Number.isFinite(measuredSec) && measuredSec > 0) return formatDurationSeconds(measuredSec);

  const acquisitionSec = sessionAcquisitionDurationSec(meta);
  if (acquisitionSec !== null && acquisitionSec > 0) return formatDurationSeconds(acquisitionSec);

  if (isUploadedCsvSession(meta) || meta?.processed_csv) {
    const uploadedDur = uploadedCsvDurationSeconds(meta);
    if (uploadedDur !== null) return formatDurationSeconds(uploadedDur);
  }

  const st = sessionAcquisitionStartMs(meta);
  const en = sessionAcquisitionEndMs(meta);
  const configuredSec = Number(meta?.duration_s);

  if (!st && Number.isFinite(configuredSec) && configuredSec > 0) return formatDurationSeconds(configuredSec);
  if (!st) return "—";

  const tailMs = en > 0 ? en : Date.now();
  return formatDurationSeconds(Math.max(0, (tailMs - st) / 1000));
}

async function fetchSessionCsv(sessionId) {
  const snap = await db.ref(`ml_logs/${sessionId}`).get();
  if (!snap.exists()) return "";
  const chunksObj = snap.val() || {};
  const keys = Object.keys(chunksObj);
  const hasMonotonicSessionSeq = keys.every((k) => Number.isFinite(Number(chunksObj[k]?.session_chunk_seq)));
  keys.sort((a, b) => {
    const ax = chunksObj[a] || {};
    const bx = chunksObj[b] || {};

    const aSessionSeq = Number(ax.session_chunk_seq);
    const bSessionSeq = Number(bx.session_chunk_seq);
    if (hasMonotonicSessionSeq && aSessionSeq !== bSessionSeq) return aSessionSeq - bSessionSeq;

    const aFirstUp = Number(ax.first_uptime_ms);
    const bFirstUp = Number(bx.first_uptime_ms);
    if (Number.isFinite(aFirstUp) && Number.isFinite(bFirstUp) && aFirstUp !== bFirstUp) return aFirstUp - bFirstUp;

    const aFirstEpoch = Number(ax.first_epoch_ms);
    const bFirstEpoch = Number(bx.first_epoch_ms);
    if (Number.isFinite(aFirstEpoch) && Number.isFinite(bFirstEpoch) && aFirstEpoch !== bFirstEpoch) return aFirstEpoch - bFirstEpoch;

    const aCreated = Number(ax.created_at || 0);
    const bCreated = Number(bx.created_at || 0);
    if (Number.isFinite(aCreated) && Number.isFinite(bCreated) && aCreated !== bCreated) return aCreated - bCreated;

    const aSeq = Number(ax.chunk_seq);
    const bSeq = Number(bx.chunk_seq);
    if (Number.isFinite(aSeq) && Number.isFinite(bSeq) && aSeq !== bSeq) return aSeq - bSeq;

    const ai = Number(ax.chunk_index);
    const bi = Number(bx.chunk_index);
    if (Number.isFinite(ai) && Number.isFinite(bi) && ai !== bi) return ai - bi;

    return String(a).localeCompare(String(b));
  });
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
  rows = sortSessionCsvLines(header, rows);
  return header + "\n" + rows.join("\n") + "\n";
}

function sessionFilename(meta, sessionId) {
  return loggerFilename(meta, sessionId, "session");
}

if (mlLogEnable) {
  db.ref("ml_log").on("value", (s) => {
    const v = s.val() || {};
    const enabledNow = !!v.enabled;
    mlLogEnable.checked = enabledNow;
    if (typeof v.duration_s === "number" && mlLogDur) mlLogDur.value = String(v.duration_s);
    if (mlLoadType && v.load_type) mlLoadType.value = v.load_type;
    if (mlDeviceFamily && v.device_family) mlDeviceFamily.value = normalizeDeviceFamilyToken(v.device_family);
    if (mlDeviceName && v.device_name) mlDeviceName.value = v.device_name;
    if (mlTrialNumber && Number(v.trial_number || 0) > 0) mlTrialNumber.value = String(v.trial_number);
    if (mlDivisionTag && v.division_tag) mlDivisionTag.value = normalizeDivisionTagToken(v.division_tag);
    if (mlSessionNotes && v.notes !== undefined) mlSessionNotes.value = String(v.notes || "");
    if (mlTrustedNormal) mlTrustedNormal.checked = !!v.trusted_normal_session;
    if (v.session_id) currentSessionId = v.session_id;

    if (enabledNow && Number(v.start_ms_device_ack || 0) > 0 && Number(v.duration_s || 0) > 0) {
      startLoggerCountdown(Number(v.start_ms_device_ack), Number(v.duration_s));
    }

    if (lastMlLogEnabled === true && !enabledNow) {
      const sid = (v.last_completed_session_id || currentSessionId || "—").toString();
      if (mlLogStatus) mlLogStatus.textContent = `Logging finished by device. Session closed: ${sid}`;
      stopLoggerCountdown();
      pendingLoggerStart = null;
      toast("Logger finished automatically.", "ok");
    }
    lastMlLogEnabled = enabledNow;
  });

  mlLogEnable.addEventListener("change", async () => {
    const enabled = !!mlLogEnable.checked;
    const dur = Math.max(1, parseInt(mlLogDur?.value || "10", 10) || 10);
    const family = normalizeDeviceFamilyToken(mlDeviceFamily?.value || "unknown");
    const device = normalizeDeviceNameToken(mlDeviceName?.value || mlLoadType?.value || `session_${Date.now()}`, "unknown_device");
    const load = device;
    const trial = Math.max(1, parseInt(mlTrialNumber?.value || "1", 10) || 1);
    const division = normalizeDivisionTagToken(mlDivisionTag?.value || "steady");
    const notes = normalizeNotesText(mlSessionNotes?.value || "");
    const trustedNormal = !!(mlTrustedNormal?.checked);

    if (enabled) {
      const sid = makeSessionId();
      const token = `ml_log_enable_${Date.now()}`;
      currentSessionId = sid;
      pendingLoggerStart = { token, sid, durationS: dur };
      stopLoggerCountdown();
      await db.ref("ml_log").update({ enabled: true, duration_s: dur, session_id: sid, load_type: load, device_family: family, device_name: device, trial_number: trial, division_tag: division, notes, trusted_normal_session: trustedNormal ? 1 : 0, request_token: token, requested_at: firebase.database.ServerValue.TIMESTAMP, start_ms_device_ack: null });
      await db.ref(`ml_sessions/${sid}`).set({
        requested_ms: Date.now(),
        start_ms: null,
        start_ms_device_ack: null,
        end_ms: null,
        load_type: load,
        device_family: family,
        device_family_code: deviceFamilyCodeFromToken(family),
        device_name: device,
        trial_number: trial,
        division_tag: division,
        notes,
        trusted_normal_session: trustedNormal ? 1 : 0,
        duration_s: dur,
        source_duration_s: dur,
        source_file: buildStructuredDatasetFilename({ device_family: family, device_name: device, trial_number: trial, division_tag: division }, sid),
        request_token: token,
        label_override: -1
      });
      if (mlLogStatus) mlLogStatus.textContent = `Collect request sent. Waiting for device ack… Session: ${sid}`;
      toast(`Logger request sent for ${dur}s.`, "ok");
    } else {
      await db.ref("ml_log").update({ enabled: false });
      if (mlLogStatus) mlLogStatus.textContent = `Stop request sent. Waiting for device to flush and close the session…`;
      toast("Logger stop requested.", "ok");
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
  const next = normalizeDeviceNameToken(mlLoadType.value || "unknown", "unknown_device");
  await db.ref("ml_log").update({ load_type: next, device_name: next });
});
mlDeviceFamily?.addEventListener("change", async () => {
  const nextFamily = normalizeDeviceFamilyToken(mlDeviceFamily.value || "unknown");
  await db.ref("ml_log").update({ device_family: nextFamily, device_family_code: deviceFamilyCodeFromToken(nextFamily) });
});
mlDeviceName?.addEventListener("change", async () => {
  const next = normalizeDeviceNameToken(mlDeviceName.value || mlLoadType?.value || "unknown", "unknown_device");
  if (mlLoadType) mlLoadType.value = next;
  await db.ref("ml_log").update({ device_name: next, load_type: next });
});
mlTrialNumber?.addEventListener("change", async () => {
  const next = Math.max(1, parseInt(mlTrialNumber.value || "1", 10) || 1);
  mlTrialNumber.value = String(next);
  await db.ref("ml_log").update({ trial_number: next });
});
mlDivisionTag?.addEventListener("change", async () => {
  await db.ref("ml_log").update({ division_tag: normalizeDivisionTagToken(mlDivisionTag.value || "steady") });
});
mlSessionNotes?.addEventListener("change", async () => {
  await db.ref("ml_log").update({ notes: normalizeNotesText(mlSessionNotes.value || "") });
});
mlTrustedNormal?.addEventListener("change", async () => {
  await db.ref("ml_log").update({ trusted_normal_session: mlTrustedNormal.checked ? 1 : 0 });
});

async function fetchStoredCsv(path) {
  const snap = await db.ref(path).get();
  if (!snap.exists()) return "";
  const chunksObj = snap.val() || {};
  const keys = Object.keys(chunksObj).sort((a, b) => String(a).localeCompare(String(b)));
  let header = "";
  let rows = [];
  for (const k of keys) {
    const csv = chunksObj[k]?.csv || "";
    if (!csv) continue;
    const lines = String(csv).split(/\r?\n/).filter((x) => x.trim().length);
    if (!lines.length) continue;
    if (!header) {
      header = lines[0];
      rows.push(...lines.slice(1));
    } else {
      const startIdx = (lines[0].trim() === header.trim()) ? 1 : 0;
      rows.push(...lines.slice(startIdx));
    }
  }
  if (!header) return "";
  rows = sortSessionCsvLines(header, rows);
  return `${header}\n${rows.join("\n")}\n`;
}

function mergeDefinedSessionFields(base = {}, patch = {}) {
  const out = { ...(base || {}) };
  Object.entries(patch || {}).forEach(([key, value]) => {
    if (value === undefined || value === null || value === "") return;
    out[key] = value;
  });
  return out;
}

function deriveSessionMetaFromChunks(sid, chunkSet = {}, kind = "original") {
  const chunkKeys = Object.keys(chunkSet || {});
  if (!chunkKeys.length) return null;

  const chunks = chunkKeys.map((key) => chunkSet[key] || {}).filter(Boolean);
  const sample = chunks.find((chunk) => typeof chunk === "object") || {};
  const createdVals = chunks.map((chunk) => Number(chunk?.created_at || 0)).filter((v) => Number.isFinite(v) && v > 0);
  const requestedMs = createdVals.length ? Math.min(...createdVals) : 0;
  const savedMs = createdVals.length ? Math.max(...createdVals) : requestedMs;

  const parsed = parseDatasetFilenameMeta(sample?.source_file || sid);
  const firstEpochCandidates = chunks.map((chunk) => Number(chunk?.first_epoch_ms || chunk?.capture_start_ms || 0)).filter((v) => Number.isFinite(v) && v > 0);
  const lastEpochCandidates = chunks.map((chunk) => Number(chunk?.last_epoch_ms || chunk?.capture_end_ms || 0)).filter((v) => Number.isFinite(v) && v > 0);
  const sourceDurationCandidates = chunks.map((chunk) => Number(chunk?.meta?.source_duration_s || chunk?.source_duration_s || 0)).filter((v) => Number.isFinite(v) && v > 0);
  const derived = {
    requested_ms: requestedMs || Date.now(),
    saved_at_ms: savedMs || requestedMs || Date.now(),
    start_ms: requestedMs || 0,
    end_ms: 0,
    first_epoch_ms: firstEpochCandidates.length ? Math.min(...firstEpochCandidates) : 0,
    last_epoch_ms: lastEpochCandidates.length ? Math.max(...lastEpochCandidates) : 0,
    capture_start_ms: firstEpochCandidates.length ? Math.min(...firstEpochCandidates) : 0,
    capture_end_ms: lastEpochCandidates.length ? Math.max(...lastEpochCandidates) : 0,
    source_duration_s: sourceDurationCandidates.length ? Math.max(...sourceDurationCandidates) : 0,
    source_file: sample?.source_file || buildStructuredDatasetFilename({
      device_family: sample?.device_family || parsed.deviceFamily || "unknown",
      device_name: sample?.device_name || parsed.deviceName || canonicalDatasetStem(sid, sid),
      trial_number: sample?.trial_number || parsed.trial || 1,
      division_tag: sample?.division_tag || parsed.division || "steady"
    }, sid),
    load_type: sample?.device_name || sample?.load_type || parsed.deviceName || canonicalDatasetStem(sid, sid),
    device_family: sample?.device_family || parsed.deviceFamily || "unknown",
    device_family_code: Number.isFinite(Number(sample?.device_family_code)) ? Number(sample.device_family_code) : deviceFamilyCodeFromToken(sample?.device_family || parsed.deviceFamily || "unknown"),
    device_name: sample?.device_name || parsed.deviceName || canonicalDatasetStem(sid, sid),
    trial_number: Math.max(1, parseInt(sample?.trial_number || parsed.trial || 1, 10) || 1),
    division_tag: sample?.division_tag || parsed.division || "steady",
    notes: normalizeNotesText(sample?.notes || ""),
    trusted_normal_session: Number(sample?.trusted_normal_session || 0) > 0 ? 1 : 0,
    uploaded_csv: kind !== "processed" ? 1 : 0,
    processed_csv: kind === "processed" ? 1 : 0,
    chunk_count: chunks.length,
  };
  return derived;
}

function mergeSessionCollection(metaObj = {}, logObj = {}, kind = "original") {
  const merged = {};
  const ids = new Set([...Object.keys(metaObj || {}), ...Object.keys(logObj || {})]);
  ids.forEach((sid) => {
    const meta = metaObj?.[sid] || {};
    const derived = deriveSessionMetaFromChunks(sid, logObj?.[sid] || {}, kind) || {};
    merged[sid] = mergeDefinedSessionFields(derived, meta);
  });
  return merged;
}

function renderSessionRows(bodyEl, obj, kind = "original") {
  if (!bodyEl) return;
  const ids = Object.keys(obj || {}).sort((a, b) => {
    const av = obj[a] || {};
    const bv = obj[b] || {};
    const at = sessionAcquisitionStartMs(av) || Number(av.saved_at_ms || av.requested_ms || 0);
    const bt = sessionAcquisitionStartMs(bv) || Number(bv.saved_at_ms || bv.requested_ms || 0);
    return at - bt;
  });
  if (!ids.length) {
    bodyEl.innerHTML = `<tr><td colspan="7" class="muted">No ${kind} sessions yet.</td></tr>`;
    return;
  }
  bodyEl.innerHTML = ids.map((sid, idx) => {
    const meta = obj[sid] || {};
    const itemNo = idx + 1;
    const activeCls = sid === activeViewedSessionSid ? "is-active-view" : "";

    if (kind === "processed") {
      const savedMs = Number(meta.saved_at_ms || meta.start_ms || 0);
      const sourceText = titleizeTokenText(meta.source_session_id || canonicalDatasetStem(meta.source_file || sid, sid) || "Processed", "Processed");
      return `<tr data-session-row="${sid}" class="${activeCls}"><td class="mono">#${itemNo}</td><td><div class="mono session-name-primary">${sessionDisplayName(meta, sid)}</div><div class="muted small mono session-name-secondary">${sessionSecondaryText(meta, sid)}</div></td><td class="mono">${savedMs ? formatDisplayDateOnly(savedMs) : "—"}</td><td class="mono">${savedMs ? formatDisplayTimeOnly(savedMs) : "—"}<div class="muted small mono">${sourceText}</div></td><td class="mono">${durationText(meta)}</td><td class="mono">${sessionLoadText(meta)}</td><td class="session-actions"><button type="button" class="btn btn-small" data-processed-view-sid="${sid}">View</button><button type="button" class="btn btn-small" data-processed-sid="${sid}">Download</button><button type="button" class="btn btn-small btn-danger" data-del-processed-sid="${sid}">Delete</button></td></tr>`;
    }

    const stMs = sessionCaptureStartMs(meta) || sessionAcquisitionStartMs(meta) || Number(meta.requested_ms || 0);
    const enMs = sessionCaptureEndMs(meta) || sessionAcquisitionEndMs(meta) || Number(meta.end_ms || 0);
    const dateMs = stMs || enMs || Number(meta.requested_ms || 0);
    return `<tr data-session-row="${sid}" class="${activeCls}"><td class="mono">#${itemNo}</td><td><div class="mono session-name-primary">${sessionDisplayName(meta, sid)}</div><div class="muted small mono session-name-secondary">${sessionSecondaryText(meta, sid)}</div></td><td class="mono">${dateMs ? formatDisplayDateOnly(dateMs) : "—"}</td><td class="mono">${stMs ? formatDisplayTimeRange(stMs, enMs, "Pending") : (Number(meta.requested_ms || 0) ? "Pending" : "—")}</td><td class="mono">${durationText(meta)}</td><td class="mono">${sessionLoadText(meta)}</td><td class="session-actions"><button type="button" class="btn btn-small" data-view-sid="${sid}">View</button><button type="button" class="btn btn-small" data-sid="${sid}">Download</button><button type="button" class="btn btn-small btn-danger" data-del-sid="${sid}">Delete</button></td></tr>`;
  }).join("");

  bodyEl.querySelectorAll("button[data-sid]").forEach(btn => btn.addEventListener("click", async () => {
    const sid = btn.getAttribute("data-sid");
    const meta = obj[sid] || {};
    const csv = await fetchSessionCsv(sid);
    if (!csv) { toast("No data for this session yet.", "err"); return; }
    downloadTextFileGeneric(sessionFilename(meta, sid), csv);
    if (mlLogStatus) mlLogStatus.textContent = `Downloaded session: ${sid}`;
    toast("Session CSV downloaded.", "ok");
  }));

  bodyEl.querySelectorAll("button[data-view-sid]").forEach(btn => btn.addEventListener("click", async (ev) => {
    ev.preventDefault();
    ev.stopPropagation();
    const sid = btn.getAttribute("data-view-sid");
    const meta = obj[sid] || {};
    if (typeof window.openSessionViewer !== "function") await waitForSessionViewerFns();
    if (typeof window.openSessionViewer === "function") window.openSessionViewer(sid, meta);
  }));

  bodyEl.querySelectorAll("button[data-del-sid]").forEach(btn => btn.addEventListener("click", async () => {
    const sid = btn.getAttribute("data-del-sid");
    if (!sid || !confirm(`Delete session ${sid}? This will remove both metadata and CSV chunks.`)) return;
    try {
      await db.ref(`ml_logs/${sid}`).remove();
      await db.ref(`ml_sessions/${sid}`).remove();
      toast("Session deleted.", "ok");
    } catch (e) {
      console.error(e);
      toast("Failed to delete this session.", "err");
    }
  }));

  bodyEl.querySelectorAll("button[data-processed-sid]").forEach(btn => btn.addEventListener("click", async () => {
    const sid = btn.getAttribute("data-processed-sid");
    const meta = obj[sid] || {};
    const csv = await fetchStoredCsv(`ml_processed_logs/${sid}`);
    if (!csv) { toast("No processed data for this session yet.", "err"); return; }
    downloadTextFileGeneric(sessionFilename(meta, sid), csv);
    toast("Processed CSV downloaded.", "ok");
  }));

  bodyEl.querySelectorAll("button[data-processed-view-sid]").forEach(btn => btn.addEventListener("click", async (ev) => {
    ev.preventDefault();
    ev.stopPropagation();
    const sid = btn.getAttribute("data-processed-view-sid");
    const meta = obj[sid] || {};
    const csv = await fetchStoredCsv(`ml_processed_logs/${sid}`);
    if (!csv) { toast("No processed data for this session yet.", "err"); return; }
    if (typeof window.openSessionViewerFromCsv !== "function") await waitForSessionViewerFns();
    if (typeof window.openSessionViewerFromCsv === "function") window.openSessionViewerFromCsv(meta?.source_file || sid, csv, meta);
  }));

  bodyEl.querySelectorAll("button[data-del-processed-sid]").forEach(btn => btn.addEventListener("click", async () => {
    const sid = btn.getAttribute("data-del-processed-sid");
    if (!sid || !confirm(`Delete processed session ${sid}?`)) return;
    try {
      await db.ref(`ml_processed_logs/${sid}`).remove();
      await db.ref(`ml_processed_sessions/${sid}`).remove();
      toast("Processed session deleted.", "ok");
    } catch (e) {
      console.error(e);
      toast("Failed to delete this processed session.", "err");
    }
  }));

  applyActiveMlSessionRow();
}

let loggerOriginalMetaCache = {};
let loggerOriginalLogCache = {};
let loggerProcessedMetaCache = {};
let loggerProcessedLogCache = {};

function renderLoggerOriginalSessions() {
  renderSessionRows(mlSessionBody, mergeSessionCollection(loggerOriginalMetaCache, loggerOriginalLogCache, "original"), "original");
}

function renderLoggerProcessedSessions() {
  renderSessionRows(mlProcessedBody, mergeSessionCollection(loggerProcessedMetaCache, loggerProcessedLogCache, "processed"), "processed");
}

if (mlSessionBody) {
  db.ref("ml_sessions").limitToLast(200).on("value", (s) => {
    loggerOriginalMetaCache = s.val() || {};
    renderLoggerOriginalSessions();
  });
  db.ref("ml_logs").limitToLast(200).on("value", (s) => {
    loggerOriginalLogCache = s.val() || {};
    renderLoggerOriginalSessions();
  });
}
if (mlProcessedBody) {
  db.ref("ml_processed_sessions").limitToLast(200).on("value", (s) => {
    loggerProcessedMetaCache = s.val() || {};
    renderLoggerProcessedSessions();
  });
  db.ref("ml_processed_logs").limitToLast(200).on("value", (s) => {
    loggerProcessedLogCache = s.val() || {};
    renderLoggerProcessedSessions();
  });
}

btnDownloadAllMl?.addEventListener("click", async () => {
  const isProcessed = activeLoggerTab === "processed";
  const metaPath = isProcessed ? "ml_processed_sessions" : "ml_sessions";
  const logPath = isProcessed ? "ml_processed_logs" : "ml_logs";
  const [sessionsSnap, logsSnap] = await Promise.all([db.ref(metaPath).get(), db.ref(logPath).get()]);
  const sessions = mergeSessionCollection(
    sessionsSnap.exists() ? sessionsSnap.val() : {},
    logsSnap.exists() ? logsSnap.val() : {},
    isProcessed ? "processed" : "original"
  );
  const ids = Object.keys(sessions || {});
  if (!ids.length) {
    toast(isProcessed ? "No processed sessions yet." : "No sessions yet.", "err");
    return;
  }

  ids.sort((a, b) => {
    const av = sessions[a] || {};
    const bv = sessions[b] || {};
    const at = Number(av.start_ms_device_ack || av.start_ms || av.saved_at_ms || av.requested_ms || 0);
    const bt = Number(bv.start_ms_device_ack || bv.start_ms || bv.saved_at_ms || bv.requested_ms || 0);
    return at - bt;
  });

  const jobs = [];
  for (const sid of ids) {
    const meta = sessions[sid] || {};
    const csv = isProcessed ? await fetchStoredCsv(`ml_processed_logs/${sid}`) : await fetchSessionCsv(sid);
    if (!csv) continue;
    jobs.push({ filename: sessionFilename(meta, sid), csv });
  }

  if (!jobs.length) {
    toast(isProcessed ? "No processed session rows found." : "No session rows found.", "err");
    return;
  }

  jobs.forEach((job, idx) => {
    setTimeout(() => downloadTextFileGeneric(job.filename, job.csv), idx * 180);
  });
  if (mlLogStatus) mlLogStatus.textContent = `Downloading ${jobs.length} ${isProcessed ? "processed" : "original"} session CSVs in order.`;
  toast(`Downloading ${jobs.length} ${isProcessed ? "processed" : "original"} CSVs.`, "ok");
});

function safeSessionToken(name) {
  return String(name || "uploaded_csv")
    .replace(/\.csv$/i, "")
    .replace(/[^a-zA-Z0-9_-]+/g, "_")
    .replace(/^_+|_+$/g, "")
    .slice(0, 42) || "uploaded_csv";
}

function waitForSessionViewerFns(timeoutMs = 1200) {
  return new Promise((resolve) => {
    const start = Date.now();
    const tick = () => {
      const ready = (typeof window.openSessionViewer === "function") || (typeof window.openSessionViewerFromCsv === "function");
      if (ready) return resolve(true);
      if ((Date.now() - start) >= timeoutMs) return resolve(false);
      setTimeout(tick, 50);
    };
    tick();
  });
}

async function storeCsvSession(fileName, csvText, kind = "original", metaPatch = {}) {
  const clean = String(csvText || "").replace(/^\ufeff/, "").trim();
  const lines = clean.split(/\r?\n/).filter((line) => line.trim().length);
  if (lines.length < 2) throw new Error("CSV needs a header and at least one row.");

  const cleanStem = safeSessionToken(canonicalDatasetStem(fileName, kind === "processed" ? "processed" : "uploaded_csv"));
  const sid = `${kind === "processed" ? "processed" : "upload"}_${Date.now()}_${cleanStem}`;
  const header = lines[0].trimEnd();
  const rows = lines.slice(1);
  const timing = deriveUploadedCsvTimingMeta(clean);
  const fileMeta = parseDatasetFilenameMeta(fileName);
  const nowMs = Date.now();
  const durationS = timing.durationS;
  const deviceFamily = normalizeDeviceFamilyToken(metaPatch?.device_family || fileMeta.deviceFamily || "unknown");
  const deviceName = normalizeDeviceNameToken(metaPatch?.device_name || fileMeta.deviceName || fileMeta.loadType || cleanStem, cleanStem);
  const trialNumber = Math.max(1, parseInt(metaPatch?.trial_number || fileMeta.trial || 1, 10) || 1);
  const divisionTag = normalizeDivisionTagToken(metaPatch?.division_tag || fileMeta.division || "steady");
  const durationMs = durationS != null ? Math.max(0, Math.round(durationS * 1000)) : 0;
  const startMs = Number.isFinite(timing.startMs) && timing.startMs > 0 ? timing.startMs : nowMs;
  const endMs = Number.isFinite(timing.endMs) && timing.endMs > startMs ? timing.endMs : (durationMs > 0 ? startMs + durationMs : null);
  const sourceFile = buildStructuredDatasetFilename({
    device_family: deviceFamily,
    device_family_code: deviceFamilyCodeFromToken(deviceFamily),
    device_name: deviceName,
    trial_number: trialNumber,
    division_tag: divisionTag,
  }, cleanStem);

  const baseMeta = {
    start_ms: startMs,
    end_ms: endMs,
    load_type: deviceName,
    device_family: deviceFamily,
    device_family_code: deviceFamilyCodeFromToken(deviceFamily),
    device_name: deviceName,
    trial_number: trialNumber,
    division_tag: divisionTag,
    notes: normalizeNotesText(metaPatch?.notes || ""),
    trusted_normal_session: metaPatch?.trusted_normal_session ? 1 : 0,
    duration_s: durationS,
    source_duration_s: durationS,
    label_override: -1,
    uploaded_csv: kind !== "processed",
    processed_csv: kind === "processed",
    source_file: sourceFile,
    row_count: timing.rowCount || rows.length,
    source_sample_rate_hz: timing.sourceSampleRateHz,
    saved_at_ms: nowMs,
  };
  const meta = { ...baseMeta, ...metaPatch };

  const sessionPath = kind === "processed" ? `ml_processed_sessions/${sid}` : `ml_sessions/${sid}`;
  const logPathBase = kind === "processed" ? `ml_processed_logs/${sid}` : `ml_logs/${sid}`;
  await db.ref(sessionPath).set(meta);

  const CHUNK_ROWS = 250;
  const updates = {};
  const createdBase = Date.now();
  for (let i = 0; i < rows.length; i += CHUNK_ROWS) {
    const chunkRows = rows.slice(i, i + CHUNK_ROWS);
    const key = `chunk_${String((i / CHUNK_ROWS) + 1).padStart(4, "0")}`;
    updates[`${logPathBase}/${key}`] = {
      csv: `${header}\n${chunkRows.join("\n")}\n`,
      count: chunkRows.length,
      created_at: createdBase + Math.floor(i / CHUNK_ROWS),
      uploaded_csv: kind !== "processed",
      processed_csv: kind === "processed",
      source_file: sourceFile,
      device_family: deviceFamily,
      device_family_code: deviceFamilyCodeFromToken(deviceFamily),
      device_name: deviceName,
      trial_number: trialNumber,
      division_tag: divisionTag,
      notes: normalizeNotesText(metaPatch?.notes || ""),
      trusted_normal_session: metaPatch?.trusted_normal_session ? 1 : 0,
    };
  }
  await db.ref().update(updates);
  return { sid, meta };
}

async function storeUploadedCsvSession(fileName, csvText) {
  return storeCsvSession(fileName, csvText, "original");
}

window.storeCsvSession = storeCsvSession;
window.storeUploadedCsvSession = storeUploadedCsvSession;

mlCsvUpload?.addEventListener("change", async (ev) => {
  const file = ev.target?.files?.[0];
  if (!file) return;
  try {
    const text = await file.text();
    if (!text || text.indexOf(",") < 0) throw new Error("Invalid CSV");

    let opened = false;
    try {
      const stored = await storeUploadedCsvSession(file.name, text);
      currentSessionId = stored.sid;
      if (mlLogStatus) mlLogStatus.textContent = `Uploaded CSV stored as session: ${stored.sid}`;
      if (typeof window.openSessionViewer !== "function") await waitForSessionViewerFns();
      if (typeof window.openSessionViewer === "function") {
        window.openSessionViewer(stored.sid, stored.meta);
        opened = true;
      }
      toast("CSV uploaded to cloud logger and saved as a session.", "ok");
    } catch (cloudErr) {
      console.error(cloudErr);
      const clean = String(text || "").replace(/^\uFEFF/, "").trim();
      const timing = deriveUploadedCsvTimingMeta(clean);
      const fileMeta = parseDatasetFilenameMeta(file.name);
      const startMs = Number.isFinite(timing.startMs) && timing.startMs > 0 ? timing.startMs : Date.now();
      const endMs = Number.isFinite(timing.endMs) && timing.endMs > startMs ? timing.endMs : (timing.durationS != null ? startMs + Math.max(0, Math.round(timing.durationS * 1000)) : null);
      const meta = {
        load_type: fileMeta.deviceName || fileMeta.loadType || "uploaded_csv",
        device_family: fileMeta.deviceFamily || "unknown",
        device_family_code: deviceFamilyCodeFromToken(fileMeta.deviceFamily || "unknown"),
        device_name: fileMeta.deviceName || fileMeta.loadType || canonicalDatasetStem(file.name, file.name),
        uploaded_csv: true,
        row_count: timing.rowCount,
        duration_s: timing.durationS,
        source_duration_s: timing.durationS,
        start_ms: startMs,
        end_ms: endMs,
        source_file: buildStructuredDatasetFilename({ device_family: fileMeta.deviceFamily || "unknown", device_name: fileMeta.deviceName || fileMeta.loadType || canonicalDatasetStem(file.name, file.name), trial_number: fileMeta.trial || 1, division_tag: fileMeta.division || "steady" }, file.name),
        source_sample_rate_hz: timing.sourceSampleRateHz,
        trial_number: fileMeta.trial,
        division_tag: fileMeta.division || ""
      };
      if (typeof window.openSessionViewerFromCsv !== "function") await waitForSessionViewerFns();
      if (typeof window.openSessionViewerFromCsv === "function") {
        window.openSessionViewerFromCsv(file.name, text, meta);
        opened = true;
      }
      const reason = cloudErr?.message || cloudErr?.code || String(cloudErr || "Unknown cloud save error");
      if (mlLogStatus) mlLogStatus.textContent = `Uploaded CSV opened locally. Cloud save failed: ${reason}`;
      toast(`CSV opened locally. Cloud save failed: ${reason}`, "warn");
    }

    if (!opened) toast("CSV viewer failed to initialize. Please refresh once.", "err");
  } catch (e) {
    console.error(e);
    toast("Failed to load CSV.", "err");
  } finally {
    if (mlCsvUpload) mlCsvUpload.value = "";
  }
});

window.addEventListener("beforeinstallprompt", (event) => {
  event.preventDefault();
  deferredInstallPrompt = event;
  updateInstallHelp();
});

window.addEventListener("appinstalled", () => {
  deferredInstallPrompt = null;
  saveBool(LS_INSTALL_DISMISS, true);
  btnInstallApp?.classList.add("hidden");
  installHelp?.classList.add("hidden");
  toast("App installed.", "ok");
});

if ("serviceWorker" in navigator) {
  navigator.serviceWorker.register("./sw.js", { scope: "./" }).then((reg) => {
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
