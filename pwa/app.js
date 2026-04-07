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
const healthWifiChip = el("healthWifiChip");
const healthWifiBars = el("healthWifiBars");
const healthWifiText = el("healthWifiText");
const healthLatencyChip = el("healthLatencyChip");
const healthLatencyText = el("healthLatencyText");
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
const btnInstallApp = el("btnInstallApp");
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

const spectralFluxVal = el("spectralFluxVal");
const residualCrestVal = el("residualCrestVal");
const edgeSpikeVal = el("edgeSpikeVal");
const midbandRatioVal = el("midbandRatioVal");
const cycleNmseVal = el("cycleNmseVal");
const peakFluctVal = el("peakFluctVal");
const thdIVal = el("thdIVal");
const hfEnergyDeltaVal = el("hfEnergyDeltaVal");
const zcvVal = el("zcvVal");
const irmsZscoreVal = el("irmsZscoreVal");

const alertEnable = el("alertEnable");
const soundEnable = el("soundEnable");

const historyNormalBody = el("historyNormalBody");
const historyFaultBody = el("historyFaultBody");
const historyNormalCount = el("historyNormalCount");
const historyFaultCount = el("historyFaultCount");
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
const STALE_MS = 15000;
const HISTORY_LIMIT = 5000;
const MAX_RENDER_ROWS = 180;

const OTA_RELEASE_BASE = "https://github.com/ancy-gg/TinyML_Smart-Plug/releases/download/";
const OTA_RELEASE_TAG  = "updates";
const OTA_DEFAULT_BIN  = "";

const UI_OVERLOAD_WARN_A = 10;
const UI_SHORT_CIRCUIT_A = 20;
const UI_NORMAL_V_MIN = 200;
const UI_NORMAL_V_MAX = 250;
const UI_UNDERVOLTAGE_MIN_V = 100;
const UI_UNDERVOLTAGE_V = 200;
const UI_OVERVOLTAGE_V = 250;
const UI_OVERVOLTAGE_DELAY_V = 250;
const UI_OVERVOLTAGE_FAST_V = 265;
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
let unpluggedUiSinceMs = 0;
let pluggedUiSinceMs = 0;
let latchedUiPowerCondition = "UNKNOWN";
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

const HISTORY_FAULT_SET = new Set(["DEVICE_DISCONNECTED", "UNPLUGGED", "OVERLOAD", "SUSTAINED_OVERLOAD", "HEATING", "ARCING", "OVERVOLTAGE", "UNDERVOLTAGE", "SAFE_MODE"]);

const STATUS_INFO = {
  NORMAL: {
    title: "Monitoring",
    summary: "The plug is online and watching the load normally.",
    details: [
      "Live values are updating and no active trip is latched.",
      "This is the expected state during stable operation."
    ]
  },
  DEVICE_ON: {
    title: "Device On",
    summary: "The device booted and started running.",
    details: ["This is a normal startup event before steady monitoring continues."]
  },
  DEVICE_ONLINE: {
    title: "Device Online",
    summary: "The dashboard is receiving live updates from the plug.",
    details: ["Connection is healthy enough for monitoring and commands."]
  },
  DEVICE_PLUGGED_IN: {
    title: "Device Plugged In",
    summary: "Mains returned to the plug input.",
    details: ["Voltage rose back above the unplugged region."]
  },
  FIRMWARE_UPDATED: {
    title: "Firmware Updated",
    summary: "A new firmware image was applied.",
    details: ["Check live readings once the device reconnects to confirm the update is healthy."]
  },
  DEVICE_DISCONNECTED: {
    title: "Device Disconnected",
    summary: "The dashboard stopped receiving fresh packets.",
    details: ["This can be Wi‑Fi loss, reboot, power loss, or a stalled network link."],
    action: "Check power first, then Wi‑Fi and the device status." 
  },
  UNPLUGGED: {
    title: "Unplugged / No Mains",
    summary: "Measured voltage fell into the no-mains region.",
    details: ["This usually means the plug was unplugged, the outlet lost supply, or the contact opened."],
    action: "Restore mains safely, then confirm voltage has returned before re-enabling the load."
  },
  OVERLOAD: {
    title: "Overload Alarm",
    summary: "Current crossed the overload warning region.",
    details: ["This is an alert state that signals the load is drawing heavy current."],
    action: "Reduce load or verify the appliance current before continued use."
  },
  SUSTAINED_OVERLOAD: {
    title: "Sustained Overload",
    summary: "Heavy current stayed high long enough to trip protection.",
    details: ["This is more serious than a short overload burst and normally forces a relay trip."],
    action: "Disconnect or reduce the load and inspect for overheating before restoring power."
  },
  HEATING: {
    title: "Heating",
    summary: "Socket temperature reached the protection region.",
    details: ["This often points to overloaded contacts, poor contact pressure, or localized heating."],
    action: "Let the device cool, inspect the socket and plug contact, and do not immediately re-energize."
  },
  ARCING: {
    title: "Arc Fault",
    summary: "Waveform evidence matched arcing behavior.",
    details: ["The decision is based on multiple waveform features and model evidence, not one value alone."],
    action: "Disconnect the load and inspect plugs, cords, and contact points before reuse."
  },
  UNDERVOLTAGE: {
    title: "Undervoltage",
    summary: "Voltage dropped below the dashboard undervoltage threshold.",
    details: ["In this PWA, values below 200 V are treated as undervoltage for interpretation."],
    action: "Wait for supply to stabilize and check for loose or resistive contacts."
  },
  OVERVOLTAGE: {
    title: "Overvoltage",
    summary: "Voltage rose above the dashboard overvoltage threshold.",
    details: ["In this PWA, values above 250 V are treated as overvoltage for interpretation."],
    action: "Do not keep reconnecting the load until the supply is back in range."
  },
  SAFE_MODE: {
    title: "Safe Mode",
    summary: "The device fell back to a reduced-protection state.",
    details: ["This usually follows a serious boot or update problem."],
    action: "Review firmware and reboot behavior before using the plug for protection again."
  },
  WIFI_CONNECTING: {
    title: "Wi‑Fi Connecting",
    summary: "The plug is trying to reach the saved network.",
    details: ["This is normal during startup or after a network change."]
  },
  CONFIG_PORTAL: {
    title: "Wi‑Fi Portal",
    summary: "The configuration portal is open.",
    details: ["Use this to enter or change Wi‑Fi credentials."]
  },
  STARTUP_STABILIZING: {
    title: "Startup Stabilizing",
    summary: "The plug is in its startup settling period.",
    details: ["Give the device a moment before treating the readings as fully stable."]
  },
  OTA_UPDATING: {
    title: "OTA Updating",
    summary: "Firmware download or installation is in progress.",
    details: ["Avoid power interruption until the update completes."]
  }
};

