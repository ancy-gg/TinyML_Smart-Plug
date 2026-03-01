// session.js (TSPweb-v0.3.4)
console.log("session.js loaded: TSPweb-v0.3.4");

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

if (!firebase.apps?.length) firebase.initializeApp(firebaseConfig);
const db = firebase.database();

const el = (id) => document.getElementById(id);

const sid = new URLSearchParams(location.search).get("sid") || "";
const titleEl = el("title");
const metaEl = el("meta");
const toggleList = el("toggleList");
const statusLine = el("statusLine");
const valueLine = el("valueLine");

const btnBack = el("btnBack");
const btnDownload = el("btnDownload");

const btnPlay = el("btnPlay");
const btnPause = el("btnPause");
const selSpeed = el("selSpeed");
const scrub = el("scrub");
const timeReadout = el("timeReadout");
const chkNormalize = el("chkNormalize");
const chkInterp = el("chkInterp");
const chkFollow = el("chkFollow");
const btnResetZoom = el("btnResetZoom");

btnBack.onclick = () => (location.href = "index.html");

function downloadTextFile(filename, text, mime="text/csv;charset=utf-8") {
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
  return header + "\n" + rows.join("\n") + "\n";
}

function pickTimeAxis(rows) {
  const hasEpoch = rows.length && rows[0].epoch_ms !== undefined;
  if (!hasEpoch) return rows.map((_, i) => i);

  const t0 = Number(rows[0].epoch_ms) || 0;
  return rows.map(r => ((Number(r.epoch_ms) || t0) - t0) / 1000.0);
}

function buildSeriesKeys(rows) {
  if (!rows.length) return [];
  const keys = Object.keys(rows[0]);
  return keys.filter(k => {
    if (k === "timestamp" || k === "session_id" || k === "load_type") return false;
    if (k === "epoch_ms") return false;
    const v = rows[0][k];
    return v !== "" && v !== null && v !== undefined && !Number.isNaN(Number(v));
  });
}

function fmt(v) {
  if (v == null || !Number.isFinite(v)) return "—";
  const av = Math.abs(v);
  if (av >= 100) return v.toFixed(1);
  if (av >= 10) return v.toFixed(2);
  if (av >= 1) return v.toFixed(3);
  return v.toFixed(4);
}

function avgDt(x) {
  if (!x || x.length < 2) return 1;
  return (x[x.length - 1] - x[0]) / (x.length - 1);
}

function binarySearchNearest(arr, val) {
  let lo = 0, hi = arr.length - 1;
  while (lo < hi) {
    const mid = (lo + hi) >> 1;
    if (arr[mid] < val) lo = mid + 1;
    else hi = mid;
  }
  if (lo <= 0) return 0;
  if (lo >= arr.length) return arr.length - 1;
  const a = arr[lo - 1], b = arr[lo];
  return (Math.abs(val - a) <= Math.abs(val - b)) ? (lo - 1) : lo;
}

const palette = [
  "#2ecc71", "#e74c3c", "#3498db", "#f1c40f",
  "#9b59b6", "#1abc9c", "#e67e22", "#ecf0f1",
  "#00bcd4", "#ff4081", "#cddc39", "#ff9800",
];

let plot = null;

// Playback state
let playing = false;
let playIdxF = 0;
let playIdx = 0;
let lastRAF = 0;
let speed = 1.0;

// Data
let ROWS = [];
let X = [];
let KEYS = [];
let DATA_RAW = null;
let DATA_NORM = null;

let FULL_X_MIN = 0;
let FULL_X_MAX = 0;

// User preferences
const showPref = new Map(); // key -> bool
const axisPref = new Map(); // key -> "y"|"y2"

function makeData(rows, x, keys) {
  const data = [x];
  keys.forEach(k => {
    data.push(rows.map(r => {
      const v = r[k];
      const num = (typeof v === "number") ? v : Number(String(v ?? "").trim());
      return Number.isFinite(num) ? num : null;
    }));
  });
  return data;
}

