console.log("session.js loaded: TSPweb-v0.3.9");

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
const chkSmooth = el("chkSmooth");
const rngSmooth = el("rngSmooth");
const smoothReadout = el("smoothReadout");

const chkCurves = el("chkCurves");
const chkFollow = el("chkFollow");
const btnResetZoom = el("btnResetZoom");

const chkEvents = el("chkEvents");

// NEW: stats
const chkStats = el("chkStats");
const statsWrap = el("statsWrap");
const statsHint = el("statsHint");
const statsBody = el("statsBody");
const btnCopyStats = el("btnCopyStats");

// NEW: series drawer (mobile)
const btnSeries = el("btnSeries");
const btnCloseSeries = el("btnCloseSeries");

const seriesSearch = el("seriesSearch");
const btnSelDefault = el("btnSelDefault");
const btnSelAll = el("btnSelAll");
const btnSelNone = el("btnSelNone");

btnBack.onclick = () => (location.href = "index.html");

function downloadTextFile(filename, text, mime = "text/csv;charset=utf-8") {
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
  keys.sort((a, b) => (chunksObj[a]?.created_at || 0) - (chunksObj[b]?.created_at || 0));

  let header = "";
  let rows = [];

  for (const k of keys) {
    const csv = chunksObj[k]?.csv || "";
    if (!csv) continue;

    const lines = csv.split("\n").filter((x) => x.trim().length);
    if (!lines.length) continue;

    if (!header) {
      header = lines[0].trimEnd();
      rows.push(...lines.slice(1));
    } else {
      const startIdx = lines[0].trim() === header.trim() ? 1 : 0;
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
  return rows.map((r) => ((Number(r.epoch_ms) || t0) - t0) / 1000.0);
}

function buildSeriesKeys(rows) {
  if (!rows.length) return [];
  const keys = Object.keys(rows[0]);
  return keys.filter((k) => {
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
  const dt = (x[x.length - 1] - x[0]) / (x.length - 1);
  return Number.isFinite(dt) && dt > 0 ? dt : 0;
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
  return Math.abs(val - a) <= Math.abs(val - b) ? lo - 1 : lo;
}

function lowerBound(arr, val) {
  let lo = 0, hi = arr.length;
  while (lo < hi) {
    const mid = (lo + hi) >> 1;
    if (arr[mid] < val) lo = mid + 1;
    else hi = mid;
  }
  return lo;
}

function upperBound(arr, val) {
  let lo = 0, hi = arr.length;
  while (lo < hi) {
    const mid = (lo + hi) >> 1;
    if (arr[mid] <= val) lo = mid + 1;
    else hi = mid;
  }
  return lo;
}

const palette = [
  "#2ecc71",
  "#e74c3c",
  "#3498db",
  "#f1c40f",
  "#9b59b6",
  "#1abc9c",
  "#e67e22",
  "#ecf0f1",
  "#00bcd4",
  "#ff4081",
  "#cddc39",
  "#ff9800",
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
let DATA_SMOOTH = null;
let DATA_NORM = null;
let DATA_SMOOTH_NORM = null;

let FULL_X_MIN = 0;
let FULL_X_MAX = 0;

// Guard: some browsers fire input when setting value programmatically
let INTERNAL_SCRUB_UPDATE = false;

// Events
let ARC_SERIES_INDEX = -1;
let ARC_IDXS = [];

// User preferences
const showPref = new Map();
const axisPref = new Map();

// Defaults
const DEFAULT_ON = new Set(["i_rms", "v_rms", "thd_pct", "spectral_entropy", "spectral_flatness"]);
const DEFAULT_Y2 = new Set(["thd_pct", "v_rms", "temp_c"]);

function makeData(rows, x, keys) {
  const data = [x];
  keys.forEach((k) => {
    data.push(
      rows.map((r) => {
        const v = r[k];
        const num = typeof v === "number" ? v : Number(String(v ?? "").trim());
        return Number.isFinite(num) ? num : null;
      })
    );
  });
  return data;
}

function normalizeData(dataRaw) {
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
    const den = mx - mn;
    const norm = new Array(arr.length);
    for (let i = 0; i < arr.length; i++) {
      const v = arr[i];
      if (v == null) norm[i] = null;
      else norm[i] = den > 1e-12 ? (v - mn) / den : 0.0;
    }
    out.push(norm);
  }
  return out;
}

function smoothArrayCentered(arr, win) {
  if (!win || win <= 1) return arr.slice();
  const w = Math.max(1, win | 0);
  const hw = (w / 2) | 0;
  const out = new Array(arr.length);

  for (let i = 0; i < arr.length; i++) {
    let sum = 0;
    let cnt = 0;
    const a = Math.max(0, i - hw);
    const b = Math.min(arr.length - 1, i + hw);
    for (let j = a; j <= b; j++) {
      const v = arr[j];
      if (v == null) continue;
      sum += v;
      cnt++;
    }
    out[i] = cnt ? sum / cnt : null;
  }

  return out;
}

function makeSmoothedData(dataRaw, win) {
  const out = [dataRaw[0]];
  for (let si = 1; si < dataRaw.length; si++) {
    out.push(smoothArrayCentered(dataRaw[si], win));
  }
  return out;
}

function currentData() {
  const smoothOn = !!chkSmooth?.checked;
  const normOn = !!chkNormalize?.checked;
  if (smoothOn && normOn) return DATA_SMOOTH_NORM;
  if (smoothOn && !normOn) return DATA_SMOOTH;
  if (!smoothOn && normOn) return DATA_NORM;
  return DATA_RAW;
}

function visibleSeriesCount() {
  let c = 0;
  for (const k of KEYS) if (showPref.get(k)) c++;
  return c;
}

function hasVisibleY2() {
  if (chkNormalize.checked) return false;
  for (const k of KEYS) {
    if (!showPref.get(k)) continue;
    if ((axisPref.get(k) || "y") === "y2") return true;
  }
  return false;
}

function chartSize() {
  const c = el("chart");
  const r = c.getBoundingClientRect();
  const w = Math.max(320, Math.floor(r.width || c.clientWidth || 0));
  const h = Math.max(240, Math.floor(r.height || c.clientHeight || 0));
  return { w, h };
}

function calcVisibleRange(u, scaleKey) {
  let lo = Infinity, hi = -Infinity;

  for (let si = 1; si < u.series.length; si++) {
    const s = u.series[si];
    if (!s.show) continue;
    if ((s.scale || "y") !== scaleKey) continue;

    const arr = u.data[si];
    if (!arr) continue;

    for (let i = 0; i < arr.length; i++) {
      const v = arr[i];
      if (v == null) continue;
      if (v < lo) lo = v;
      if (v > hi) hi = v;
    }
  }

  if (!Number.isFinite(lo) || !Number.isFinite(hi)) return null;
  if (lo === hi) {
    const pad = (Math.abs(lo) || 1) * 0.06 + 1e-9;
    return [lo - pad, hi + pad];
  }
  const pad = (hi - lo) * 0.08;
  return [lo - pad, hi + pad];
}

// Plugin: zoom/pan + click to move playhead
function zoomPanPlugin() {
  let isPanning = false;
  let panStartX = 0;
  let startMin = 0, startMax = 0;

  const onMouseMove = (u, e) => {
    if (!isPanning) return;
    const rect = u.over.getBoundingClientRect();
    const x0 = panStartX - rect.left;
    const x1 = e.clientX - rect.left;
    const v0 = u.posToVal(x0, "x");
    const v1 = u.posToVal(x1, "x");
    const dv = v0 - v1;
    u.setScale("x", { min: startMin + dv, max: startMax + dv });
  };

  const onMouseUp = () => { isPanning = false; };

  return {
    hooks: {
      ready: (u) => {
        u.root.addEventListener("dblclick", (e) => {
          e.preventDefault();
          resetZoom();
        });

        u.over.addEventListener("wheel", (e) => {
          e.preventDefault();
          const rect = u.over.getBoundingClientRect();
          const px = e.clientX - rect.left;
          const xVal = u.posToVal(px, "x");
          const sc = u.scales.x;
          const min = sc.min, max = sc.max;
          const range = max - min || 1;

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

        u.root.addEventListener("mousemove", (e) => onMouseMove(u, e));
        window.addEventListener("mouseup", onMouseUp);

        u.over.addEventListener("click", (e) => {
          const rect = u.over.getBoundingClientRect();
          const px = e.clientX - rect.left;
          const xVal = u.posToVal(px, "x");
          const idx = binarySearchNearest(X, xVal);
          pause();
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
      },
    },
  };
}

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
        ctx.strokeStyle = playing ? "rgba(46,204,113,0.70)" : "rgba(255,255,255,0.40)";
        ctx.lineWidth = 2;
        ctx.beginPath();
        ctx.moveTo(Math.round(xPos) + 0.5, u.bbox.top);
        ctx.lineTo(Math.round(xPos) + 0.5, u.bbox.top + u.bbox.height);
        ctx.stroke();
        ctx.restore();
      },
    },
  };
}

function eventsPlugin() {
  return {
    hooks: {
      draw: (u) => {
        if (!chkEvents?.checked) return;
        if (!ARC_IDXS.length) return;
        const ctx = u.ctx;
        ctx.save();
        ctx.strokeStyle = "rgba(231,76,60,0.55)";
        ctx.lineWidth = 1;
        for (const idx of ARC_IDXS) {
          const xPos = u.valToPos(X[idx], "x", true);
          ctx.beginPath();
          ctx.moveTo(Math.round(xPos) + 0.5, u.bbox.top);
          ctx.lineTo(Math.round(xPos) + 0.5, u.bbox.top + u.bbox.height);
          ctx.stroke();
        }
        ctx.restore();
      },
    },
  };
}

// NEW: stats update whenever x-scale changes (zoom/pan/follow)
function statsPlugin() {
  return {
    hooks: {
      setScale: (u, key) => {
        if (key !== "x") return;
        // throttle to next frame
        requestAnimationFrame(updateStats);
      },
    },
  };
}

function resetZoom() {
  if (!plot) return;
  plot.setScale("x", { min: FULL_X_MIN, max: FULL_X_MAX });
}

function buildArcIndexes() {
  ARC_SERIES_INDEX = -1;
  ARC_IDXS = [];

  const kIdx = KEYS.indexOf("label_arc");
  if (kIdx < 0) return;

  ARC_SERIES_INDEX = kIdx + 1; // +1 because data[0] is x
  const arr = DATA_RAW?.[ARC_SERIES_INDEX];
  if (!arr) return;

  for (let i = 0; i < arr.length; i++) {
    const v = arr[i];
    if (v == null) continue;
    if (v >= 0.5) ARC_IDXS.push(i);
  }
}

function buildPlot() {
  if (!ROWS.length) return;

  if (visibleSeriesCount() === 0) {
    statusLine.textContent = "No series selected. Check at least one series on the left.";
  }

  const data = currentData();
  const normalize = chkNormalize.checked;

  const useCurves = !!chkCurves?.checked;
  const splinePaths = (useCurves && uPlot?.paths?.spline) ? uPlot.paths.spline({}) : null;

  const { w, h } = chartSize();

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

  const gridStroke = "rgba(255,255,255,0.06)";
  const axisStroke = "rgba(255,255,255,0.75)";

  const showY2 = !normalize && hasVisibleY2();

  const axes = [
    { stroke: axisStroke, grid: { stroke: gridStroke } },
    { stroke: axisStroke, grid: { stroke: gridStroke }, scale: "y" },
  ];
  if (showY2) axes.push({ stroke: axisStroke, grid: { stroke: gridStroke }, scale: "y2", side: 1 });

  const scales = {
    x: { time: false },
    y: { auto: true, range: (u) => calcVisibleRange(u, "y") || [0, 1] },
  };
  if (showY2) scales.y2 = { auto: true, range: (u) => calcVisibleRange(u, "y2") || [0, 1] };

  const opts = {
    title: "Session Timeseries",
    width: w,
    height: h,
    series,
    scales,
    axes,
    legend: { show: false },
    cursor: { show: true, lock: true, points: { show: false } },
    select: { show: true },
    plugins: [zoomPanPlugin(), eventsPlugin(), statsPlugin(), playheadPlugin()],
  };

  if (plot) plot.destroy();
  plot = new uPlot(opts, data, el("chart"));

  FULL_X_MIN = X[0];
  FULL_X_MAX = X[X.length - 1];

  btnResetZoom.onclick = () => resetZoom();

  setPlayIdx(Math.min(playIdx, X.length - 1), true);
  updateValueReadout();
  updateStats();
}

function setPlayIdx(idx, updateCursor = false) {
  if (!X.length) return;
  playIdx = Math.max(0, Math.min(X.length - 1, idx | 0));
  playIdxF = playIdx;

  INTERNAL_SCRUB_UPDATE = true;
  scrub.value = String(playIdx);
  INTERNAL_SCRUB_UPDATE = false;

  timeReadout.textContent = `t=${fmt(X[playIdx])}s`;

  if (plot && updateCursor && plot.setCursor) {
    const xPos = plot.valToPos(X[playIdx], "x", true);
    plot.setCursor({ left: xPos, top: 0 });
  }

  updateValueReadout();

  if (plot) plot.redraw();

  // Follow:
  // - Playing: keep centered
  // - Paused: gentle follow if near edges
  if (plot && (chkFollow?.checked || playing)) {
    const sc = plot.scales.x;
    const x = X[playIdx];
    const min = sc.min, max = sc.max;
    const r = (max - min) || 1;

    if (playing) {
      plot.setScale("x", { min: x - 0.45 * r, max: x + 0.55 * r });
    } else {
      if (x < min + 0.12 * r || x > max - 0.12 * r) {
        plot.setScale("x", { min: x - 0.5 * r, max: x + 0.5 * r });
      }
    }
  }
}

function clearValueReadout() {
  if (!valueLine) return;
  valueLine.innerHTML = "";
}

function updateValueReadout() {
  if (!valueLine || !DATA_RAW || !KEYS.length) return;

  valueLine.innerHTML = "";

  const meta = document.createElement("div");
  meta.className = "vchip";

  const metaDot = document.createElement("span");
  metaDot.className = "vdot";
  metaDot.style.background = "rgba(255,255,255,0.35)";

  const metaKey = document.createElement("span");
  metaKey.className = "vk";
  metaKey.textContent = `idx ${playIdx}/${X.length - 1}`;

  const metaVal = document.createElement("span");
  metaVal.className = "vv";
  metaVal.textContent = `t ${fmt(X[playIdx])}s`;

  meta.appendChild(metaDot);
  meta.appendChild(metaKey);
  meta.appendChild(metaVal);
  valueLine.appendChild(meta);

  const maxChips = 12;
  let shown = 0;

  for (let i = 0; i < KEYS.length; i++) {
    const k = KEYS[i];
    if (!showPref.get(k)) continue;
    if (shown >= maxChips) break;

    const v = DATA_RAW[i + 1]?.[playIdx];
    const chip = document.createElement("div");
    chip.className = "vchip";

    const dot = document.createElement("span");
    dot.className = "vdot";
    dot.style.background = palette[i % palette.length];

    const key = document.createElement("span");
    key.className = "vk";
    key.textContent = k;

    const val = document.createElement("span");
    val.className = "vv";
    val.textContent = fmt(v);

    chip.appendChild(dot);
    chip.appendChild(key);
    chip.appendChild(val);

    valueLine.appendChild(chip);
    shown++;
  }

  const totalSelected = visibleSeriesCount();
  if (totalSelected > maxChips) {
    const more = document.createElement("div");
    more.className = "vchip";

    const dot = document.createElement("span");
    dot.className = "vdot";
    dot.style.background = "rgba(255,255,255,0.20)";

    const key = document.createElement("span");
    key.className = "vk";
    key.textContent = `+${totalSelected - maxChips} more`;

    const val = document.createElement("span");
    val.className = "vv";
    val.textContent = "";

    more.appendChild(dot);
    more.appendChild(key);
    more.appendChild(val);
    valueLine.appendChild(more);
  }
}

// ---------------- Range Stats ----------------
function computeStats(arr, i0, i1) {
  // Welford
  let n = 0;
  let mean = 0;
  let m2 = 0;
  let mn = Infinity;
  let mx = -Infinity;

  for (let i = i0; i <= i1; i++) {
    const v = arr[i];
    if (v == null) continue;
    n++;
    if (v < mn) mn = v;
    if (v > mx) mx = v;
    const delta = v - mean;
    mean += delta / n;
    const delta2 = v - mean;
    m2 += delta * delta2;
  }

  if (n === 0) return { n: 0, mean: null, min: null, max: null, std: null };
  const variance = n > 1 ? (m2 / (n - 1)) : 0;
  const std = Math.sqrt(Math.max(0, variance));
  return { n, mean, min: mn, max: mx, std };
}

function currentXRangeIdx() {
  if (!plot || !X.length) return [0, Math.max(0, X.length - 1)];
  const xmin = plot.scales.x.min;
  const xmax = plot.scales.x.max;
  let i0 = lowerBound(X, xmin);
  let i1 = upperBound(X, xmax) - 1;
  i0 = Math.max(0, Math.min(X.length - 1, i0));
  i1 = Math.max(0, Math.min(X.length - 1, i1));
  if (i1 < i0) [i0, i1] = [i1, i0];
  return [i0, i1];
}

function updateStats() {
  if (!statsWrap || !statsBody || !statsHint) return;

  const enabled = !!chkStats?.checked;
  statsWrap.style.display = enabled ? "" : "none";
  if (!enabled) return;

  if (!DATA_RAW || !KEYS.length || !plot) {
    statsHint.textContent = "Stats unavailable.";
    statsBody.innerHTML = "";
    return;
  }

  const [i0, i1] = currentXRangeIdx();
  const t0 = X[i0];
  const t1 = X[i1];

  const selectedKeys = KEYS.filter(k => showPref.get(k) && k !== "label_arc");
  const cap = 16;
  const shownKeys = selectedKeys.slice(0, cap);
  const extra = Math.max(0, selectedKeys.length - shownKeys.length);

  statsHint.textContent = `Stats (RAW) | idx ${i0}-${i1} | t=${fmt(t0)}s..${fmt(t1)}s` + (extra ? ` | +${extra} more hidden` : "");

  statsBody.innerHTML = shownKeys.map((k) => {
    const si = KEYS.indexOf(k) + 1;
    const arr = DATA_RAW[si];
    const st = computeStats(arr, i0, i1);

    return `
      <tr>
        <td class="mono">${k}</td>
        <td class="mono">${fmt(st.mean)}</td>
        <td class="mono">${fmt(st.min)}</td>
        <td class="mono">${fmt(st.max)}</td>
        <td class="mono">${fmt(st.std)}</td>
        <td class="mono">${st.n}</td>
      </tr>
    `;
  }).join("");
}

btnCopyStats?.addEventListener("click", async () => {
  if (!DATA_RAW || !KEYS.length || !plot) return;
  const [i0, i1] = currentXRangeIdx();

  const selectedKeys = KEYS.filter(k => showPref.get(k) && k !== "label_arc");
  const shownKeys = selectedKeys.slice(0, 64);

  let out = "series,mean,min,max,std,n\n";
  for (const k of shownKeys) {
    const si = KEYS.indexOf(k) + 1;
    const st = computeStats(DATA_RAW[si], i0, i1);
    out += `${k},${st.mean ?? ""},${st.min ?? ""},${st.max ?? ""},${st.std ?? ""},${st.n}\n`;
  }

  try {
    await navigator.clipboard.writeText(out);
    btnCopyStats.textContent = "Copied";
    setTimeout(() => (btnCopyStats.textContent = "Copy"), 900);
  } catch {
    // fallback
    downloadTextFile(`TSP_stats_${sid}.csv`, out, "text/csv;charset=utf-8");
  }
});

chkStats?.addEventListener("change", updateStats);

// ---------------- Playback ----------------
function tick(ts) {
  if (!playing) return;
  if (!lastRAF) lastRAF = ts;
  const dtMs = ts - lastRAF;
  lastRAF = ts;

  const dt = dtMs / 1000.0;
  const dT = avgDt(X);

  let step = 1;
  if (dT > 1e-9) {
    step = (dt / dT) * speed;
    if (!Number.isFinite(step) || step <= 0) step = 1;
  } else {
    step = Math.max(1, Math.round(speed));
  }

  playIdxF += step;

  if (playIdxF >= X.length) {
    playIdxF = X.length - 1;
    pause();
    setPlayIdx(playIdxF | 0, true);
    return;
  }

  const newIdx = playIdxF | 0;
  if (newIdx !== playIdx) setPlayIdx(newIdx, true);
  requestAnimationFrame(tick);
}

function play() {
  if (!X.length) return;
  playing = true;
  btnPlay.disabled = true;
  btnPause.disabled = false;
  lastRAF = 0;

  // force follow ON while playing to keep chart "scrolling"
  if (chkFollow) chkFollow.checked = true;

  requestAnimationFrame(tick);
}

function pause() {
  playing = false;
  btnPlay.disabled = false;
  btnPause.disabled = true;
  lastRAF = 0;
  if (plot) plot.redraw();
}

btnPlay.onclick = () => play();
btnPause.onclick = () => pause();
selSpeed.onchange = () => { speed = Number(selSpeed.value) || 1.0; };

scrub.oninput = () => {
  if (INTERNAL_SCRUB_UPDATE) return;
  pause();
  setPlayIdx(Number(scrub.value) || 0, true);
};

function rebuildForDataMode() {
  const keep = playIdx;
  buildPlot();
  setPlayIdx(keep, true);
}

chkNormalize.onchange = rebuildForDataMode;
chkSmooth.onchange = rebuildForDataMode;
chkCurves.onchange = rebuildForDataMode;

chkEvents.onchange = () => { if (plot) plot.redraw(); };

rngSmooth.oninput = () => {
  const win = Number(rngSmooth.value) || 1;
  if (smoothReadout) smoothReadout.textContent = `win=${win}`;
  DATA_SMOOTH = makeSmoothedData(DATA_RAW, win);
  DATA_SMOOTH_NORM = normalizeData(DATA_SMOOTH);
  rebuildForDataMode();
};

// Series filter
function applySeriesFilter() {
  const q = (seriesSearch?.value || "").trim().toLowerCase();
  const rows = toggleList?.querySelectorAll?.(".row") || [];
  rows.forEach((r) => {
    const key = r.dataset.key || "";
    const ok = !q || key.toLowerCase().includes(q);
    r.style.display = ok ? "" : "none";
  });
}
seriesSearch?.addEventListener("input", applySeriesFilter);

function setAllSeries(on) {
  KEYS.forEach((k, i) => {
    showPref.set(k, !!on);
    const cb = toggleList?.querySelector?.(`.row[data-key="${k}"] input[type="checkbox"]`);
    if (cb) cb.checked = !!on;
    if (plot) plot.setSeries(i + 1, { show: !!on });
  });
  if (plot) plot.setData(plot.data, false);
  updateValueReadout();
  updateStats();
}

function applyDefaultSelection() {
  KEYS.forEach((k, i) => {
    const on = DEFAULT_ON.has(k);
    showPref.set(k, on);
    axisPref.set(k, DEFAULT_Y2.has(k) ? "y2" : "y");

    const row = toggleList?.querySelector?.(`.row[data-key="${k}"]`);
    if (row) {
      const cb = row.querySelector('input[type="checkbox"]');
      const sel = row.querySelector('select');
      if (cb) cb.checked = on;
      if (sel) sel.value = axisPref.get(k);
    }

    if (plot) plot.setSeries(i + 1, { show: on });
  });
  rebuildForDataMode();
  updateStats();
}

btnSelAll?.addEventListener("click", () => setAllSeries(true));
btnSelNone?.addEventListener("click", () => setAllSeries(false));
btnSelDefault?.addEventListener("click", () => applyDefaultSelection());

// Resize
let resizeQueued = false;
window.addEventListener("resize", () => {
  if (!plot || resizeQueued) return;
  resizeQueued = true;
  requestAnimationFrame(() => {
    resizeQueued = false;
    if (!plot) return;
    const { w, h } = chartSize();
    plot.setSize({ width: w, height: h });
    updateStats();
  });
});

// Mobile series drawer
function openSeries() { document.body.classList.add("showSeries"); }
function closeSeries() { document.body.classList.remove("showSeries"); }
btnSeries?.addEventListener("click", openSeries);
btnCloseSeries?.addEventListener("click", closeSeries);
window.addEventListener("keydown", (e) => {
  if (e.key === "Escape") closeSeries();
});

(async function main() {
  if (!sid) {
    titleEl.textContent = "Session Viewer";
    statusLine.textContent = "Missing sid (open from the Sessions table).";
    clearValueReadout();
    return;
  }

  titleEl.textContent = `Session: ${sid}`;

  try {
    const metaSnap = await db.ref(`ml_sessions/${sid}`).get();
    const meta = metaSnap.exists() ? metaSnap.val() : {};
    metaEl.textContent = `load=${meta.load_type ?? "—"}  duration=${meta.duration_s ?? "—"}s`;

    statusLine.textContent = "Fetching CSV chunks…";
    const csv = await fetchSessionCsv(sid);
    if (!csv) {
      statusLine.textContent = "No CSV chunks found for this session.";
      clearValueReadout();
      return;
    }

    btnDownload.onclick = () => downloadTextFile(`TSP_ML_${sid}.csv`, csv);

    statusLine.textContent = "Parsing CSV…";
    const parsed = Papa.parse(csv.trim(), { header: true, dynamicTyping: true, skipEmptyLines: true });
    ROWS = (parsed.data || []).filter((r) => r && Object.keys(r).length);

    if (!ROWS.length) {
      statusLine.textContent = "Parsed 0 rows. (CSV exists but has no data rows.)";
      clearValueReadout();
      return;
    }

    X = pickTimeAxis(ROWS);
    KEYS = buildSeriesKeys(ROWS);

    if (!KEYS.length) {
      statusLine.textContent = "No numeric series found in this session CSV.";
      clearValueReadout();
      return;
    }

    DATA_RAW = makeData(ROWS, X, KEYS);

    // smoothing init
    const win0 = Number(rngSmooth?.value) || 15;
    if (smoothReadout) smoothReadout.textContent = `win=${win0}`;
    DATA_SMOOTH = makeSmoothedData(DATA_RAW, win0);

    DATA_NORM = normalizeData(DATA_RAW);
    DATA_SMOOTH_NORM = normalizeData(DATA_SMOOTH);

    buildArcIndexes();

    KEYS.forEach((k) => {
      showPref.set(k, DEFAULT_ON.has(k));
      axisPref.set(k, DEFAULT_Y2.has(k) ? "y2" : "y");
    });

    // Build toggle UI
    toggleList.innerHTML = "";
    KEYS.forEach((k, i) => {
      const wrap = document.createElement("div");
      wrap.className = "row";
      wrap.dataset.key = k;

      const cb = document.createElement("input");
      cb.type = "checkbox";
      cb.checked = !!showPref.get(k);
      cb.onchange = () => {
        const on = !!cb.checked;
        showPref.set(k, on);
        if (plot) {
          plot.setSeries(i + 1, { show: on });
          plot.setData(plot.data, false);
        }
        updateValueReadout();
        updateStats();
      };

      const name = document.createElement("div");
      name.textContent = k;

      const sel = document.createElement("select");
      sel.className = "axisSel";
      sel.innerHTML = `<option value="y">Y1</option><option value="y2">Y2</option>`;
      sel.value = axisPref.get(k) || "y";
      sel.onchange = () => {
        axisPref.set(k, sel.value);
        rebuildForDataMode();
      };

      wrap.appendChild(cb);
      wrap.appendChild(name);
      wrap.appendChild(sel);
      toggleList.appendChild(wrap);
    });

    applySeriesFilter();

    scrub.max = String(Math.max(0, X.length - 1));
    speed = Number(selSpeed.value) || 1.0;

    // Arc markers checkbox only if label_arc exists
    if (chkEvents) {
      chkEvents.disabled = ARC_SERIES_INDEX < 0;
      if (ARC_SERIES_INDEX < 0) chkEvents.checked = false;
    }

    statusLine.textContent = `Rows: ${ROWS.length} | Click plot to move playhead | Play to animate`;
    clearValueReadout();

    requestAnimationFrame(() => {
      buildPlot();
      setPlayIdx(0, true);
      updateStats();
    });
  } catch (e) {
    console.error(e);
    statusLine.textContent = "Failed to load this session (check Firebase rules / network).";
    clearValueReadout();
  }
})();