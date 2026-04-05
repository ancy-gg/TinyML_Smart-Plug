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
  const seriesPanel = $("seriesPanel");
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

  const btnStats = $("btnStats");
  const statsWrap = $("statsWrap");
  const statsHint = $("statsHint");
  const statsBody = $("statsBody");
  const btnCopyStats = $("btnCopyStats");

  const segLabelSelect = $("segLabelSelect");
  const segStartInput = $("segStartInput");
  const segEndInput = $("segEndInput");
  const segLoadInput = $("segLoadInput");
  const segTrialInput = $("segTrialInput");
  const btnSegStartFromPlay = $("btnSegStartFromPlay");
  const btnSegEndFromPlay = $("btnSegEndFromPlay");
  const btnAddSegment = $("btnAddSegment");
  const btnClearSegment = $("btnClearSegment");
  const segmentList = $("segmentList");

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
    if (nextHidden) {
      plotDrawer.setAttribute("hidden", "");
      plotDrawer.style.display = "none";
    } else {
      plotDrawer.removeAttribute("hidden");
      plotDrawer.style.removeProperty("display");
    }
    plotDrawer.classList.toggle("collapsed", nextHidden);
    plotDrawer.setAttribute("aria-hidden", nextHidden ? "true" : "false");
  }

  let sid = "";
  let currentCsv = "";
  let csvHeaders = [];
  let currentDownloadName = "TSP_ML_session.csv";
  let currentMeta = {};
  let SEGMENTS = [];

  function formatDisplayTimestamp(ms) {
    if (!ms || ms <= 0) return "—";
    const d = new Date(ms);
    const datePart = new Intl.DateTimeFormat("en-US", {
      timeZone: "Asia/Manila",
      month: "long",
      day: "2-digit",
      year: "numeric"
    }).format(d);
    const timePart = new Intl.DateTimeFormat("en-US", {
      timeZone: "Asia/Manila",
      hour: "2-digit",
      minute: "2-digit",
      second: "2-digit",
      hour12: true
    }).format(d);
    return `${datePart} | ${timePart}`;
  }

  function titleizeTokenText(v, fallback = "Session") {
    const base = String(v || "")
      .replace(/\.[a-z0-9]+$/i, "")
      .replace(/[_-]+/g, " ")
      .replace(/\s+/g, " ")
      .trim();
    if (!base) return fallback;
    return base.replace(/\b\w+/g, (m) => m.charAt(0).toUpperCase() + m.slice(1).toLowerCase());
  }

  function safeFilenameSegment(v, fallback = "session") {
    return String(v || fallback)
      .replace(/\.[a-z0-9]+$/i, "")
      .replace(/[^a-zA-Z0-9_-]+/g, "_")
      .replace(/^_+|_+$/g, "")
      .slice(0, 64) || fallback;
  }

  function isUploadedCsvSession(meta) {
    return !!(meta?.uploaded_csv || String(meta?.load_type || "").trim().toLowerCase() === "uploaded_csv");
  }

  function viewerDisplayName(meta, sessionId) {
    const sourceFile = String(meta?.source_file || "").trim();
    if (sourceFile) return titleizeTokenText(sourceFile, sourceFile);
    const load = String(meta?.load_type || "").trim();
    if (load && load.toLowerCase() !== "uploaded_csv" && load.toLowerCase() !== "unknown") return titleizeTokenText(load, load);
    if (isUploadedCsvSession(meta)) return "Uploaded CSV";
    const start = Number(meta?.start_ms || 0);
    if (start) return `Session ${formatDisplayTimestamp(start)}`;
    return titleizeTokenText(sessionId || "session", "Session");
  }

  function viewerDownloadName(meta, sessionId) {
    const base = viewerDisplayName(meta, sessionId);
    return `TSP_ML_${safeFilenameSegment(base, safeFilenameSegment(sessionId || 'session', 'session'))}.csv`;
  }

  function announceActiveSession(sessionId) {
    const activeSid = String(sessionId || "").trim();
    window.__tspActiveViewerSid = activeSid;
    document.dispatchEvent(new CustomEvent("tsp-active-session-changed", { detail: { sid: activeSid } }));
  }

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

  function viewerMetaText(meta) {
    const load = String(meta?.load_type || meta?.source_file || "—").trim();
    const dur = viewerDurationSeconds(meta);
    const durText = (dur === null)
      ? "—"
      : (Math.abs(dur - Math.round(dur)) < 0.05 ? `${Math.round(dur)}s` : `${dur.toFixed(1)}s`);
    const startText = Number(meta?.start_ms || 0) > 0 ? formatDisplayTimestamp(Number(meta.start_ms)) : "—";
    return `${titleizeTokenText(load, "Unknown")} • ${durText} • ${startText}`;
  }

  function parseDatasetFilenameMeta(name) {
    const stem = String(name || "").replace(/\.[a-z0-9]+$/i, "").trim();
    const parts = stem.split(/[_\s-]+/).filter(Boolean);
    const aliases = { startup: "start", start: "start", steady: "steady", baseline: "steady", arc: "arc", close: "close", closing: "close" };
    const last = String(parts[parts.length - 1] || "").toLowerCase();
    const division = aliases[last] || "";
    let trial = 1;
    let endIdx = parts.length;
    if (division) endIdx -= 1;
    if (endIdx > 0) {
      const maybeTrial = Number(parts[endIdx - 1]);
      if (Number.isInteger(maybeTrial) && maybeTrial > 0) {
        trial = maybeTrial;
        endIdx -= 1;
      }
    }
    const loadType = safeFilenameSegment(parts.slice(0, Math.max(0, endIdx)).join("_"), "session");
    return { loadType, trial, division };
  }

  function safePositiveInt(value, fallback = 1) {
    const n = Number(value);
    return Number.isInteger(n) && n > 0 ? n : fallback;
  }

  function segmentLabelTitle(label) {
    const norm = String(label || "steady").trim().toLowerCase();
    return norm ? norm.charAt(0).toUpperCase() + norm.slice(1) : "Steady";
  }

  function segmentColor(label, alpha = 0.16) {
    const a = Math.max(0.04, Math.min(0.85, alpha));
    const colors = {
      start: `rgba(77,163,255,${a})`,
      steady: `rgba(46,204,113,${a})`,
      arc: `rgba(255,140,26,${a})`,
      close: `rgba(232,91,83,${a})`,
    };
    return colors[label] || `rgba(255,255,255,${a})`;
  }

  function normalizeSegmentLabel(label) {
    const raw = String(label || "").trim().toLowerCase();
    const aliases = { startup: "start", start: "start", steady: "steady", baseline: "steady", arc: "arc", close: "close", closing: "close" };
    return aliases[raw] || "steady";
  }

  function getSegmentLoadValue() {
    return safeFilenameSegment((segLoadInput?.value || currentMeta?.load_type || parseDatasetFilenameMeta(currentMeta?.source_file || sid).loadType || "session").trim(), "session");
  }

  function getSegmentTrialValue() {
    return safePositiveInt(segTrialInput?.value || currentMeta?.trial_number || parseDatasetFilenameMeta(currentMeta?.source_file || sid).trial || 1, 1);
  }

  function applySegmentDefaults(meta = {}) {
    const parsed = parseDatasetFilenameMeta(meta?.source_file || sid || "session");
    if (segLoadInput) {
      const preferredLoad = String(meta?.load_type || "").trim();
      segLoadInput.value = safeFilenameSegment(preferredLoad && preferredLoad.toLowerCase() !== "uploaded_csv" ? preferredLoad : parsed.loadType, "session");
    }
    if (segTrialInput) segTrialInput.value = String(safePositiveInt(meta?.trial_number || parsed.trial || 1, 1));
    if (segLabelSelect) segLabelSelect.value = normalizeSegmentLabel(meta?.division_tag || parsed.division || "steady");
    if (segStartInput) segStartInput.value = "";
    if (segEndInput) segEndInput.value = "";
  }

  function sortedSegments() {
    return SEGMENTS.slice().sort((a, b) => (a.start - b.start) || (a.end - b.end) || String(a.label).localeCompare(String(b.label)));
  }

  function validateSegmentRange(startIdx, endIdx) {
    if (!ROWS.length) throw new Error("No rows available for segmentation.");
    if (!Number.isInteger(startIdx) || !Number.isInteger(endIdx)) throw new Error("Segment start/end must be whole row indices.");
    if (startIdx < 0 || endIdx < 0 || startIdx >= ROWS.length || endIdx >= ROWS.length) throw new Error(`Segment indices must stay within 0-${Math.max(0, ROWS.length - 1)}.`);
    if (endIdx < startIdx) throw new Error("Segment end must be greater than or equal to start.");
    for (const seg of SEGMENTS) {
      const overlap = !(endIdx < seg.start || startIdx > seg.end);
      if (overlap) throw new Error(`Segment overlaps existing ${segmentLabelTitle(seg.label)} range ${seg.start}-${seg.end}.`);
    }
  }

  function updateSegmentHint() {
    const hint = $("segmentHint");
    if (!hint) return;
    hint.textContent = !ROWS.length
      ? "Create non-overlapping Start / Steady / Arc / Close ranges for split CSV export."
      : `Rows 0-${Math.max(0, ROWS.length - 1)} • Segments=${SEGMENTS.length} • Download splits by load/trial/label.`;
  }

  function renderSegmentList() {
    if (!segmentList) return;
    if (!SEGMENTS.length) {
      segmentList.innerHTML = '<div class="segment-empty">No divider ranges yet. Add a non-overlapping Start / Steady / Arc / Close segment.</div>';
      updateSegmentHint();
      if (plot) plot.redraw();
      return;
    }
    const sorted = sortedSegments();
    segmentList.innerHTML = sorted.map((seg, idx) => `
      <div class="segment-row" data-segment-id="${seg.id}">
        <div class="segment-row-main">
          <span class="segment-pill segment-pill-${seg.label}">${segmentLabelTitle(seg.label)}</span>
          <span>#${idx + 1}</span>
          <span>rows ${seg.start}-${seg.end}</span>
          <span>(${Math.max(1, (seg.end - seg.start) + 1)} rows)</span>
        </div>
        <button type="button" class="btn btn-small btn-danger" data-remove-segment="${seg.id}">Remove</button>
      </div>
    `).join("");
    segmentList.querySelectorAll("[data-remove-segment]").forEach((btn) => {
      btn.addEventListener("click", () => {
        const id = btn.getAttribute("data-remove-segment");
        SEGMENTS = SEGMENTS.filter((seg) => seg.id !== id);
        renderSegmentList();
        refreshDownloadBinding();
      });
    });
    updateSegmentHint();
    if (plot) plot.redraw();
  }

  function addSegmentFromInputs() {
    const startRaw = String(segStartInput?.value || "").trim();
    const endRaw = String(segEndInput?.value || "").trim();
    const start = startRaw ? Number(startRaw) : playIdx;
    const end = endRaw ? Number(endRaw) : start;
    validateSegmentRange(start, end);
    SEGMENTS.push({ id: `seg_${Date.now()}_${Math.random().toString(36).slice(2, 8)}`, label: normalizeSegmentLabel(segLabelSelect?.value || "steady"), start, end });
    SEGMENTS = sortedSegments();
    if (segStartInput && !startRaw) segStartInput.value = String(start);
    if (segEndInput && !endRaw) segEndInput.value = String(end);
    renderSegmentList();
    refreshDownloadBinding();
    setStatus(`Added ${segmentLabelTitle(segLabelSelect?.value || "steady")} segment ${start}-${end}.`);
  }

  function removeSegmentFromInputs() {
    if (!SEGMENTS.length) {
      setStatus("No segments to remove.");
      return;
    }
    const startRaw = String(segStartInput?.value || "").trim();
    const endRaw = String(segEndInput?.value || "").trim();
    const start = startRaw ? Number(startRaw) : playIdx;
    const end = endRaw ? Number(endRaw) : start;
    const before = SEGMENTS.length;
    SEGMENTS = SEGMENTS.filter((seg) => (end < seg.start || start > seg.end));
    renderSegmentList();
    refreshDownloadBinding();
    setStatus(before !== SEGMENTS.length ? `Removed ${before - SEGMENTS.length} segment(s).` : "No segment matched that selection.");
  }

  function buildSegmentCsvFiles() {
    if (!SEGMENTS.length) return [];
    csvHeaders = ensureCsvHeadersAndLabelArc(ROWS, csvHeaders);
    const loadToken = getSegmentLoadValue();
    const trial = getSegmentTrialValue();
    const sorted = sortedSegments();
    const labelTotals = {};
    sorted.forEach((seg) => { labelTotals[seg.label] = (labelTotals[seg.label] || 0) + 1; });
    const labelSeen = {};
    return sorted.map((seg, idx) => {
      labelSeen[seg.label] = (labelSeen[seg.label] || 0) + 1;
      const rows = ROWS.slice(seg.start, seg.end + 1).map((row) => ({ ...row, load_type: loadToken, session_id: `${safeFilenameSegment(sid || 'session', 'session')}__${seg.label}_${String(idx + 1).padStart(2, '0')}` }));
      const labelTitle = segmentLabelTitle(seg.label);
      const suffix = labelTotals[seg.label] > 1 ? `_${labelSeen[seg.label]}` : "";
      const filename = safeCsvFilename(`${loadToken}_${trial}_${labelTitle}${suffix}.csv`);
      const text = Papa.unparse({ fields: csvHeaders, data: rows.map((row) => csvHeaders.map((field) => row?.[field] ?? ((field === 'label_arc') ? 0 : ""))) });
      return { filename, text };
    });
  }

  function downloadSegmentedCsvs() {
    const files = buildSegmentCsvFiles();
    if (!files.length) {
      setStatus("No segments available for split download.");
      return;
    }
    files.forEach((file, idx) => setTimeout(() => downloadTextFile(file.filename, file.text), idx * 140));
    setStatus(`Downloading ${files.length} split CSV file(s).`);
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
    const hasSegments = SEGMENTS.length > 0;
    btnDownload.textContent = hasSegments ? `Download Split (${SEGMENTS.length})` : "Download CSV";
    btnDownload.onclick = () => {
      if (hasSegments) {
        downloadSegmentedCsvs();
        return;
      }
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
    const targetLabel = indices.length === 1 ? `Row ${indices[0]}` : `${changed}/${indices.length} rows`;
    setStatus(`${targetLabel}: label_arc=${next}. Download will include the edited CSV.`);
  }

  function setArcMarkerAt(idx, on) {
    setArcMarkers([idx], on);
  }

  function setStatus(text) {
    if (!statusLine) return;
    const msg = String(text || "").trim();
    statusLine.textContent = msg;
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
    keys.sort((a, b) => {
      const ax = chunksObj[a] || {};
      const bx = chunksObj[b] || {};
      const aSeq = Number(ax.chunk_seq);
      const bSeq = Number(bx.chunk_seq);
      if (Number.isFinite(aSeq) && Number.isFinite(bSeq) && aSeq !== bSeq) return aSeq - bSeq;
      const aCreated = Number(ax.created_at || 0);
      const bCreated = Number(bx.created_at || 0);
      if (Number.isFinite(aCreated) && Number.isFinite(bCreated) && aCreated !== bCreated) return aCreated - bCreated;
      const aFirstUp = Number(ax.first_uptime_ms);
      const bFirstUp = Number(bx.first_uptime_ms);
      if (Number.isFinite(aFirstUp) && Number.isFinite(bFirstUp) && aFirstUp !== bFirstUp) return aFirstUp - bFirstUp;
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

  let TIME_AXIS_SOURCE = "index";
  let TIME_AXIS_REPAIRED_GAPS = 0;

  function medianPositiveDiff(values) {
    const diffs = [];
    for (let i = 1; i < values.length; i++) {
      const d = Number(values[i]) - Number(values[i - 1]);
      if (Number.isFinite(d) && d > 0) diffs.push(d);
    }
    if (!diffs.length) return 0;
    diffs.sort((a, b) => a - b);
    const mid = diffs.length >> 1;
    return (diffs.length & 1) ? diffs[mid] : 0.5 * (diffs[mid - 1] + diffs[mid]);
  }

  function buildContinuousTimeAxisFromField(rows, field, scale = 1.0) {
    const raw = rows.map((r) => Number(r?.[field]));
    if (!raw.length || !raw.some((v) => Number.isFinite(v) && v > 0)) return null;

    const medianStep = Math.max(1e-6, medianPositiveDiff(raw) * scale || (1 / 30));
    const gapThreshold = Math.max(medianStep * 4.0, 0.75);
    const out = new Array(rows.length).fill(0);
    TIME_AXIS_REPAIRED_GAPS = 0;

    let prevRaw = Number.isFinite(raw[0]) ? raw[0] : 0;
    for (let i = 1; i < rows.length; i++) {
      const curRaw = Number.isFinite(raw[i]) ? raw[i] : prevRaw;
      let dt = (curRaw - prevRaw) * scale;
      if (!Number.isFinite(dt) || dt <= 0) dt = medianStep;
      if (dt > gapThreshold) {
        dt = medianStep;
        TIME_AXIS_REPAIRED_GAPS++;
      }
      out[i] = out[i - 1] + dt;
      prevRaw = curRaw;
    }
    return out;
  }

  function pickTimeAxis(rows) {
    TIME_AXIS_SOURCE = "index";
    TIME_AXIS_REPAIRED_GAPS = 0;

    const fromEpoch = rows.length && rows[0].epoch_ms !== undefined
      ? buildContinuousTimeAxisFromField(rows, "epoch_ms", 0.001)
      : null;
    if (fromEpoch) {
      TIME_AXIS_SOURCE = "epoch_ms";
      return fromEpoch;
    }

    const fromUptime = rows.length && rows[0].uptime_ms !== undefined
      ? buildContinuousTimeAxisFromField(rows, "uptime_ms", 0.001)
      : null;
    if (fromUptime) {
      TIME_AXIS_SOURCE = "uptime_ms";
      return fromUptime;
    }

    return rows.map((_, i) => i / 30.0);
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
      uptime_ms: "uptime_ms",
      adc_fs_hz: "adc_fs_hz",
      feature_space_version: "feature_space_version",
      spectral_flux: "spectral_flux_midhf",
      spectral_flux_midhf: "spectral_flux_midhf",
      midhf_flux: "spectral_flux_midhf",
      residual_crest_factor: "residual_crest_factor",
      resid_crest_factor: "residual_crest_factor",
      edge_spike_ratio: "edge_spike_ratio",
      pre_dip_spike_ratio: "edge_spike_ratio",
      midband_residual_ratio: "midband_residual_ratio",
      cycle_nmse: "cycle_nmse",
      peak_fluct_cv: "peak_fluct_cv",
      thd_i: "thd_i",
      hf_energy_delta: "hf_energy_delta",
      zcv: "zcv",
      abs_irms_zscore_vs_baseline: "abs_irms_zscore_vs_baseline"
    };
    return aliases[slug] || slug || raw;
  }

  function buildSeriesKeys(rows) {
    if (!rows.length) return [];
    const keys = Object.keys(rows[0]);
    return keys.filter((k) => {
      if (k === "timestamp" || k === "session_id" || k === "load_type") return false;
      if (k === "epoch_ms" || k === "uptime_ms" || k === "feature_space_version" || k === "wpe_entropy" || k === "dip_rebound_ratio") return false;
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
      nameEl.textContent = displaySeriesName(k);

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

    const axisNote = TIME_AXIS_REPAIRED_GAPS > 0
      ? ` | Time axis=${TIME_AXIS_SOURCE} (repaired ${TIME_AXIS_REPAIRED_GAPS} gaps)`
      : ` | Time axis=${TIME_AXIS_SOURCE}`;
    const readyStatus = Object.prototype.hasOwnProperty.call(options, "readyStatus")
      ? String(options.readyStatus || "")
      : `Rows: ${ROWS.length} | Arc markers=${ARC_IDXS.length}${axisNote}`;
    setStatus(readyStatus.replace(/__ROWS__/g, String(ROWS.length)).replace(/__ARCS__/g, String(ARC_IDXS.length)));
    clearValueReadout();

    requestAnimationFrame(() => {
      buildPlot();
      if (X.length) {
        setPlayIdx(0, true);
        applyZoomPercent(100, X[0]);
      }
      updateStats();
      renderSegmentList();
      refreshDownloadBinding();
      updateArcEditButtons();
    });

    return true;
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
  let statsVisible = true;

  const SERIES_META = {
    spectral_flux_midhf: { label: "Spectral Flux (Mid/HF)", unit: "%", decimals: 2 },
    residual_crest_factor: { label: "Residual Crest Factor", unit: "dB", decimals: 2 },
    edge_spike_ratio: { label: "Edge Spike Ratio", unit: "dB", decimals: 2 },
    midband_residual_ratio: { label: "Midband Residual Ratio", unit: "dB", decimals: 2 },
    cycle_nmse: { label: "Cycle NMSE", unit: "%", decimals: 2 },
    peak_fluct_cv: { label: "Peak Fluctuation CV", unit: "%", decimals: 2 },
    thd_i: { label: "Current THD", unit: "%", decimals: 2 },
    hf_energy_delta: { label: "HF Energy Delta", unit: "dB", decimals: 2 },
    zcv: { label: "ZC Variance", unit: "ms", decimals: 3 },
    abs_irms_zscore_vs_baseline: { label: "Absolute Z-Deviation", unit: "σ", decimals: 2 },
    i_rms: { label: "Current", unit: "A", decimals: 3 },
    current: { label: "Current", unit: "A", decimals: 3 },
    v_rms: { label: "Voltage", unit: "V", decimals: 1 },
    voltage: { label: "Voltage", unit: "V", decimals: 1 },
    temp_c: { label: "Temperature", unit: "°C", decimals: 1 },
    temp: { label: "Temperature", unit: "°C", decimals: 1 },
    label_arc: { label: "Arc Label", unit: "", decimals: 0 },
    model_pred: { label: "Model Prediction", unit: "", decimals: 0 },
    feat_valid: { label: "Feature Valid", unit: "", decimals: 0 },
    current_valid: { label: "Current Valid", unit: "", decimals: 0 },
    fault_state: { label: "Fault State", unit: "", decimals: 0 },
    arc_counter: { label: "Arc Counter", unit: "", decimals: 0 },
    adc_fs_hz: { label: "ADC Rate", unit: "Hz", decimals: 1 },
    auto_capture: { label: "Auto Capture", unit: "", decimals: 0 },
  };

  const showPref = new Map();
  const axisPref = new Map();

  const DEFAULT_ON = new Set(["i_rms", "current", "label_arc", "model_pred"]);
  const DEFAULT_Y2 = new Set(["residual_crest_factor", "edge_spike_ratio", "midband_residual_ratio", "hf_energy_delta", "i_rms", "current", "v_rms", "voltage", "temp_c", "temp"]);

  setViewerOpen(false);
  ensureArcEditorControls();
  refreshDownloadBinding();
  applyStatsVisibility();


  function seriesMeta(key) {
    return SERIES_META[key] || null;
  }

  function displaySeriesName(key) {
    const meta = seriesMeta(key);
    if (!meta) return key;
    return meta.unit ? `${meta.label} [${meta.unit}]` : meta.label;
  }

  function formatSeriesValue(key, value) {
    if (value == null || Number.isNaN(Number(value))) return "—";
    const meta = seriesMeta(key);
    const n = Number(value);
    const decimals = meta?.decimals ?? 3;
    const body = Number.isInteger(n) && decimals === 0 ? String(Math.round(n)) : n.toFixed(decimals);
    return meta?.unit ? `${body} ${meta.unit}` : body;
  }

  function preferredDefaultKeys(keys) {
    const preferred = ["i_rms", "current", "label_arc", "model_pred"];
    const picked = preferred.filter((k) => keys.includes(k));
    if (picked.length) return picked;
    return keys.filter((k) => k === "label_arc" || k === "model_pred" || k === "i_rms" || k === "current").slice(0, 4);
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
    currentMeta = {};
    SEGMENTS = [];
    if (toggleList) toggleList.innerHTML = "";
    if (valueLine) valueLine.innerHTML = "";
    if (statsBody) statsBody.innerHTML = "";
    if (statsHint) statsHint.textContent = "Window statistics";
    applyStatsVisibility();
    updateArcReadout();
    setStatus("Loading…");
    if (scrub) { scrub.min = "0"; scrub.max = "0"; scrub.value = "0"; }
    if (timeReadout) timeReadout.textContent = "t=—";
    applySegmentDefaults({});
    renderSegmentList();
    refreshDownloadBinding();
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
            const { ctx } = u;
            ctx.save();
            for (const seg of sortedSegments()) {
              const x0 = X[seg.start];
              const x1 = X[seg.end];
              if (!Number.isFinite(x0) || !Number.isFinite(x1)) continue;
              if (x1 < u.scales.x.min || x0 > u.scales.x.max) continue;
              const left = Math.round(u.valToPos(x0, "x", true));
              const right = Math.round(u.valToPos(x1, "x", true));
              const width = Math.max(1, right - left);
              ctx.fillStyle = segmentColor(seg.label, 0.10);
              ctx.fillRect(left, u.bbox.top, width, u.bbox.height);
              ctx.strokeStyle = segmentColor(seg.label, 0.55);
              ctx.lineWidth = 1;
              ctx.beginPath();
              ctx.moveTo(left + 0.5, u.bbox.top);
              ctx.lineTo(left + 0.5, u.bbox.top + u.bbox.height);
              ctx.moveTo(right + 0.5, u.bbox.top);
              ctx.lineTo(right + 0.5, u.bbox.top + u.bbox.height);
              ctx.stroke();
              ctx.fillStyle = segmentColor(seg.label, 0.90);
              ctx.font = "12px system-ui";
              ctx.fillText(segmentLabelTitle(seg.label), left + 6, u.bbox.top + 14);
            }
            if (chkEvents?.checked && ARC_SERIES_INDEX >= 0 && ARC_IDXS.length) {
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
      chip.innerHTML = `<span class="vdot" style="background:${palette[i % palette.length]}"></span><span class="vk">${displaySeriesName(k)}</span><span class="vv">${formatSeriesValue(k, v)}</span>`;
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

  function applyStatsVisibility() {
    if (!statsWrap) return;
    statsWrap.style.display = statsVisible ? "block" : "none";
    btnStats?.classList.toggle("is-active", !!statsVisible);
    if (btnStats) btnStats.textContent = statsVisible ? "Hide Stats" : "Stats";
  }

  function updateStats() {
    if (!statsWrap || !statsBody || !statsHint) return;
    applyStatsVisibility();
    if (!statsVisible) return;

    if (!KEYS.length) {
      statsHint.textContent = "Window statistics";
      statsBody.innerHTML = `<tr><td colspan="6" class="mono">No dataset loaded yet.</td></tr>`;
      return;
    }

    const source = currentData() || DATA_RAW;
    if (!source || !plot) {
      statsHint.textContent = "Window statistics";
      statsBody.innerHTML = `<tr><td colspan="6" class="mono">Plot is not ready yet.</td></tr>`;
      return;
    }

    const [i0, i1] = currentXRangeIdx();
    const t0 = X[i0];
    const t1 = X[i1];
    const visibleKeys = KEYS.filter((k) => showPref.get(k));
    const selectedKeys = visibleKeys.length ? visibleKeys : preferredDefaultKeys(KEYS);
    const shownKeys = selectedKeys.slice(0, 20);

    statsHint.textContent = `Visible window • idx ${i0}-${i1} • t=${fmt(t0)}s..${fmt(t1)}s`;

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
          <td class="mono">${displaySeriesName(k)}</td>
          <td class="mono">${formatSeriesValue(k, st.mean)}</td>
          <td class="mono">${formatSeriesValue(k, st.min)}</td>
          <td class="mono">${formatSeriesValue(k, st.max)}</td>
          <td class="mono">${formatSeriesValue(k, st.std)}</td>
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

  btnStats?.addEventListener("click", () => {
    statsVisible = !statsVisible;
    applyStatsVisibility();
    if (statsVisible) updateStats();
  });

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
  btnSegStartFromPlay?.addEventListener("click", () => { if (ROWS.length) segStartInput.value = String(playIdx); });
  btnSegEndFromPlay?.addEventListener("click", () => { if (ROWS.length) segEndInput.value = String(playIdx); });
  btnAddSegment?.addEventListener("click", () => { try { addSegmentFromInputs(); } catch (err) { setStatus(err?.message || "Failed to add segment."); } });
  btnClearSegment?.addEventListener("click", () => removeSegmentFromInputs());
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
  function toggleSeries(ev) {
    ev?.preventDefault?.();
    ev?.stopPropagation?.();
    document.body.classList.toggle("showSeries");
  }
  btnSeries?.addEventListener("click", toggleSeries);
  btnCloseSeries?.addEventListener("click", (ev) => {
    ev?.preventDefault?.();
    ev?.stopPropagation?.();
    closeSeries();
  });
  document.addEventListener("click", (ev) => {
    if (!document.body.classList.contains("showSeries")) return;
    const target = ev.target;
    if (seriesPanel?.contains?.(target) || btnSeries?.contains?.(target)) return;
    closeSeries();
  });
  window.addEventListener("keydown", (e) => {
    if (e.key !== "Escape") return;
    if (document.body.classList.contains("showSeries")) {
      closeSeries();
      return;
    }
    if (!plotDrawer.classList.contains("collapsed")) closeSessionViewer();
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

    try {
      let meta = metaOverride || null;
      if (!meta) {
        const metaSnap = await db.ref(`ml_sessions/${sid}`).get();
        meta = metaSnap.exists() ? metaSnap.val() : {};
      }
      currentMeta = meta || {};
      SEGMENTS = [];
      titleEl.textContent = viewerDisplayName(meta, sid);
      metaEl.textContent = `${viewerMetaText(meta)} • ${sid}`;
      currentDownloadName = viewerDownloadName(meta, sid);
      applySegmentDefaults(currentMeta);
      renderSegmentList();
      refreshDownloadBinding();

      setStatus("Fetching CSV chunks…");
      const csv = await fetchSessionCsv(sid);
      currentCsv = csv;
      if (!csv) {
        setStatus("No CSV chunks found for this session.");
        clearValueReadout();
        return;
      }

      refreshDownloadBinding();

      setStatus("Parsing CSV…");
      const parsed = Papa.parse(csv.trim(), { header: true, dynamicTyping: true, skipEmptyLines: true, transformHeader: normalizeHeaderName });
      ingestParsedCsv(parsed, {
        downloadName: currentDownloadName,
        emptyStatus: "Parsed 0 rows. (CSV exists but has no data rows.)",
        noNumericStatus: "No numeric series found in this session CSV.",
        readyStatus: ""
      });
    } catch (e) {
      console.error(e);
      setStatus("Failed to load this session (check Firebase rules / network).");
      clearValueReadout();
    }
  }


  function openSessionViewer(targetSid, metaOverride = null) {
    announceActiveSession(targetSid);
    setViewerOpen(true);
    setTimeout(() => {
      plotDrawer?.scrollIntoView?.({ behavior: "smooth", block: "start" });
    }, 10);
    loadSession(targetSid, metaOverride);
  }

  function openSessionViewerFromCsv(name, csvText, metaOverride = null) {
    announceActiveSession("");
    setViewerOpen(true);
    setTimeout(() => {
      plotDrawer?.scrollIntoView?.({ behavior: "smooth", block: "start" });
    }, 10);

    sid = (name || "uploaded_csv").replace(/[^a-zA-Z0-9_.-]/g, "_");
    resetState();
    const meta = metaOverride || {};
    currentMeta = meta || {};
    SEGMENTS = [];
    titleEl.textContent = viewerDisplayName(meta, sid);
    metaEl.textContent = `${viewerMetaText(meta)} • Local CSV`;
    currentDownloadName = viewerDownloadName(meta, sid);
    currentCsv = csvText || "";
    applySegmentDefaults(currentMeta);
    renderSegmentList();
    refreshDownloadBinding();

    try {
      if (!currentCsv.trim()) {
        setStatus("Uploaded CSV is empty.");
        clearValueReadout();
        return;
      }

      setStatus("Parsing uploaded CSV…");
      const parsed = Papa.parse(currentCsv.trim(), { header: true, dynamicTyping: true, skipEmptyLines: true, transformHeader: normalizeHeaderName });
      ingestParsedCsv(parsed, {
        downloadName: currentDownloadName,
        emptyStatus: "Parsed 0 rows from uploaded CSV.",
        noNumericStatus: "No numeric series found in uploaded CSV.",
        readyStatus: ""
      });
    } catch (e) {
      console.error(e);
      setStatus("Failed to parse uploaded CSV.");
      clearValueReadout();
    }
  }

  function closeSessionViewer() {
    announceActiveSession("");
    closeSeries();
    resetState();
    refreshDownloadBinding();
    setViewerOpen(false);
    titleEl.textContent = "Plot Viewer";
    metaEl.textContent = "Open a session from the ML table above.";
    setStatus("");
    if (location.hash === "#plotDrawer") {
      history.replaceState(null, "", location.pathname + location.search);
    }
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