function makeNormalizedData(dataRaw) {
  const out = [dataRaw[0]];
  for (let si = 1; si < dataRaw.length; si++) {
    const arr = dataRaw[si];
    let mn = Infinity, mx = -Infinity;
    for (let i = 0; i < arr.length; i++) {
      const v = arr[i];
      if (v == null) continue;
      if (v < mn) mn = v;
      if (v > mx) mx = v;
    }
    const den = (mx - mn);
    const norm = new Array(arr.length);
    for (let i = 0; i < arr.length; i++) {
      const v = arr[i];
      if (v == null) norm[i] = null;
      else norm[i] = (den > 1e-12) ? ((v - mn) / den) : 0.0;
    }
    out.push(norm);
  }
  return out;
}

// Plugin: zoom/pan + click to move playhead
function zoomPanPlugin() {
  let isPanning = false;
  let panStartX = 0;
  let startMin = 0, startMax = 0;

  return {
    hooks: {
      ready: (u) => {
        u.root.addEventListener("dblclick", (e) => { e.preventDefault(); resetZoom(); });

        u.over.addEventListener("wheel", (e) => {
          e.preventDefault();
          const rect = u.over.getBoundingClientRect();
          const px = e.clientX - rect.left;
          const xVal = u.posToVal(px, "x");
          const sc = u.scales.x;
          const min = sc.min, max = sc.max;
          const range = max - min;

          const zoomIn = e.deltaY < 0;
          const factor = zoomIn ? 0.85 : 1.18;
          const newRange = range * factor;

          const leftRatio = (xVal - min) / range;
          const newMin = xVal - leftRatio * newRange;
          const newMax = newMin + newRange;

          u.setScale("x", { min: newMin, max: newMax });
        }, { passive: false });

        u.over.addEventListener("mousedown", (e) => {
          if (e.button !== 0) return;
          if (!e.shiftKey) return;
          isPanning = true;
          panStartX = e.clientX;
          startMin = u.scales.x.min;
          startMax = u.scales.x.max;
          e.preventDefault();
        });

        window.addEventListener("mousemove", (e) => {
          if (!isPanning) return;
          const rect = u.over.getBoundingClientRect();
          const x0 = panStartX - rect.left;
          const x1 = e.clientX - rect.left;
          const v0 = u.posToVal(x0, "x");
          const v1 = u.posToVal(x1, "x");
          const dv = v0 - v1;
          u.setScale("x", { min: startMin + dv, max: startMax + dv });
        });

        window.addEventListener("mouseup", () => { isPanning = false; });

        u.over.addEventListener("click", (e) => {
          const rect = u.over.getBoundingClientRect();
          const px = e.clientX - rect.left;
          const xVal = u.posToVal(px, "x");
          const idx = binarySearchNearest(X, xVal);
          setPlayIdx(idx, true);
        });
      },

      setSelect: (u) => {
        const sel = u.select;
        if (!sel || sel.width < 10) return;
        const xMin = u.posToVal(sel.left, "x");
        const xMax = u.posToVal(sel.left + sel.width, "x");
        u.setScale("x", { min: xMin, max: xMax });
        u.setSelect({ left: 0, width: 0, top: 0, height: 0 });
      }
    }
  };
}

// Plugin: draw vertical playhead
function playheadPlugin() {
  return {
    hooks: {
      draw: (u) => {
        if (!X.length) return;
        const idx = Math.max(0, Math.min(X.length - 1, playIdx));
        const xVal = X[idx];
        const xPos = u.valToPos(xVal, "x", true);
        const ctx = u.ctx;
        ctx.save();
        ctx.strokeStyle = "rgba(255,255,255,0.35)";
        ctx.lineWidth = 1;
        ctx.beginPath();
        ctx.moveTo(Math.round(xPos) + 0.5, u.bbox.top);
        ctx.lineTo(Math.round(xPos) + 0.5, u.bbox.top + u.bbox.height);
        ctx.stroke();
        ctx.restore();
      }
    }
  };
}

function resetZoom() {
  if (!plot) return;
  plot.setScale("x", { min: FULL_X_MIN, max: FULL_X_MAX });
}

