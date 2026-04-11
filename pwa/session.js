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
  const FEATURE_SCHEMA = window.TinyMLFeatureSchema || null;

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
  const btnSaveProcessed = $("btnSaveProcessed");

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
  const segFamilyInput = $("segFamilyInput");
  const segDeviceInput = $("segDeviceInput");
  const segTrialInput = $("segTrialInput");
  const segNotesInput = $("segNotesInput");
  const segTrustedNormal = $("segTrustedNormal");
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
  const chkSelectedOnly = $("chkSelectedOnly");
  const seriesCountText = $("seriesCountText");
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
  let currentDownloadName = "session.csv";
  let currentMeta = {};
  let SEGMENTS = [];
  let currentViewStateKey = "";

  const LS_VIEWER_SERIES_PREFS = "tsp_viewer_series_prefs_v1";
  const LS_VIEWER_VIEW_PREFS = "tsp_viewer_view_prefs_v3";
  const VIEWER_STATE_BY_KEY = new Map();

  function loadPersistedSeriesPrefs() {
    const out = { show: new Map(), axis: new Map() };
    try {
      const raw = localStorage.getItem(LS_VIEWER_SERIES_PREFS);
      if (!raw) return out;
      const parsed = JSON.parse(raw);
      const showObj = parsed && typeof parsed.show === "object" ? parsed.show : {};
      const axisObj = parsed && typeof parsed.axis === "object" ? parsed.axis : {};
      Object.entries(showObj).forEach(([key, value]) => {
        if (!key) return;
        out.show.set(String(key), !!value);
      });
      Object.entries(axisObj).forEach(([key, value]) => {
        if (!key) return;
        out.axis.set(String(key), String(value || "y").toLowerCase() === "y2" ? "y2" : "y");
      });
    } catch (err) {
      console.warn("Failed to load persisted series prefs", err);
    }
    return out;
  }

  function persistSeriesPrefs() {
    try {
      const show = {};
      const axis = {};
      showPref.forEach((value, key) => {
        if (!key) return;
        show[String(key)] = !!value;
      });
      axisPref.forEach((value, key) => {
        if (!key) return;
        axis[String(key)] = String(value || "y").toLowerCase() === "y2" ? "y2" : "y";
      });
      localStorage.setItem(LS_VIEWER_SERIES_PREFS, JSON.stringify({ show, axis }));
    } catch (err) {
      console.warn("Failed to persist series prefs", err);
    }
  }

  function loadPersistedViewPrefs() {
    try {
      const raw = localStorage.getItem(LS_VIEWER_VIEW_PREFS);
      if (!raw) return null;
      return cloneViewerState(JSON.parse(raw));
    } catch (err) {
      console.warn("Failed to load persisted viewer prefs", err);
      return null;
    }
  }

  function persistViewPrefs(state) {
    const normalized = cloneViewerState(state);
    if (!normalized) return null;
    try {
      localStorage.setItem(LS_VIEWER_VIEW_PREFS, JSON.stringify(normalized));
    } catch (err) {
      console.warn("Failed to persist viewer prefs", err);
    }
    return normalized;
  }

  function cloneViewerState(state) {
    if (!state || typeof state !== "object") return null;
    const xWindow = state.xWindow && Number.isFinite(state.xWindow.min) && Number.isFinite(state.xWindow.max) && state.xWindow.max > state.xWindow.min
      ? { min: Number(state.xWindow.min), max: Number(state.xWindow.max) }
      : null;
    const xWindowFrac = state.xWindowFrac
      && Number.isFinite(state.xWindowFrac.min)
      && Number.isFinite(state.xWindowFrac.max)
      && state.xWindowFrac.max > state.xWindowFrac.min
      ? {
          min: Math.max(0, Math.min(1, Number(state.xWindowFrac.min))),
          max: Math.max(0, Math.min(1, Number(state.xWindowFrac.max))),
        }
      : null;
    return {
      playIdx: Math.max(0, Number(state.playIdx) | 0),
      xWindow,
      xWindowFrac,
      zoomPct: Math.max(5, Math.min(100, Number(state.zoomPct || 100) || 100)),
      centerX: Number.isFinite(Number(state.centerX)) ? Number(state.centerX) : null,
      centerFrac: Number.isFinite(Number(state.centerFrac)) ? Math.max(0, Math.min(1, Number(state.centerFrac))) : null,
      activeSeriesKey: String(state.activeSeriesKey || ""),
    };
  }

  function viewerStateKeyFor(sessionId = sid, meta = currentMeta, fallbackName = currentDownloadName) {
    const sidToken = String(sessionId || "").trim();
    if (sidToken) return `sid:${sidToken}`;
    const nameToken = safeFilenameSegment(meta?.source_file || fallbackName || meta?.device_name || "local_csv", "local_csv");
    return `csv:${nameToken}`;
  }

  function setCurrentViewerStateKey(nextKey) {
    currentViewStateKey = String(nextKey || "").trim();
    return currentViewStateKey;
  }

  function getRememberedViewerState(key = currentViewStateKey) {
    const targetKey = String(key || "").trim();
    const hit = targetKey ? VIEWER_STATE_BY_KEY.get(targetKey) : null;
    return cloneViewerState(hit) || loadPersistedViewPrefs();
  }

  function rememberViewerState(state, key = currentViewStateKey) {
    const normalized = persistViewPrefs(state);
    const targetKey = String(key || "").trim();
    if (!normalized) return null;
    if (targetKey) VIEWER_STATE_BY_KEY.set(targetKey, normalized);
    return normalized;
  }

  function rememberCurrentViewerState() {
    if (!currentViewStateKey || !X.length) return null;
    return rememberViewerState(captureViewerState(), currentViewStateKey);
  }

  function resolveViewerState(preferredState = null) {
    return cloneViewerState(preferredState) || getRememberedViewerState(currentViewStateKey) || null;
  }

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


  function normalizeDeviceFamilyToken(v, fallback = "unknown") {
    const raw = String(v || "").trim().toLowerCase().replace(/[^a-z0-9]+/g, "_").replace(/^_+|_+$/g, "");
    const alias = { resistive: "resistive_linear", resistive_linear: "resistive_linear", heater: "resistive_linear", heating: "resistive_linear", inductive: "inductive_motor", motor: "inductive_motor", fan: "inductive_motor", inductive_motor: "inductive_motor", smps: "rectifier_smps", rectifier: "rectifier_smps", rectifier_smps: "rectifier_smps", charger: "rectifier_smps", adapter: "rectifier_smps", dimmer: "phase_angle_controlled", phase: "phase_angle_controlled", dimmer_phase: "phase_angle_controlled", phase_angle: "phase_angle_controlled", phase_angle_controlled: "phase_angle_controlled", universal: "brush_universal_motor", universal_motor: "brush_universal_motor", brush: "brush_universal_motor", brush_universal_motor: "brush_universal_motor", vacuum: "brush_universal_motor", mixed: "other_mixed", mixed_unknown: "other_mixed", other: "other_mixed", other_mixed: "other_mixed", unknown: "unknown" };
    return alias[raw] || fallback;
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

  function deviceFamilyCodeFromToken(v) {
    const token = normalizeDeviceFamilyToken(v, "unknown");
    return ({ unknown: -1, resistive_linear: 0, inductive_motor: 1, rectifier_smps: 2, phase_angle_controlled: 3, brush_universal_motor: 4, other_mixed: 5 })[token] ?? -1;
  }

  function familyTitle(v) { return titleizeTokenText(String(v || "").replace(/_/g, " "), "Mixed Unknown"); }
  function deviceTitle(v) { return titleizeTokenText(v, "Unknown Device"); }

  function pickViewerFamily(meta) {
    return normalizeDeviceFamilyToken(meta?.device_family || meta?.load_family || meta?.parsed_load_family || "unknown");
  }

  function pickViewerDevice(meta, fallback = "unknown_device") {
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

  function mergeRowMetadata(row, meta) {
    row.device_family = normalizeDeviceFamilyToken(meta?.device_family || row?.device_family || "unknown");
    row.device_family_code = deviceFamilyCodeFromToken(row.device_family);
    row.device_name = normalizeDeviceNameToken(meta?.device_name || row?.device_name || row?.load_type || "unknown_device", "unknown_device");
    row.trial_number = Math.max(1, parseInt(meta?.trial_number || row?.trial_number || 1, 10) || 1);
    row.division_tag = normalizeDivisionTagToken(meta?.division_tag || row?.division_tag || "steady");
    row.notes = normalizeNotesText(meta?.notes || row?.notes || "");
    row.trusted_normal_session = meta?.trusted_normal_session ? 1 : (Number(row?.trusted_normal_session || 0) > 0 ? 1 : 0);
    row.load_type = row.device_name;
    return row;
  }

  function deriveRowTimingWindow(rows) {
    const numericSeries = (field) => rows.map((row) => Number(row?.[field])).filter((v) => Number.isFinite(v));
    const epochVals = numericSeries("epoch_ms");
    if (epochVals.length >= 2) {
      const startMs = epochVals[0];
      const endMs = epochVals[epochVals.length - 1];
      if (endMs > startMs) return { durationS: (endMs - startMs) / 1000, startMs, endMs, sourceSampleRateHz: null };
    }
    const frameSpan = buildContinuousSecondsFromField(rows, "frame_start_uptime_ms", 0.001) || buildContinuousSecondsFromField(rows, "frame_end_uptime_ms", 0.001) || buildContinuousSecondsFromField(rows, "uptime_ms", 0.001);
    const sourceFs = rows.map((row) => Number(row?.source_sample_rate_hz ?? row?.feature_frame_rate_hz ?? row?.adc_fs_hz)).find((v) => Number.isFinite(v) && v > 0) || null;
    if (frameSpan != null) return { durationS: frameSpan, startMs: null, endMs: null, sourceSampleRateHz: sourceFs };
    if (sourceFs && rows.length > 1) return { durationS: Math.max(0, (rows.length - 1) / sourceFs), startMs: null, endMs: null, sourceSampleRateHz: sourceFs };
    if (sourceFs && rows.length > 0) return { durationS: rows.length / sourceFs, startMs: null, endMs: null, sourceSampleRateHz: sourceFs };
    return { durationS: null, startMs: null, endMs: null, sourceSampleRateHz: sourceFs };
  }

  function canonicalDatasetStem(name, fallback = "session") {
    let stem = String(name || "").replace(/\.[a-z0-9]+$/i, "").trim();
    if (!stem) return fallback;
    stem = stem.replace(/^TSP_ML_/i, "");
    stem = stem.replace(/^upload_\d+_/i, "");
    stem = stem.replace(/^processed_\d+_/i, "");
    stem = stem.replace(/_(AUTO|ARC|NORMAL)_[0-9]{4}-[0-9]{2}-[0-9]{2}[-_0-9]*$/i, "");
    stem = stem.replace(/__[a-z]+_\d+$/i, "");
    stem = stem.replace(/(?:_|-)(?:19|20)\d{2}(?:[_-]?\d{2}){2}(?:[_-]?\d{2}){1,3}$/i, "");
    stem = stem.replace(/(?:_|-)processed$/i, "");
    stem = stem.replace(/_+/g, "_").replace(/^_+|_+$/g, "");
    return stem || fallback;
  }

  function isUploadedCsvSession(meta) {
    return !!(meta?.uploaded_csv || String(meta?.load_type || "").trim().toLowerCase() === "uploaded_csv");
  }

  function viewerDisplayName(meta, sessionId) {
    const device = deviceTitle(pickViewerDevice(meta, sessionId || "session"));
    const trial = Math.max(1, parseInt(meta?.trial_number || 1, 10) || 1);
    const division = titleizeTokenText(normalizeDivisionTagToken(meta?.division_tag || "steady"), "Steady");
    return `${device} - Trial ${trial} - ${division}`;
  }

  function viewerDownloadName(meta, sessionId) {
    return buildStructuredDatasetFilename(meta || {}, safeFilenameSegment(sessionId || 'session', 'session'));
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
    let out = String(name || `${sid || "session"}.csv`).trim();
    if (!out.toLowerCase().endsWith(".csv")) out += ".csv";
    return out.replace(/[\/:*?"<>|]+/g, "_");
  }

  function viewerDurationSeconds(meta) {
    const st = Number(meta?.start_ms || 0);
    const en = Number(meta?.end_ms || 0);
    if (st > 0 && en > st) return (en - st) / 1000;
    const explicit = Number(meta?.source_duration_s ?? meta?.duration_s);
    if (Number.isFinite(explicit) && explicit > 0) return explicit;
    const rows = Number(meta?.row_count || 0);
    const sourceFs = Number(meta?.source_sample_rate_hz || 0);
    if (rows > 1 && Number.isFinite(sourceFs) && sourceFs > 0) return (rows - 1) / sourceFs;
    if (rows > 0 && Number.isFinite(sourceFs) && sourceFs > 0) return rows / sourceFs;
    return null;
  }

  function deriveCsvTimingMetaLocal(csvText) {
    const clean = String(csvText || "").replace(/^﻿/, "").trim();
    if (!clean) return { durationS: null, rowCount: 0, sourceSampleRateHz: null, startMs: null, endMs: null };
    const parsed = Papa.parse(clean, { header: true, dynamicTyping: true, skipEmptyLines: true, transformHeader: normalizeHeaderName });
    const rows = (parsed?.data || []).filter((row) => row && Object.keys(row).length);
    if (!rows.length) return { durationS: null, rowCount: 0, sourceSampleRateHz: null, startMs: null, endMs: null };
    const timing = deriveRowTimingWindow(rows);
    return { durationS: timing.durationS, rowCount: rows.length, sourceSampleRateHz: timing.sourceSampleRateHz, startMs: timing.startMs, endMs: timing.endMs };
  }

  function viewerMetaText(meta) {
    const dur = viewerDurationSeconds(meta);
    const durText = (dur === null) ? "—" : (Math.abs(dur - Math.round(dur)) < 0.05 ? `${Math.round(dur)}s` : `${dur.toFixed(1)}s`);
    const startSourceMs = Number(meta?.first_epoch_ms || meta?.start_ms || 0);
    const dateText = startSourceMs > 0
      ? new Intl.DateTimeFormat("en-US", { timeZone: "Asia/Manila", month: "long", day: "2-digit", year: "numeric" }).format(new Date(startSourceMs))
      : "—";
    const family = familyTitle(pickViewerFamily(meta));
    return `${family} • ${durText} • ${dateText}`;
  }

  function parseDatasetFilenameMeta(name) {
    const stem = String(name || "").replace(/\.[a-z0-9]+$/i, "").trim();
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
    const device = safeFilenameSegment(parts.slice(0, Math.max(0, endIdx)).join("_"), "session");
    return { deviceFamily: "unknown", deviceName: device, loadType: device, trial, division };
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
    };
    return colors[label] || `rgba(255,255,255,${a})`;
  }

  function normalizeSegmentLabel(label) {
    const raw = String(label || "").trim().toLowerCase();
    const aliases = { startup: "start", start: "start", steady: "steady", baseline: "steady", arc: "arc", close: "steady", closing: "steady", end: "steady", ending: "steady" };
    return aliases[raw] || "steady";
  }

  function getSegmentFamilyValue() {
    return normalizeDeviceFamilyToken(segFamilyInput?.value || currentMeta?.device_family || parseDatasetFilenameMeta(currentMeta?.source_file || sid).deviceFamily || "unknown");
  }

  function getSegmentDeviceValue() {
    return normalizeDeviceNameToken(segDeviceInput?.value || currentMeta?.device_name || currentMeta?.load_type || parseDatasetFilenameMeta(currentMeta?.source_file || sid).deviceName || "session", "session");
  }

  function getSegmentTrialValue() {
    return safePositiveInt(segTrialInput?.value || currentMeta?.trial_number || parseDatasetFilenameMeta(currentMeta?.source_file || sid).trial || 1, 1);
  }

  function applySegmentDefaults(meta = {}) {
    const parsed = parseDatasetFilenameMeta(meta?.source_file || sid || "session");
    if (segFamilyInput) segFamilyInput.value = normalizeDeviceFamilyToken(meta?.device_family || parsed.deviceFamily || "unknown");
    if (segDeviceInput) segDeviceInput.value = normalizeDeviceNameToken(meta?.device_name || meta?.load_type || parsed.deviceName || "session", "session");
    if (segTrialInput) segTrialInput.value = String(safePositiveInt(meta?.trial_number || parsed.trial || 1, 1));
    if (segLabelSelect) segLabelSelect.value = normalizeSegmentLabel(meta?.division_tag || parsed.division || "steady");
    if (segNotesInput) segNotesInput.value = normalizeNotesText(meta?.notes || "");
    if (segTrustedNormal) segTrustedNormal.checked = Number(meta?.trusted_normal_session || 0) > 0;
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
      ? "Create Start / Steady / Arc / Close ranges."
      : `Rows 0-${Math.max(0, ROWS.length - 1)} • Segments ${SEGMENTS.length} • Split export ready.`;
  }

  function renderSegmentList() {
    if (!segmentList) return;
    if (!SEGMENTS.length) {
      segmentList.innerHTML = '<div class="segment-empty">No ranges yet. Add a Start / Steady / Arc / Close segment.</div>';
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
    const familyToken = getSegmentFamilyValue();
    const deviceToken = getSegmentDeviceValue();
    const trial = getSegmentTrialValue();
    const notes = normalizeNotesText(segNotesInput?.value || currentMeta?.notes || "");
    const trustedNormal = !!(segTrustedNormal?.checked || Number(currentMeta?.trusted_normal_session || 0) > 0);
    const sorted = sortedSegments();
    const labelTotals = {};
    sorted.forEach((seg) => { labelTotals[seg.label] = (labelTotals[seg.label] || 0) + 1; });
    const labelSeen = {};
    return sorted.map((seg, idx) => {
      labelSeen[seg.label] = (labelSeen[seg.label] || 0) + 1;
      const rows = ROWS.slice(seg.start, seg.end + 1).map((row) => mergeRowMetadata({ ...row, session_id: `${safeFilenameSegment(sid || 'session', 'session')}__${seg.label}_${String(idx + 1).padStart(2, '0')}` }, { device_family: familyToken, device_name: deviceToken, trial_number: trial, division_tag: seg.label, notes, trusted_normal_session: trustedNormal ? 1 : 0 }));
      const labelTitle = segmentLabelTitle(seg.label);
      const suffix = labelTotals[seg.label] > 1 ? `_${labelSeen[seg.label]}` : "";
      const filename = safeCsvFilename(buildStructuredDatasetFilename({ device_family: familyToken, device_name: deviceToken, trial_number: trial, division_tag: seg.label }, `${deviceToken}_${suffix || (idx + 1)}`));
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
    ["label_arc", "device_family", "device_family_code", "device_name", "trial_number", "division_tag", "notes", "trusted_normal_session", "load_type", "context_family_code_runtime", "context_family_code_provisional", "context_family_confidence", "context_family_confidence_provisional", "context_acquiring", "context_latched"].forEach((name) => { if (!headers.includes(name)) headers.push(name); });

    rows.forEach((row) => {
      if (!row || typeof row !== "object") return;
      row.label_arc = normalizeBinaryLabel(row.label_arc);
      mergeRowMetadata(row, currentMeta || {});
      if (!("context_family_code_runtime" in row)) row.context_family_code_runtime = row.device_family_code;
      if (!("context_family_code_provisional" in row)) row.context_family_code_provisional = row.context_family_code_runtime;
      if (!("context_family_confidence" in row)) row.context_family_confidence = row.device_family_code >= 0 ? 1 : 0;
      if (!("context_family_confidence_provisional" in row)) row.context_family_confidence_provisional = row.context_family_confidence;
      if (!("context_acquiring" in row)) row.context_acquiring = 0;
      if (!("context_latched" in row)) row.context_latched = 0;
      headers.forEach((key) => {
        if (!(key in row)) row[key] = (key === "label_arc") ? 0 : "";
      });
    });

    return orderCsvHeaders(headers);
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
      downloadTextFile(currentDownloadName || safeCsvFilename(`${sid || "session"}.csv`), csv);
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
      btnAddArc.textContent = isArc ? "Added" : "Add";
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
          btnAddArc.textContent = "Add";
          host.appendChild(btnAddArc);
        }
        if (!btnClearArc) {
          btnClearArc = document.createElement("button");
          btnClearArc.type = "button";
          btnClearArc.id = "btnClearArc";
          btnClearArc.className = "btn btn-small btn-danger";
          btnClearArc.textContent = "Remove";
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

    const viewState = {
      playIdx: Math.max(0, Math.min(X.length - 1, indices[0])),
      xWindow: plot ? { min: plot.scales.x.min, max: plot.scales.x.max } : null,
    };

    if (!refreshDerivedData(false)) return;

    rememberViewerState(viewState);
    buildPlot(viewState);

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

    const fromFrameStart = rows.length && rows[0].frame_start_uptime_ms !== undefined
      ? buildContinuousTimeAxisFromField(rows, "frame_start_uptime_ms", 0.001)
      : null;
    if (fromFrameStart) {
      TIME_AXIS_SOURCE = "frame_start_uptime_ms";
      return fromFrameStart;
    }

    const fromUptime = rows.length && rows[0].uptime_ms !== undefined
      ? buildContinuousTimeAxisFromField(rows, "uptime_ms", 0.001)
      : null;
    if (fromUptime) {
      TIME_AXIS_SOURCE = "uptime_ms";
      return fromUptime;
    }

    TIME_AXIS_SOURCE = "row_index";
    return rows.map((_, i) => i);
  }

  function normalizeHeaderName(name) {
    const schemaKey = FEATURE_SCHEMA?.normalizeFeatureKey?.(name);
    if (schemaKey) return schemaKey;
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
      abs_irms_zscore_vs_baseline: "abs_irms_zscore_vs_baseline",
      delta_irms_abs: "delta_irms_abs",
      halfcycle_asymmetry: "halfcycle_asymmetry",
      suspicious_run_energy: "suspicious_run_energy",
      delta_hf_energy: "delta_hf_energy",
      delta_flux: "delta_flux",
      v_sag_pct: "v_sag_pct"
    };
    return aliases[slug] || slug || raw;
  }

const PREFERRED_EXPORT_HEADER_ORDER = FEATURE_SCHEMA?.csvHeaderOrder || [
  "epoch_ms",
  "uptime_ms",
  "frame_start_uptime_ms",
  "frame_end_uptime_ms",
  "feature_compute_end_uptime_ms",
  "log_enqueue_uptime_ms",
  "frame_dt_ms",
  "compute_time_ms",
  "timing_skew_ms",
  "v_rms",
  "voltage",
  "i_rms",
  "current",
  "temp_c",
  "temperature",

  "abs_irms_zscore_vs_baseline",
  "delta_irms_abs",
  "halfcycle_asymmetry",
  "cycle_nmse",
  "v_sag_pct",
  "suspicious_run_energy",
  "delta_hf_energy",
  "delta_flux",
  "midband_residual_ratio",
  "zcv",
  "spectral_flux_midhf",
  "peak_fluct_cv",
  "residual_crest_factor",
  "thd_i",
  "hf_energy_delta",
  "edge_spike_ratio",

  "model_pred",
  "label_arc",
  "feat_valid",
  "current_valid",
  "sampling_quality_bad",
  "invalid_loaded_flag",
  "invalid_off_flag",
  "relay_blank_active",
  "turnon_blank_active",
  "transient_blank_active",
  "suspicious_run_len",
  "invalid_loaded_run_len",
  "restrike_count_short",

  "device_family",
  "device_family_code",
  "device_name",
  "trial_number",
  "division_tag",
  "notes",
  "trusted_normal_session",
  "load_type",
  "session_id",
  "context_family_code_runtime",
  "context_family_code_provisional",
  "context_family_confidence",
  "context_family_confidence_provisional",
  "context_acquiring",
  "context_latched",
  "adc_fs_hz",
  "feature_space_version"
];

function orderCsvHeaders(headers) {
  const seen = new Set();
  const ordered = [];
  const push = (name) => {
    const key = String(name || "").trim();
    if (!key || seen.has(key) || !headers.includes(key)) return;
    seen.add(key);
    ordered.push(key);
  };
  PREFERRED_EXPORT_HEADER_ORDER.forEach(push);
  headers.forEach(push);
  return ordered;
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

  function updateSeriesListMeta() {
    if (!seriesCountText) return;
    const total = KEYS.length;
    const selected = KEYS.filter((k) => !!showPref.get(k)).length;
    const mode = chkSelectedOnly?.checked ? "shown" : "all";
    seriesCountText.textContent = `${selected}/${total} ${mode}`;
  }


  function setActiveSeriesKey(nextKey = "") {
    const normalized = String(nextKey || "").trim();
    const keepWindow = captureXWindow();
    const keepIdx = playIdx;
    const resume = playing;
    if (resume) pause();

    const nextActiveKey = KEYS.includes(normalized) ? normalized : "";
    const wasHidden = !!nextActiveKey && !showPref.get(nextActiveKey);
    activeSeriesKey = nextActiveKey;
    if (activeSeriesKey && wasHidden) showPref.set(activeSeriesKey, true);

    const row = activeSeriesKey ? toggleList?.querySelector?.(`.row[data-key="${activeSeriesKey}"]`) : null;
    const cb = row?.querySelector?.('input[type="checkbox"]');
    if (cb && activeSeriesKey) cb.checked = true;
    row?.classList?.add("is-selected");

    const viewState = captureViewerState() || {};
    viewState.playIdx = keepIdx;
    viewState.activeSeriesKey = activeSeriesKey;
    if (keepWindow) {
      viewState.xWindow = { ...keepWindow };
      viewState.xWindowFrac = null;
      viewState.zoomPct = Math.max(5, Math.min(100, Math.round(((keepWindow.max - keepWindow.min) / Math.max(1e-9, FULL_X_MAX - FULL_X_MIN)) * 100)));
      viewState.centerX = (keepWindow.min + keepWindow.max) * 0.5;
      viewState.centerFrac = Number.isFinite(viewState.centerX) && FULL_X_MAX > FULL_X_MIN
        ? Math.max(0, Math.min(1, (viewState.centerX - FULL_X_MIN) / (FULL_X_MAX - FULL_X_MIN)))
        : null;
    }

    rememberViewerState(viewState);

    if (plot) {
      if (wasHidden) {
        const idx = KEYS.indexOf(activeSeriesKey);
        if (idx >= 0) {
          plot.setSeries(idx + 1, { show: true }, false);
          try { plot.setData(currentData(), false); } catch (_) {}
        }
      }
      syncActiveSeriesStyles();
      if (keepWindow) restoreXWindow(keepWindow);
      setPlayIdx(keepIdx, true, true);
    } else {
      buildPlot(viewState);
      if (keepWindow) restoreXWindow(keepWindow);
      setPlayIdx(keepIdx, true, true);
    }

    toggleList?.querySelectorAll?.(".row")?.forEach?.((node) => {
      node.classList.toggle("is-focused", !!activeSeriesKey && node.dataset.key === activeSeriesKey);
    });
    if (resume) play();
    rememberCurrentViewerState();
    applySeriesFilter();
    updateValueReadout();
    updateStats();
  }

  function renderToggleList() {
    if (!toggleList) return;
    toggleList.innerHTML = "";

    KEYS.forEach((k, i) => {
      const wrap = document.createElement("div");
      wrap.className = "row";
      wrap.dataset.key = k;
      wrap.dataset.search = `${k} ${displaySeriesName(k)}`.toLowerCase();
      wrap.classList.toggle("is-selected", !!showPref.get(k));
      wrap.classList.toggle("is-focused", activeSeriesKey === k);

      const cb = document.createElement("input");
      cb.type = "checkbox";
      cb.checked = !!showPref.get(k);
      cb.onchange = () => {
        const on = !!cb.checked;
        const viewState = captureViewerState();
        showPref.set(k, on);
        persistSeriesPrefs();
        wrap.classList.toggle("is-selected", on);
        if (activeSeriesKey === k && !on) activeSeriesKey = "";
        if (plot) {
          plot.setSeries(i + 1, { show: on });
          plot.setData(plot.data, false);
          restoreViewerState(viewState);
        }
        applySeriesFilter();
        updateSeriesListMeta();
        updateValueReadout();
        updateStats();
      };

      const nameWrap = document.createElement("div");
      nameWrap.className = "seriesNameWrap";
      const nameEl = document.createElement("div");
      nameEl.className = "seriesName";
      nameEl.textContent = displaySeriesName(k);
      const keyEl = document.createElement("div");
      keyEl.className = "seriesKey mono";
      keyEl.textContent = k;
      nameWrap.appendChild(nameEl);
      nameWrap.appendChild(keyEl);

      const sel = document.createElement("select");
      sel.className = "axisSel";
      sel.innerHTML = `<option value="y">Y1</option><option value="y2">Y2</option>`;
      sel.value = axisPref.get(k) || "y";
      sel.onchange = () => {
        axisPref.set(k, sel.value);
        persistSeriesPrefs();
        rebuildForDataMode();
      };

      wrap.addEventListener("click", (ev) => {
        if (ev.target === cb || ev.target === sel || ev.target.closest("select") || ev.target.closest("input")) return;
        setActiveSeriesKey(activeSeriesKey === k ? "" : k);
      });

      wrap.appendChild(cb);
      wrap.appendChild(nameWrap);
      wrap.appendChild(sel);
      toggleList.appendChild(wrap);
    });

    applySeriesFilter();
    updateSeriesListMeta();
  }

  function refreshDerivedData(resetPrefs = false) {
    if (!ROWS.length) return false;

    csvHeaders = ensureCsvHeadersAndLabelArc(ROWS, csvHeaders);
    X = pickTimeAxis(ROWS);
    KEYS = buildSeriesKeys(ROWS);

    if (!KEYS.length) return false;

    DATA_RAW = makeData(ROWS, X, KEYS);

    const win0 = Number(rngSmooth?.value) || 15;
    DATA_SMOOTH = makeSmoothedData(DATA_RAW, win0);
    DATA_NORM = normalizeData(DATA_RAW);
    DATA_SMOOTH_NORM = normalizeData(DATA_SMOOTH);

    buildArcIndexes();

    const persistedPrefs = loadPersistedSeriesPrefs();
    const defaults = new Set(preferredDefaultKeys(KEYS));
    KEYS.forEach((k) => {
      if (resetPrefs) {
        showPref.set(k, defaults.has(k));
        axisPref.set(k, DEFAULT_Y2.has(k) ? "y2" : "y");
        return;
      }
      if (!showPref.has(k)) {
        showPref.set(k, persistedPrefs.show.has(k) ? !!persistedPrefs.show.get(k) : defaults.has(k));
      }
      if (!axisPref.has(k)) {
        axisPref.set(k, persistedPrefs.axis.has(k) ? (persistedPrefs.axis.get(k) || "y") : (DEFAULT_Y2.has(k) ? "y2" : "y"));
      }
    });

    persistSeriesPrefs();

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

    const timing = deriveRowTimingWindow(ROWS);
    currentMeta = {
      ...(currentMeta || {}),
      row_count: ROWS.length,
      source_duration_s: (Number.isFinite(timing?.durationS) && timing.durationS > 0) ? timing.durationS : (currentMeta?.source_duration_s || currentMeta?.duration_s || null),
      source_sample_rate_hz: (Number.isFinite(timing?.sourceSampleRateHz) && timing.sourceSampleRateHz > 0) ? timing.sourceSampleRateHz : (currentMeta?.source_sample_rate_hz || null),
      first_epoch_ms: (Number.isFinite(timing?.startMs) && timing.startMs > 0) ? timing.startMs : (currentMeta?.first_epoch_ms || currentMeta?.start_ms || null),
      last_epoch_ms: (Number.isFinite(timing?.endMs) && timing.endMs > 0) ? timing.endMs : (currentMeta?.last_epoch_ms || currentMeta?.end_ms || null),
    };
    if (metaEl) metaEl.textContent = viewerMetaText(currentMeta);

    csvHeaders = ensureCsvHeadersAndLabelArc(ROWS, parsed?.meta?.fields || []);
    currentDownloadName = safeCsvFilename(options.downloadName || `${sid || "session"}.csv`);

    if (!refreshDerivedData(false)) {
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

    const initialViewState = cloneViewerState(PENDING_OPEN_VIEW_STATE) || resolveViewerState();
    PENDING_OPEN_VIEW_STATE = null;
    buildPlotAfterLayout(initialViewState);

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
    "#2ecc71", "#e85b53", "#4da3ff", "#f1c40f",
    "#9b6dff", "#19c6b3", "#ff9b42", "#d7dde8",
    "#6ad0ff", "#ff6fae", "#c7d84f", "#ffbf5a",
  ];


  function withAlpha(hex, alpha) {
    const raw = String(hex || "").trim();
    const a = Math.max(0, Math.min(1, Number(alpha)));
    const short = raw.match(/^#([0-9a-f]{3})$/i);
    const full = raw.match(/^#([0-9a-f]{6})$/i);
    let value = "";
    if (short) value = short[1].split("").map((ch) => ch + ch).join("");
    else if (full) value = full[1];
    if (!value) return raw || `rgba(255,255,255,${a})`;
    const r = parseInt(value.slice(0, 2), 16);
    const g = parseInt(value.slice(2, 4), 16);
    const b = parseInt(value.slice(4, 6), 16);
    return `rgba(${r}, ${g}, ${b}, ${a})`;
  }

  function seriesStrokeColor(index, isDimmed) {
    const base = palette[index % palette.length];
    return isDimmed ? withAlpha(base, 0.18) : base;
  }

  function isViewerLaidOut() {
    const chart = $("chart");
    if (!plotDrawer || !chart || plotDrawer.hidden) return false;
    const drawerRect = plotDrawer.getBoundingClientRect();
    const chartRect = chart.getBoundingClientRect();
    return drawerRect.width > 0 && drawerRect.height > 0 && chartRect.width >= 240 && chartRect.height >= 220;
  }

  function runAfterViewerLayout(task, maxFrames = 32) {
    let frames = 0;
    let stable = 0;
    let lastW = 0;
    let lastH = 0;
    const tick = () => {
      const chart = $("chart");
      const rect = chart?.getBoundingClientRect?.() || { width: 0, height: 0 };
      const w = Math.round(rect.width || 0);
      const h = Math.round(rect.height || 0);
      if (isViewerLaidOut()) {
        stable = (Math.abs(w - lastW) <= 1 && Math.abs(h - lastH) <= 1) ? (stable + 1) : 0;
        lastW = w;
        lastH = h;
        if (stable >= 2 || frames >= maxFrames) {
          task();
          return;
        }
      }
      frames += 1;
      if (frames >= maxFrames) {
        task();
        return;
      }
      requestAnimationFrame(tick);
    };
    requestAnimationFrame(tick);
  }

  function openViewerThen(task) {
    setViewerOpen(true);
    plotDrawer?.offsetHeight;
    $("chart")?.offsetWidth;
    $("chart")?.offsetHeight;
    setTimeout(() => {
      plotDrawer?.scrollIntoView?.({ behavior: "smooth", block: "start" });
    }, 10);
    runAfterViewerLayout(task);
  }

  function buildPlotAfterLayout(initialViewState = null) {
    runAfterViewerLayout(() => {
      buildPlot(initialViewState);
      updateStats();
      renderSegmentList();
      refreshDownloadBinding();
      updateArcEditButtons();
    });
  }

  let plot = null;
  let playing = false;
  let playIdxF = 0;
  let playIdx = 0;
  let lastRAF = 0;
  let speed = 1.0;
  let activeSeriesKey = "";

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
  let statsVisible = false;

  const SERIES_META = FEATURE_SCHEMA?.featureMetaByKey || {
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
    fs_err_hz: { label: "ADC Rate Error", unit: "Hz", decimals: 1 },
    frame_start_uptime_ms: { label: "Frame Start", unit: "ms", decimals: 0 },
    frame_end_uptime_ms: { label: "Frame End", unit: "ms", decimals: 0 },
    frame_dt_ms: { label: "Frame Dt", unit: "ms", decimals: 2 },
    compute_time_ms: { label: "Compute Time", unit: "ms", decimals: 2 },
    queue_drop_count: { label: "Queue Drops", unit: "", decimals: 0 },
    sampling_quality_bad: { label: "Sampling Quality Bad", unit: "", decimals: 0 },
    invalid_loaded_flag: { label: "Invalid Loaded", unit: "", decimals: 0 },
    invalid_off_flag: { label: "Invalid Off", unit: "", decimals: 0 },
    relay_blank_active: { label: "Relay Blank", unit: "", decimals: 0 },
    turnon_blank_active: { label: "Turn-on Blank", unit: "", decimals: 0 },
    transient_blank_active: { label: "Transient Blank", unit: "", decimals: 0 },
    suspicious_run_len: { label: "Suspicious Run Len", unit: "", decimals: 0 },
    suspicious_run_energy: { label: "Suspicious Run Energy", unit: "", decimals: 3 },
    invalid_loaded_run_len: { label: "Invalid Loaded Run", unit: "", decimals: 0 },
    delta_irms_abs: { label: "|Δ Irms|", unit: "A", decimals: 4 },
    delta_hf_energy: { label: "|Δ HF|", unit: "dB", decimals: 4 },
    delta_flux: { label: "|Δ Flux|", unit: "", decimals: 4 },
    v_sag_pct: { label: "Voltage Sag", unit: "%", decimals: 3 },
    restrike_count_short: { label: "Restrikes Short", unit: "", decimals: 0 },
    halfcycle_asymmetry: { label: "Half-cycle Asym", unit: "%", decimals: 3 },
    auto_capture: { label: "Auto Capture", unit: "", decimals: 0 },
    device_family_code: { label: "Device Family Code", unit: "", decimals: 0 },
    context_family_code_runtime: { label: "Context Family Runtime", unit: "", decimals: 0 },
    context_family_code_provisional: { label: "Context Family Provisional", unit: "", decimals: 0 },
    context_family_confidence: { label: "Context Confidence", unit: "", decimals: 3 },
    context_family_confidence_provisional: { label: "Provisional Context Confidence", unit: "", decimals: 3 },
    context_acquiring: { label: "Context Acquiring", unit: "", decimals: 0 },
    context_latched: { label: "Context Latched", unit: "", decimals: 0 },
  };

  const showPref = new Map();
  const axisPref = new Map();
  {
    const persisted = loadPersistedSeriesPrefs();
    persisted.show.forEach((v, k) => showPref.set(k, !!v));
    persisted.axis.forEach((v, k) => axisPref.set(k, String(v || "y")));
  }

  const DEFAULT_ON = new Set(["voltage", "v_rms", "current", "i_rms"]);
  const DEFAULT_Y2 = new Set(["residual_crest_factor", "edge_spike_ratio", "midband_residual_ratio", "hf_energy_delta", "i_rms", "current", "temp_c", "temp"]);

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
    const voltageKey = ["voltage", "v_rms"].find((k) => keys.includes(k));
    const currentKey = ["current", "i_rms"].find((k) => keys.includes(k));
    const picked = [voltageKey, currentKey].filter(Boolean);
    if (picked.length) return picked;
    return keys.filter((k) => k === "voltage" || k === "v_rms" || k === "current" || k === "i_rms").slice(0, 2);
  }

  function resetState(options = {}) {
    const preserveViewKey = !!options.preserveViewKey;
    VIEW_RESTORE_TOKEN += 1;
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
    activeSeriesKey = "";
    currentCsv = "";
    csvHeaders = [];
    LAST_X_WINDOW = null;
    LAST_ZOOM_PCT = 100;
    currentDownloadName = "session.csv";
    currentMeta = {};
    SEGMENTS = [];
    if (!preserveViewKey) currentViewStateKey = "";
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
    LAST_ZOOM_PCT = pct;
    rngZoom.value = String(pct);
  }

  let LAST_X_WINDOW = null;
  let LAST_ZOOM_PCT = 100;
  let VIEW_RESTORE_TOKEN = 0;
  let PENDING_OPEN_VIEW_STATE = null;

  function makeCarryoverOpenState(state) {
    const base = cloneViewerState(state) || captureViewerState() || null;
    if (!base) return null;
    return {
      playIdx: 0,
      xWindow: null,
      xWindowFrac: null,
      zoomPct: Math.max(5, Math.min(100, Number(base.zoomPct || LAST_ZOOM_PCT || 100) || 100)),
      centerX: null,
      centerFrac: null,
      activeSeriesKey: String(base.activeSeriesKey || activeSeriesKey || ""),
    };
  }

  function captureXWindow() {
    const min = plot?.scales?.x?.min;
    const max = plot?.scales?.x?.max;
    const out = Number.isFinite(min) && Number.isFinite(max) && max > min
      ? { min, max }
      : (LAST_X_WINDOW && Number.isFinite(LAST_X_WINDOW.min) && Number.isFinite(LAST_X_WINDOW.max) && LAST_X_WINDOW.max > LAST_X_WINDOW.min
          ? { ...LAST_X_WINDOW }
          : null);
    if (out) LAST_X_WINDOW = { ...out };
    return out;
  }

  function currentViewFractions(windowState) {
    const full = Math.max(1e-9, FULL_X_MAX - FULL_X_MIN);
    const win = windowState && Number.isFinite(windowState.min) && Number.isFinite(windowState.max) && windowState.max > windowState.min
      ? windowState
      : captureXWindow();
    if (!win || full <= 0) return null;
    return {
      min: Math.max(0, Math.min(1, (win.min - FULL_X_MIN) / full)),
      max: Math.max(0, Math.min(1, (win.max - FULL_X_MIN) / full)),
    };
  }

  function resolveWindowFromState(state) {
    if (!state) return null;
    if (state.xWindowFrac && Number.isFinite(state.xWindowFrac.min) && Number.isFinite(state.xWindowFrac.max) && state.xWindowFrac.max > state.xWindowFrac.min && FULL_X_MAX > FULL_X_MIN) {
      const full = FULL_X_MAX - FULL_X_MIN;
      return {
        min: FULL_X_MIN + (full * state.xWindowFrac.min),
        max: FULL_X_MIN + (full * state.xWindowFrac.max),
      };
    }
    if (state.xWindow && Number.isFinite(state.xWindow.min) && Number.isFinite(state.xWindow.max) && state.xWindow.max > state.xWindow.min) {
      return state.xWindow;
    }
    return null;
  }

  function restoreXWindow(windowState) {
    if (!plot || !windowState) return;
    const [min, max] = clampXWindow(windowState.min, windowState.max);
    LAST_X_WINDOW = { min, max };
    plot.setScale("x", { min, max });
  }

  function captureViewerState() {
    const xWindow = captureXWindow();
    const xWindowFrac = currentViewFractions(xWindow);
    const fallbackCenterX = X.length ? X[Math.max(0, Math.min(X.length - 1, playIdx | 0))] : null;
    const centerX = xWindow ? (xWindow.min + xWindow.max) * 0.5 : fallbackCenterX;
    const full = Math.max(1e-9, FULL_X_MAX - FULL_X_MIN);
    const zoomPctFromScale = xWindow ? Math.max(5, Math.min(100, Math.round(((xWindow.max - xWindow.min) / full) * 100))) : (Number(rngZoom?.value || LAST_ZOOM_PCT || 100) || 100);
    return {
      playIdx: Math.max(0, Math.min(X.length ? X.length - 1 : 0, playIdx | 0)),
      xWindow,
      xWindowFrac,
      zoomPct: zoomPctFromScale,
      centerX,
      centerFrac: Number.isFinite(centerX) && FULL_X_MAX > FULL_X_MIN
        ? Math.max(0, Math.min(1, (centerX - FULL_X_MIN) / (FULL_X_MAX - FULL_X_MIN)))
        : null,
      activeSeriesKey,
    };
  }

  function scheduleDeferredViewerRestore(state, frames = 3) {
    const snapshot = cloneViewerState(state);
    if (!snapshot || !X.length) return;
    const token = ++VIEW_RESTORE_TOKEN;
    const rerun = (remaining) => {
      requestAnimationFrame(() => {
        if (token != VIEW_RESTORE_TOKEN || !plot || !X.length) return;
        restoreViewerState(snapshot);
        if (remaining > 1) rerun(remaining - 1);
      });
    };
    rerun(Math.max(1, frames | 0));
  }

  function restoreViewerState(state) {
    if (!X.length) return;
    const normalizedState = cloneViewerState(state) || resolveViewerState(null);
    const nextPlayIdx = Math.max(0, Math.min(X.length ? X.length - 1 : 0, (normalizedState?.playIdx ?? 0) | 0));
    const resolvedWindow = resolveWindowFromState(normalizedState);
    if (resolvedWindow) {
      restoreXWindow(resolvedWindow);
    } else if (plot) {
      const pct = Number(normalizedState?.zoomPct || 100) || 100;
      const centerFromFrac = Number.isFinite(normalizedState?.centerFrac) && FULL_X_MAX > FULL_X_MIN
        ? FULL_X_MIN + ((FULL_X_MAX - FULL_X_MIN) * normalizedState.centerFrac)
        : null;
      if (pct >= 99.5) {
        LAST_ZOOM_PCT = 100;
        LAST_X_WINDOW = { min: FULL_X_MIN, max: FULL_X_MAX };
        plot.setScale("x", { min: FULL_X_MIN, max: FULL_X_MAX });
      } else {
        applyZoomPercent(
          pct,
          Number.isFinite(normalizedState?.centerX)
            ? normalizedState.centerX
            : (Number.isFinite(centerFromFrac) ? centerFromFrac : X[nextPlayIdx])
        );
      }
    }
    activeSeriesKey = KEYS.includes(String(normalizedState?.activeSeriesKey || "")) ? String(normalizedState.activeSeriesKey) : activeSeriesKey;
    setPlayIdx(nextPlayIdx, true, true);
    syncZoomSlider();
    rememberCurrentViewerState();
  }

  function applyZoomPercent(pct, centerX = null) {
    if (!plot || !X.length) return;
    LAST_ZOOM_PCT = Math.max(5, Math.min(100, Number(pct || 100) || 100));
    const full = Math.max(1e-9, FULL_X_MAX - FULL_X_MIN);
    const ratio = Math.max(0.05, Math.min(1.0, LAST_ZOOM_PCT / 100.0));
    const width = full * ratio;
    const anchorX = Number.isFinite(centerX) ? centerX : X[playIdx];
    const [min, max] = clampXWindow(anchorX - (width * 0.5), anchorX + (width * 0.5));
    plot.setScale("x", { min, max });
    syncZoomSlider();
    rememberCurrentViewerState();
  }

  function resetZoom() {
    if (!plot || !X.length) return;
    const keepIdx = playIdx;
    LAST_ZOOM_PCT = 100;
    LAST_X_WINDOW = { min: FULL_X_MIN, max: FULL_X_MAX };
    plot.setScale("x", { min: FULL_X_MIN, max: FULL_X_MAX });
    if (rngZoom) rngZoom.value = "100";
    setPlayIdx(keepIdx, true, true);
    syncZoomSlider();
    rememberCurrentViewerState();
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

  function focusGlowPlugin() {
    return {
      hooks: {
        draw: [
          () => {
            // Disabled on purpose.
            // The manual overlay path does not match spline/curve rendering exactly,
            // which caused a false second line and stale highlight artifacts.
          }
        ]
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


  function syncActiveSeriesStyles() {
    if (!plot || !KEYS.length) return;
    for (let i = 0; i < KEYS.length; i++) {
      const k = KEYS[i];
      const isFocused = !!activeSeriesKey && activeSeriesKey === k;
      const isDimmed = !!activeSeriesKey && activeSeriesKey !== k;
      plot.setSeries(i + 1, {
        stroke: seriesStrokeColor(i, isDimmed),
        width: isFocused ? 2.15 : 2,
        dash: [],
      }, false);
    }
    plot.redraw();
  }

  function scheduleViewerPrime(state = null) {
    const snapshot = cloneViewerState(state) || captureViewerState() || resolveViewerState();
    const token = ++VIEW_RESTORE_TOKEN;

    const primeOnce = () => {
      if (token !== VIEW_RESTORE_TOKEN || !plot) return;
      const { w, h } = chartSize();
      if (w > 0 && h > 0) plot.setSize({ width: w, height: h });
      try {
        plot.setData(currentData(), false);
      } catch (_) {}
      if (snapshot) restoreViewerState(snapshot);
      syncActiveSeriesStyles();
      plot.redraw();
      updateStats();
    };

    const kick = (remaining) => {
      requestAnimationFrame(() => {
        primeOnce();
        if (remaining > 1) kick(remaining - 1);
      });
    };

    kick(5);
    setTimeout(primeOnce, 40);
    setTimeout(primeOnce, 120);
    setTimeout(primeOnce, 240);
  }

  function buildPlot(preservedState = null) {
    if (!DATA_RAW || !X.length) return;
    if (visibleSeriesCount() === 0) setStatus("No series selected. Check at least one series.");

    const data = currentData();
    const normalize = chkNormalize.checked;
    const useCurves = !!chkCurves?.checked;
    const splinePaths = (useCurves && uPlot?.paths?.spline) ? uPlot.paths.spline({}) : null;
    const { w, h } = chartSize();
    const viewState = resolveViewerState(preservedState);
    const requestedZoomPct = Number(viewState?.zoomPct || rngZoom?.value || LAST_ZOOM_PCT || 100) || 100;
    const requestedWindow = resolveWindowFromState(viewState);

    const series = [{ label: "t(s)" }];
    KEYS.forEach((k, i) => {
      const show = showPref.get(k) ?? false;
      const scale = normalize ? "y" : (axisPref.get(k) ?? "y");
      const isFocused = !!activeSeriesKey && activeSeriesKey === k;
      const isDimmed = !!activeSeriesKey && activeSeriesKey !== k;
      const strokeBase = seriesStrokeColor(i, isDimmed);
      series.push({
        label: k,
        show,
        stroke: strokeBase,
        width: isFocused ? 2.15 : 2,
        dash: [],
        scale,
        ...(splinePaths ? { paths: splinePaths } : {}),
      });
    });

    const axisStroke = "rgba(255,255,255,0.75)";
    const showY2 = !normalize && hasVisibleY2();

    const axes = [
      { stroke: axisStroke, grid: { show: false }, size: 60 },
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
          if (plot?.scales?.x && Number.isFinite(plot.scales.x.min) && Number.isFinite(plot.scales.x.max) && plot.scales.x.max > plot.scales.x.min) {
            LAST_X_WINDOW = { min: plot.scales.x.min, max: plot.scales.x.max };
          }
          syncZoomSlider();
          rememberCurrentViewerState();
          updateStats();
        }],
      },
      plugins: [zoomPanPlugin(), eventsPlugin(), statsPlugin(), playheadPlugin()],
    };

    if (plot) plot.destroy();
    plot = new uPlot(opts, data, $("chart"));

    FULL_X_MIN = X[0];
    FULL_X_MAX = X[X.length - 1];

    if (KEYS.includes(String(viewState?.activeSeriesKey || ""))) activeSeriesKey = String(viewState.activeSeriesKey || "");
    if (btnResetZoom) btnResetZoom.onclick = () => resetZoom();

    const nextPlayIdx = Math.min(viewState?.playIdx ?? playIdx, X.length - 1);
    setPlayIdx(nextPlayIdx, false, true);

    if (requestedWindow) {
      restoreXWindow(requestedWindow);
    } else {
      const centerFromFrac = Number.isFinite(viewState?.centerFrac) && FULL_X_MAX > FULL_X_MIN
        ? FULL_X_MIN + ((FULL_X_MAX - FULL_X_MIN) * viewState.centerFrac)
        : null;
      applyZoomPercent(
        requestedZoomPct,
        Number.isFinite(viewState?.centerX)
          ? viewState.centerX
          : (Number.isFinite(centerFromFrac) ? centerFromFrac : (X[nextPlayIdx] ?? X[0]))
      );
    }

    setPlayIdx(nextPlayIdx, true, true);
    syncZoomSlider();
    rememberCurrentViewerState();
    updateValueReadout();
    updateArcReadout();
    updateStats();

    requestAnimationFrame(() => {
      if (!plot) return;
      const { w: nextW, h: nextH } = chartSize();
      plot.setSize({ width: nextW, height: nextH });
      try {
        plot.setData(currentData(), false);
      } catch (_) {}
      if (requestedWindow) restoreXWindow(requestedWindow);
      syncActiveSeriesStyles();
      syncZoomSlider();
      updateStats();
      plot.redraw();
    });

    scheduleViewerPrime(viewState);
  }

  function setPlayIdx(idx, updateCursor = false, preserveWindow = false) {
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

    if (plot && !preserveWindow && (chkFollow?.checked || playing)) {
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
    if (plot && !playing) rememberCurrentViewerState();
  }

  function clearValueReadout() {
    if (valueLine) valueLine.innerHTML = "";
  }

  function updateValueReadout() {
    const source = currentData() || DATA_RAW;
    if (!valueLine || !source || !KEYS.length) return;
    valueLine.innerHTML = "";

    const meta = document.createElement("div");
    meta.className = "vchip";
    meta.innerHTML = `<span class="vdot" style="background:rgba(255,255,255,0.35)"></span><span class="vk">idx ${playIdx}/${X.length - 1}</span><span class="vv">t ${fmt(X[playIdx])}s</span>`;
    valueLine.appendChild(meta);

    const maxChips = 20;
    let shown = 0;

    for (let i = 0; i < KEYS.length; i++) {
      const k = KEYS[i];
      if (!showPref.get(k)) continue;
      if (shown >= maxChips) break;

      const v = source[i + 1]?.[playIdx];
      const chip = document.createElement("button");
      chip.type = "button";
      chip.className = `vchip vchip-series${activeSeriesKey === k ? " is-focused" : ""}`;
      chip.dataset.key = k;
      chip.innerHTML = `<span class="vdot" style="background:${palette[i % palette.length]}"></span><span class="vk">${displaySeriesName(k)}</span><span class="vv">${formatSeriesValue(k, v)}</span>`;
      chip.addEventListener("click", () => setActiveSeriesKey(activeSeriesKey === k ? "" : k));
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
    const keepWindow = captureXWindow();
    const resume = playing;
    const keepActiveSeriesKey = activeSeriesKey;
    const viewState = captureViewerState() || {};
    if (resume) pause();

    viewState.playIdx = keep;
    viewState.activeSeriesKey = keepActiveSeriesKey;
    if (keepWindow) {
      viewState.xWindow = { ...keepWindow };
      viewState.xWindowFrac = null;
      viewState.zoomPct = Math.max(5, Math.min(100, Math.round(((keepWindow.max - keepWindow.min) / Math.max(1e-9, FULL_X_MAX - FULL_X_MIN)) * 100)));
      viewState.centerX = (keepWindow.min + keepWindow.max) * 0.5;
      viewState.centerFrac = Number.isFinite(viewState.centerX) && FULL_X_MAX > FULL_X_MIN
        ? Math.max(0, Math.min(1, (viewState.centerX - FULL_X_MIN) / (FULL_X_MAX - FULL_X_MIN)))
        : null;
    }

    rememberViewerState(viewState);
    buildPlot(viewState);
    setPlayIdx(keep, true, true);
    if (keepWindow) restoreXWindow(keepWindow);
    if (resume) play();
    rememberCurrentViewerState();
    updateValueReadout();
    updateStats();
  }

  chkNormalize.onchange = rebuildForDataMode;
  chkSmooth.onchange = rebuildForDataMode;
  chkCurves.onchange = rebuildForDataMode;
  chkEvents.onchange = () => { if (plot) { plot.redraw(); updateStats(); } };
  const applyZoomFromSlider = () => {
    if (!plot || !rngZoom) return;
    applyZoomPercent(Number(rngZoom.value) || 100);
  };
  rngZoom?.addEventListener("input", applyZoomFromSlider);
  rngZoom?.addEventListener("change", applyZoomFromSlider);

  rngSmooth.oninput = () => {
    const win = Number(rngSmooth.value) || 1;
    DATA_SMOOTH = makeSmoothedData(DATA_RAW, win);
    DATA_SMOOTH_NORM = normalizeData(DATA_SMOOTH);
    rebuildForDataMode();
  };

  function applySeriesFilter() {
    const q = (seriesSearch?.value || "").trim().toLowerCase();
    const selectedOnly = !!chkSelectedOnly?.checked;
    const rows = toggleList?.querySelectorAll?.(".row") || [];
    rows.forEach((r) => {
      const haystack = r.dataset.search || r.dataset.key || "";
      const selected = !!showPref.get(r.dataset.key || "");
      const matchesQuery = !q || haystack.includes(q);
      const matchesSelected = !selectedOnly || selected;
      const ok = matchesQuery && matchesSelected;
      r.style.display = ok ? "" : "none";
      r.classList.toggle("is-dim", !selected);
    });
    updateSeriesListMeta();
  }
  seriesSearch?.addEventListener("input", applySeriesFilter);
  chkSelectedOnly?.addEventListener("change", applySeriesFilter);

  function setAllSeries(on) {
    const viewState = captureViewerState();
    KEYS.forEach((k, i) => {
      showPref.set(k, !!on);
      const row = toggleList?.querySelector?.(`.row[data-key="${k}"]`);
      const cb = row?.querySelector?.('input[type="checkbox"]');
      if (cb) cb.checked = !!on;
      row?.classList.toggle("is-selected", !!on);
      if (plot) plot.setSeries(i + 1, { show: !!on });
    });
    if (plot) {
      plot.setData(plot.data, false);
      restoreViewerState(viewState);
    }
    persistSeriesPrefs();
    applySeriesFilter();
    updateSeriesListMeta();
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
        row.classList.toggle("is-selected", on);
      }

      if (plot) plot.setSeries(i + 1, { show: on });
    });
    persistSeriesPrefs();
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
      const viewState = captureViewerState();
      const { w, h } = chartSize();
      plot.setSize({ width: w, height: h });
      restoreViewerState(viewState);
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
    rememberCurrentViewerState();
    sid = targetSid || "";
    setCurrentViewerStateKey(viewerStateKeyFor(sid, metaOverride || {}, targetSid || "session"));
    resetState({ preserveViewKey: true });

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
      metaEl.textContent = viewerMetaText(meta);
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
    const carryState = makeCarryoverOpenState(captureViewerState());
    rememberCurrentViewerState();
    announceActiveSession(targetSid);
    closeSeries();
    PENDING_OPEN_VIEW_STATE = carryState;
    openViewerThen(() => loadSession(targetSid, metaOverride));
  }

  function openSessionViewerFromCsv(name, csvText, metaOverride = null) {
    const carryState = makeCarryoverOpenState(captureViewerState());
    rememberCurrentViewerState();
    announceActiveSession("");
    closeSeries();
    PENDING_OPEN_VIEW_STATE = carryState;

    openViewerThen(() => {
      sid = (name || "uploaded_csv").replace(/[^a-zA-Z0-9_.-]/g, "_");
      setCurrentViewerStateKey(viewerStateKeyFor("", metaOverride || {}, sid || "uploaded_csv"));
      resetState({ preserveViewKey: true });
      const meta = metaOverride || {};
      currentMeta = meta || {};
      SEGMENTS = [];
      titleEl.textContent = viewerDisplayName(meta, sid);
      metaEl.textContent = viewerMetaText(meta);
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
    });
  }


  async function saveProcessedSession() {
    if (!ROWS.length) {
      setStatus("No processed rows to save.");
      return;
    }
    if (typeof window.storeCsvSession !== "function") {
      setStatus("Processed save is not ready yet.");
      return;
    }

    const sourceSid = String(sid || "").trim();
    const sourceFile = String(currentMeta?.source_file || currentDownloadName || sourceSid || "processed_csv").trim();
    const baseMeta = {
      source_session_id: sourceSid || "local_csv",
      uploaded_csv: false,
      processed_csv: true,
      start_ms: Number(currentMeta?.start_ms || 0) || Date.now(),
      end_ms: Number(currentMeta?.end_ms || 0) || null,
      source_sample_rate_hz: Number(currentMeta?.source_sample_rate_hz || 0) || null,
      device_family: normalizeDeviceFamilyToken(segFamilyInput?.value || currentMeta?.device_family || "unknown"),
      device_family_code: deviceFamilyCodeFromToken(segFamilyInput?.value || currentMeta?.device_family || "unknown"),
      device_name: normalizeDeviceNameToken(segDeviceInput?.value || currentMeta?.device_name || currentMeta?.load_type || "processed_csv", "processed_csv"),
      trial_number: Math.max(1, parseInt(segTrialInput?.value || currentMeta?.trial_number || 1, 10) || 1),
      division_tag: normalizeDivisionTagToken(segLabelSelect?.value || currentMeta?.division_tag || "steady"),
      notes: normalizeNotesText(segNotesInput?.value || currentMeta?.notes || ""),
      trusted_normal_session: segTrustedNormal?.checked ? 1 : (Number(currentMeta?.trusted_normal_session || 0) > 0 ? 1 : 0),
    };

    const button = btnSaveProcessed;
    const oldText = button?.textContent || "Save Processed";
    if (button) {
      button.disabled = true;
      button.textContent = "Saving...";
    }

    try {
      const savedItems = [];
      const segmentFiles = buildSegmentCsvFiles();

      if (segmentFiles.length) {
        for (const file of segmentFiles) {
          const parsedName = parseDatasetFilenameMeta(file.filename);
          const parsed = Papa.parse(file.text.trim(), { header: true, dynamicTyping: true, skipEmptyLines: true, transformHeader: normalizeHeaderName });
          const segRows = (parsed?.data || []).filter((row) => row && Object.keys(row).length);
          const segMeta = {
            ...baseMeta,
            source_file: file.filename,
            load_type: parsedName.deviceName || parsedName.loadType || String(currentMeta?.device_name || currentMeta?.load_type || "processed_csv").trim() || "processed_csv",
            device_family: parsedName.deviceFamily || baseMeta.device_family,
            device_family_code: deviceFamilyCodeFromToken(parsedName.deviceFamily || baseMeta.device_family),
            device_name: parsedName.deviceName || baseMeta.device_name,
            duration_s: viewerDurationSeconds({
              row_count: segRows.length,
              source_sample_rate_hz: Number(currentMeta?.source_sample_rate_hz || 0) || null
            }),
            row_count: segRows.length,
            trial_number: parsedName.trial || baseMeta.trial_number,
            division_tag: String(parsedName.division || baseMeta.division_tag || "").trim(),
            notes: baseMeta.notes,
            trusted_normal_session: baseMeta.trusted_normal_session,
          };
          const timing = (deriveCsvTimingMetaLocal(file.text));
          if (timing && Number.isFinite(timing.durationS) && timing.durationS > 0) {
            segMeta.duration_s = timing.durationS;
            segMeta.source_duration_s = timing.durationS;
          }
          if (timing && Number.isFinite(timing.startMs) && timing.startMs > 0) segMeta.start_ms = timing.startMs;
          if (timing && Number.isFinite(timing.endMs) && timing.endMs > Number(segMeta.start_ms || 0)) segMeta.end_ms = timing.endMs;
          if (timing && Number.isFinite(timing.sourceSampleRateHz) && timing.sourceSampleRateHz > 0) segMeta.source_sample_rate_hz = timing.sourceSampleRateHz;
          const stored = await window.storeCsvSession(file.filename, file.text, "processed", segMeta);
          savedItems.push(stored);
        }
        setStatus(`Saved ${savedItems.length} processed split CSVs.`);
        if (window.toast) window.toast(`Saved ${savedItems.length} processed split CSVs.`, "ok");
      } else {
        const csv = buildCsvFromRows();
        if (!csv) {
          setStatus("No processed CSV to save.");
          return;
        }
        const timing = (deriveCsvTimingMetaLocal(csv));
        const metaPatch = {
          ...baseMeta,
          source_file: sourceFile,
          load_type: normalizeDeviceNameToken(segDeviceInput?.value || currentMeta?.device_name || currentMeta?.load_type || "processed_csv", "processed_csv"),
          device_family: normalizeDeviceFamilyToken(segFamilyInput?.value || currentMeta?.device_family || "unknown"),
      device_family_code: deviceFamilyCodeFromToken(segFamilyInput?.value || currentMeta?.device_family || "unknown"),
          device_name: normalizeDeviceNameToken(segDeviceInput?.value || currentMeta?.device_name || currentMeta?.load_type || "processed_csv", "processed_csv"),
          duration_s: timing?.durationS ?? viewerDurationSeconds(currentMeta),
          source_duration_s: timing?.durationS ?? viewerDurationSeconds(currentMeta),
          row_count: ROWS.length,
          trial_number: Math.max(1, parseInt(segTrialInput?.value || currentMeta?.trial_number || 1, 10) || 1),
          division_tag: normalizeDivisionTagToken(segLabelSelect?.value || currentMeta?.division_tag || "steady"),
          notes: normalizeNotesText(segNotesInput?.value || currentMeta?.notes || ""),
          trusted_normal_session: segTrustedNormal?.checked ? 1 : (Number(currentMeta?.trusted_normal_session || 0) > 0 ? 1 : 0),
        };
        const stored = await window.storeCsvSession(sourceFile, csv, "processed", metaPatch);
        setStatus(`Processed CSV saved as ${stored.sid}.`);
        if (window.toast) window.toast("Processed CSV saved.", "ok");
      }
    } catch (err) {
      console.error(err);
      setStatus("Failed to save processed CSV.");
      if (window.toast) window.toast("Failed to save processed CSV.", "err");
    } finally {
      if (button) {
        button.disabled = false;
        button.textContent = oldText;
      }
    }
  }

  function closeSessionViewer() {
    rememberCurrentViewerState();
    announceActiveSession("");
    closeSeries();
    resetState();
    refreshDownloadBinding();
    PENDING_OPEN_VIEW_STATE = null;
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
  if (btnSaveProcessed) {
    btnSaveProcessed.type = "button";
    btnSaveProcessed.addEventListener("click", async (ev) => {
      ev.preventDefault();
      ev.stopPropagation();
      await saveProcessedSession();
    });
  }
  refreshDownloadBinding();

  window.openSessionViewer = openSessionViewer;
  window.openSessionViewerFromCsv = openSessionViewerFromCsv;
  window.closeSessionViewer = closeSessionViewer;

  const sidFromQuery = new URLSearchParams(location.search).get("sid") || "";
  if (sidFromQuery) openSessionViewer(sidFromQuery);
})();