const METRIC_INFO = {
  voltage: {
    title: "Voltage",
    summary: "This is the live RMS mains voltage reading.",
    details: [
      "It tracks the input supply seen by the plug.",
      "Around 200–250 V is treated as the normal dashboard band.",
      "Low readings can point to sag, bad contact, or mains loss. High readings point to overvoltage."
    ]
  },
  current: {
    title: "Current",
    summary: "This is the live RMS current of the connected load.",
    details: [
      "It is the main indicator of whether the load is idle, light, or heavy.",
      "Fast jumps, repeated dips, or unusually high sustained current can indicate abnormal load behavior."
    ]
  },
  apparent_power: {
    title: "Apparent Power",
    summary: "This is the voltage-current product in VA.",
    details: [
      "It gives a quick sense of load activity even when current alone looks small.",
      "Higher values mean the plug is supplying a more demanding load."
    ]
  },
  temperature: {
    title: "Temperature",
    summary: "This is the socket temperature reading used for heating protection.",
    details: [
      "A steady low or moderate value is normal.",
      "A rising trend under load can point to poor contact, overload, or hot ambient conditions."
    ]
  },
  spectral_flux_midhf: {
    title: "Spectral Flux (Mid/HF)",
    summary: "Measures frame-to-frame change in the normalized square-root spectrum inside the configured mid/high-frequency band.",
    details: [
      "Low and steady values usually mean stable waveform content.",
      "Spikes often appear when fast transients or arc-like bursts introduce changing HF content."
    ]
  },
  residual_crest_factor: {
    title: "Residual Crest Factor",
    summary: "Converts the residual waveform crest ratio to dB after separating a faster cleaned waveform from a slower base envelope.",
    details: [
      "Lower values are usually calmer.",
      "Higher values mean sharper impulsive peaks, which can happen with switching spikes or arcs."
    ]
  },
  edge_spike_ratio: {
    title: "Edge Spike Ratio",
    summary: "Measures the strongest residual burst around the sharpest detected waveform edge and compares it against the rolling current baseline in dB.",
    details: [
      "Stable loads usually keep this modest.",
      "Large rises suggest sharper, more abrupt events near waveform edges."
    ]
  },
  midband_residual_ratio: {
    title: "Midband Residual Ratio",
    summary: "Measures residual midband spectral energy, compares it against the rolling current baseline, and expresses the ratio in dB.",
    details: [
      "A stable normal load tends to stay relatively consistent.",
      "Raised values can mean added disturbance or rougher waveform structure."
    ]
  },
  cycle_nmse: {
    title: "Cycle NMSE",
    summary: "Compares adjacent cycles and measures how different they are.",
    details: [
      "Low values mean one cycle looks much like the next.",
      "Higher values mean poorer cycle-to-cycle repeatability, which can happen during unstable operation."
    ]
  },
  peak_fluct_cv: {
    title: "Peak Fluctuation CV",
    summary: "Measures how much peak levels vary over time.",
    details: [
      "Stable loads keep peak variation tighter.",
      "A large spread points to unstable peaks, pulsing behavior, or disturbance."
    ]
  },
  thd_i: {
    title: "Current THD",
    summary: "This is current total harmonic distortion.",
    details: [
      "Some non-linear loads naturally have higher THD than simple resistive loads.",
      "What matters most is sudden change or unusually high distortion relative to the same load."
    ]
  },
  hf_energy_delta: {
    title: "HF Energy Delta",
    summary: "Tracks the change in high-frequency band share relative to the rolling baseline and expresses that change in dB.",
    details: [
      "Stable operation keeps this from swinging too sharply.",
      "Large positive changes can happen when HF noise or bursty activity appears."
    ]
  },
  zcv: {
    title: "ZCV",
    summary: "Measures the spread of zero-cross timing intervals derived from hysteresis-based zero-cross detection.",
    details: [
      "It is derived from the standard deviation of zero-cross interval timing, expressed in milliseconds.",
      "Smaller values mean more consistent crossing timing. Larger values mean more timing irregularity."
    ]
  },
  abs_irms_zscore_vs_baseline: {
    title: "Absolute Z-Deviation",
    summary: "Shows how far the present RMS current is from the rolling baseline current, in baseline standard-deviation units.",
    details: [
      "Near-zero means the load looks close to its recent baseline.",
      "Large values mean the load has drifted far from its normal level for that session."
    ]
  },
  connection: {
    title: "Connection",
    summary: "Shows whether the dashboard is still getting fresh packets.",
    details: ["Offline or stale here means the page is no longer seeing current telemetry from the plug."]
  },
  protection: {
    title: "Protection",
    summary: "Summarizes the active protection state or current trip condition.",
    details: ["Use this together with History to see whether the device is monitoring, alarming, or tripped."]
  },
  firmware: {
    title: "Firmware",
    summary: "Displays the currently reported device firmware build.",
    details: ["Use it to confirm the plug is running the expected firmware after OTA or rollback."]
  },
  last_event: {
    title: "Last Event",
    summary: "Shows the newest history event captured by the dashboard.",
    details: ["It is a quick way to see the last important state change without opening the full history pane."]
  }
};

let infoPopoverEl = null;