function buildPlot() {
  if (!ROWS.length) return;
  const useInterp = chkInterp.checked;
  const splinePaths = (useInterp && uPlot?.paths?.spline) ? uPlot.paths.spline({}) : null;
  const normalize = chkNormalize.checked;
  const data = normalize ? DATA_NORM : DATA_RAW;

  const series = [{ label: "t(s)" }];
  KEYS.forEach((k, i) => {
    const show = showPref.get(k) ?? false;
    const scale = normalize ? "y" : (axisPref.get(k) ?? "y");
    series.push({
    label: k,
    show,
    stroke: palette[i % palette.length],
    width: 2,
    scale,
    ...(splinePaths ? { paths: splinePaths } : {}),
    });
  });

    chkInterp.onchange = () => {
    const keep = playIdx;
    buildPlot();
    setPlayIdx(keep, true);
    };
  
  const gridStroke = "rgba(255,255,255,0.06)";
  const axisStroke = "rgba(255,255,255,0.75)";

  const axes = normalize ? [
    { stroke: axisStroke, grid: { stroke: gridStroke } },                 // x
    { stroke: axisStroke, grid: { stroke: gridStroke }, scale: "y" },     // left y
  ] : [
    { stroke: axisStroke, grid: { stroke: gridStroke } },                 // x
    { stroke: axisStroke, grid: { stroke: gridStroke }, scale: "y" },     // left y
    { stroke: axisStroke, grid: { stroke: gridStroke }, scale: "y2", side: 1 }, // right y
  ];

  const opts = {
    title: "Session Timeseries",
    width: el("chart").clientWidth,
    height: 520,
    series,
    scales: { x: { time: false }, y: { auto: true }, y2: { auto: true } },
    axes,
    legend: { show: true, live: true },
    cursor: { show: true, lock: true, points: { show: false } },
    select: { show: true },
    plugins: [zoomPanPlugin(), playheadPlugin()],
  };

  if (plot) plot.destroy();
  plot = new uPlot(opts, data, el("chart"));

  FULL_X_MIN = X[0];
  FULL_X_MAX = X[X.length - 1];
  btnResetZoom.onclick = () => resetZoom();

  setPlayIdx(Math.min(playIdx, X.length - 1), true);

  window.addEventListener("resize", () => {
    if (!plot) return;
    plot.setSize({ width: el("chart").clientWidth, height: 520 });
  });
}

function setPlayIdx(idx, updateCursor = false) {
  if (!X.length) return;
  playIdx = Math.max(0, Math.min(X.length - 1, idx | 0));
  playIdxF = playIdx;

  scrub.value = String(playIdx);
  timeReadout.textContent = `t=${fmt(X[playIdx])}s`;

  if (plot && updateCursor) plot.setCursor({ idx: playIdx });

  updateValueReadout();
  if (plot) plot.redraw();

  if (plot && chkFollow.checked) {
    const sc = plot.scales.x;
    const x = X[playIdx];
    const min = sc.min, max = sc.max;
    const r = max - min;
    if (x < min + 0.12 * r || x > max - 0.12 * r) {
      plot.setScale("x", { min: x - 0.5 * r, max: x + 0.5 * r });
    }
  }
}

function updateValueReadout() {
  if (!plot || !DATA_RAW) return;
  const parts = [];
  parts.push(`idx=${playIdx}/${X.length - 1}`);
  parts.push(`t=${fmt(X[playIdx])}s`);

  for (let si = 1; si < plot.series.length; si++) {
    if (!plot.series[si].show) continue;
    const label = plot.series[si].label;
    const v = DATA_RAW[si][playIdx]; // always show RAW values
    parts.push(`${label}=${fmt(v)}`);
  }

  valueLine.textContent = parts.join("  |  ");
}

// Playback
function tick(ts) {
  if (!playing) return;
  if (!lastRAF) lastRAF = ts;
  const dtMs = ts - lastRAF;
  lastRAF = ts;

  const dt = dtMs / 1000.0;
  const dT = avgDt(X);
  const step = (dT > 1e-9) ? (dt / dT) * speed : 1;

  playIdxF += step;

  if (playIdxF >= X.length) {
    playIdxF = X.length - 1;
    pause();
    setPlayIdx(playIdxF | 0, true);
    return;
  }

  setPlayIdx(playIdxF | 0, true);
  requestAnimationFrame(tick);
}

