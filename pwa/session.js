(function () {
  console.log("session.js loaded: merged drawer");

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

  const $ = (id) => document.getElementById(id);

  const plotDrawer = $("plotDrawer");
  const titleEl = $("title");
  const metaEl = $("meta");
  const toggleList = $("toggleList");
  const statusLine = $("statusLine");
  const valueLine = $("valueLine");

  const btnBack = $("btnBack");
  const btnDownload = $("btnDownload");

  const btnPlay = $("btnPlay");
  const btnPause = $("btnPause");
  const selSpeed = $("selSpeed");
  const scrub = $("scrub");
  const timeReadout = $("timeReadout");

  const chkNormalize = $("chkNormalize");
  const chkSmooth = $("chkSmooth");
  const rngSmooth = $("rngSmooth");
  const smoothReadout = $("smoothReadout");

  const chkCurves = $("chkCurves");
  const chkFollow = $("chkFollow");
  const btnResetZoom = $("btnResetZoom");
  const chkEvents = $("chkEvents");

  const chkStats = $("chkStats");
  const statsWrap = $("statsWrap");
  const statsHint = $("statsHint");
  const statsBody = $("statsBody");
  const btnCopyStats = $("btnCopyStats");

  const btnSeries = $("btnSeries");
  const btnCloseSeries = $("btnCloseSeries");
  const seriesSearch = $("seriesSearch");
  const btnSelDefault = $("btnSelDefault");
  const btnSelAll = $("btnSelAll");
  const btnSelNone = $("btnSelNone");

  if (!plotDrawer) return;

  let sid = "";
  let currentCsv = "";

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
      const val = rows.find((r) => r[k] !== undefined && r[k] !== null)?.[k];
      if (val === undefined) return false;
      const num = typeof val === "number" ? val : Number(String(val).trim());
      return Number.isFinite(num);
    });
  }

  function fmt(x, d = 4) {
    if (x == null || Number.isNaN(Number(x))) return "—";
    const n = Number(x);
    if (Math.abs(n) >= 1000) return n.toFixed(1);
    if (Math.abs(n) >= 100) return n.toFixed(2);
    if (Math.abs(n) >= 10) return n.toFixed(3);
    return n.toFixed(d);
  }

  function avgDt(x) {
    if (!x || x.length < 2) return 0;
    return (x[x.length - 1] - x[0]) / Math.max(1, x.length - 1);
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
    "#2ecc71", "#e74c3c", "#3498db", "#f1c40f",
    "#9b59b6", "#1abc9c", "#e67e22", "#ecf0f1",
    "#00bcd4", "#ff4081", "#cddc39", "#ff9800",
  ];

  let plot = null;
  let playing = false;
  let playIdxF = 0;
  let playIdx = 0;
  let lastRAF = 0;
  let speed = 1.0;

  let ROWS = [];
  let X = [];
  let KEYS = [];

  let DATA_RAW = null;
  let DATA_SMOOTH = null;
  let DATA_NORM = null;
  let DATA_SMOOTH_NORM = null;

  let FULL_X_MIN = 0;
  let FULL_X_MAX = 0;
  let INTERNAL_SCRUB_UPDATE = false;

  let ARC_SERIES_INDEX = -1;
  let ARC_IDXS = [];

  const showPref = new Map();
  const axisPref = new Map();

  const DEFAULT_ON = new Set(["i_rms", "v_rms", "thd_pct", "spectral_entropy", "spectral_flatness"]);
  const DEFAULT_Y2 = new Set(["thd_pct", "v_rms", "temp_c"]);

  function resetState() {
    pause();
    if (plot) {
      plot.destroy();
      plot = null;
    }
    ROWS = [];
    X = [];
    KEYS = [];
    DATA_RAW = null;
    DATA_SMOOTH = null;
    DATA_NORM = null;
    DATA_SMOOTH_NORM = null;
    FULL_X_MIN = 0;
    FULL_X_MAX = 0;
    ARC_SERIES_INDEX = -1;
    ARC_IDXS = [];
    playIdx = 0;
    playIdxF = 0;
    lastRAF = 0;
    showPref.clear();
    axisPref.clear();
    currentCsv = "";
    if (toggleList) toggleList.innerHTML = "";
    if (valueLine) valueLine.innerHTML = "";
    if (statsBody) statsBody.innerHTML = "";
    if (statsHint) statsHint.textContent = "—";
    if (statusLine) statusLine.textContent = "Loading…";
    if (scrub) { scrub.min = "0"; scrub.max = "0"; scrub.value = "0"; }
    if (timeReadout) timeReadout.textContent = "t=—";
  }

  function makeData(rows, x, keys) {
    const data = [x];
    keys.forEach((k) => {
      data.push(rows.map((r) => {
        const v = r[k];
        const num = typeof v === "number" ? v : Number(String(v ?? "").trim());
        return Number.isFinite(num) ? num : null;
      }));
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
    for (let si = 1; si < dataRaw.length; si++) out.push(smoothArrayCentered(dataRaw[si], win));
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
    const c = $("chart");
    const r = c.getBoundingClientRect();
    const w = Math.max(320, Math.floor(r.width || c.clientWidth || 0));
    const h = Math.max(300, Math.floor(r.height || c.clientHeight || 0));
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

  function buildArcIndexes() {
    ARC_SERIES_INDEX = KEYS.indexOf("label_arc");
    ARC_IDXS = [];
    if (ARC_SERIES_INDEX < 0 || !DATA_RAW) return;
    const arr = DATA_RAW[ARC_SERIES_INDEX + 1];
    for (let i = 0; i < arr.length; i++) {
      if (Number(arr[i]) === 1) ARC_IDXS.push(i);
    }
  }

  function resetZoom() {
    if (!plot || !X.length) return;
    plot.setScale("x", { min: FULL_X_MIN, max: FULL_X_MAX });
  }

  function zoomPanPlugin() {
    let isPanning = false;
    let panStartX = 0;
    let startMin = 0, startMax = 0;

    const onMouseMove = (u, e) => {
      if (!isPanning) return;
      const rect = u.root.getBoundingClientRect();
      const x = e.clientX - rect.left;
      const dxPx = x - panStartX;
      const dxVal = u.posToVal(0, "x") - u.posToVal(dxPx, "x");
      u.setScale("x", { min: startMin + dxVal, max: startMax + dxVal });
    };
    const onMouseUp = () => { isPanning = false; };

    return {
      hooks: {
        ready: [
          (u) => {
            u.root.addEventListener("mousedown", (e) => {
              if (!e.shiftKey || e.button !== 0) return;
              isPanning = true;
              panStartX = e.offsetX;
              startMin = u.scales.x.min;
              startMax = u.scales.x.max;
              e.preventDefault();
            });
            window.addEventListener("mousemove", (e) => onMouseMove(u, e));
            window.addEventListener("mouseup", onMouseUp);

            u.root.addEventListener("wheel", (e) => {
              e.preventDefault();
              const left = e.offsetX;
              const xVal = u.posToVal(left, "x");
              const sc = u.scales.x;
              const min = sc.min, max = sc.max;
              const range = max - min;
              const factor = e.deltaY < 0 ? 0.85 : 1.18;
              const newRange = Math.min((FULL_X_MAX - FULL_X_MIN), Math.max(avgDt(X) * 8, range * factor));
              const frac = (xVal - min) / range;
              const newMin = xVal - frac * newRange;
              const newMax = newMin + newRange;
              u.setScale("x", { min: newMin, max: newMax });
            }, { passive: false });

            u.root.addEventListener("dblclick", () => resetZoom());
            u.root.addEventListener("click", (e) => {
              const x = u.posToVal(e.offsetX, "x");
              const idx = lowerBound(X, x);
              if (idx >= 0 && idx < X.length) setPlayIdx(idx, true);
            });
          }
        ]
      }
    };
  }

  function eventsPlugin() {
    return {
      hooks: {
        draw: [
          (u) => {
            if (!chkEvents?.checked || ARC_SERIES_INDEX < 0 || !ARC_IDXS.length) return;
            const { ctx } = u;
            ctx.save();
            ctx.strokeStyle = "rgba(255,90,60,0.85)";
            ctx.lineWidth = 1;
            for (const idx of ARC_IDXS) {
              const x = X[idx];
              if (x < u.scales.x.min || x > u.scales.x.max) continue;
              const px = Math.round(u.valToPos(x, "x", true));
              ctx.beginPath();
              ctx.moveTo(px + 0.5, u.bbox.top);
              ctx.lineTo(px + 0.5, u.bbox.top + u.bbox.height);
              ctx.stroke();
            }
            ctx.restore();
          }
        ]
      }
    };
  }

  function statsPlugin() {
    return {
      hooks: {
        setScale: [() => updateStats()]
      }
    };
  }

  function playheadPlugin() {
    return {
      hooks: {
        draw: [
          (u) => {
            if (!X.length) return;
            const px = Math.round(u.valToPos(X[playIdx], "x", true));
            const ctx = u.ctx;
            ctx.save();
            ctx.strokeStyle = "rgba(255,255,255,0.65)";
            ctx.lineWidth = 1;
            ctx.beginPath();
            ctx.moveTo(px + 0.5, u.bbox.top);
            ctx.lineTo(px + 0.5, u.bbox.top + u.bbox.height);
            ctx.stroke();
            ctx.restore();
          }
        ]
      }
    };
  }

  function buildPlot() {
    if (!DATA_RAW || !X.length) return;
    if (visibleSeriesCount() === 0) statusLine.textContent = "No series selected. Check at least one series.";

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

    const axisStroke = "rgba(255,255,255,0.75)";
    const showY2 = !normalize && hasVisibleY2();

    const axes = [
      { stroke: axisStroke, grid: { show: false } },
      { stroke: axisStroke, grid: { show: false }, scale: "y" },
    ];
    if (showY2) axes.push({ stroke: axisStroke, grid: { show: false }, scale: "y2", side: 1 });

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
    plot = new uPlot(opts, data, $("chart"));

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

    if (timeReadout) timeReadout.textContent = `t=${fmt(X[playIdx])}s`;

    if (plot && updateCursor && plot.setCursor) {
      const xPos = plot.valToPos(X[playIdx], "x", true);
      plot.setCursor({ left: xPos, top: 0 });
    }

    updateValueReadout();
    if (plot) plot.redraw();

    if (plot && (chkFollow?.checked || playing)) {
      const sc = plot.scales.x;
      const x = X[playIdx];
      const min = sc.min, max = sc.max;
      const r = (max - min) || 1;

      if (playing) {
        plot.setScale("x", { min: x - 0.45 * r, max: x + 0.55 * r });
      } else if (x < min + 0.12 * r || x > max - 0.12 * r) {
        plot.setScale("x", { min: x - 0.5 * r, max: x + 0.5 * r });
      }
    }
  }

  function clearValueReadout() {
    if (valueLine) valueLine.innerHTML = "";
  }

  function updateValueReadout() {
    if (!valueLine || !DATA_RAW || !KEYS.length) return;
    valueLine.innerHTML = "";

    const meta = document.createElement("div");
    meta.className = "vchip";
    meta.innerHTML = `<span class="vdot" style="background:rgba(255,255,255,0.35)"></span><span class="vk">idx ${playIdx}/${X.length - 1}</span><span class="vv">t ${fmt(X[playIdx])}s</span>`;
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
      chip.innerHTML = `<span class="vdot" style="background:${palette[i % palette.length]}"></span><span class="vk">${k}</span><span class="vv">${fmt(v)}</span>`;
      valueLine.appendChild(chip);
      shown++;
    }

    const totalSelected = visibleSeriesCount();
    if (totalSelected > maxChips) {
      const more = document.createElement("div");
      more.className = "vchip";
      more.innerHTML = `<span class="vdot" style="background:rgba(255,255,255,0.20)"></span><span class="vk">+${totalSelected - maxChips} more</span><span class="vv"></span>`;
      valueLine.appendChild(more);
    }
  }

  function computeStats(arr, i0, i1) {
    let n = 0, mean = 0, m2 = 0, mn = Infinity, mx = -Infinity;
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
      downloadTextFile(`TSP_stats_${sid}.csv`, out, "text/csv;charset=utf-8");
    }
  });

  chkStats?.addEventListener("change", updateStats);

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
    if (chkFollow) chkFollow.checked = true;
    requestAnimationFrame(tick);
  }

  function pause() {
    playing = false;
    if (btnPlay) btnPlay.disabled = false;
    if (btnPause) btnPause.disabled = true;
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

  function openSeries() { document.body.classList.add("showSeries"); }
  function closeSeries() { document.body.classList.remove("showSeries"); }
  btnSeries?.addEventListener("click", openSeries);
  btnCloseSeries?.addEventListener("click", closeSeries);
  window.addEventListener("keydown", (e) => {
    if (e.key === "Escape") {
      closeSeries();
      if (!plotDrawer.classList.contains("collapsed")) closeSessionViewer();
    }
  });

  async function loadSession(targetSid, metaOverride = null) {
    sid = targetSid || "";
    resetState();

    if (!sid) {
      titleEl.textContent = "Plot Viewer";
      metaEl.textContent = "Open a session from the ML table above.";
      statusLine.textContent = "Missing sid.";
      clearValueReadout();
      return;
    }

    titleEl.textContent = `Session: ${sid}`;

    try {
      let meta = metaOverride || null;
      if (!meta) {
        const metaSnap = await db.ref(`ml_sessions/${sid}`).get();
        meta = metaSnap.exists() ? metaSnap.val() : {};
      }
      metaEl.textContent = `load=${meta?.load_type ?? "—"}  duration=${meta?.duration_s ?? "—"}s`;

      statusLine.textContent = "Fetching CSV chunks…";
      const csv = await fetchSessionCsv(sid);
      currentCsv = csv;
      if (!csv) {
        statusLine.textContent = "No CSV chunks found for this session.";
        clearValueReadout();
        return;
      }

      btnDownload.onclick = () => downloadTextFile(`TSP_ML_${sid}.csv`, currentCsv);

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
  }

  function openSessionViewer(targetSid, metaOverride = null) {
    plotDrawer.classList.remove("collapsed");
    setTimeout(() => {
      plotDrawer.scrollIntoView({ behavior: "smooth", block: "start" });
    }, 10);
    loadSession(targetSid, metaOverride);
  }

  function closeSessionViewer() {
    closeSeries();
    resetState();
    plotDrawer.classList.add("collapsed");
    titleEl.textContent = "Plot Viewer";
    metaEl.textContent = "Open a session from the ML table above.";
    statusLine.textContent = "Pick a session above to load the plot viewer.";
  }

  btnBack.onclick = () => closeSessionViewer();
  btnDownload.onclick = () => {
    if (sid && currentCsv) downloadTextFile(`TSP_ML_${sid}.csv`, currentCsv);
  };

  window.openSessionViewer = openSessionViewer;
  window.closeSessionViewer = closeSessionViewer;

  const sidFromQuery = new URLSearchParams(location.search).get("sid") || "";
  if (sidFromQuery) openSessionViewer(sidFromQuery);
})();