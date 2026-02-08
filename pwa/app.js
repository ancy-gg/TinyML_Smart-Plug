// =============================
// TinyML Smart Plug PWA - app.js
// =============================

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

const vVal   = el("vVal");
const iVal   = el("iVal");
const tVal   = el("tVal");
const zcvVal = el("zcvVal");
const thdVal = el("thdVal");
const entVal = el("entVal");

const historyBody = el("historyBody");
const btnClearHistory = el("btnClearHistory");
const btnDownloadCSV = el("btnDownloadCSV");
const toastEl = el("toast");

const STALE_MS = 8000;
const DISPLAY_TZ = "Asia/Manila";

let lastSeenMs = 0;
let lastEpochMs = 0;
let lastStatus = "DISCONNECTED";

function toast(msg, kind = "ok") {
  if (!toastEl) return;
  toastEl.textContent = msg;
  toastEl.className = `toast toast-${kind} show`;
  clearTimeout(toastEl._t);
  toastEl._t = setTimeout(() => {
    toastEl.className = `toast toast-${kind}`;
  }, 2400);
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

function classifyStatus(s) {
  const u = (s || "").toUpperCase();
  if (u.includes("DISCON")) return "DISCONNECTED";
  if (u.includes("ARC")) return "ARCING";
  if (u.includes("HEAT")) return "HEATING";
  if (u.includes("OVER")) return "OVERLOAD";
  if (u.includes("WARN")) return "WARNING";
  if (u.includes("NORM")) return "NORMAL";
  return u || "OK";
}

function statusBadgeHTML(kind) {
  switch (kind) {
    case "OVERLOAD":
      return `OVERLOAD <span class="emoji blink">⚠️</span>`;
    case "HEATING":
      return `HEATING <span class="emoji flicker">🔥</span>`;
    case "ARCING":
      return `ARCING <span class="emoji zap">⚡</span>`;
    case "DISCONNECTED":
      return `DISCONNECTED`;
    case "NORMAL":
      return `NORMAL`;
    default:
      return `${kind}`;
  }
}

function setTopStatus(kind) {
  statusBadge.className = "status";
  const k = classifyStatus(kind);

  if (k === "DISCONNECTED") statusBadge.classList.add("status-DISCONNECTED");
  else if (k === "OVERLOAD") statusBadge.classList.add("status-OVERLOAD");
  else if (k === "HEATING") statusBadge.classList.add("status-HEATING");
  else if (k === "ARCING") statusBadge.classList.add("status-ARCING");
  else if (k === "WARNING") statusBadge.classList.add("status-WARN");
  else statusBadge.classList.add("status-OK");

  statusBadge.innerHTML = statusBadgeHTML(k);

  // Animate only when NOT disconnected
  if (k !== "DISCONNECTED") {
    statusBadge.classList.remove("bump");
    void statusBadge.offsetWidth; // reflow
    statusBadge.classList.add("bump");
  }

  lastStatus = k;
}

function renderLastUpdate() {
  lastUpdateText.textContent = formatEpochMsTZ(lastEpochMs);
}

function pillHTML(kind) {
  const k = classifyStatus(kind);
  if (k === "OVERLOAD") return `<span class="pill pill-OVERLOAD">OVERLOAD <span class="emoji blink">⚠️</span></span>`;
  if (k === "HEATING") return `<span class="pill pill-HEATING">HEATING <span class="emoji flicker">🔥</span></span>`;
  if (k === "ARCING") return `<span class="pill pill-ARCING">ARCING <span class="emoji zap">⚡</span></span>`;
  if (k === "DISCONNECTED") return `<span class="pill pill-DIS">DISCONNECTED</span>`;
  if (k === "NORMAL") return `<span class="pill pill-OK">NORMAL</span>`;
  return `<span class="pill pill-OK">${k}</span>`;
}

// Animate number updates (CSS class toggled)
function animateNumber(elm) {
  if (!elm) return;
  elm.classList.remove("tick");
  void elm.offsetWidth;
  elm.classList.add("tick");
}

// =============================
// Live data
// =============================
db.ref("live_data").on("value", (snap) => {
  const data = snap.val();
  if (!data) return;

  lastSeenMs = Date.now();

  if (typeof data.server_ts === "number" && data.server_ts > 0) lastEpochMs = data.server_ts;
  else if (typeof data.ts_epoch_ms === "number" && data.ts_epoch_ms > 0) lastEpochMs = data.ts_epoch_ms;

  const v = toFixedOrDash(data.voltage, 1);
  const i = toFixedOrDash(data.current, 2);
  const t = toFixedOrDash(data.temp, 1);
  const z = toFixedOrDash(data.zcv, 2);
  const th= toFixedOrDash(data.thd, 1);
  const en= toFixedOrDash(data.entropy, 3);

  // Only animate if changed
  if (vVal.textContent !== v) { vVal.textContent = v; animateNumber(vVal); }
  if (iVal.textContent !== i) { iVal.textContent = i; animateNumber(iVal); }
  if (tVal.textContent !== t) { tVal.textContent = t; animateNumber(tVal); }
  if (zcvVal.textContent !== z) { zcvVal.textContent = z; animateNumber(zcvVal); }
  if (thdVal.textContent !== th) { thdVal.textContent = th; animateNumber(thdVal); }
  if (entVal.textContent !== en) { entVal.textContent = en; animateNumber(entVal); }

  const status = (data.status ?? "OK").toString();
  setTopStatus(status);
  renderLastUpdate();
}, (err) => {
  console.error(err);
  setTopStatus("DISCONNECTED");
  lastEpochMs = 0;
  renderLastUpdate();
});

// =============================
// History (range filter)
// =============================
const rangeSelect = document.getElementById("rangeSelect");
const historyHint = document.getElementById("historyHint");

// Pull enough data for 30 days. Increase if you log very frequently.
const HISTORY_LIMIT = 5000;

// cache last fetched history
let historyCache = [];

// Helpers for time ranges in Asia/Manila
function startOfDayEpochMs(epochMs) {
  // Convert epochMs to a Date, then compute midnight in DISPLAY_TZ via Intl parts.
  const d = new Date(epochMs);

  const parts = new Intl.DateTimeFormat("en-CA", {
    timeZone: DISPLAY_TZ,
    year: "numeric",
    month: "2-digit",
    day: "2-digit"
  }).formatToParts(d);

  const get = (t) => parts.find(p => p.type === t)?.value ?? "00";
  const yyyy = Number(get("year"));
  const MM = Number(get("month"));
  const dd = Number(get("day"));

  // Create a Date in UTC for that local midnight by using "YYYY-MM-DDT00:00:00" in DISPLAY_TZ:
  // JS can't directly construct TZ-local Date, so we approximate by:
  // - take the "local date components" and treat them as if they're in DISPLAY_TZ
  // - compute using Intl offset via Date parsing trick:
  const iso = `${yyyy.toString().padStart(4,"0")}-${MM.toString().padStart(2,"0")}-${dd.toString().padStart(2,"0")}T00:00:00`;
  // Interpret iso as local system time; then we correct by using the epoch of formatted time itself is tricky.
  // Simpler & stable for filtering: use DISPLAY_TZ formatted strings comparison windows via epoch boundaries:
  // We'll compute boundaries relative to "now" using epoch, and then compare with epoch directly:
  // So this function is not used for exact tz-midnight conversions. We'll instead compute ranges using "now" and subtract days.
  return new Date(iso).getTime();
}

// More robust range boundaries: use "now" in epoch and subtract durations,
// then rely on DISPLAY_TZ formatting only for display, not boundary math.
function getRangeBounds(rangeKey) {
  const now = Date.now();

  // Determine today's midnight in DISPLAY_TZ by using formatted "YYYY-MM-DD" for now,
  // then parsing that as local and using it as boundary (good enough for PH users).
  const todayStr = new Intl.DateTimeFormat("en-CA", {
    timeZone: DISPLAY_TZ,
    year: "numeric",
    month: "2-digit",
    day: "2-digit"
  }).format(new Date(now)); // "YYYY-MM-DD"
  const todayMidnight = new Date(`${todayStr}T00:00:00`).getTime();

  if (rangeKey === "today") {
    return { start: todayMidnight, end: todayMidnight + 24*3600*1000 };
  }
  if (rangeKey === "yesterday") {
    const start = todayMidnight - 24*3600*1000;
    return { start, end: todayMidnight };
  }
  if (rangeKey === "7d") {
    return { start: now - 7*24*3600*1000, end: now + 1 };
  }
  if (rangeKey === "30d") {
    return { start: now - 30*24*3600*1000, end: now + 1 };
  }
  return { start: now - 7*24*3600*1000, end: now + 1 };
}

function applyHistoryFilter() {
  const key = rangeSelect?.value || "7d";
  const bounds = getRangeBounds(key);

  const filtered = historyCache.filter(r => {
    const epoch = (r.server_ts || r.ts_epoch_ms || 0);
    return epoch >= bounds.start && epoch < bounds.end;
  });

  // newest first
  filtered.sort((a, b) => (b.server_ts || b.ts_epoch_ms || 0) - (a.server_ts || a.ts_epoch_ms || 0));

  // Update hint
  const label =
    key === "today" ? "Today" :
    key === "yesterday" ? "Yesterday" :
    key === "30d" ? "Last 30 days" :
    "Last 7 days";

  if (historyHint) historyHint.textContent = `Showing: ${label} (${filtered.length})`;

  if (!filtered.length) {
    historyBody.innerHTML = `<tr><td colspan="8" class="muted">No history in this range.</td></tr>`;
    return;
  }

  const rows = filtered.slice(0, 300).map((r, idx) => { // cap render for speed
    const epoch = (r.server_ts || r.ts_epoch_ms || 0);
    const timeStr = formatEpochMsTZ(epoch);

    return `
      <tr class="row-in" style="animation-delay:${Math.min(idx, 10) * 25}ms">
        <td class="mono">${timeStr}</td>
        <td>${pillHTML(r.status)}</td>
        <td class="mono">${toFixedOrDash(r.voltage, 1)}</td>
        <td class="mono">${toFixedOrDash(r.current, 2)}</td>
        <td class="mono">${toFixedOrDash(r.temp, 1)}</td>
        <td class="mono">${toFixedOrDash(r.zcv, 2)}</td>
        <td class="mono">${toFixedOrDash(r.thd, 1)}</td>
        <td class="mono">${toFixedOrDash(r.entropy, 3)}</td>
      </tr>
    `;
  }).join("");

  historyBody.innerHTML = rows;
}

function currentRangeLabel() {
  const key = rangeSelect?.value || "7d";
  if (key === "today") return "today";
  if (key === "yesterday") return "yesterday";
  if (key === "30d") return "last30days";
  return "last7days";
}

function escapeCSV(val) {
  const s = String(val ?? "");
  // Quote if it contains comma, quote, newline
  if (/[",\n]/.test(s)) return `"${s.replace(/"/g, '""')}"`;
  return s;
}

function downloadCSV(rows) {
  // Columns for training
  const header = [
    "timestamp",
    "epoch_ms",
    "status",
    "voltage",
    "current",
    "temp",
    "zcv",
    "thd",
    "entropy"
  ];

  const lines = [header.join(",")];

  for (const r of rows) {
    const epoch = (r.server_ts || r.ts_epoch_ms || 0);
    const ts = formatEpochMsTZ(epoch);

    const line = [
      escapeCSV(ts),
      escapeCSV(epoch),
      escapeCSV(r.status ?? ""),
      escapeCSV(r.voltage ?? ""),
      escapeCSV(r.current ?? ""),
      escapeCSV(r.temp ?? ""),
      escapeCSV(r.zcv ?? ""),
      escapeCSV(r.thd ?? ""),
      escapeCSV(r.entropy ?? "")
    ].join(",");

    lines.push(line);
  }

  const blob = new Blob([lines.join("\n")], { type: "text/csv;charset=utf-8" });
  const url = URL.createObjectURL(blob);

  const a = document.createElement("a");
  const fname = `TSP_${currentRangeLabel()}_${new Date().toISOString().slice(0,10)}.csv`;
  a.href = url;
  a.download = fname;
  document.body.appendChild(a);
  a.click();
  a.remove();
  URL.revokeObjectURL(url);
}

if (btnDownloadCSV) {
  btnDownloadCSV.addEventListener("click", () => {
    const key = rangeSelect?.value || "7d";
    const { start, end } = getRangeBounds(key);

    // Use current cache, filter same as UI
    const filtered = historyCache
      .filter(r => {
        const epoch = (r.server_ts || r.ts_epoch_ms || 0);
        return epoch >= start && epoch < end;
      })
      .sort((a, b) => (a.server_ts || a.ts_epoch_ms || 0) - (b.server_ts || b.ts_epoch_ms || 0)); // oldest->newest

    if (!filtered.length) {
      toast("No data in this range.", "err");
      return;
    }

    downloadCSV(filtered);
    toast("CSV downloaded.", "ok");
  });
}

// Fetch history
db.ref("history").orderByChild("server_ts").limitToLast(HISTORY_LIMIT).on("value", (snap) => {
  const obj = snap.val();
  if (!obj) {
    historyCache = [];
    applyHistoryFilter();
    return;
  }
  historyCache = Object.values(obj);
  applyHistoryFilter();
});

if (rangeSelect) {
  rangeSelect.addEventListener("change", applyHistoryFilter);
}

// =============================
// Clear history button
// =============================
if (btnClearHistory) {
  btnClearHistory.addEventListener("click", async () => {
    const ok = confirm("Clear ALL history logs in Firebase? This cannot be undone.");
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
      btnClearHistory.textContent = "Clear history";
    }
  });
}

// =============================
// Disconnected watchdog
// =============================
setInterval(() => {
  if (!lastSeenMs) {
    setTopStatus("DISCONNECTED");
    return;
  }
  const age = Date.now() - lastSeenMs;
  if (age > STALE_MS) setTopStatus("DISCONNECTED");
}, 500);

// =============================
// PWA service worker
// =============================
if ("serviceWorker" in navigator) {
  window.addEventListener("load", async () => {
    try {
      const reg = await navigator.serviceWorker.register("sw.js");

      // If a new SW is waiting, activate it immediately
      if (reg.waiting) reg.waiting.postMessage({ type: "SKIP_WAITING" });

      reg.addEventListener("updatefound", () => {
        const sw = reg.installing;
        if (!sw) return;
        sw.addEventListener("statechange", () => {
          if (sw.state === "installed" && navigator.serviceWorker.controller) {
            // New version installed; reload to use it
            window.location.reload();
          }
        });
      });
    } catch (e) {
      console.error(e);
    }
  });
}