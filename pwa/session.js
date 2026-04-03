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
  const rngZoom = $("rngZoom");
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
  const btnPrevArc = $("btnPrevArc");
  const btnNextArc = $("btnNextArc");
  const arcReadout = $("arcReadout");
  const arcIndexInput = $("arcIndexInput");
  let btnAddArc = $("btnAddArc");
  let btnClearArc = $("btnClearArc");

  if (!plotDrawer) return;

  function setViewerOpen(open) {
    if (!plotDrawer) return;
    const nextHidden = !open;
    plotDrawer.hidden = nextHidden;
    if (nextHidden) plotDrawer.setAttribute("hidden", "");
    else plotDrawer.removeAttribute("hidden");
    plotDrawer.classList.toggle("collapsed", nextHidden);
    plotDrawer.setAttribute("aria-hidden", nextHidden ? "true" : "false");
  }

  let sid = "";
  let currentCsv = "";
  let csvHeaders = [];
  let currentDownloadName = "TSP_ML_session.csv";

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

  const DEFAULT_ON = new Set(["i_rms", "label_arc", "model_pred", "irms_drop_vs_baseline"]);
  const DEFAULT_Y2 = new Set(["i_rms", "current", "v_rms", "voltage", "temp_c", "temp"]);

  setViewerOpen(false);
  ensureArcEditorControls();
  refreshDownloadBinding();

  function normalizeBinaryLabel(v) {
    if (typeof v === "boolean") return v ? 1 : 0;
    const n = Number(String(v ?? "").trim());
    return Number.isFinite(n) && n >= 0.5 ? 1 : 0;
  }

  function safeCsvFilename(name) {
    let out = String(name || `TSP_ML_${sid || "session"}.csv`).trim();
    if (!out.toLowerCase().endsWith(".csv")) out += ".csv";
    return out.replace(/[\/:*?"<>|]+/g, "_");
  }

  function viewerDurationSeconds(meta) {
    const explicit = Number(meta?.duration_s);
    if (Number.isFinite(explicit) && explicit > 0) return explicit;
    const rows = Number(meta?.row_count || 0);
    if (rows > 0) return rows / 30;
    return null;
  }

  function viewerMetaText(meta) {
    const load = meta?.load_type ?? "—";
    const dur = viewerDurationSeconds(meta);
    const durText = (dur === null) ? "—" : (Math.abs(dur - Math.round(dur)) < 0.05 ? String(Math.round(dur)) : dur.toFixed(1));
    return `load=${load}  duration=${durText}s`;
  }

  function ensureCsvHeadersAndLabelArc(rows, parsedFields = []) {
    const normalizedFields = Array.isArray(parsedFields)
      ? parsedFields.map(normalizeHeaderName).filter(Boolean)
      : [];
    const headers = normalizedFields.length ? normalizedFields.slice() : Object.keys(rows[0] || {});
    if (!headers.includes("label_arc")) headers.push("label_arc");

    rows.forEach((row) => {
      if (!row || typeof row !== "object") return;
      row.label_arc = normalizeBinaryLabel(row.label_arc);
      headers.forEach((key) => {
        if (!(key in row)) row[key] = (key === "label_arc") ? 0 : "";
      });
    });

    return headers;
  }

  function buildCsvFromRows() {
    if (!ROWS.length) return currentCsv || "";
    csvHeaders = ensureCsvHeadersAndLabelArc(ROWS, csvHeaders);
    return Papa.unparse({
      fields: csvHeaders,
      data: ROWS.map((row) => csvHeaders.map((key) => row?.[key] ?? ((key === "label_arc") ? 0 : "")))
    });
  }

  function refreshDownloadBinding() {
    if (!btnDownload) return;
    btnDownload.onclick = () => {
      const csv = buildCsvFromRows();
      if (!csv) return;
      currentCsv = csv;
      downloadTextFile(currentDownloadName || safeCsvFilename(`TSP_ML_${sid || "session"}.csv`), csv);
    };
  }

  function patchArcMarkerLabelText() {
    const labelEl = chkEvents?.closest("label");
    if (!labelEl || labelEl.dataset.arcViewPatched === "1") return;
    labelEl.dataset.arcViewPatched = "1";
    labelEl.innerHTML = "";
    labelEl.appendChild(chkEvents);
    labelEl.append(document.createTextNode(" View arc markers"));
  }

  function currentLabelArcAtPlayhead() {
    if (!ROWS.length || playIdx < 0 || playIdx >= ROWS.length) return 0;
    return normalizeBinaryLabel(ROWS[playIdx]?.label_arc);
  }

  function updateArcEditButtons() {
    const hasRows = ROWS.length > 0 && playIdx >= 0 && playIdx < ROWS.length;
    const isArc = hasRows ? currentLabelArcAtPlayhead() === 1 : false;

    if (btnAddArc) {
      btnAddArc.disabled = !hasRows || isArc;
      btnAddArc.textContent = isArc ? "Arc Marker Added" : "Add Arc Marker";
    }

    if (btnClearArc) {
      btnClearArc.disabled = !hasRows || !isArc;
    }
  }

  function ensureArcEditorControls() {
    patchArcMarkerLabelText();

    if (!btnAddArc || !btnClearArc) {
      const host = chkEvents?.closest(".chip")?.parentElement
        || document.querySelector("#plotDrawer .controlsbar .right")
        || document.querySelector("#plotDrawer .controlsbar .left")
        || document.querySelector("#plotDrawer .controlsbar");

      if (host) {
        if (!btnAddArc) {
          btnAddArc = document.createElement("button");
          btnAddArc.type = "button";
          btnAddArc.id = "btnAddArc";
          btnAddArc.className = "btn btn-small btn-info";
          btnAddArc.textContent = "Add Arc Marker";
          host.appendChild(btnAddArc);
        }
        if (!btnClearArc) {
          btnClearArc = document.createElement("button");
          btnClearArc.type = "button";
          btnClearArc.id = "btnClearArc";
          btnClearArc.className = "btn btn-small btn-danger";
          btnClearArc.textContent = "Clear Arc Marker";
          host.appendChild(btnClearArc);
        }
      }
    }

    if (btnAddArc && btnAddArc.dataset.bound !== "1") {
      btnAddArc.dataset.bound = "1";
      btnAddArc.addEventListener("click", () => {
        try {
          setArcMarkers(getArcEditTargetIndices(), true);
        } catch (err) {
          setStatus(err?.message || "Invalid arc index selection.");
        }
      });
    }

    if (btnClearArc && btnClearArc.dataset.bound !== "1") {
      btnClearArc.dataset.bound = "1";
      btnClearArc.addEventListener("click", () => {
        try {
          setArcMarkers(getArcEditTargetIndices(), false);
        } catch (err) {
          setStatus(err?.message || "Invalid arc index selection.");
        }
      });
    }

    updateArcEditButtons();
  }

  function setStatus(text) {
    if (statusLine) statusLine.textContent = text;
  }

  function updateArcReadout() {
    if (!arcReadout) return;
    if (!X.length) {
      arcReadout.textContent = "arcs=—";
      return;
    }
    const total = ARC_IDXS.length;
    if (!total) {
      arcReadout.textContent = "arcs=0";
      return;
    }
    let current = 0;
    for (let i = 0; i < ARC_IDXS.length; i++) {
      if (ARC_IDXS[i] >= playIdx) { current = i + 1; break; }
    }
    if (!current) current = total;
    arcReadout.textContent = `arcs=${total} • ${current}/${total}`;
  }

  function nearestArcIndex(direction) {
    if (!ARC_IDXS.length) return -1;
    if (direction < 0) {
      for (let i = ARC_IDXS.length - 1; i >= 0; i--) {
        if (ARC_IDXS[i] < playIdx) return ARC_IDXS[i];
      }
      return ARC_IDXS[ARC_IDXS.length - 1];
    }
    for (let i = 0; i < ARC_IDXS.length; i++) {
      if (ARC_IDXS[i] > playIdx) return ARC_IDXS[i];
    }
    return ARC_IDXS[0];
  }

  function jumpToArc(direction) {
    const idx = nearestArcIndex(direction);
    if (idx < 0) {
      setStatus("No arc markers available in this session.");
      return;
    }
    pause();
    setPlayIdx(idx, true);
    updateStats();
  }

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

  function normalizeHeaderName(name) {
    const raw = String(name || "").trim();
    const slug = raw.toLowerCase().replace(/[^a-z0-9]+/g, "_").replace(/^_+|_+$/g, "");
    const aliases = {
      i_rms: "i_rms",
      current: "current",
      v_rms: "v_rms",
      voltage: "voltage",
      temp_c: "temp_c",
      temperature: "temp_c",
      model_pred: "model_pred",
      label_arc: "label_arc",
      feat_valid: "feat_valid",
      current_valid: "current_valid",
      session_id: "session_id",
      load_type: "load_type",
      epoch_ms: "epoch_ms",
      adc_fs_hz: "adc_fs_hz",
      cycle_nmse: "cycle_nmse",
      zcv: "zcv",
      zc_dwell_ratio: "zc_dwell_ratio",
      cycle_rms_drop_ratio: "cycle_rms_drop_ratio",
      peak_fluct_cv: "peak_fluct_cv",
      midband_residual_rms: "midband_residual_rms",
      hf_band_energy_ratio: "hf_band_energy_ratio",
      spec_entropy: "spec_entropy",
      neg_dip_event_ratio: "neg_dip_event_ratio",
      pre_dip_spike_ratio: "irms_drop_vs_baseline",
      irms_drop_vs_baseline: "irms_drop_vs_baseline",
      thd_i: "thd_i"
    };
    return aliases[slug] || slug || raw;
  }

  function buildSeriesKeys(rows) {
    if (!rows.length) return [];
    const keys = Object.keys(rows[0]);
    return keys.filter((k) => {
      if (k === "timestamp" || k === "session_id" || k === "load_type") return false;
      if (k === "epoch_ms" || k === "thd_i" || k === "wpe_entropy" || k === "dip_rebound_ratio") return false;
      const val = rows.find((r) => r[k] !== undefined && r[k] !== null)?.[k];
      if (val === undefined) return false;
      const num = typeof val === "number" ? val : Number(String(val).trim());
      return Number.isFinite(num);
    });
  }

  function renderToggleList() {
    if (!toggleList) return;
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

      const nameEl = document.createElement("div");
      nameEl.textContent = k;

      const sel = document.createElement("select");
      sel.className = "axisSel";
      sel.innerHTML = `<option value="y">Y1</option><option value="y2">Y2</option>`;
      sel.value = axisPref.get(k) || "y";
      sel.onchange = () => {
        axisPref.set(k, sel.value);
        rebuildForDataMode();
      };

      wrap.appendChild(cb);
      wrap.appendChild(nameEl);
      wrap.appendChild(sel);
      toggleList.appendChild(wrap);
    });

    applySeriesFilter();
  }

  function refreshDerivedData(resetPrefs = false) {
    if (!ROWS.length) return false;

    csvHeaders = ensureCsvHeadersAndLabelArc(ROWS, csvHeaders);
    X = pickTimeAxis(ROWS);
    KEYS = buildSeriesKeys(ROWS);

    if (!KEYS.length) return false;

    DATA_RAW = makeData(ROWS, X, KEYS);

    const win0 = Number(rngSmooth?.value) || 15;
    if (smoothReadout) smoothReadout.textContent = `win=${win0}`;
    DATA_SMOOTH = makeSmoothedData(DATA_RAW, win0);
    DATA_NORM = normalizeData(DATA_RAW);
    DATA_SMOOTH_NORM = normalizeData(DATA_SMOOTH);

    buildArcIndexes();

    const defaults = new Set(preferredDefaultKeys(KEYS));
    KEYS.forEach((k) => {
      if (resetPrefs || !showPref.has(k)) showPref.set(k, defaults.has(k));
      if (resetPrefs || !axisPref.has(k)) axisPref.set(k, DEFAULT_Y2.has(k) ? "y2" : "y");
    });

    Array.from(showPref.keys()).forEach((k) => { if (!KEYS.includes(k)) showPref.delete(k); });
    Array.from(axisPref.keys()).forEach((k) => { if (!KEYS.includes(k)) axisPref.delete(k); });

    renderToggleList();
    scrub.max = String(Math.max(0, X.length - 1));
    speed = Number(selSpeed?.value) || 1.0;
    currentCsv = buildCsvFromRows();
    refreshDownloadBinding();
    updateArcEditButtons();
    return true;
  }

  function ingestParsedCsv(parsed, options = {}) {
    ROWS = (parsed?.data || []).filter((r) => r && Object.keys(r).length);

    if (!ROWS.length) {
      setStatus(options.emptyStatus || "Parsed 0 rows.");
      clearValueReadout();
      updateArcEditButtons();
      return false;
    }

    csvHeaders = ensureCsvHeadersAndLabelArc(ROWS, parsed?.meta?.fields || []);
    currentDownloadName = safeCsvFilename(options.downloadName || `TSP_ML_${sid || "session"}.csv`);

    if (!refreshDerivedData(true)) {
      setStatus(options.noNumericStatus || "No numeric series found in this CSV.");
      clearValueReadout();
      updateArcEditButtons();
      return false;
    }

    setStatus(options.readyStatus ? options.readyStatus.replace(/__ROWS__/g, String(ROWS.length)).replace(/__ARCS__/g, String(ARC_IDXS.length)) : `Rows: ${ROWS.length} | Arc markers=${ARC_IDXS.length}`);
    clearValueReadout();

    requestAnimationFrame(() => {
      buildPlot();
      if (X.length) {
        setPlayIdx(0, true);
        applyZoomPercent(100, X[0]);
      }
      updateStats();
      updateArcEditButtons();
    });

    return true;
  }

  function parseArcIndexSelection(raw) {
    const text = String(raw || "").trim();
    if (!text) return [];

    const out = new Set();
    const tokens = text.split(/[\s,]+/).map((part) => part.trim()).filter(Boolean);
    const maxIdx = Math.max(0, ROWS.length - 1);

    for (const token of tokens) {
      const rangeMatch = token.match(/^(\d+)\s*[-:]\s*(\d+)$/);
      if (rangeMatch) {
        let a = Number(rangeMatch[1]);
        let b = Number(rangeMatch[2]);
        if (!Number.isFinite(a) || !Number.isFinite(b)) throw new Error(`Invalid range: ${token}`);
        if (a > b) [a, b] = [b, a];
        for (let idx = a; idx <= b; idx++) {
          if (idx >= 0 && idx <= maxIdx) out.add(idx);
        }
        continue;
      }

      const single = Number(token);
      if (!Number.isInteger(single)) throw new Error(`Invalid index: ${token}`);
      if (single >= 0 && single <= maxIdx) out.add(single);
    }

    return Array.from(out).sort((a, b) => a - b);
  }

  function getArcEditTargetIndices() {
    const typed = String(arcIndexInput?.value || "").trim();
    if (!typed) {
      if (!ROWS.length || playIdx < 0 || playIdx >= ROWS.length) return [];
      return [playIdx];
    }
    return parseArcIndexSelection(typed);
  }

  function setArcMarkers(indices, on) {
    if (!ROWS.length || !Array.isArray(indices) || !indices.length) {
      setStatus("No valid row indices selected for arc editing.");
      updateArcEditButtons();
      return;
    }

    pause();
    const next = on ? 1 : 0;
    let changed = 0;

    indices.forEach((idx) => {
      if (idx < 0 || idx >= ROWS.length) return;
      const prev = normalizeBinaryLabel(ROWS[idx]?.label_arc);
      if (prev === next) return;
      ROWS[idx].label_arc = next;
      changed += 1;
    });

    if (!changed) {
      const label = indices.length === 1 ? `Row ${indices[0]}` : `${indices.length} selected rows`;
      setStatus(`${label}: label_arc already ${next}.`);
      updateArcEditButtons();
      return;
    }

    currentCsv = buildCsvFromRows();

    const keepIdx = Math.max(0, Math.min(X.length - 1, indices[0]));
    const keepScale = plot ? { min: plot.scales.x.min, max: plot.scales.x.max } : null;

    if (!refreshDerivedData(false)) return;

    buildPlot();
    if (X.length) {
      setPlayIdx(keepIdx, true);
      if (plot && keepScale && Number.isFinite(keepScale.min) && Number.isFinite(keepScale.max)) {
        const [min, max] = clampXWindow(keepScale.min, keepScale.max);
        plot.setScale("x", { min, max });
        syncZoomSlider();
      }
    }

    updateStats();
    updateArcEditButtons();
    const targetLabel = indices.length === 1
      ? `Row ${indices[0]}`
      : `${changed}/${indices.length} rows`;
    setStatus(`${targetLabel}: label_arc=${next}. Download will include the edited CSV.`);
  }

  function setArcMarkerAt(idx, on) {
    setArcMarkers([idx], on);
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

  function preferredDefaultKeys(keys) {
    const preferred = ["i_rms", "label_arc", "model_pred", "irms_drop_vs_baseline", "cycle_rms_drop_ratio", "cycle_nmse", "zcv", "neg_dip_event_ratio", "v_rms", "temp_c", "current", "voltage", "temp"];
    const picked = preferred.filter((k) => keys.includes(k));
    if (picked.length) return picked.slice(0, 8);
    return keys.filter((k) => k !== "label_arc").slice(0, Math.min(8, keys.length));
  }

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
    csvHeaders = [];
    currentDownloadName = "TSP_ML_session.csv";
    if (toggleList) toggleList.innerHTML = "";
    if (valueLine) valueLine.innerHTML = "";
    if (statsBody) statsBody.innerHTML = "";
    if (statsHint) statsHint.textContent = "Stats enabled. Load a session or uploaded CSV to see window statistics.";
    if (btnPrevArc) btnPrevArc.disabled = true;
    if (btnNextArc) btnNextArc.disabled = true;
    updateArcReadout();
    setStatus("Loading…");
    if (scrub) { scrub.min = "0"; scrub.max = "0"; scrub.value = "0"; }
    if (arcIndexInput) arcIndexInput.value = "";
    if (timeReadout) timeReadout.textContent = "t=—";
    updateArcEditButtons();
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
    if (!c) return { w: 320, h: 300 };

    const style = window.getComputedStyle(c);
    const cssWidth = parseFloat(style.width);
    const cssHeight = parseFloat(style.height);
    const parentWidth = c.parentElement?.clientWidth || 0;

    const w = Math.max(320, Math.floor((Number.isFinite(cssWidth) ? cssWidth : 0) || c.clientWidth || parentWidth || 0));
    const h = Math.max(300, Math.floor((Number.isFinite(cssHeight) ? cssHeight : 0) || c.clientHeight || 0));
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
    if (!DATA_RAW) return;

    const labelIdx = KEYS.indexOf("label_arc");
    const labelArr = labelIdx >= 0 ? DATA_RAW[labelIdx + 1] : null;

    const n = X.length || ((labelArr && labelArr.length) || 0);
    for (let i = 0; i < n; i++) {
      const isLabel = labelArr ? Number(labelArr[i]) === 1 : false;
      if (isLabel) ARC_IDXS.push(i);
    }

    if (btnPrevArc) btnPrevArc.disabled = ARC_IDXS.length === 0;
    if (btnNextArc) btnNextArc.disabled = ARC_IDXS.length === 0;
    if (chkEvents) {
      chkEvents.disabled = ARC_IDXS.length === 0;
      if (ARC_IDXS.length > 0) chkEvents.checked = true;
    }
    updateArcReadout();
    updateArcEditButtons();
  }

  function clampXWindow(min, max) {
    if (!X.length) return [0, 0];
    const fullMin = FULL_X_MIN;
    const fullMax = FULL_X_MAX;
    const fullRange = Math.max(0, fullMax - fullMin);
    if (fullRange <= 0) return [fullMin, fullMax];

    let lo = Number.isFinite(min) ? min : fullMin;
    let hi = Number.isFinite(max) ? max : fullMax;
    let range = hi - lo;

    const minRange = Math.max(avgDt(X) * 8, fullRange / Math.max(32, Math.min(X.length, 512)));
    range = Math.max(minRange, Math.min(fullRange, range || fullRange));

    if (lo < fullMin) {
      hi += fullMin - lo;
      lo = fullMin;
    }
    if (hi > fullMax) {
      lo -= hi - fullMax;
      hi = fullMax;
    }

    lo = Math.max(fullMin, lo);
    hi = Math.min(fullMax, hi);

    if ((hi - lo) < range) {
      const short = range - (hi - lo);
      const pushLeft = Math.min(lo - fullMin, short * 0.5);
      const pushRight = Math.min(fullMax - hi, short - pushLeft);
      lo -= pushLeft;
      hi += pushRight;
    }

    return [Math.max(fullMin, lo), Math.min(fullMax, hi)];
  }

  function syncZoomSlider() {
    if (!rngZoom || !plot || !X.length) return;
    const full = Math.max(1e-9, FULL_X_MAX - FULL_X_MIN);
    const cur = Math.max(1e-9, plot.scales.x.max - plot.scales.x.min);
    const pct = Math.round(Math.max(5, Math.min(100, (cur / full) * 100)));
    rngZoom.value = String(pct);
  }

  function applyZoomPercent(pct, centerX = null) {
    if (!plot || !X.length) return;
    const full = Math.max(1e-9, FULL_X_MAX - FULL_X_MIN);
    const ratio = Math.max(0.05, Math.min(1.0, Number(pct || 100) / 100.0));
    const width = full * ratio;
    const anchorX = Number.isFinite(centerX) ? centerX : X[playIdx];
    const [min, max] = clampXWindow(anchorX - (width * 0.5), anchorX + (width * 0.5));
    plot.setScale("x", { min, max });
    syncZoomSlider();
  }

  function resetZoom() {
    if (!plot || !X.length) return;
    const keepIdx = playIdx;
    const resume = playing;
    pause();
    buildPlot();
    setPlayIdx(keepIdx, true);
    applyZoomPercent(100, X[keepIdx]);
    if (resume) play();
  }

  function zoomPanPlugin() {
    let isPanning = false;
    let panStartX = 0;
    let startMin = 0, startMax = 0;
    let dragged = false;
    let mouseMoveHandler = null;
    let mouseUpHandler = null;
    let wheelHandler = null;
    let dblClickHandler = null;
    let clickHandler = null;
    let mouseDownHandler = null;

    const onMouseMove = (u, e) => {
      if (!isPanning) return;
      dragged = true;
      const rect = u.root.getBoundingClientRect();
      const x = e.clientX - rect.left;
      const dxPx = x - panStartX;
      const dxVal = u.posToVal(0, "x") - u.posToVal(dxPx, "x");
      const [min, max] = clampXWindow(startMin + dxVal, startMax + dxVal);
      u.setScale("x", { min, max });
    };
    const onMouseUp = () => { isPanning = false; };

    return {
      hooks: {
        ready: [
          (u) => {
            mouseDownHandler = (e) => {
              if (!e.shiftKey || e.button !== 0) return;
              isPanning = true;
              dragged = false;
              panStartX = e.offsetX;
              startMin = u.scales.x.min;
              startMax = u.scales.x.max;
              e.preventDefault();
            };
            mouseMoveHandler = (e) => onMouseMove(u, e);
            mouseUpHandler = () => onMouseUp();
            wheelHandler = (e) => {
              e.preventDefault();
              const left = e.offsetX;
              const xVal = u.posToVal(left, "x");
              const sc = u.scales.x;
              const min = sc.min, max = sc.max;
              const range = max - min;
              const factor = e.deltaY < 0 ? 0.85 : 1.18;
              const newRange = Math.min((FULL_X_MAX - FULL_X_MIN), Math.max(avgDt(X) * 8, range * factor));
              const frac = range > 0 ? (xVal - min) / range : 0.5;
              const newMin = xVal - frac * newRange;
              const newMax = newMin + newRange;
              const [clampedMin, clampedMax] = clampXWindow(newMin, newMax);
              u.setScale("x", { min: clampedMin, max: clampedMax });
            };
            dblClickHandler = () => resetZoom();
            clickHandler = (e) => {
              if (dragged) {
                dragged = false;
                return;
              }
              const x = u.posToVal(e.offsetX, "x");
              const idx = lowerBound(X, x);
              if (idx >= 0 && idx < X.length) setPlayIdx(idx, true);
            };

            u.root.addEventListener("mousedown", mouseDownHandler);
            window.addEventListener("mousemove", mouseMoveHandler);
            window.addEventListener("mouseup", mouseUpHandler);
            u.root.addEventListener("wheel", wheelHandler, { passive: false });
            u.root.addEventListener("dblclick", dblClickHandler);
            u.root.addEventListener("click", clickHandler);
          }
        ],
        destroy: [
          (u) => {
            if (mouseDownHandler) u.root.removeEventListener("mousedown", mouseDownHandler);
            if (wheelHandler) u.root.removeEventListener("wheel", wheelHandler);
            if (dblClickHandler) u.root.removeEventListener("dblclick", dblClickHandler);
            if (clickHandler) u.root.removeEventListener("click", clickHandler);
            if (mouseMoveHandler) window.removeEventListener("mousemove", mouseMoveHandler);
            if (mouseUpHandler) window.removeEventListener("mouseup", mouseUpHandler);
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
        setScale: [() => { updateStats(); syncZoomSlider(); }]
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
    if (visibleSeriesCount() === 0) setStatus("No series selected. Check at least one series.");

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
      cursor: { show: true, lock: true, points: { show: false }, drag: { x: true, y: false, setScale: true } },
      select: { show: true },
      hooks: {
        setScale: [() => {
          syncZoomSlider();
          updateStats();
        }],
      },
      plugins: [zoomPanPlugin(), eventsPlugin(), statsPlugin(), playheadPlugin()],
    };

    if (plot) plot.destroy();
    plot = new uPlot(opts, data, $("chart"));

    FULL_X_MIN = X[0];
    FULL_X_MAX = X[X.length - 1];

    btnResetZoom.onclick = () => resetZoom();

    setPlayIdx(Math.min(playIdx, X.length - 1), true);
    syncZoomSlider();
    updateValueReadout();
    updateArcReadout();
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
    updateArcReadout();
    updateArcEditButtons();
    if (plot) plot.redraw();

    if (plot && (chkFollow?.checked || playing)) {
      const sc = plot.scales.x;
      const x = X[playIdx];
      const min = sc.min, max = sc.max;
      const r = (max - min) || 1;

      if (playing) {
        const [nextMin, nextMax] = clampXWindow(x - 0.45 * r, x + 0.55 * r);
        plot.setScale("x", { min: nextMin, max: nextMax });
      } else if (x < min + 0.12 * r || x > max - 0.12 * r) {
        const [nextMin, nextMax] = clampXWindow(x - 0.5 * r, x + 0.5 * r);
        plot.setScale("x", { min: nextMin, max: nextMax });
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

    if (!KEYS.length) {
      statsHint.textContent = "Stats enabled. Load a session or uploaded CSV to see window statistics.";
      statsBody.innerHTML = `<tr><td colspan="6" class="mono">No dataset loaded yet.</td></tr>`;
      return;
    }

    const source = currentData() || DATA_RAW;
    if (!source || !plot) {
      statsHint.textContent = "Stats unavailable.";
      statsBody.innerHTML = `<tr><td colspan="6" class="mono">Plot is not ready yet.</td></tr>`;
      return;
    }

    const [i0, i1] = currentXRangeIdx();
    const t0 = X[i0];
    const t1 = X[i1];
    const visibleKeys = KEYS.filter((k) => showPref.get(k));
    const selectedKeys = visibleKeys.length ? visibleKeys : preferredDefaultKeys(KEYS);
    const cap = 20;
    const shownKeys = selectedKeys.slice(0, cap);
    const extra = Math.max(0, selectedKeys.length - shownKeys.length);
    const arcInView = ARC_IDXS.filter((idx) => idx >= i0 && idx <= i1).length;

    statsHint.textContent = `Stats | idx ${i0}-${i1} | t=${fmt(t0)}s..${fmt(t1)}s | arcs in view=${arcInView}` + (extra ? ` | +${extra} more hidden` : "");

    if (!shownKeys.length) {
      statsBody.innerHTML = `<tr><td colspan="6" class="mono">No visible numeric series selected.</td></tr>`;
      return;
    }

    statsBody.innerHTML = shownKeys.map((k) => {
      const si = KEYS.indexOf(k) + 1;
      const arr = source[si];
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
    const source = currentData() || DATA_RAW;
    if (!source || !KEYS.length || !plot) return;
    const [i0, i1] = currentXRangeIdx();
    const selectedKeys = KEYS.filter(k => showPref.get(k));
    const shownKeys = (selectedKeys.length ? selectedKeys : preferredDefaultKeys(KEYS)).slice(0, 64);

    let out = "series,mean,min,max,std,n\n";
    for (const k of shownKeys) {
      const si = KEYS.indexOf(k) + 1;
      const st = computeStats(source[si], i0, i1);
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
  chkEvents.onchange = () => { if (plot) { plot.redraw(); updateStats(); } };
  rngZoom?.addEventListener("input", () => { if (plot) applyZoomPercent(Number(rngZoom.value) || 100); });
  btnPrevArc?.addEventListener("click", () => jumpToArc(-1));
  btnNextArc?.addEventListener("click", () => jumpToArc(1));

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
    const defaults = new Set(preferredDefaultKeys(KEYS));
    KEYS.forEach((k, i) => {
      const on = defaults.has(k);
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
  arcIndexInput?.addEventListener("keydown", (ev) => {
    if (ev.key !== "Enter") return;
    ev.preventDefault();
    try {
      setArcMarkers(getArcEditTargetIndices(), true);
    } catch (err) {
      setStatus(err?.message || "Invalid arc index selection.");
    }
  });

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
      setStatus("Missing sid.");
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
      metaEl.textContent = viewerMetaText(meta);

      setStatus("Fetching CSV chunks…");
      const csv = await fetchSessionCsv(sid);
      currentCsv = csv;
      if (!csv) {
        setStatus("No CSV chunks found for this session.");
        clearValueReadout();
        return;
      }

      btnDownload.onclick = () => downloadTextFile(`TSP_ML_${sid}.csv`, currentCsv);

      setStatus("Parsing CSV…");
      const parsed = Papa.parse(csv.trim(), { header: true, dynamicTyping: true, skipEmptyLines: true, transformHeader: normalizeHeaderName });
      ingestParsedCsv(parsed, {
        downloadName: `TSP_ML_${sid}.csv`,
        emptyStatus: "Parsed 0 rows. (CSV exists but has no data rows.)",
        noNumericStatus: "No numeric series found in this session CSV.",
        readyStatus: `Rows: __ROWS__ | Arc markers=__ARCS__ | Click plot to choose row, then add or clear marker`
      });
    } catch (e) {
      console.error(e);
      setStatus("Failed to load this session (check Firebase rules / network).");
      clearValueReadout();
    }
  }


  function openSessionViewer(targetSid, metaOverride = null) {
    setViewerOpen(true);
    setTimeout(() => {
      plotDrawer?.scrollIntoView?.({ behavior: "smooth", block: "start" });
    }, 10);
    loadSession(targetSid, metaOverride);
  }

  function openSessionViewerFromCsv(name, csvText, metaOverride = null) {
    setViewerOpen(true);
    setTimeout(() => {
      plotDrawer?.scrollIntoView?.({ behavior: "smooth", block: "start" });
    }, 10);

    sid = (name || "uploaded_csv").replace(/[^a-zA-Z0-9_.-]/g, "_");
    resetState();
    titleEl.textContent = `CSV: ${sid}`;
    const meta = metaOverride || {};
    metaEl.textContent = viewerMetaText(meta);
    currentCsv = csvText || "";

    try {
      if (!currentCsv.trim()) {
        setStatus("Uploaded CSV is empty.");
        clearValueReadout();
        return;
      }

      setStatus("Parsing uploaded CSV…");
      const parsed = Papa.parse(currentCsv.trim(), { header: true, dynamicTyping: true, skipEmptyLines: true, transformHeader: normalizeHeaderName });
      ingestParsedCsv(parsed, {
        downloadName: `TSP_ML_${sid}.csv`,
        emptyStatus: "Parsed 0 rows from uploaded CSV.",
        noNumericStatus: "No numeric series found in uploaded CSV.",
        readyStatus: `Rows: __ROWS__ | Uploaded CSV ready | Arc markers=__ARCS__`
      });
    } catch (e) {
      console.error(e);
      setStatus("Failed to parse uploaded CSV.");
      clearValueReadout();
    }
  }

  function closeSessionViewer() {
    closeSeries();
    resetState();
    refreshDownloadBinding();
    setViewerOpen(false);
    titleEl.textContent = "Plot Viewer";
    metaEl.textContent = "Open a session from the ML table above.";
    setStatus("Pick a session above to load the plot viewer.");
  }

  if (btnBack) {
    btnBack.type = "button";
    btnBack.addEventListener("click", (ev) => {
      ev.preventDefault();
      ev.stopPropagation();
      closeSessionViewer();
    });
  }
  refreshDownloadBinding();

  window.openSessionViewer = openSessionViewer;
  window.openSessionViewerFromCsv = openSessionViewerFromCsv;
  window.closeSessionViewer = closeSessionViewer;

  const sidFromQuery = new URLSearchParams(location.search).get("sid") || "";
  if (sidFromQuery) openSessionViewer(sidFromQuery);
})();