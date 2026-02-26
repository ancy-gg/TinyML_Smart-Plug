// =============================
// TinyML Smart Plug PWA - app.js
// - No lag between history and top status
// - Shows Device IP when connected
// - CSV export + today/yesterday/7d/30d filters
// - OTA Release Control (writes /ota/*)
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

// ---------- DOM helpers ----------
const el = (id) => document.getElementById(id);

const statusBadge = el("statusBadge");
const lastUpdateText = el("lastUpdateText");

const deviceIpLine = el("deviceIpLine");
const deviceIpText = el("deviceIpText");

const vVal   = el("vVal");
const iVal   = el("iVal");
const tVal   = el("tVal");
const zcvVal = el("zcvVal");
const thdVal = el("thdVal");
const entVal = el("entVal");

const historyBody = el("historyBody");
const rangeSelect = el("rangeSelect");
const historyHint = el("historyHint");

const btnDownloadCSV = el("btnDownloadCSV");
const btnClearHistory = el("btnClearHistory");
const toastEl = el("toast");

// ---------- Settings ----------
const DISPLAY_TZ = "Asia/Manila";
const STALE_MS = 12000;
const HISTORY_LIMIT = 5000;
const MAX_RENDER_ROWS = 300;

// ---------- OTA repo host (your GitHub firmware folder) ----------
const OTA_REPO_RAW_BASE = "https://raw.githubusercontent.com/ancy-gg/TinyML_Smart-Plug/main/firmware/";
const OTA_DEFAULT_BIN   = "firmware.bin";

// ---------- State ----------
let lastSeenMs = 0;
let lastEpochMs = 0;
let topStatusSourceEpoch = 0;

let historyCache = [];
let currentFilteredHistory = [];

// ---------- OTA helpers ----------
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

// ---------- Toast ----------
function toast(msg, kind = "ok") {
  if (!toastEl) return;
  toastEl.textContent = msg;
  toastEl.className = `toast toast-${kind} show`;
  clearTimeout(toastEl._t);
  toastEl._t = setTimeout(() => {
    toastEl.className = `toast toast-${kind}`;
  }, 2400);
}

// ---------- Formatting ----------
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

// ---------- Status mapping ----------
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
    case "OVERLOAD": return `OVERLOAD <span class="emoji blink">⚠️</span>`;
    case "HEATING":  return `HEATING <span class="emoji flicker">🔥</span>`;
    case "ARCING":   return `ARCING <span class="emoji zap">⚡</span>`;
    case "DISCONNECTED": return `DISCONNECTED`;
    case "NORMAL": return `NORMAL`;
    default: return `${kind}`;
  }
}

function setTopStatus(kind) {
  if (!statusBadge) return;

  const k = classifyStatus(kind);
  statusBadge.className = "status";

  if (k === "DISCONNECTED") statusBadge.classList.add("status-DISCONNECTED");
  else if (k === "OVERLOAD") statusBadge.classList.add("status-OVERLOAD");
  else if (k === "HEATING") statusBadge.classList.add("status-HEATING");
  else if (k === "ARCING") statusBadge.classList.add("status-ARCING");
  else if (k === "WARNING") statusBadge.classList.add("status-WARN");
  else statusBadge.classList.add("status-OK");

  statusBadge.innerHTML = statusBadgeHTML(k);

  if (k !== "DISCONNECTED") {
    statusBadge.classList.remove("bump");
    void statusBadge.offsetWidth;
    statusBadge.classList.add("bump");
  }
}

function pillHTML(kind) {
  const k = classifyStatus(kind);
  if (k === "OVERLOAD") return `<span class="pill pill-OVERLOAD">OVERLOAD <span class="emoji blink">⚠️</span></span>`;
  if (k === "HEATING")  return `<span class="pill pill-HEATING">HEATING <span class="emoji flicker">🔥</span></span>`;
  if (k === "ARCING")   return `<span class="pill pill-ARCING">ARCING <span class="emoji zap">⚡</span></span>`;
  if (k === "DISCONNECTED") return `<span class="pill pill-DIS">DISCONNECTED</span>`;
  if (k === "NORMAL") return `<span class="pill pill-OK">NORMAL</span>`;
  return `<span class="pill pill-OK">${k}</span>`;
}