function play() {
  if (!X.length) return;
  playing = true;
  btnPlay.disabled = true;
  btnPause.disabled = false;
  lastRAF = 0;
  requestAnimationFrame(tick);
}

function pause() {
  playing = false;
  btnPlay.disabled = false;
  btnPause.disabled = true;
  lastRAF = 0;
}

btnPlay.onclick = () => play();
btnPause.onclick = () => pause();
selSpeed.onchange = () => { speed = Number(selSpeed.value) || 1.0; };

scrub.oninput = () => {
  pause();
  setPlayIdx(Number(scrub.value) || 0, true);
};

chkNormalize.onchange = () => buildPlot();

(async function main(){
  if (!sid) {
    titleEl.textContent = "Session Viewer";
    statusLine.textContent = "Missing sid (open from the Sessions table).";
    return;
  }

  titleEl.textContent = `Session: ${sid}`;

  const metaSnap = await db.ref(`ml_sessions/${sid}`).get();
  const meta = metaSnap.exists() ? metaSnap.val() : {};
  metaEl.textContent = `load=${meta.load_type ?? "—"}  duration=${meta.duration_s ?? "—"}s`;

  statusLine.textContent = "Fetching CSV chunks…";
  const csv = await fetchSessionCsv(sid);
  if (!csv) { statusLine.textContent = "No CSV chunks found for this session."; return; }
  btnDownload.onclick = () => downloadTextFile(`TSP_ML_${sid}.csv`, csv);

  statusLine.textContent = "Parsing CSV…";
  const parsed = Papa.parse(csv.trim(), { header: true, dynamicTyping: true, skipEmptyLines: true });
  ROWS = (parsed.data || []).filter(r => r && Object.keys(r).length);

  X = pickTimeAxis(ROWS);
  KEYS = buildSeriesKeys(ROWS);

  DATA_RAW = makeData(ROWS, X, KEYS);
  DATA_NORM = makeNormalizedData(DATA_RAW);

  // Defaults: show a few; axis defaults: big ones on Y2
  const defaultOn = new Set(["i_rms", "thd_pct", "spectral_entropy", "spectral_flatness"]);
  const defaultY2 = new Set(["thd_pct", "v_rms", "temp_c"]);

  KEYS.forEach(k => {
    showPref.set(k, defaultOn.has(k));
    axisPref.set(k, defaultY2.has(k) ? "y2" : "y");
  });

  // Build the left panel rows (checkbox + axis selector)
  toggleList.innerHTML = "";
  KEYS.forEach((k) => {
    const wrap = document.createElement("div");
    wrap.className = "row";

    const cb = document.createElement("input");
    cb.type = "checkbox";
    cb.checked = showPref.get(k);
    cb.onchange = () => {
      showPref.set(k, cb.checked);
      // update series visibility without full rebuild
      const si = KEYS.indexOf(k) + 1;
      if (plot) plot.setSeries(si, { show: cb.checked });
      updateValueReadout();
    };

    const name = document.createElement("div");
    name.textContent = k;

    const sel = document.createElement("select");
    sel.className = "axisSel";
    sel.innerHTML = `<option value="y">Y1</option><option value="y2">Y2</option>`;
    sel.value = axisPref.get(k) || "y";
    sel.onchange = () => {
      axisPref.set(k, sel.value);
      // safest: rebuild plot to apply new scale mapping
      const keepIdx = playIdx;
      buildPlot();
      setPlayIdx(keepIdx, true);
    };

    wrap.appendChild(cb);
    wrap.appendChild(name);
    wrap.appendChild(sel);
    toggleList.appendChild(wrap);
  });

  scrub.max = String(Math.max(0, X.length - 1));
  speed = Number(selSpeed.value) || 1.0;

  statusLine.textContent = `Rows: ${ROWS.length} | Click plot to move playhead | Play to animate`;
  valueLine.textContent = "—";

  buildPlot();
  setPlayIdx(0, true);
})();