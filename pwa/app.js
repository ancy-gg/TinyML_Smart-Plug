// 1) Firebase config
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

// =============================
// 2) UI helpers
// =============================
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

const STALE_MS = 8000; // after this with no updates -> DISCONNECTED

let lastSeenMs = 0;       // when we last received live_data (browser time)
let lastServerEpoch = 0;  // server_ts / ts_epoch_ms if present
let lastIso = "";         // ts_iso if present

function toFixedOrDash(x, digits = 2) {
  if (x === null || x === undefined || x === "" || Number.isNaN(Number(x))) return "—";
  return Number(x).toFixed(digits);
}

function formatEpochMs(ms) {
  if (!ms || ms <= 0) return "—";
  const d = new Date(ms);
  const pad = (v, n=2) => String(v).padStart(n, "0");
  return `${d.getFullYear()}-${pad(d.getMonth()+1)}-${pad(d.getDate())} `
       + `${pad(d.getHours())}:${pad(d.getMinutes())}:${pad(d.getSeconds())}.`
       + `${pad(d.getMilliseconds(), 3)}`;
}

function setStatus(label, kind) {
  statusBadge.textContent = label;

  statusBadge.className = "status";
  const k = (kind || label || "DISCONNECTED").toUpperCase();

  if (k.includes("DISCON")) statusBadge.classList.add("status-DISCONNECTED");
  else if (k.includes("ARC")) statusBadge.classList.add("status-ARCING");
  else if (k.includes("OVERLOAD") || k.includes("WARN")) statusBadge.classList.add("status-WARN");
  else if (k.includes("HEAT") || k.includes("TEMP")) statusBadge.classList.add("status-WARN");
  else statusBadge.classList.add("status-OK");
}

function renderLastUpdate() {
  if (lastIso && lastIso.trim().length > 0) {
    lastUpdateText.textContent = lastIso;
    return;
  }
  if (lastServerEpoch && lastServerEpoch > 0) {
    lastUpdateText.textContent = formatEpochMs(lastServerEpoch);
    return;
  }
  if (lastSeenMs > 0) {
    lastUpdateText.textContent = "(time not synced)";
    return;
  }
  lastUpdateText.textContent = "—";
}

// =============================
// 3) Live data listener
// =============================
const liveRef = db.ref("live_data");
liveRef.on("value", (snap) => {
  const data = snap.val();
  if (!data) return;

  lastSeenMs = Date.now();

  // Firebase server timestamp if you write server_ts/.sv="timestamp"
  if (typeof data.server_ts === "number") lastServerEpoch = data.server_ts;

  // Device time (if you send it)
  if (typeof data.ts_epoch_ms === "number" && data.ts_epoch_ms > 0) lastServerEpoch = data.ts_epoch_ms;
  if (typeof data.ts_iso === "string") lastIso = data.ts_iso;

  vVal.textContent   = toFixedOrDash(data.voltage, 1);
  iVal.textContent   = toFixedOrDash(data.current, 2);
  tVal.textContent   = toFixedOrDash(data.temp, 1);
  zcvVal.textContent = toFixedOrDash(data.zcv, 2);
  thdVal.textContent = toFixedOrDash(data.thd, 1);
  entVal.textContent = toFixedOrDash(data.entropy, 3);

  const status = (data.status ?? "CONNECTED").toString();
  setStatus(status.toUpperCase(), status.toUpperCase());
  renderLastUpdate();
}, (err) => {
  console.error(err);
  setStatus("DISCONNECTED", "DISCONNECTED");
  renderLastUpdate();
});

// =============================
// 4) History (latest 50)
// =============================
const histRef = db.ref("history").orderByChild("server_ts").limitToLast(50);

histRef.on("value", (snap) => {
  const obj = snap.val();
  if (!obj) {
    historyBody.innerHTML = `<tr><td colspan="8" class="muted">No history yet.</td></tr>`;
    return;
  }

  const arr = Object.values(obj);
  arr.sort((a, b) => (b.server_ts || b.ts_epoch_ms || 0) - (a.server_ts || a.ts_epoch_ms || 0));

  const rows = arr.map((r) => {
    const timeStr =
      (r.ts_iso && r.ts_iso.trim()) ? r.ts_iso :
      formatEpochMs(r.server_ts || r.ts_epoch_ms || 0);

    const status = (r.status ?? "").toString();
    const upper = status.toUpperCase();

    const statusClass =
      upper.includes("ARC") ? "pill pill-ARC" :
      upper.includes("DISCON") ? "pill pill-DIS" :
      upper.includes("WARN") || upper.includes("OVER") ? "pill pill-WARN" :
      "pill pill-OK";

    return `
      <tr>
        <td class="mono">${timeStr || "—"}</td>
        <td><span class="${statusClass}">${status || "—"}</span></td>
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
});

// =============================
// 5) Disconnected watchdog
// =============================
setInterval(() => {
  if (!lastSeenMs) {
    setStatus("DISCONNECTED", "DISCONNECTED");
    renderLastUpdate();
    return;
  }
  const age = Date.now() - lastSeenMs;
  if (age > STALE_MS) setStatus("DISCONNECTED", "DISCONNECTED");
}, 500);

// =============================
// 6) PWA service worker
// =============================
if ("serviceWorker" in navigator) {
  window.addEventListener("load", () => {
    navigator.serviceWorker.register("sw.js").catch(console.error);
  });
}