function escapeHtml(value) {
  return String(value ?? "")
    .replace(/&/g, "&amp;")
    .replace(/</g, "&lt;")
    .replace(/>/g, "&gt;")
    .replace(/"/g, "&quot;")
    .replace(/'/g, "&#39;");
}

function normalizeInfoEntry(entry) {
  if (!entry) return null;
  return {
    title: String(entry.title || "Info"),
    summary: String(entry.summary || ""),
    details: Array.isArray(entry.details) ? entry.details.map((v) => String(v || "").trim()).filter(Boolean) : [],
    action: String(entry.action || "").trim()
  };
}

function ensureInfoPopover() {
  if (infoPopoverEl) return infoPopoverEl;
  infoPopoverEl = document.createElement("div");
  infoPopoverEl.className = "info-popover";
  infoPopoverEl.hidden = true;
  document.body.appendChild(infoPopoverEl);

  const closeMaybe = (ev) => {
    if (!infoPopoverEl || infoPopoverEl.hidden) return;
    if (ev && infoPopoverEl.contains(ev.target)) return;
    if (ev && ev.target?.closest?.(".tsp-info-btn")) return;
    hideInfoPopover();
  };

  document.addEventListener("click", closeMaybe);
  window.addEventListener("resize", hideInfoPopover);
  window.addEventListener("scroll", hideInfoPopover, true);
  document.addEventListener("keydown", (ev) => {
    if (ev.key === "Escape") hideInfoPopover();
  });
  return infoPopoverEl;
}

function hideInfoPopover() {
  if (!infoPopoverEl) return;
  infoPopoverEl.hidden = true;
  infoPopoverEl.classList.remove("show");
}

function showInfoPopover(anchor, entry, tone = "") {
  const box = ensureInfoPopover();
  const item = normalizeInfoEntry(entry);
  if (!anchor || !item) return;

  const detailsHtml = item.details.length
    ? `<ul>${item.details.map((line) => `<li>${escapeHtml(line)}</li>`).join("")}</ul>`
    : "";
  const actionHtml = item.action
    ? `<div class="info-popover-action"><strong>What to do:</strong> ${escapeHtml(item.action)}</div>`
    : "";

  box.className = `info-popover${tone ? ` ${tone}` : ""}`;
  box.innerHTML = `
    <div class="info-popover-title">${escapeHtml(item.title)}</div>
    ${item.summary ? `<div class="info-popover-summary">${escapeHtml(item.summary)}</div>` : ""}
    ${detailsHtml}
    ${actionHtml}
  `;
  box.hidden = false;
  box.classList.add("show");

  const rect = anchor.getBoundingClientRect();
  const boxRect = box.getBoundingClientRect();
  const pad = 12;
  let left = rect.right - boxRect.width;
  if (!Number.isFinite(left)) left = rect.left;
  left = Math.max(pad, Math.min(window.innerWidth - boxRect.width - pad, left));

  let top = rect.bottom + 10;
  if ((top + boxRect.height) > (window.innerHeight - pad)) {
    top = rect.top - boxRect.height - 10;
  }
  top = Math.max(pad, Math.min(window.innerHeight - boxRect.height - pad, top));

  box.style.left = `${Math.round(left)}px`;
  box.style.top = `${Math.round(top)}px`;
}

function statusInfoEntry(kind) {
  return normalizeInfoEntry(STATUS_INFO[classifyStatus(kind)] || {
    title: prettyStatus(kind) || "Status",
    summary: "This is the current recorded status.",
    details: ["No extra description was mapped for this status yet."]
  });
}

function metricInfoEntry(key) {
  return normalizeInfoEntry(METRIC_INFO[key]);
}

function bindHoldInfo(target, entry, tone = "") {
  if (!target) return;
  const normalized = normalizeInfoEntry(entry);
  if (!normalized) return;
  if (target.dataset.holdInfoBound === "1") return;
  target.dataset.holdInfoBound = "1";
  target.classList.add("has-hold-info");

  const show = (ev) => {
    if (ev?.pointerType === "mouse" && ev.button !== 0) return;
    showInfoPopover(target, normalized, tone);
  };
  const hide = () => hideInfoPopover();

  target.addEventListener("pointerdown", show);
  target.addEventListener("pointerup", hide);
  target.addEventListener("pointerleave", hide);
  target.addEventListener("pointercancel", hide);
  target.addEventListener("lostpointercapture", hide);
  target.addEventListener("contextmenu", (ev) => ev.preventDefault());
  target.addEventListener("keydown", (ev) => {
    if (ev.key === "Enter" || ev.key === " ") {
      ev.preventDefault();
      showInfoPopover(target, normalized, tone);
    }
  });
  target.addEventListener("keyup", (ev) => {
    if (ev.key === "Enter" || ev.key === " ") hideInfoPopover();
  });
  target.addEventListener("blur", hide);
}

function initDashboardInfoButtons() {

  const mappings = [
    [vVal?.closest?.(".card"), metricInfoEntry("voltage")],
    [iVal?.closest?.(".card"), metricInfoEntry("current")],
    [pVal?.closest?.(".card"), metricInfoEntry("apparent_power")],
    [tVal?.closest?.(".card"), metricInfoEntry("temperature")],
    [spectralFluxVal?.closest?.(".card"), metricInfoEntry("spectral_flux_midhf")],
    [residualCrestVal?.closest?.(".card"), metricInfoEntry("residual_crest_factor")],
    [edgeSpikeVal?.closest?.(".card"), metricInfoEntry("edge_spike_ratio")],
    [midbandRatioVal?.closest?.(".card"), metricInfoEntry("midband_residual_ratio")],
    [cycleNmseVal?.closest?.(".card"), metricInfoEntry("cycle_nmse")],
    [peakFluctVal?.closest?.(".card"), metricInfoEntry("peak_fluct_cv")],
    [thdIVal?.closest?.(".card"), metricInfoEntry("thd_i")],
    [hfEnergyDeltaVal?.closest?.(".card"), metricInfoEntry("hf_energy_delta")],
    [zcvVal?.closest?.(".card"), metricInfoEntry("zcv")],
    [irmsZscoreVal?.closest?.(".card"), metricInfoEntry("abs_irms_zscore_vs_baseline")],
    [ovConnectivity?.closest?.(".summary-card"), metricInfoEntry("connection")],
    [ovProtection?.closest?.(".summary-card"), metricInfoEntry("protection")],
    [ovFirmware?.closest?.(".summary-card"), metricInfoEntry("firmware")],
    [ovLastEvent?.closest?.(".summary-card"), metricInfoEntry("last_event")],
  ];
  mappings.forEach(([node, entry]) => bindHoldInfo(node, entry, ""));
}

function isFaultHistoryStatus(kind) {
  return HISTORY_FAULT_SET.has(classifyStatus(kind));
}

function renderHistoryPane(container, items, isFaultPane = false) {
  if (!container) return;
  if (!items.length) {
    container.innerHTML = `<div class="history-empty">No ${isFaultPane ? "fault" : "normal-operation"} history in this range.</div>`;
    return;
  }
  container.innerHTML = items.map((record, idx) => {
    const kind = classifyStatus(record?.status || "NORMAL");
    const label = prettyStatus(kind);
    const tone = isFaultPane ? "history-status-chip-fault" : "history-status-chip-normal";
    return `
      <div class="history-item row-in" style="animation-delay:${Math.min(idx, 10) * 25}ms">
        <div class="history-item-time mono">${escapeHtml(formatDisplayTimestamp(getRecordEpochMs(record)))}</div>
        <div class="history-item-status">
          <span class="history-status-chip ${tone}" data-history-kind="${escapeHtml(kind)}" tabindex="0" role="button" aria-label="Explain ${escapeHtml(label)}">${escapeHtml(label)}</span>
        </div>
      </div>
    `;
  }).join("");
  container.querySelectorAll("[data-history-kind]").forEach((chip) => {
    bindHoldInfo(chip, statusInfoEntry(chip.getAttribute("data-history-kind") || "NORMAL"), isFaultPane ? "fault" : "");
  });
}

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

const FILENAME_DIVISION_ALIASES = {
  startup: "start",
  start: "start",
  steady: "steady",
  baseline: "steady",
  arc: "arc",
  close: "close",
  closing: "close",
  end: "close",
  ending: "close",
};
const FILENAME_SUFFIX_TOKENS = new Set(["capture", "captured", "processed", "process", "labeled", "labelled", "edited", "edit", "review", "original", "raw"]);
const FILENAME_PREFIX_TOKENS = new Set(["proc", "processed", "upload", "uploaded", "sess", "session", "logger", "log"]);

function parseTrialToken(token) {
  const raw = String(token || "").trim().toLowerCase();
  if (!raw) return null;
  if (/^\d+$/.test(raw)) return Math.max(1, Number(raw));
  const tagged = raw.match(/^(?:t|trial)(\d{1,4})$/i);
  if (tagged) return Math.max(1, Number(tagged[1]));
  return null;
}

function formatTrialToken(trial) {
  return `t${String(safePositiveInt(trial, 1)).padStart(2, "0")}`;
}

function normalizeDivisionToken(value) {
  return FILENAME_DIVISION_ALIASES[String(value || "").trim().toLowerCase()] || "";
}

function divisionFileToken(division) {
  const normalized = normalizeDivisionToken(division);
  if (!normalized) return "";
  return normalized === "close" ? "end" : normalized;
}

function divisionDisplayLabel(division, fallback = "") {
  const normalized = normalizeDivisionToken(division);
  if (!normalized) return fallback;
  return ({ start: "Start", steady: "Steady", arc: "Arc", close: "End" })[normalized] || fallback || titleizeTokenText(normalized, fallback || "Section");
}

function trimFilenameLoadTokens(tokens) {
  const out = Array.from(tokens || []).filter(Boolean);
  while (out.length > 1 && (FILENAME_PREFIX_TOKENS.has(String(out[0] || "").toLowerCase()) || /^\d{6,}$/.test(String(out[0] || "")))) out.shift();
  while (out.length && FILENAME_SUFFIX_TOKENS.has(String(out[out.length - 1] || "").toLowerCase())) out.pop();
  return out;
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
  const sourceFile = String(meta?.source_file || "").trim();
  const parsed = parseDatasetFilenameMeta(sourceFile || sessionId || "");
  const load = safeFilenameSegment(meta?.load_type || parsed.loadType || "", "").toLowerCase();
  const trial = safePositiveInt(meta?.trial_number || parsed.trial || 1, 1);
  const division = normalizeDivisionToken(meta?.division_tag || parsed.division || "");
  if (load && load !== "uploaded_csv" && load !== "unknown") {
    const base = `${titleizeTokenText(load, load)} • ${formatTrialToken(trial).toUpperCase()}`;
    return division ? `${base} • ${divisionDisplayLabel(division)}` : base;
  }
  if (isUploadedCsvSession(meta)) return sourceFile ? titleizeTokenText(sourceFile, "Uploaded CSV") : "Uploaded CSV";
  const start = Number(meta?.start_ms || 0);
  if (start) return `Session ${formatSessionStamp(start)}`;
  return titleizeTokenText(sessionId || "session", "Session");
}

function sessionSecondaryText(meta, sessionId) {
  const sourceFile = String(meta?.source_file || "").trim();
  const parsed = parseDatasetFilenameMeta(sourceFile || sessionId || "");
  const division = normalizeDivisionToken(meta?.division_tag || parsed.division || "");
  const category = loggerSessionCategory(meta);
  const parts = [];
  parts.push(category === "processed" ? "Processed" : "Original");
  if (division) parts.push(divisionDisplayLabel(division));
  if (sourceFile) parts.push(sourceFile);
  else if (sessionId) parts.push(sessionId);
  return parts.join(" • ");
}

function sessionLoadText(meta) {
  const sourceFile = String(meta?.source_file || "").trim();
  const parsed = parseDatasetFilenameMeta(sourceFile || "");
  const load = safeFilenameSegment(meta?.load_type || parsed.loadType || "", "").toLowerCase();
  if (load && load !== "uploaded_csv" && load !== "unknown") return titleizeTokenText(load, load);
  if (isUploadedCsvSession(meta)) return sourceFile ? titleizeTokenText(sourceFile, sourceFile) : "Uploaded CSV";
  return "Unknown";
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

const CONTROL_PULSE_DEBOUNCE_MS = 1500;

async function sendControlPulse(buttonEl, busyText, okText, errText, payload, ackInfo = null) {
  setHeaderMenuOpen(false);
  const now = Date.now();
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
        localSentMs: Date.now()
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
  const token = `relay_on_${Date.now()}`;
  await sendControlPulse(
    btnRelayOn,
    "Sending...",
    "Relay ON pulse sent.",
    "Failed to send Relay ON.",
    {
      relay_off_token: "",
      relay_on_token: token,
      relay_on_requested_at: firebase.database.ServerValue.TIMESTAMP
    },
    {
      kind: "relay_on",
      token,
      requestPath: "/controls/relay_on_requested_at"
    }
  );
});

btnRelayOff?.addEventListener("click", async () => {
  const token = `relay_off_${Date.now()}`;
  await sendControlPulse(
    btnRelayOff,
    "Sending...",
    "Relay OFF pulse sent.",
    "Failed to send Relay OFF.",
    {
      relay_on_token: "",
      relay_off_token: token,
      relay_off_requested_at: firebase.database.ServerValue.TIMESTAMP
    },
    {
      kind: "relay_off",
      token,
      requestPath: "/controls/relay_off_requested_at"
    }
  );
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

  initDashboardInfoButtons();

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
    [spectralFluxVal, "0.000"], [residualCrestVal, "0.000"], [edgeSpikeVal, "0.000"],
    [midbandRatioVal, "0.000"], [cycleNmseVal, "0.000"], [peakFluctVal, "0.000"],
    [thdIVal, "0.000"], [hfEnergyDeltaVal, "0.000"], [zcvVal, "0.000"], [irmsZscoreVal, "0.000"]
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
  if (raw === "UNDERVOLTAGE") return "UNDERVOLTAGE";
  if (raw === "UNPLUGGED") return "UNPLUGGED";

  if (v <= UI_MAINS_ABSENT_V) return "UNPLUGGED";
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
    spectralFluxVal, residualCrestVal, edgeSpikeVal, midbandRatioVal, cycleNmseVal,
    peakFluctVal, thdIVal, hfEnergyDeltaVal, zcvVal, irmsZscoreVal
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
  if (!(liveIsFresh() && lastLiveData)) {
    unpluggedUiSinceMs = 0;
    pluggedUiSinceMs = 0;
    latchedUiPowerCondition = "UNKNOWN";
    return "DEVICE_DISCONNECTED";
  }

  const raw = derivePowerConditionFromLive(lastLiveData);
  const now = Date.now();
  if (raw === "UNPLUGGED") {
    if (!unpluggedUiSinceMs) unpluggedUiSinceMs = now;
    pluggedUiSinceMs = 0;
    if ((now - unpluggedUiSinceMs) >= 5000) latchedUiPowerCondition = "UNPLUGGED";
    return latchedUiPowerCondition === "UNPLUGGED" ? "UNPLUGGED" : (previousPowerCondition === "UNPLUGGED" ? "UNPLUGGED" : "UNKNOWN");
  }

  unpluggedUiSinceMs = 0;
  if (raw === "NORMAL") {
    if (!pluggedUiSinceMs) pluggedUiSinceMs = now;
    if (previousPowerCondition === "UNPLUGGED" && (now - pluggedUiSinceMs) < 5000) return "UNPLUGGED";
  } else {
    pluggedUiSinceMs = 0;
  }

  latchedUiPowerCondition = raw;
  return raw;
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
  relayRocker?.classList.toggle("is-disabled", locked);
  if (btnRelayOn) btnRelayOn.disabled = locked;
  if (btnRelayOff) btnRelayOff.disabled = locked;
  if (btnFaultCleared) btnFaultCleared.disabled = !liveIsFresh();
}

function applyHistoryFilter() {
  const key = rangeSelect?.value || "7d";
  const { start, end } = getRangeBounds(key);

  const filtered = historyCache.filter((r) => {
    const epoch = getRecordEpochMs(r);
    return epoch && epoch >= start && epoch < end;
  });

  filtered.sort((a, b) => getRecordEpochMs(b) - getRecordEpochMs(a));
  currentFilteredHistory = filtered.slice();

  const normalRows = filtered.filter((r) => !isFaultHistoryStatus(r?.status || "NORMAL")).slice(0, MAX_RENDER_ROWS);
  const faultRows = filtered.filter((r) => isFaultHistoryStatus(r?.status || "NORMAL")).slice(0, MAX_RENDER_ROWS);

  if (historyHint) historyHint.textContent = `Showing: ${rangeLabel(key)} (${filtered.length})`;
  if (historyNormalCount) historyNormalCount.textContent = String(normalRows.length);
  if (historyFaultCount) historyFaultCount.textContent = String(faultRows.length);

  renderHistoryPane(historyNormalBody, normalRows, false);
  renderHistoryPane(historyFaultBody, faultRows, true);
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
  const n = Number(value);
  if (!Number.isFinite(n)) return "—";
  const decimals = {
    spectral_flux_midhf: 2,
    residual_crest_factor: 2,
    edge_spike_ratio: 2,
    midband_residual_ratio: 2,
    cycle_nmse: 2,
    peak_fluct_cv: 2,
    thd_i: 2,
    hf_energy_delta: 2,
    zcv: 3,
    abs_irms_zscore_vs_baseline: 2,
  }[key] ?? 3;
  return n.toFixed(decimals);
}

function updateLiveDom(data) {
  const v   = toFixedOrDash(data.voltage, 1);
  const i   = toFixedOrDash(data.current, 3);
  const p   = toFixedOrDash(data.apparent_power, 1);
  const t   = toFixedOrDash(data.temp, 1);
  const sf  = formatFeatureValue("spectral_flux_midhf", data.spectral_flux_midhf);
  const rcf = formatFeatureValue("residual_crest_factor", data.residual_crest_factor);
  const esr = formatFeatureValue("edge_spike_ratio", data.edge_spike_ratio);
  const mrr = formatFeatureValue("midband_residual_ratio", data.midband_residual_ratio);
  const cn  = formatFeatureValue("cycle_nmse", data.cycle_nmse);
  const pf  = formatFeatureValue("peak_fluct_cv", data.peak_fluct_cv);
  const thd = formatFeatureValue("thd_i", data.thd_i);
  const hfd = formatFeatureValue("hf_energy_delta", data.hf_energy_delta);
  const z   = formatFeatureValue("zcv", data.zcv);
  const iz  = formatFeatureValue("abs_irms_zscore_vs_baseline", data.abs_irms_zscore_vs_baseline);

  if (vVal && vVal.textContent !== v) { vVal.textContent = v; animateNumber(vVal); }
  if (iVal && iVal.textContent !== i) { iVal.textContent = i; animateNumber(iVal); }
  if (pVal && pVal.textContent !== p) { pVal.textContent = p; animateNumber(pVal); }
  if (tVal && tVal.textContent !== t) { tVal.textContent = t; animateNumber(tVal); }
  if (spectralFluxVal && spectralFluxVal.textContent !== sf) { spectralFluxVal.textContent = sf; animateNumber(spectralFluxVal); }
  if (residualCrestVal && residualCrestVal.textContent !== rcf) { residualCrestVal.textContent = rcf; animateNumber(residualCrestVal); }
  if (edgeSpikeVal && edgeSpikeVal.textContent !== esr) { edgeSpikeVal.textContent = esr; animateNumber(edgeSpikeVal); }
  if (midbandRatioVal && midbandRatioVal.textContent !== mrr) { midbandRatioVal.textContent = mrr; animateNumber(midbandRatioVal); }
  if (cycleNmseVal && cycleNmseVal.textContent !== cn) { cycleNmseVal.textContent = cn; animateNumber(cycleNmseVal); }
  if (peakFluctVal && peakFluctVal.textContent !== pf) { peakFluctVal.textContent = pf; animateNumber(peakFluctVal); }
  if (thdIVal && thdIVal.textContent !== thd) { thdIVal.textContent = thd; animateNumber(thdIVal); }
  if (hfEnergyDeltaVal && hfEnergyDeltaVal.textContent !== hfd) { hfEnergyDeltaVal.textContent = hfd; animateNumber(hfEnergyDeltaVal); }
  if (zcvVal && zcvVal.textContent !== z) { zcvVal.textContent = z; animateNumber(zcvVal); }
  if (irmsZscoreVal && irmsZscoreVal.textContent !== iz) { irmsZscoreVal.textContent = iz; animateNumber(irmsZscoreVal); }

  applyMetricHints(data);
}

db.ref("live_data").on("value", (snap) => {
  const data = snap.val();
  if (!data) return;

  syntheticDisconnectActive = false;
  lastReceiptMs = Date.now();
  lastSeenMs = getRecordEpochMs(data) || lastSeenMs || lastReceiptMs;
  lastLiveData = data;

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
  }

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
    toast("Release firmware URL filled.", "ok");
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
const mlLabelOverride = el("mlLabelOverride");
const btnDownloadSessionMl = el("btnDownloadSessionMl");
const btnDownloadAllMl = el("btnDownloadAllMl");
const btnClearMlLogs = el("btnClearMlLogs");
const btnUploadCsv = el("btnUploadCsv");
const mlCsvUpload = el("mlCsvUpload");
const mlLogStatus = el("mlLogStatus");
const mlSessionBody = el("mlSessionBody");
const btnMlTabOriginal = el("btnMlTabOriginal");
const btnMlTabProcessed = el("btnMlTabProcessed");
let currentSessionId = "";
let lastMlLogEnabled = null;
let activeMlLogTab = "original";
let mlSessionsCache = {};
const mlDurationCache = new Map();

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

btnMlTabOriginal?.addEventListener("click", () => setActiveMlLogTab("original"));
btnMlTabProcessed?.addEventListener("click", () => setActiveMlLogTab("processed"));

function downloadTextFileGeneric(filename, text, mime = "text/csv;charset=utf-8") {
  const safeName = safeCsvFilename(filename || "download.csv");
  const blob = new Blob([text], { type: mime });
  if (navigator.msSaveOrOpenBlob) {
    navigator.msSaveOrOpenBlob(blob, safeName);
    return;
  }
  const url = URL.createObjectURL(blob);
  const a = document.createElement("a");
  a.href = url;
  a.download = safeName;
  a.rel = "noopener";
  a.style.display = "none";
  document.body.appendChild(a);
  a.click();
  window.setTimeout(() => {
    a.remove();
    URL.revokeObjectURL(url);
  }, 1200);
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
  const stem = String(name || "").replace(/\.[a-z0-9]+$/i, "").trim();
  const parts = stem.split(/[_\s-]+/).filter(Boolean);
  const normParts = parts.map((p) => String(p || "").toLowerCase());

  let division = "";
  let trial = 1;
  let segmentIndex = 0;
  let divisionIdx = -1;
  let trialIdx = -1;

  for (let idx = normParts.length - 1; idx >= 0; idx--) {
    const maybeDivision = normalizeDivisionToken(normParts[idx]);
    if (maybeDivision) {
      division = maybeDivision;
      divisionIdx = idx;
      break;
    }
  }

  for (let idx = normParts.length - 1; idx >= 0; idx--) {
    if (idx === divisionIdx) continue;
    const maybeTrial = parseTrialToken(normParts[idx]);
    if (maybeTrial) {
      trial = maybeTrial;
      trialIdx = idx;
      break;
    }
  }

  let keepTokens = parts.filter((_, idx) => idx !== divisionIdx && idx !== trialIdx);
  const maybeSegment = String(keepTokens[keepTokens.length - 1] || "").match(/^(?:seg|part|slice)(\d{1,4})$/i);
  if (maybeSegment) {
    segmentIndex = Math.max(1, Number(maybeSegment[1] || 0));
    keepTokens.pop();
  }
  keepTokens = trimFilenameLoadTokens(keepTokens);

  const loadType = safeFilenameSegment(keepTokens.join("_"), "uploaded_csv").toLowerCase();
  return { loadType, trial, division, segmentIndex };
}

function safePositiveInt(value, fallback = 1) {
  const n = Number(value);
  return Number.isInteger(n) && n > 0 ? n : fallback;
}

function stripCsvExtension(name) {
  return String(name || "").replace(/\.csv$/i, "").trim();
}

function canonicalizeDatasetStem(name, fallback = "session") {
  const parsed = parseDatasetFilenameMeta(name || fallback);
  const stem = stripCsvExtension(canonicalSourceFileName(name || fallback, {
    load_type: parsed.loadType || fallback,
    trial_number: parsed.trial || 1,
    division_tag: parsed.division || "",
    segment_index: parsed.segmentIndex || 0,
  }, parsed.division || parsed.segmentIndex ? "processed" : "original"));
  return stem || fallback;
}

function loggerSessionCategory(meta) {
  return String(meta?.session_category || (meta?.processed_csv ? "processed" : "original")).toLowerCase() === "processed" ? "processed" : "original";
}

function canonicalSourceFileName(name, meta = {}, category = "original") {
  const parsed = parseDatasetFilenameMeta(name || meta?.source_file || "");
  const load = safeFilenameSegment(meta?.load_type || parsed.loadType || "session", "session").toLowerCase();
  const trial = safePositiveInt(meta?.trial_number || parsed.trial || 1, 1);
  const division = normalizeDivisionToken(meta?.division_tag || parsed.division || "");
  const segmentIndex = safePositiveInt(meta?.segment_index || parsed.segmentIndex || 0, 0);
  const parts = [load, formatTrialToken(trial)];
  if (segmentIndex > 1) parts.push(`seg${String(segmentIndex).padStart(2, "0")}`);
  if (division) parts.push(divisionFileToken(division));
  else if (category === "processed") parts.push("labeled");
  return safeCsvFilename(`${parts.join("_")}.csv`);
}

function nextTrialNumberForLoad(loadType, category = "original") {
  const token = safeFilenameSegment(loadType || "session", "session").toLowerCase();
  let maxTrial = 0;
  Object.values(mlSessionsCache || {}).forEach((meta) => {
    if (loggerSessionCategory(meta) !== category) return;
    const parsed = parseDatasetFilenameMeta(String(meta?.source_file || ""));
    const load = safeFilenameSegment(meta?.load_type || parsed.loadType || "", "").toLowerCase();
    if (load !== token) return;
    const trial = safePositiveInt(meta?.trial_number || parsed.trial || 0, 0);
    if (trial > maxTrial) maxTrial = trial;
  });
  return Math.max(1, maxTrial + 1);
}

function setActiveMlLogTab(tab) {
  activeMlLogTab = tab === "processed" ? "processed" : "original";
  btnMlTabOriginal?.classList.toggle("is-active", activeMlLogTab === "original");
  btnMlTabProcessed?.classList.toggle("is-active", activeMlLogTab === "processed");
  if (mlSessionBody) applyActiveMlSessionRow();
  if (typeof renderMlSessions === "function") renderMlSessions();
}

function deriveUploadedCsvTimingMeta(csvText) {
  const clean = String(csvText || "").replace(/^\uFEFF/, "").trim();
  if (!clean) return { rowCount: 0, durationS: null, startMs: null, endMs: null, sourceSampleRateHz: null };
  const parsed = Papa.parse(clean, { header: true, dynamicTyping: true, skipEmptyLines: true, transformHeader: normalizeUploadedCsvHeader });
  const rows = (parsed?.data || []).filter((row) => row && Object.keys(row).length);
  const rowCount = rows.length;
  if (!rowCount) return { rowCount: 0, durationS: null, startMs: null, endMs: null, sourceSampleRateHz: null };

  const epochVals = rows.map((row) => Number(row?.epoch_ms)).filter((v) => Number.isFinite(v) && v > 0);
  if (epochVals.length >= 2) {
    const startMs = epochVals[0];
    const endMs = epochVals[epochVals.length - 1];
    return { rowCount, durationS: endMs > startMs ? ((endMs - startMs) / 1000) : null, startMs, endMs, sourceSampleRateHz: null };
  }

  const uptimeDuration = buildContinuousSecondsFromField(rows, "uptime_ms", 0.001);
  if (uptimeDuration != null) {
    const nowMs = Date.now();
    return { rowCount, durationS: uptimeDuration, startMs: nowMs, endMs: nowMs + Math.round(uptimeDuration * 1000), sourceSampleRateHz: null };
  }

  const sourceFs = rows.map((row) => Number(row?.source_sample_rate_hz ?? row?.adc_fs_hz)).find((v) => Number.isFinite(v) && v > 0);
  if (Number.isFinite(sourceFs) && sourceFs > 0) {
    return { rowCount, durationS: rowCount / sourceFs, startMs: null, endMs: null, sourceSampleRateHz: sourceFs };
  }

  return { rowCount, durationS: null, startMs: null, endMs: null, sourceSampleRateHz: null };
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
  const explicit = Number(meta?.duration_s ?? meta?.source_duration_s);
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
function durationText(meta, sessionId = "") {
  const cached = sessionId ? mlDurationCache.get(sessionId) : null;
  if (Number.isFinite(cached) && cached >= 0) return formatDurationSeconds(cached);

  if (isUploadedCsvSession(meta)) {
    const uploadedDur = uploadedCsvDurationSeconds(meta);
    if (uploadedDur !== null) return formatDurationSeconds(uploadedDur);
  }

  const configuredSec = Number(meta?.duration_s ?? meta?.source_duration_s);
  const st = Number(meta?.start_ms || 0);
  const en = Number(meta?.end_ms || 0);
  const wallSec = (st && en > st) ? ((en - st) / 1000) : null;

  if (Number.isFinite(configuredSec) && configuredSec > 0 && !!meta?.closed_by_device) return formatDurationSeconds(configuredSec);
  if (Number.isFinite(configuredSec) && configuredSec > 0 && !st) return formatDurationSeconds(configuredSec);
  if (wallSec !== null && (!Number.isFinite(configuredSec) || Math.abs(wallSec - configuredSec) <= 2)) return formatDurationSeconds(Math.max(0, wallSec));
  if (Number.isFinite(configuredSec) && configuredSec > 0) return formatDurationSeconds(configuredSec);
  if (wallSec !== null) return formatDurationSeconds(Math.max(0, wallSec));
  if (!st) return "—";
  const durMs = Date.now() - st;
  return formatDurationSeconds(Math.max(0, durMs / 1000));
}

async function resolveSessionDurationSeconds(sessionId, meta = {}) {
  if (!sessionId) return null;
  if (mlDurationCache.has(sessionId)) return mlDurationCache.get(sessionId);

  const direct = uploadedCsvDurationSeconds(meta);
  if (Number.isFinite(direct) && direct > 0 && !!meta?.uploaded_csv) {
    mlDurationCache.set(sessionId, direct);
    return direct;
  }

  try {
    const csv = await fetchSessionCsv(sessionId);
    if (!csv) return null;
    const timing = deriveUploadedCsvTimingMeta(csv);
    if (Number.isFinite(timing?.durationS) && timing.durationS >= 0) {
      mlDurationCache.set(sessionId, timing.durationS);
      return timing.durationS;
    }
  } catch (err) {
    console.warn("Failed to derive accurate logger duration", sessionId, err);
  }
  return null;
}

async function hydrateVisibleSessionDurations(ids, sessionsObj) {
  const visibleIds = (ids || []).slice(0, 24);
  await Promise.all(visibleIds.map(async (sessionId) => {
    const meta = sessionsObj?.[sessionId] || {};
    const duration = await resolveSessionDurationSeconds(sessionId, meta);
    if (!Number.isFinite(duration)) return;
    const cell = mlSessionBody?.querySelector?.(`[data-duration-for="${sessionId}"]`);
    if (cell) cell.textContent = formatDurationSeconds(duration);
  }));
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
      header = lines[0].trimEnd();
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
  const sourceFile = String(meta?.source_file || "").trim();
  if (sourceFile) return safeCsvFilename(sourceFile);
  const category = loggerSessionCategory(meta);
  return safeCsvFilename(canonicalSourceFileName(sessionId, meta, category));
}

if (mlLogEnable) {
  db.ref("ml_log").on("value", (s) => {
    const v = s.val() || {};
    const enabledNow = !!v.enabled;
    mlLogEnable.checked = enabledNow;
    if (typeof v.duration_s === "number" && mlLogDur) mlLogDur.value = String(v.duration_s);
    if (mlLoadType && v.load_type) mlLoadType.value = v.load_type;
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
    const labelOv = -1;

    if (enabled) {
      const sid = makeSessionId();
      const trialNumber = nextTrialNumberForLoad(load, "original");
      const sourceFile = canonicalSourceFileName("", { load_type: load, trial_number: trialNumber }, "original");
      currentSessionId = sid;
      await db.ref("ml_log").update({ enabled: true, duration_s: dur, session_id: sid, load_type: load, label_override: labelOv });
      await db.ref(`ml_sessions/${sid}`).set({
        start_ms: firebase.database.ServerValue.TIMESTAMP,
        end_ms: null,
        load_type: load,
        duration_s: dur,
        label_override: labelOv,
        session_category: "original",
        trial_number: trialNumber,
        source_file: sourceFile
      });
      if (mlLogStatus) mlLogStatus.textContent = `Logging enabled. Session: ${sourceFile} • Total ${dur}s • Uploads on finish or buffer pressure`;
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

function renderMlSessions() {
  if (!mlSessionBody) return;
  const obj = mlSessionsCache || {};
  const ids = Object.keys(obj)
    .filter((sid) => loggerSessionCategory(obj[sid]) === activeMlLogTab)
    .sort((a,b) => (obj[b]?.start_ms||0) - (obj[a]?.start_ms||0));

  if (!ids.length) {
    mlSessionBody.innerHTML = `<tr><td colspan="7" class="muted">No ${activeMlLogTab} logs yet.</td></tr>`;
    return;
  }

  mlSessionBody.innerHTML = ids.map((sid) => {
    const meta = obj[sid] || {};
    const st = meta.start_ms ? formatDisplayTimestamp(meta.start_ms) : "—";
    const en = meta.end_ms ? formatDisplayTimestamp(meta.end_ms) : "—";
    const load = sessionLoadText(meta);
    const parsedMeta = parseDatasetFilenameMeta(String(meta?.source_file || sid || ""));
    const divisionLabel = divisionDisplayLabel(meta?.division_tag || parsedMeta.division || "", "");
    const kind = loggerSessionCategory(meta) === "processed" ? (divisionLabel ? `PROCESSED • ${divisionLabel.toUpperCase()}` : "PROCESSED") : "ORIGINAL";
    const isActiveView = sid === activeViewedSessionSid;
    return `
      <tr data-session-row="${sid}" class="${isActiveView ? "is-active-view" : ""}">
        <td><div class="mono">${sessionDisplayName(meta, sid)}</div><div class="muted small mono">${sessionSecondaryText(meta, sid)}</div></td>
        <td class="mono">${st}</td>
        <td class="mono">${en}</td>
        <td class="mono" data-duration-for="${sid}">${durationText(meta, sid)}</td>
        <td class="mono">${load}</td>
        <td class="mono">${kind}</td>
        <td class="session-actions">
          <button type="button" class="btn btn-small" data-view-sid="${sid}">View</button>
          <button type="button" class="btn btn-small" data-sid="${sid}">Download</button>
          <button type="button" class="btn btn-small btn-danger" data-del-sid="${sid}">Delete</button>
        </td>
      </tr>
    `;
  }).join("");

  mlSessionBody.querySelectorAll("button[data-sid]").forEach(btn => {
      btn.addEventListener("click", async (ev) => {
        ev.preventDefault();
        ev.stopPropagation();
        const sid = btn.getAttribute("data-sid");
        const meta = obj[sid] || {};
        const oldText = btn.textContent;
        btn.disabled = true;
        btn.textContent = "Preparing…";
        try {
          const csv = await fetchSessionCsv(sid);
          if (!csv) { toast("No data for this session yet.", "err"); return; }
          const fileName = sessionFilename(meta, sid);
          downloadTextFileGeneric(fileName, csv);
          if (mlLogStatus) mlLogStatus.textContent = `Downloaded session: ${fileName}`;
          toast(`Downloaded ${fileName}`, "ok");
        } catch (err) {
          console.error(err);
          toast("Failed to download this session.", "err");
        } finally {
          btn.disabled = false;
          btn.textContent = oldText;
        }
      });
    });

    mlSessionBody.querySelectorAll("button[data-view-sid]").forEach(btn => {
      btn.addEventListener("click", async (ev) => {
        ev.preventDefault();
        ev.stopPropagation();
        const sid = btn.getAttribute("data-view-sid");
        const meta = obj[sid] || {};
        if (typeof window.openSessionViewer !== "function") await waitForSessionViewerFns();
        if (typeof window.openSessionViewer === "function") {
          window.openSessionViewer(sid, meta);
          return;
        }
        toast("Plot viewer failed to initialize. Please refresh once.", "warn");
      });
    });

    applyActiveMlSessionRow();

  hydrateVisibleSessionDurations(ids, obj);

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
}

if (mlSessionBody) {
  db.ref("ml_sessions").limitToLast(120).on("value", (s) => {
    mlSessionsCache = s.val() || {};
    renderMlSessions();
  });
}

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


function safeSessionToken(name) {
  return String(name || "uploaded_csv")
    .replace(/\.csv$/i, "")
    .replace(/[^a-zA-Z0-9_-]+/g, "_")
    .replace(/^_+|_+$/g, "")
    .slice(0, 42) || "uploaded_csv";
}

function inferUploadSessionCategory(fileName, meta = {}) {
  const parsed = parseDatasetFilenameMeta(fileName || meta?.source_file || "");
  if (normalizeDivisionToken(meta?.division_tag || parsed.division || "")) return "processed";
  const stem = stripCsvExtension(String(fileName || "")).toLowerCase();
  if (/(?:^|[_\s-])(processed|labeled|labelled|edited|review)(?:$|[_\s-])/.test(stem)) return "processed";
  return "original";
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

async function storeUploadedCsvSession(fileName, csvText, options = {}) {
  const clean = String(csvText || "").replace(/^﻿/, "").trim();
  const lines = clean.split(/\r?\n/).filter((line) => line.trim().length);
  if (lines.length < 2) throw new Error("CSV needs a header and at least one row.");

  const category = inferUploadSessionCategory(fileName, options.meta || {}) === "processed" || options.category === "processed" ? "processed" : "original";
  const canonicalSourceFile = canonicalSourceFileName(fileName, options.meta || {}, category);
  const sid = `${category === "processed" ? "proc" : "upload"}_${Date.now()}_${safeSessionToken(canonicalSourceFile)}`;
  const header = lines[0].trimEnd();
  const rows = lines.slice(1);
  const timing = deriveUploadedCsvTimingMeta(clean);
  const fileMeta = parseDatasetFilenameMeta(canonicalSourceFile);
  const nowMs = Date.now();
  const durationS = timing.durationS;
  const durationMs = durationS != null ? Math.max(0, Math.round(durationS * 1000)) : 0;
  const startMs = Number.isFinite(timing.startMs) && timing.startMs > 0 ? timing.startMs : nowMs;
  const endMs = Number.isFinite(timing.endMs) && timing.endMs > startMs ? timing.endMs : (durationMs > 0 ? startMs + durationMs : null);

  const meta = {
    start_ms: startMs,
    end_ms: endMs,
    load_type: fileMeta.loadType || options.meta?.load_type || "uploaded_csv",
    duration_s: durationS,
    label_override: -1,
    uploaded_csv: true,
    processed_csv: category === "processed",
    session_category: category,
    source_file: canonicalSourceFile,
    original_source_file: fileName || canonicalSourceFile,
    row_count: timing.rowCount || rows.length,
    source_sample_rate_hz: timing.sourceSampleRateHz,
    trial_number: safePositiveInt(options.meta?.trial_number || fileMeta.trial || 1, 1),
    division_tag: options.meta?.division_tag || fileMeta.division || "",
    source_session_id: options.meta?.source_session_id || ""
  };

  await db.ref(`ml_sessions/${sid}`).set(meta);

  const CHUNK_ROWS = 250;
  const updates = {};
  const createdBase = Date.now();
  for (let i = 0; i < rows.length; i += CHUNK_ROWS) {
    const chunkRows = rows.slice(i, i + CHUNK_ROWS);
    const chunkSeq = (i / CHUNK_ROWS) + 1;
    const key = `chunk_${String(chunkSeq).padStart(4, "0")}`;
    updates[`ml_logs/${sid}/${key}`] = {
      csv: `${header}\n${chunkRows.join("\n")}\n`,
      count: chunkRows.length,
      created_at: createdBase + (i / CHUNK_ROWS),
      uploaded_csv: true,
      processed_csv: category === "processed",
      source_file: canonicalSourceFile,
      session_chunk_seq: chunkSeq
    };
  }
  await db.ref().update(updates);
  return { sid, meta };
}

btnUploadCsv?.addEventListener("click", () => mlCsvUpload?.click());
mlCsvUpload?.addEventListener("change", async (ev) => {
  const file = ev.target?.files?.[0];
  if (!file) return;
  try {
    const text = await file.text();
    if (!text || text.indexOf(",") < 0) throw new Error("Invalid CSV");

    let opened = false;
    try {
      const uploadMeta = parseDatasetFilenameMeta(file.name);
      const uploadCategory = inferUploadSessionCategory(file.name, uploadMeta);
      const stored = await storeUploadedCsvSession(file.name, text, { category: uploadCategory, meta: { load_type: uploadMeta.loadType, trial_number: uploadMeta.trial, division_tag: uploadMeta.division || "", segment_index: uploadMeta.segmentIndex || 0 } });
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
      const uploadCategory = inferUploadSessionCategory(file.name, fileMeta);
      const startMs = Number.isFinite(timing.startMs) && timing.startMs > 0 ? timing.startMs : Date.now();
      const endMs = Number.isFinite(timing.endMs) && timing.endMs > startMs ? timing.endMs : (timing.durationS != null ? startMs + Math.max(0, Math.round(timing.durationS * 1000)) : null);
      const meta = {
        load_type: fileMeta.loadType || "uploaded_csv",
        uploaded_csv: true,
        row_count: timing.rowCount,
        duration_s: timing.durationS,
        start_ms: startMs,
        end_ms: endMs,
        source_file: canonicalSourceFileName(file.name, { load_type: fileMeta.loadType, trial_number: fileMeta.trial, division_tag: fileMeta.division || "", segment_index: fileMeta.segmentIndex || 0 }, uploadCategory),
        session_category: uploadCategory,
        processed_csv: uploadCategory === "processed",
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

initDashboardInfoButtons();

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