function animateNumber(node) {
  if (!node) return;
  node.classList.remove("tick");
  void node.offsetWidth;
  node.classList.add("tick");
}

// ---------- Range filter ----------
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
    historyBody.innerHTML = `<tr><td colspan="8" class="muted">No fault history in this range.</td></tr>`;
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
if (rangeSelect) rangeSelect.addEventListener("change", applyHistoryFilter);

// ---------- LIVE DATA ----------
db.ref("live_data").on("value", (snap) => {
  const data = snap.val();
  if (!data) return;

  lastSeenMs = Date.now();

  // device IP line
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

  // timestamps
  const liveEpoch =
    (typeof data.server_ts === "number" && data.server_ts > 0) ? data.server_ts :
    (typeof data.ts_epoch_ms === "number" && data.ts_epoch_ms > 0) ? data.ts_epoch_ms :
    (typeof data.ts_iso === "string") ? parseTsIsoToEpoch(data.ts_iso) : 0;

  if (liveEpoch > 0) lastEpochMs = liveEpoch;

  // values
  const v  = toFixedOrDash(data.voltage, 1);
  const i  = toFixedOrDash(data.current, 2);
  const t  = toFixedOrDash(data.temp, 1);
  const z  = toFixedOrDash(data.zcv, 2);
  const th = toFixedOrDash(data.thd, 1);
  const en = toFixedOrDash(data.entropy, 3);

  if (vVal && vVal.textContent !== v) { vVal.textContent = v; animateNumber(vVal); }
  if (iVal && iVal.textContent !== i) { iVal.textContent = i; animateNumber(iVal); }
  if (tVal && tVal.textContent !== t) { tVal.textContent = t; animateNumber(tVal); }
  if (zcvVal && zcvVal.textContent !== z) { zcvVal.textContent = z; animateNumber(zcvVal); }
  if (thdVal && thdVal.textContent !== th) { thdVal.textContent = th; animateNumber(thdVal); }
  if (entVal && entVal.textContent !== en) { entVal.textContent = en; animateNumber(entVal); }

  // NO LAG: only update top status if this live update is newer than what we've shown
  if (liveEpoch >= topStatusSourceEpoch) {
    topStatusSourceEpoch = liveEpoch;
    setTopStatus((data.status ?? "OK").toString());
    if (lastUpdateText) lastUpdateText.textContent = formatEpochMsTZ(liveEpoch);
  } else {
    if (lastUpdateText && topStatusSourceEpoch > 0) lastUpdateText.textContent = formatEpochMsTZ(topStatusSourceEpoch);
  }

}, (err) => {
  console.error(err);
  setTopStatus("DISCONNECTED");
  if (lastUpdateText) lastUpdateText.textContent = "—";
  if (deviceIpLine) deviceIpLine.style.display = "none";
});

// ---------- HISTORY ----------
db.ref("history")
  .orderByChild("server_ts")
  .limitToLast(HISTORY_LIMIT)
  .on("value", (snap) => {
    const obj = snap.val();
    if (!obj) {
      historyCache = [];
      applyHistoryFilter();
      return;
    }

    historyCache = Object.values(obj);

    // NO LAG: find most recent history item and, if newer than top, drive top badge
    let bestEpoch = 0;
    let bestRec = null;
    for (const r of historyCache) {
      const ep = getRecordEpochMs(r);
      if (ep > bestEpoch) {
        bestEpoch = ep;
        bestRec = r;
      }
    }

    if (bestRec && bestEpoch > topStatusSourceEpoch) {
      topStatusSourceEpoch = bestEpoch;
      setTopStatus((bestRec.status ?? "OK").toString());
      if (lastUpdateText) lastUpdateText.textContent = formatEpochMsTZ(bestEpoch);
    }

    applyHistoryFilter();
  });

// ---------- CSV ----------
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

if (btnDownloadCSV) {
  btnDownloadCSV.addEventListener("click", () => {
    if (!currentFilteredHistory.length) {
      toast("No data to export in this range.", "err");
      return;
    }

    const rows = currentFilteredHistory
      .slice()
      .sort((a, b) => getRecordEpochMs(a) - getRecordEpochMs(b)); // oldest->newest

    const header = ["timestamp","epoch_ms","status","voltage","current","temp","zcv","thd","entropy"];
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
        csvEscape(r.zcv ?? ""),
        csvEscape(r.thd ?? ""),
        csvEscape(r.entropy ?? "")
      ].join(","));
    }

    const key = rangeSelect?.value || "range";
    downloadTextFile(`TSP_faults_${key}_${new Date().toISOString().slice(0,10)}.csv`, lines.join("\n"));
    toast("CSV downloaded.", "ok");
  });
}

// ---------- Clear history ----------
if (btnClearHistory) {
  btnClearHistory.addEventListener("click", async () => {
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
      btnClearHistory.textContent = "Clear history";
    }
  });
}

// ---------- Disconnected watchdog ----------
setInterval(() => {
  if (!lastSeenMs) {
    setTopStatus("DISCONNECTED");
    return;
  }
  if (Date.now() - lastSeenMs > STALE_MS) {
    setTopStatus("DISCONNECTED");
    if (deviceIpLine) deviceIpLine.style.display = "none";
  }
}, 500);

// ---------- OTA (Option B: PWA writes /ota/*) ----------
(function initOta() {
  const otaCurVer = el("otaCurVer");
  const otaCurUrl = el("otaCurUrl");

  const otaDesiredVer = el("otaDesiredVer");
  const otaBinName = el("otaBinName");
  const otaFirmwareUrl = el("otaFirmwareUrl");

  const btnFillRepoUrl = el("btnFillRepoUrl");
  const btnPublishOta = el("btnPublishOta");

  // If panel isn't present, do nothing (prevents console errors)
  if (!btnPublishOta || !otaDesiredVer) return;

  // Live read current RTDB values
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

      // After this, you'll see "ota" appear in Firebase console.
      toast("OTA published. Devices will pull on next check.", "ok");
    } catch (e) {
      console.error(e);
      toast("Publish failed (Firebase rules may block writes).", "err");
    } finally {
      btnPublishOta.disabled = false;
      btnPublishOta.textContent = oldText || "Publish OTA";
    }
  });
})();

// ---------- TinyML Logger (Session-based) ----------
const mlLogEnable = el("mlLogEnable");
const mlLogDur = el("mlLogDur");
const mlLoadType = el("mlLoadType");
const mlLabelOverride = el("mlLabelOverride");

const btnDownloadSessionMl = el("btnDownloadSessionMl");
const btnDownloadAllMl = el("btnDownloadAllMl");
const btnClearMlLogs = el("btnClearMlLogs");

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

function makeSessionId() {
  return "sess_" + Date.now();
}
function labelText(v) {
  if (String(v) === "1") return "ARC";
  if (String(v) === "0") return "NORMAL";
  return "AUTO";
}

async function fetchSessionCsv(sessionId) {
  const snap = await db.ref(`ml_logs/${sessionId}`).get();
  if (!snap.exists()) return "";

  const chunksObj = snap.val() || {};
  const keys = Object.keys(chunksObj);

  keys.sort((a,b) => (chunksObj[a]?.created_at || 0) - (chunksObj[b]?.created_at || 0));

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

// Sync control state
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

      await db.ref("ml_log").update({
        enabled: true,
        duration_s: dur,
        session_id: sid,
        load_type: load,
        label_override: labelOv
      });

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

      if (sid) {
        await db.ref(`ml_sessions/${sid}`).update({
          end_ms: firebase.database.ServerValue.TIMESTAMP
        });
      }

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

// Session list UI
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
          <td class="mono">${String(load)}</td>
          <td class="mono">${lab}</td>
          <td><button class="btn btn-small" data-sid="${sid}">Download</button></td>
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
  });
}

// Download current session
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

// Download ALL sessions combined
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

// Clear logs button
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

// ---------- Service worker ----------
if ("serviceWorker" in navigator) {
  navigator.serviceWorker.register("sw.js").then((reg) => {
    reg.update();
    let refreshing = false;
    navigator.serviceWorker.addEventListener("controllerchange", () => {
      if (refreshing) return;
      refreshing = true;
      window.location.reload();
    });
  }).catch(console.error);
}