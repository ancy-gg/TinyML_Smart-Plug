import json
import os
import queue
import subprocess
import sys
import threading
import time
from pathlib import Path
import tkinter as tk
from tkinter import filedialog, messagebox, ttk
try:
    import pandas as pd
except Exception:
    pd = None
try:
    from trainer.tinyml_common import (
        ALL_COMPUTED_FEATURES,
        ARC_SWEEP_FEATURES,
        CONTEXT_SWEEP_FEATURES,
    )
except Exception:
    ALL_COMPUTED_FEATURES = [
        "abs_irms_zscore_vs_baseline",
        "delta_irms_abs",
        "halfcycle_asymmetry",
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
        "v_sag_pct",
        "cycle_nmse",
    ]
    ARC_SWEEP_FEATURES = [
        "thd_i",
        "spectral_flux_midhf",
        "hf_energy_delta",
        "residual_crest_factor",
        "peak_fluct_cv",
        "zcv",
        "cycle_nmse",
        "delta_hf_energy",
        "delta_flux",
        "delta_irms_abs",
        "midband_residual_ratio",
        "edge_spike_ratio",
    ]
    CONTEXT_SWEEP_FEATURES = [
        "delta_irms_abs",
        "halfcycle_asymmetry",
        "suspicious_run_energy",
        "hf_energy_delta",
        "midband_residual_ratio",
        "abs_irms_zscore_vs_baseline",
        "zcv",
        "delta_flux",
        "residual_crest_factor",
        "peak_fluct_cv",
    ]
SUBSET_SWEEP_SEARCH_DEPTH = 24
APP_BG = "#26282d"
CARD_BG = "#2f3339"
CARD_BG_2 = "#343941"
TEXT = "#eee7d6"
MUTED = "#c8b989"
ACCENT = "#c7b27a"
BORDER = "#434851"
LOG_BG = "#22252a"
DISPLAY_METRICS = [
    ("Model", "model_name"),
    ("Configured n_iter", ("settings", "n_iter")),
    ("Winner mode", ("settings", "winner_mode")),
    ("CV average precision", "cv_best_average_precision"),
    ("Test average precision", "test_average_precision"),
    ("Test ROC AUC", "test_roc_auc"),
    ("Test accuracy", "test_accuracy"),
    ("Test balanced accuracy", "test_balanced_accuracy"),
    ("Test precision", "test_precision"),
    ("Test recall", "test_recall"),
    ("Test F1", "test_f1"),
    ("Test specificity", "test_specificity"),
    ("Test NPV", "test_npv"),
    ("Test FPR", "test_fpr"),
    ("Test FNR", "test_fnr"),
    ("Test MCC", "test_mcc"),
    ("Threshold", "threshold"),
    ("Threshold source", "threshold_source"),
    ("Threshold constraints met", "threshold_constraints_met"),
    ("Estimated node count", "estimated_node_count"),
    ("Validation cost", ("validation_threshold_result", "cost")),
    ("Validation precision", "validation_precision"),
    ("Validation recall", ("validation_threshold_result", "recall")),
    ("Holdout validation precision", "holdout_validation_precision"),
    ("Holdout validation recall", "holdout_validation_recall"),
    ("Validation TN", ("validation_threshold_result", "tn")),
    ("Validation FP", ("validation_threshold_result", "fp")),
    ("Validation FN", ("validation_threshold_result", "fn")),
    ("Validation TP", ("validation_threshold_result", "tp")),
    ("Test TN", ("test_confusion_matrix", "tn")),
    ("Test FP", ("test_confusion_matrix", "fp")),
    ("Test FN", ("test_confusion_matrix", "fn")),
    ("Test TP", ("test_confusion_matrix", "tp")),
    ("Train rows", ("split_sizes", "train_rows")),
    ("Validation rows", ("split_sizes", "validation_rows")),
    ("Test rows", ("split_sizes", "test_rows")),
    ("Train positives", ("split_sizes", "train_positive")),
    ("Validation positives", ("split_sizes", "validation_positive")),
    ("Test positives", ("split_sizes", "test_positive")),
    ("Mean score", ("test_score_summary", "mean_score")),
    ("Median score", ("test_score_summary", "median_score")),
    ("Min score", ("test_score_summary", "min_score")),
    ("Max score", ("test_score_summary", "max_score")),
    ("Mean score (pos)", ("test_score_summary", "mean_score_pos")),
    ("Mean score (neg)", ("test_score_summary", "mean_score_neg")),
]
class TinyMLTrainerGUI(tk.Tk):
    def __init__(self):
        super().__init__()
        self.title("TinyML Trainer")
        self.geometry("1480x920")
        self.minsize(980, 680)
        self.resizable(True, True)
        self.configure(bg=APP_BG)
        self.proc = None
        self.log_queue = queue.Queue()
        self.paths_visible = False
        default_project_root = Path(__file__).resolve().parent.parent
        self.project_root = tk.StringVar(value=str(default_project_root))
        self.python_exe = tk.StringVar(value=sys.executable)
        self.prepare_script = tk.StringVar(value="tinyml/trainer/prepare_data.py")
        self.rf_script = tk.StringVar(value="tinyml/trainer/train_rf.py")
        self.et_script = tk.StringVar(value="tinyml/trainer/train_et.py")
        self.duel_script = tk.StringVar(value="tinyml/trainer/train_duel.py")
        self.context_script = tk.StringVar(value="tinyml/trainer/train_context.py")
        self.subset_script = tk.StringVar(value="tinyml/trainer/train_feature_subsets.py")
        self.cleaned_csv = tk.StringVar(value="tinyml/data/arc_training.csv")
        self.context_csv = tk.StringVar(value="tinyml/data/load_context.csv")
        self.rf_report = tk.StringVar(value="tinyml/benchmark/TinyMLTreeEnsemble_RF_report.json")
        self.et_report = tk.StringVar(value="tinyml/benchmark/TinyMLTreeEnsemble_ET_report.json")
        self.duel_report = tk.StringVar(value="tinyml/benchmark/benchmark_report.json")
        self.context_report = tk.StringVar(value="tinyml/benchmark/TinyMLContextModel_report.json")
        self.prepare_report = tk.StringVar(value="tinyml/benchmark/prepare_data_report.json")
        self.subset_report = tk.StringVar(value="tinyml/benchmark/TinyMLFeatureSubsetSweep_report.json")
        self.subset_csv = tk.StringVar(value="tinyml/benchmark/TinyMLFeatureSubsetSweep_results.csv")
        self.context_subset_report = tk.StringVar(value="tinyml/benchmark/TinyMLContextFeatureSubsetSweep_report.json")
        self.context_subset_csv = tk.StringVar(value="tinyml/benchmark/TinyMLContextFeatureSubsetSweep_results.csv")
        self.subset_task = tk.StringVar(value="Arc + Context")
        self.subset_feature_min = tk.StringVar(value="1")
        self.subset_feature_max = tk.StringVar(value=str(len(ARC_SWEEP_FEATURES)))
        self.subset_max_combinations = tk.StringVar(value="0")
        self.arc_subset_feature_min = tk.StringVar(value="1")
        self.arc_subset_feature_max = tk.StringVar(value=str(len(ARC_SWEEP_FEATURES)))
        self.arc_subset_max_combinations = tk.StringVar(value="0")
        self.context_subset_feature_min = tk.StringVar(value="1")
        self.context_subset_feature_max = tk.StringVar(value=str(len(CONTEXT_SWEEP_FEATURES)))
        self.context_subset_max_combinations = tk.StringVar(value="0")
        self.arc_subset_time_budget_minutes = tk.StringVar(value="60")
        self.arc_subset_shard_size = tk.StringVar(value="128")
        self.arc_subset_keep_per_shard = tk.StringVar(value="1")
        self.arc_subset_shortlist_size = tk.StringVar(value="18")
        self.arc_subset_finalist_count = tk.StringVar(value="6")
        self.arc_tolerance_mode = tk.StringVar(value="soft_positive")
        self.pre_arc_window = tk.StringVar(value="1")
        self.post_arc_window = tk.StringVar(value="3")
        self.soft_neighbor_weight = tk.StringVar(value="0.30")
        self.expanded_neighbor_weight = tk.StringVar(value="0.70")
        self.hard_negative_ring = tk.StringVar(value="2")
        self.sweep_negative_ratio = tk.StringVar(value="18")
        self.sweep_positive_oversample = tk.StringVar(value="1.25")
        self.final_negative_ratio = tk.StringVar(value="0")
        self.final_positive_oversample = tk.StringVar(value="1.0")
        self.subset_n_jobs = tk.StringVar(value="1")
        self.search_iters = tk.StringVar(value="120")
        self.max_search_iters = tk.StringVar(value="240")
        self.iter_growth = tk.StringVar(value="2.0")
        self.min_recall_goal = tk.StringVar(value="0.98")
        self.min_precision_goal = tk.StringVar(value="0.90")
        self.max_fpr_goal = tk.StringVar(value="0.03")
        self.min_threshold_goal = tk.StringVar(value="0.08")
        self.auto_max_val_fn = tk.StringVar(value="0")
        self.auto_max_val_fp = tk.StringVar(value="5")
        self.winner_mode = tk.StringVar(value="Balanced")
        self.auto_escalate = tk.BooleanVar(value=True)
        self.current_workflow = None
        self.status_text = tk.StringVar(value="Ready")
        self.last_command = tk.StringVar(value="—")
        self.progress_text = tk.StringVar(value="Idle")
        self.progress_percent_text = tk.StringVar(value="0%")
        self.cleaner_summary_text = tk.StringVar(value="Cleaner summary will appear here after running Cleaner.")
        self._pending_cleaner_summary = None
        self._progress_payload = None
        self._process_started_at = None
        self._last_eta_seconds = None
        self._progress_rate_ema = None
        self._last_progress_sample = None
        self._last_progress_stage = None
        self.elapsed_text = tk.StringVar(value="Elapsed: —")
        self.eta_text = tk.StringVar(value="ETA: —")
        self._root_canvas = None
        self._root_canvas_window = None
        self.arc_feature_vars = {name: tk.BooleanVar(value=True) for name in ARC_SWEEP_FEATURES}
        self.context_feature_vars = {name: tk.BooleanVar(value=True) for name in CONTEXT_SWEEP_FEATURES}
        self.arc_feature_summary_text = tk.StringVar(value="")
        self.context_feature_summary_text = tk.StringVar(value="")
        self.arc_feature_selection_dirty = False
        self.context_feature_selection_dirty = False
        self._adaptive_labels = []
        self._build_styles()
        self._build_ui()
        self._refresh_feature_selection_summary()
        self.bind("<Configure>", self._on_window_resize)
        self.after(120, self._drain_logs)
        self.after(500, self._tick_runtime_clock)
        self.refresh_views()
    def _build_styles(self):
        style = ttk.Style(self)
        try:
            style.theme_use("clam")
        except Exception:
            pass
        style.configure("TFrame", background=APP_BG)
        style.configure(
            "Title.TLabel",
            background=APP_BG,
            foreground=TEXT,
            font=("Segoe UI", 20, "bold"),
        )
        style.configure(
            "TLabel",
            background=APP_BG,
            foreground=TEXT,
            font=("Segoe UI", 10),
        )
        style.configure(
            "Muted.TLabel",
            background=APP_BG,
            foreground=MUTED,
            font=("Segoe UI", 9),
        )
        style.configure("TButton", font=("Segoe UI", 10), padding=(10, 8))
        style.configure(
            "Treeview",
            background=CARD_BG,
            fieldbackground=CARD_BG,
            foreground=TEXT,
            rowheight=28,
            font=("Segoe UI", 10),
            bordercolor=BORDER,
            lightcolor=BORDER,
            darkcolor=BORDER,
        )
        style.configure(
            "Treeview.Heading",
            background=CARD_BG_2,
            foreground=TEXT,
            font=("Segoe UI", 10, "bold"),
            relief="flat",
        )
        style.configure("TNotebook", background=APP_BG, borderwidth=0)
        style.configure("TNotebook.Tab", font=("Segoe UI", 10), padding=(12, 8))
        style.map(
            "TNotebook.Tab",
            background=[("selected", CARD_BG), ("active", CARD_BG_2)],
            foreground=[("selected", TEXT), ("active", TEXT)],
        )
        style.configure(
            "Horizontal.TProgressbar",
            troughcolor=CARD_BG_2,
            background=ACCENT,
            bordercolor=CARD_BG_2,
            lightcolor=ACCENT,
            darkcolor=ACCENT,
        )
    def _build_ui(self):
        self.columnconfigure(0, weight=1)
        self.rowconfigure(0, weight=1)
        shell = tk.Frame(self, bg=APP_BG)
        shell.grid(row=0, column=0, sticky="nsew")
        shell.columnconfigure(0, weight=1)
        shell.rowconfigure(0, weight=1)
        self._root_canvas = tk.Canvas(shell, bg=APP_BG, highlightthickness=0, bd=0)
        self._root_canvas.grid(row=0, column=0, sticky="nsew")
        root_scroll = ttk.Scrollbar(shell, orient="vertical", command=self._root_canvas.yview)
        root_scroll.grid(row=0, column=1, sticky="ns")
        self._root_canvas.configure(yscrollcommand=root_scroll.set)
        outer = ttk.Frame(self._root_canvas, padding=16)
        self._root_canvas_window = self._root_canvas.create_window((0, 0), window=outer, anchor="nw")
        outer.bind("<Configure>", lambda _e: self._root_canvas.configure(scrollregion=self._root_canvas.bbox("all")))
        self._root_canvas.bind("<Configure>", self._on_root_canvas_resize)
        outer.columnconfigure(0, weight=1)
        outer.rowconfigure(2, weight=1)
        top = ttk.Frame(outer)
        top.grid(row=0, column=0, sticky="ew")
        top.columnconfigure(1, weight=1)
        ttk.Label(top, text="TinyML Trainer", style="Title.TLabel").grid(
            row=0, column=0, sticky="w"
        )
        self.paths_btn = ttk.Button(top, text="Show Paths", command=self.toggle_paths)
        self.paths_btn.grid(row=0, column=1, sticky="e")
        self.paths_card = tk.Frame(
            outer,
            bg=CARD_BG,
            highlightthickness=1,
            highlightbackground=BORDER,
            padx=14,
            pady=14,
        )
        self.paths_card.columnconfigure(1, weight=1)
        self._card_label(self.paths_card, "Paths", 0)
        self._path_row(self.paths_card, 1, "Project root", self.project_root, browse_dir=True)
        self._path_row(self.paths_card, 2, "Python exe", self.python_exe, browse_file=True)
        self._path_row(self.paths_card, 3, "Prepare script", self.prepare_script)
        self._path_row(self.paths_card, 4, "Random Forest trainer", self.rf_script)
        self._path_row(self.paths_card, 5, "Extra Trees trainer", self.et_script)
        self._path_row(self.paths_card, 6, "Duel trainer", self.duel_script)
        self._path_row(self.paths_card, 7, "Arc Training CSV", self.cleaned_csv)
        self._path_row(self.paths_card, 8, "Context CSV", self.context_csv)
        self._path_row(self.paths_card, 9, "Subset sweep script", self.subset_script)
        self._path_row(self.paths_card, 10, "Random Forest report", self.rf_report)
        self._path_row(self.paths_card, 11, "Extra Trees report", self.et_report)
        self._path_row(self.paths_card, 12, "Duel report", self.duel_report)
        self._path_row(self.paths_card, 13, "Context report", self.context_report)
        self._path_row(self.paths_card, 14, "Prepare report", self.prepare_report)
        self._path_row(self.paths_card, 15, "Arc subset sweep report", self.subset_report)
        self._path_row(self.paths_card, 16, "Arc subset sweep CSV", self.subset_csv)
        self._path_row(self.paths_card, 17, "Context subset sweep report", self.context_subset_report)
        self._path_row(self.paths_card, 18, "Context subset sweep CSV", self.context_subset_csv)
        main_card = tk.Frame(
            outer,
            bg=CARD_BG,
            highlightthickness=1,
            highlightbackground=BORDER,
            padx=14,
            pady=14,
        )
        main_card.grid(row=2, column=0, sticky="nsew", pady=(14, 0))
        main_card.columnconfigure(0, weight=1)
        main_card.rowconfigure(6, weight=1)
        self._card_label(main_card, "Actions", 0)
        actions = ttk.Frame(main_card)
        actions.grid(row=1, column=0, sticky="ew", pady=(0, 12))
        for i in range(8):
            actions.columnconfigure(i, weight=1)
        self._btn(actions, "Cleaner", self.run_cleaner, 0, 0)
        self._btn(actions, "Random Forest Only", self.run_rf_only, 0, 1)
        self._btn(actions, "Extra Trees Only", self.run_et_only, 0, 2)
        self._btn(actions, "King Training", self.run_duel, 0, 3)
        self._btn(actions, "Context Trainer", self.run_context, 0, 4)
        self._btn(actions, "Subset Sweep", self.run_subset_sweep, 0, 5)
        self._btn(actions, "Refresh", self.refresh_views, 0, 6)
        self._btn(actions, "Clear Output", self.clear_output, 0, 7)
        options = tk.Frame(main_card, bg=CARD_BG_2, highlightthickness=1, highlightbackground=BORDER, padx=10, pady=10)
        options.grid(row=2, column=0, sticky="ew", pady=(0, 12))
        for i in range(6):
            options.columnconfigure(i, weight=1)
        tk.Label(options, text="Search tries", bg=CARD_BG_2, fg=MUTED, font=("Segoe UI", 9)).grid(row=0, column=0, sticky="w")
        ttk.Entry(options, textvariable=self.search_iters, width=10).grid(row=1, column=0, sticky="ew", padx=(0, 8), pady=(4, 8))
        tk.Label(options, text="Max tries", bg=CARD_BG_2, fg=MUTED, font=("Segoe UI", 9)).grid(row=0, column=1, sticky="w")
        ttk.Entry(options, textvariable=self.max_search_iters, width=10).grid(row=1, column=1, sticky="ew", padx=(0, 8), pady=(4, 8))
        tk.Label(options, text="Growth x", bg=CARD_BG_2, fg=MUTED, font=("Segoe UI", 9)).grid(row=0, column=2, sticky="w")
        ttk.Entry(options, textvariable=self.iter_growth, width=10).grid(row=1, column=2, sticky="ew", padx=(0, 8), pady=(4, 8))
        ttk.Checkbutton(options, text="Auto increase tries", variable=self.auto_escalate).grid(row=1, column=3, sticky="w", padx=(0, 8), pady=(4, 8))
        tk.Label(options, text="Winner style", bg=CARD_BG_2, fg=MUTED, font=("Segoe UI", 9)).grid(row=0, column=4, sticky="w")
        ttk.Combobox(options, textvariable=self.winner_mode, values=["Arc-first", "Balanced", "Legacy"], state="readonly", width=18).grid(row=1, column=4, sticky="ew", pady=(4, 8))
        tk.Label(options, text="Min recall", bg=CARD_BG_2, fg=MUTED, font=("Segoe UI", 9)).grid(row=2, column=0, sticky="w")
        ttk.Entry(options, textvariable=self.min_recall_goal, width=10).grid(row=3, column=0, sticky="ew", padx=(0, 8), pady=(4, 0))
        tk.Label(options, text="Min precision", bg=CARD_BG_2, fg=MUTED, font=("Segoe UI", 9)).grid(row=2, column=1, sticky="w")
        ttk.Entry(options, textvariable=self.min_precision_goal, width=10).grid(row=3, column=1, sticky="ew", padx=(0, 8), pady=(4, 0))
        tk.Label(options, text="Max FPR", bg=CARD_BG_2, fg=MUTED, font=("Segoe UI", 9)).grid(row=2, column=2, sticky="w")
        ttk.Entry(options, textvariable=self.max_fpr_goal, width=10).grid(row=3, column=2, sticky="ew", padx=(0, 8), pady=(4, 0))
        tk.Label(options, text="Min threshold", bg=CARD_BG_2, fg=MUTED, font=("Segoe UI", 9)).grid(row=2, column=3, sticky="w")
        ttk.Entry(options, textvariable=self.min_threshold_goal, width=10).grid(row=3, column=3, sticky="ew", padx=(0, 8), pady=(4, 0))
        fpfn = tk.Frame(options, bg=CARD_BG_2)
        fpfn.grid(row=3, column=4, sticky="ew")
        tk.Label(fpfn, text="Target val FN ≤", bg=CARD_BG_2, fg=MUTED, font=("Segoe UI", 9)).pack(side="left")
        ttk.Entry(fpfn, textvariable=self.auto_max_val_fn, width=5).pack(side="left", padx=(6, 8))
        tk.Label(fpfn, text="FP ≤", bg=CARD_BG_2, fg=MUTED, font=("Segoe UI", 9)).pack(side="left")
        ttk.Entry(fpfn, textvariable=self.auto_max_val_fp, width=5).pack(side="left", padx=(6, 0))
        tk.Label(options, text="Subset task", bg=CARD_BG_2, fg=MUTED, font=("Segoe UI", 9)).grid(row=4, column=0, sticky="w", pady=(10, 0))
        ttk.Combobox(options, textvariable=self.subset_task, values=["Arc + Context", "Arc Only", "Context Only"], state="readonly", width=18).grid(row=5, column=0, sticky="ew", padx=(0, 8), pady=(4, 0))
        tk.Label(options, text="Arc subset min", bg=CARD_BG_2, fg=MUTED, font=("Segoe UI", 9)).grid(row=4, column=1, sticky="w", pady=(10, 0))
        ttk.Entry(options, textvariable=self.arc_subset_feature_min, width=10).grid(row=5, column=1, sticky="ew", padx=(0, 8), pady=(4, 0))
        tk.Label(options, text="Arc subset max", bg=CARD_BG_2, fg=MUTED, font=("Segoe UI", 9)).grid(row=4, column=2, sticky="w", pady=(10, 0))
        ttk.Entry(options, textvariable=self.arc_subset_feature_max, width=10).grid(row=5, column=2, sticky="ew", padx=(0, 8), pady=(4, 0))
        tk.Label(options, text="Arc max combos (0=all)", bg=CARD_BG_2, fg=MUTED, font=("Segoe UI", 9)).grid(row=4, column=3, sticky="w", pady=(10, 0))
        ttk.Entry(options, textvariable=self.arc_subset_max_combinations, width=10).grid(row=5, column=3, sticky="ew", padx=(0, 8), pady=(4, 0))
        tk.Label(options, text="Context subset min", bg=CARD_BG_2, fg=MUTED, font=("Segoe UI", 9)).grid(row=6, column=1, sticky="w", pady=(10, 0))
        ttk.Entry(options, textvariable=self.context_subset_feature_min, width=10).grid(row=7, column=1, sticky="ew", padx=(0, 8), pady=(4, 0))
        tk.Label(options, text="Context subset max", bg=CARD_BG_2, fg=MUTED, font=("Segoe UI", 9)).grid(row=6, column=2, sticky="w", pady=(10, 0))
        ttk.Entry(options, textvariable=self.context_subset_feature_max, width=10).grid(row=7, column=2, sticky="ew", padx=(0, 8), pady=(4, 0))
        tk.Label(options, text="Context max combos (0=all)", bg=CARD_BG_2, fg=MUTED, font=("Segoe UI", 9)).grid(row=6, column=3, sticky="w", pady=(10, 0))
        ttk.Entry(options, textvariable=self.context_subset_max_combinations, width=10).grid(row=7, column=3, sticky="ew", padx=(0, 8), pady=(4, 0))
        tk.Label(options, text="Arc budget min", bg=CARD_BG_2, fg=MUTED, font=("Segoe UI", 9)).grid(row=8, column=0, sticky="w", pady=(10, 0))
        ttk.Entry(options, textvariable=self.arc_subset_time_budget_minutes, width=10).grid(row=9, column=0, sticky="ew", padx=(0, 8), pady=(4, 0))
        tk.Label(options, text="Shard size", bg=CARD_BG_2, fg=MUTED, font=("Segoe UI", 9)).grid(row=8, column=1, sticky="w", pady=(10, 0))
        ttk.Entry(options, textvariable=self.arc_subset_shard_size, width=10).grid(row=9, column=1, sticky="ew", padx=(0, 8), pady=(4, 0))
        tk.Label(options, text="Keep / shard", bg=CARD_BG_2, fg=MUTED, font=("Segoe UI", 9)).grid(row=8, column=2, sticky="w", pady=(10, 0))
        ttk.Entry(options, textvariable=self.arc_subset_keep_per_shard, width=10).grid(row=9, column=2, sticky="ew", padx=(0, 8), pady=(4, 0))
        tk.Label(options, text="Shortlist size", bg=CARD_BG_2, fg=MUTED, font=("Segoe UI", 9)).grid(row=8, column=3, sticky="w", pady=(10, 0))
        ttk.Entry(options, textvariable=self.arc_subset_shortlist_size, width=10).grid(row=9, column=3, sticky="ew", padx=(0, 8), pady=(4, 0))
        tk.Label(options, text="Finalists", bg=CARD_BG_2, fg=MUTED, font=("Segoe UI", 9)).grid(row=8, column=4, sticky="w", pady=(10, 0))
        ttk.Entry(options, textvariable=self.arc_subset_finalist_count, width=10).grid(row=9, column=4, sticky="ew", padx=(0, 8), pady=(4, 0))
        tk.Label(options, text="Sweep n_jobs", bg=CARD_BG_2, fg=MUTED, font=("Segoe UI", 9)).grid(row=8, column=5, sticky="w", pady=(10, 0))
        ttk.Entry(options, textvariable=self.subset_n_jobs, width=10).grid(row=9, column=5, sticky="ew", pady=(4, 0))
        tk.Label(options, text="Tolerance mode", bg=CARD_BG_2, fg=MUTED, font=("Segoe UI", 9)).grid(row=10, column=0, sticky="w", pady=(10, 0))
        ttk.Combobox(options, textvariable=self.arc_tolerance_mode, values=["soft_positive", "expanded_positive", "none"], state="readonly", width=16).grid(row=11, column=0, sticky="ew", padx=(0, 8), pady=(4, 0))
        tk.Label(options, text="Pre arc rows", bg=CARD_BG_2, fg=MUTED, font=("Segoe UI", 9)).grid(row=10, column=1, sticky="w", pady=(10, 0))
        ttk.Entry(options, textvariable=self.pre_arc_window, width=10).grid(row=11, column=1, sticky="ew", padx=(0, 8), pady=(4, 0))
        tk.Label(options, text="Post arc rows", bg=CARD_BG_2, fg=MUTED, font=("Segoe UI", 9)).grid(row=10, column=2, sticky="w", pady=(10, 0))
        ttk.Entry(options, textvariable=self.post_arc_window, width=10).grid(row=11, column=2, sticky="ew", padx=(0, 8), pady=(4, 0))
        tk.Label(options, text="Soft nbr weight", bg=CARD_BG_2, fg=MUTED, font=("Segoe UI", 9)).grid(row=10, column=3, sticky="w", pady=(10, 0))
        ttk.Entry(options, textvariable=self.soft_neighbor_weight, width=10).grid(row=11, column=3, sticky="ew", padx=(0, 8), pady=(4, 0))
        tk.Label(options, text="Expanded nbr weight", bg=CARD_BG_2, fg=MUTED, font=("Segoe UI", 9)).grid(row=10, column=4, sticky="w", pady=(10, 0))
        ttk.Entry(options, textvariable=self.expanded_neighbor_weight, width=10).grid(row=11, column=4, sticky="ew", padx=(0, 8), pady=(4, 0))
        tk.Label(options, text="Hard-neg ring", bg=CARD_BG_2, fg=MUTED, font=("Segoe UI", 9)).grid(row=10, column=5, sticky="w", pady=(10, 0))
        ttk.Entry(options, textvariable=self.hard_negative_ring, width=10).grid(row=11, column=5, sticky="ew", pady=(4, 0))
        tk.Label(options, text="Sweep neg ratio", bg=CARD_BG_2, fg=MUTED, font=("Segoe UI", 9)).grid(row=12, column=0, sticky="w", pady=(10, 0))
        ttk.Entry(options, textvariable=self.sweep_negative_ratio, width=10).grid(row=13, column=0, sticky="ew", padx=(0, 8), pady=(4, 0))
        tk.Label(options, text="Sweep pos over", bg=CARD_BG_2, fg=MUTED, font=("Segoe UI", 9)).grid(row=12, column=1, sticky="w", pady=(10, 0))
        ttk.Entry(options, textvariable=self.sweep_positive_oversample, width=10).grid(row=13, column=1, sticky="ew", padx=(0, 8), pady=(4, 0))
        tk.Label(options, text="Final neg ratio", bg=CARD_BG_2, fg=MUTED, font=("Segoe UI", 9)).grid(row=12, column=2, sticky="w", pady=(10, 0))
        ttk.Entry(options, textvariable=self.final_negative_ratio, width=10).grid(row=13, column=2, sticky="ew", padx=(0, 8), pady=(4, 0))
        tk.Label(options, text="Final pos over", bg=CARD_BG_2, fg=MUTED, font=("Segoe UI", 9)).grid(row=12, column=3, sticky="w", pady=(10, 0))
        ttk.Entry(options, textvariable=self.final_positive_oversample, width=10).grid(row=13, column=3, sticky="ew", padx=(0, 8), pady=(4, 0))
        subset_note = tk.Label(
            options,
            text=(
                f"Arc subset sweep now runs in stages: fast ET prescreen over the arc pool, deeper ET shortlist, then parallel RF + ET finalists. "
                f"Arc labels can use a configurable tolerance window and hard-negative-aware downsampling so event recall improves without letting FP explode. "
                f"Context subset sweep uses the locked {len(CONTEXT_SWEEP_FEATURES)}-feature start-only pool. "
                f"Subset inner depth is fixed at {SUBSET_SWEEP_SEARCH_DEPTH}; shortlist depth is auto-reduced for speed."
            ),
            bg=CARD_BG_2,
            fg=MUTED,
            justify="left",
            anchor="w",
            wraplength=1180,
        )
        subset_note.grid(row=14, column=0, columnspan=6, sticky="ew", pady=(10, 0))
        self._adaptive_labels.append((subset_note, 0.78, 440))
        self.workflow_scope_text = tk.StringVar(value="RF / ET / King train on the selected arc base features below, then append the fixed 7 runtime context inputs at export/runtime. Context training uses the selected context-only features below on start-only data. Cleaner rebuilds the datasets only. Subset Sweep can run Arc, Context, or both; for Arc it now budgets time, shards prescreen work, and reranks finalists without changing the full CSV schema.")
        scope_label = tk.Label(options, textvariable=self.workflow_scope_text, bg=CARD_BG_2, fg=MUTED, justify="left", anchor="w", wraplength=1180)
        scope_label.grid(row=15, column=0, columnspan=6, sticky="ew", pady=(10, 0))
        self._adaptive_labels.append((scope_label, 0.78, 440))
        self._build_feature_selection_card(main_card, 3)
        status = tk.Frame(main_card, bg=CARD_BG)
        status.grid(row=4, column=0, sticky="ew", pady=(0, 8))
        status.columnconfigure(3, weight=1)
        tk.Label(status, text="Status", bg=CARD_BG, fg=MUTED, font=("Segoe UI", 10)).grid(
            row=0, column=0, sticky="w"
        )
        tk.Label(
            status,
            textvariable=self.status_text,
            bg=CARD_BG,
            fg=ACCENT,
            font=("Segoe UI", 11, "bold"),
        ).grid(row=0, column=1, sticky="w", padx=(8, 18))
        ttk.Button(status, text="Stop", command=self.stop_running).grid(
            row=0, column=2, sticky="e"
        )
        tk.Label(status, text="Progress", bg=CARD_BG, fg=MUTED, font=("Segoe UI", 9)).grid(
            row=1, column=0, sticky="w", pady=(10, 0)
        )
        self.progress_label = tk.Label(
            status,
            textvariable=self.progress_text,
            bg=CARD_BG,
            fg=TEXT,
            font=("Segoe UI", 9),
            justify="left",
            anchor="w",
            wraplength=980,
        )
        self.progress_label.grid(row=1, column=1, columnspan=2, sticky="ew", padx=(8, 0), pady=(10, 0))
        self._adaptive_labels.append((self.progress_label, 0.62, 320))
        tk.Label(
            status,
            textvariable=self.progress_percent_text,
            bg=CARD_BG,
            fg=ACCENT,
            font=("Segoe UI", 10, "bold"),
            justify="right",
            anchor="e",
            width=8,
        ).grid(row=1, column=3, sticky="e", pady=(10, 0))
        tk.Label(status, textvariable=self.elapsed_text, bg=CARD_BG, fg=MUTED, font=("Segoe UI", 9)).grid(
            row=2, column=1, sticky="w", padx=(8, 0), pady=(6, 0)
        )
        tk.Label(status, textvariable=self.eta_text, bg=CARD_BG, fg=MUTED, font=("Segoe UI", 9)).grid(
            row=2, column=2, sticky="w", padx=(8, 0), pady=(6, 0)
        )
        tk.Label(status, text="Last command", bg=CARD_BG, fg=MUTED, font=("Segoe UI", 9)).grid(
            row=3, column=0, sticky="w", pady=(10, 0)
        )
        self.command_label = tk.Label(
            status,
            textvariable=self.last_command,
            bg=CARD_BG,
            fg=TEXT,
            font=("Consolas", 9),
            justify="left",
            anchor="w",
            wraplength=1120,
        )
        self.command_label.grid(row=3, column=1, columnspan=3, sticky="ew", padx=(8, 0), pady=(10, 0))
        self._adaptive_labels.append((self.command_label, 0.74, 380))
        self.pb = ttk.Progressbar(main_card, mode="determinate", maximum=100.0, value=0.0)
        self.pb.grid(row=5, column=0, sticky="ew", pady=(0, 12))
        notebook = ttk.Notebook(main_card)
        notebook.grid(row=6, column=0, sticky="nsew", pady=(0, 0))
        logs_tab = ttk.Frame(notebook, padding=10)
        data_tab = ttk.Frame(notebook, padding=10)
        reports_tab = ttk.Frame(notebook, padding=10)
        notebook.add(logs_tab, text="Live Logs")
        notebook.add(data_tab, text="Dataset")
        notebook.add(reports_tab, text="Reports")
        self._build_logs_tab(logs_tab)
        self._build_data_tab(data_tab)
        self._build_reports_tab(reports_tab)
    def _on_root_canvas_resize(self, event=None):
        if self._root_canvas is None or self._root_canvas_window is None:
            return
        width = max(100, int(getattr(event, "width", self._root_canvas.winfo_width())))
        self._root_canvas.itemconfigure(self._root_canvas_window, width=width)
    def clear_output(self):
        try:
            self.log_text.delete("1.0", "end")
        except Exception:
            pass
        self._pending_cleaner_summary = None
        self.cleaner_summary_text.set("Cleaner summary cleared.")
        self.last_command.set("—")
        self.status_text.set("Ready")
        self._reset_progress()
    def _card_label(self, parent, text, row):
        tk.Label(
            parent,
            text=text,
            bg=CARD_BG,
            fg=TEXT,
            font=("Segoe UI", 11, "bold"),
        ).grid(row=row, column=0, columnspan=3, sticky="w", pady=(0, 10))
    def _path_row(self, parent, row, label, var, browse_dir=False, browse_file=False):
        tk.Label(parent, text=label, bg=CARD_BG, fg=MUTED, font=("Segoe UI", 9)).grid(
            row=row, column=0, sticky="w", pady=4
        )
        entry = ttk.Entry(parent, textvariable=var)
        entry.grid(row=row, column=1, sticky="ew", padx=(10, 10), pady=4)
        if browse_dir:
            ttk.Button(parent, text="Browse", command=self._browse_root).grid(
                row=row, column=2, sticky="ew", pady=4
            )
        elif browse_file:
            ttk.Button(parent, text="Browse", command=lambda: self._browse_file(var)).grid(
                row=row, column=2, sticky="ew", pady=4
            )
    def _btn(self, parent, text, cmd, row, col):
        ttk.Button(parent, text=text, command=cmd).grid(
            row=row, column=col, sticky="ew", padx=4, pady=4
        )
    def _build_feature_selection_card(self, parent, row):
        card = tk.Frame(
            parent,
            bg=CARD_BG,
            highlightthickness=1,
            highlightbackground=BORDER,
            padx=10,
            pady=10,
        )
        card.grid(row=row, column=0, sticky="ew", pady=(0, 12))
        card.columnconfigure(0, weight=1)
        card.columnconfigure(1, weight=1)
        self._card_label(card, "Final Training Features", 0)
        self._build_feature_selector_panel(
            card,
            row=1,
            column=0,
            title="Arc final training base features",
            note="Only these arc base features are trained/exported. Firmware appends the fixed 7 context inputs behind them automatically.",
            feature_pool=ARC_SWEEP_FEATURES,
            feature_vars=self.arc_feature_vars,
            summary_var=self.arc_feature_summary_text,
            role="arc",
        )
        self._build_feature_selector_panel(
            card,
            row=1,
            column=1,
            title="Context final training features",
            note="These context-only features train the start-data context model. They are independent from the arc model selection.",
            feature_pool=CONTEXT_SWEEP_FEATURES,
            feature_vars=self.context_feature_vars,
            summary_var=self.context_feature_summary_text,
            role="context",
        )
    def _build_feature_selector_panel(self, parent, row, column, title, note, feature_pool, feature_vars, summary_var, role):
        panel = tk.Frame(
            parent,
            bg=CARD_BG_2,
            highlightthickness=1,
            highlightbackground=BORDER,
            padx=10,
            pady=10,
        )
        panel.grid(row=row, column=column, sticky="nsew", padx=(0, 8) if column == 0 else (8, 0))
        panel.columnconfigure(0, weight=1)
        tk.Label(panel, text=title, bg=CARD_BG_2, fg=TEXT, font=("Segoe UI", 10, "bold")).grid(row=0, column=0, sticky="w")
        note_label = tk.Label(panel, text=note, bg=CARD_BG_2, fg=MUTED, justify="left", anchor="w", wraplength=520)
        note_label.grid(row=1, column=0, sticky="ew", pady=(4, 8))
        self._adaptive_labels.append((note_label, 0.34, 280))
        btn_row = tk.Frame(panel, bg=CARD_BG_2)
        btn_row.grid(row=2, column=0, sticky="ew", pady=(0, 8))
        ttk.Button(btn_row, text="Use Sweep Recommendation", command=lambda r=role: self._apply_recommended_features(r)).pack(side="left", padx=(0, 6))
        ttk.Button(btn_row, text="Select All", command=lambda r=role: self._set_selected_feature_names(r, self._feature_pool(r), mark_dirty=True)).pack(side="left", padx=(0, 6))
        ttk.Button(btn_row, text="Clear", command=lambda r=role: self._set_selected_feature_names(r, [], allow_empty=True, mark_dirty=True)).pack(side="left")
        summary_label = tk.Label(panel, textvariable=summary_var, bg=CARD_BG_2, fg=MUTED, justify="left", anchor="w", wraplength=520)
        summary_label.grid(row=3, column=0, sticky="ew", pady=(0, 8))
        self._adaptive_labels.append((summary_label, 0.34, 280))
        grid = tk.Frame(panel, bg=CARD_BG_2)
        grid.grid(row=4, column=0, sticky="ew")
        for idx in range(2):
            grid.columnconfigure(idx, weight=1)
        for idx, feature_name in enumerate(feature_pool):
            ttk.Checkbutton(
                grid,
                text=feature_name,
                variable=feature_vars[feature_name],
                command=lambda r=role: self._on_feature_selection_changed(r),
            ).grid(row=idx // 2, column=idx % 2, sticky="w", padx=(0, 8), pady=2)
    def _feature_pool(self, role):
        return list(ARC_SWEEP_FEATURES if role == "arc" else CONTEXT_SWEEP_FEATURES)
    def _feature_var_map(self, role):
        return self.arc_feature_vars if role == "arc" else self.context_feature_vars
    def _feature_summary_var(self, role):
        return self.arc_feature_summary_text if role == "arc" else self.context_feature_summary_text
    def _selected_feature_names(self, role):
        pool = self._feature_pool(role)
        var_map = self._feature_var_map(role)
        return [name for name in pool if bool(var_map[name].get())]
    def _set_selected_feature_names(self, role, feature_names, allow_empty=False, mark_dirty=False):
        pool = self._feature_pool(role)
        wanted = {str(x).strip() for x in (feature_names or []) if str(x).strip() in pool}
        if not wanted and not allow_empty:
            wanted = set(pool)
        for name in pool:
            self._feature_var_map(role)[name].set(name in wanted)
        if role == "arc":
            self.arc_feature_selection_dirty = bool(mark_dirty)
        else:
            self.context_feature_selection_dirty = bool(mark_dirty)
        self._refresh_feature_selection_summary(role)
    def _on_feature_selection_changed(self, role):
        if role == "arc":
            self.arc_feature_selection_dirty = True
        else:
            self.context_feature_selection_dirty = True
        self._refresh_feature_selection_summary(role)
    def _refresh_feature_selection_summary(self, role=None):
        roles = [role] if role else ["arc", "context"]
        for active_role in roles:
            selected = self._selected_feature_names(active_role)
            pool = self._feature_pool(active_role)
            dirty = self.arc_feature_selection_dirty if active_role == "arc" else self.context_feature_selection_dirty
            if active_role == "arc":
                detail = "Runtime/export appends the fixed 7 context inputs after these base features."
            else:
                detail = "Context model input order matches these selected features exactly."
            prefix = "Manual override active." if dirty else "Auto-following report recommendations until you change a checkbox."
            if selected:
                summary = f"{prefix} Selected {len(selected)}/{len(pool)} features: {', '.join(selected)}. {detail}"
            else:
                summary = f"{prefix} No features selected. Select at least one feature before training. {detail}"
            self._feature_summary_var(active_role).set(summary)
    def _extract_feature_list(self, payload, *keys):
        if not isinstance(payload, dict):
            return []
        for key in keys:
            value = self._pick(payload, key) if isinstance(key, tuple) else payload.get(key)
            if isinstance(value, list):
                out = [str(x).strip() for x in value if str(x).strip()]
                if out:
                    return out
        return []
    def _recommended_feature_names(self, role):
        if role == "arc":
            candidates = [
                (
                    self._read_json(self.subset_report),
                    ("recommended_arc_base_features_global", "recommended_features_global", ("overall_best_combined_tradeoff", "features")),
                ),
                (
                    self._read_json(self.duel_report),
                    (("winner", "arc_base_feature_names"), ("winner", "feature_names")),
                ),
                (
                    self._read_json(self.rf_report),
                    ("arc_base_feature_names", "feature_names"),
                ),
                (
                    self._read_json(self.et_report),
                    ("arc_base_feature_names", "feature_names"),
                ),
            ]
            pool = set(ARC_SWEEP_FEATURES)
        else:
            candidates = [
                (
                    self._read_json(self.context_subset_report),
                    ("recommended_context_features", "recommended_features", ("overall_best_tradeoff", "features")),
                ),
                (
                    self._read_json(self.context_report),
                    ("feature_names",),
                ),
            ]
            pool = set(CONTEXT_SWEEP_FEATURES)
        for payload, keys in candidates:
            names = [name for name in self._extract_feature_list(payload, *keys) if name in pool]
            if names:
                return names
        return []
    def _auto_apply_feature_recommendations(self, force=False):
        for role in ("arc", "context"):
            selected = self._selected_feature_names(role)
            dirty = self.arc_feature_selection_dirty if role == "arc" else self.context_feature_selection_dirty
            if force or not dirty:
                recommended = self._recommended_feature_names(role)
                if recommended:
                    self._set_selected_feature_names(role, recommended, allow_empty=False, mark_dirty=False)
                elif force:
                    self._set_selected_feature_names(role, self._feature_pool(role), allow_empty=False, mark_dirty=False)
    def _apply_recommended_features(self, role):
        recommended = self._recommended_feature_names(role)
        if recommended:
            self._set_selected_feature_names(role, recommended, allow_empty=False, mark_dirty=False)
        else:
            self._set_selected_feature_names(role, self._feature_pool(role), allow_empty=False, mark_dirty=False)
        self._refresh_feature_selection_summary(role)
    def _explicit_feature_args(self, role):
        selected = self._selected_feature_names(role)
        if not selected:
            label = "arc base" if role == "arc" else "context"
            raise ValueError(f"Select at least one {label} feature before starting training.")
        return ["--features", *selected]
    def _build_logs_tab(self, parent):
        parent.columnconfigure(0, weight=1)
        parent.rowconfigure(0, weight=1)
        frame = tk.Frame(parent, bg=CARD_BG, highlightthickness=1, highlightbackground=BORDER)
        frame.grid(row=0, column=0, sticky="nsew")
        frame.columnconfigure(0, weight=1)
        frame.rowconfigure(0, weight=1)
        self.log_text = tk.Text(
            frame,
            wrap="word",
            font=("Consolas", 10),
            bg=LOG_BG,
            fg=TEXT,
            insertbackground=TEXT,
            relief="flat",
            padx=10,
            pady=10,
        )
        self.log_text.grid(row=0, column=0, sticky="nsew")
        log_scroll = ttk.Scrollbar(frame, orient="vertical", command=self.log_text.yview)
        log_scroll.grid(row=0, column=1, sticky="ns")
        self.log_text.configure(yscrollcommand=log_scroll.set)
    def _build_data_tab(self, parent):
        parent.columnconfigure(0, weight=1)
        parent.rowconfigure(2, weight=1)
        self.dataset_summary = tk.StringVar(value="No dataset loaded.")
        self.dataset_summary_label = tk.Label(
            parent, textvariable=self.dataset_summary, bg=APP_BG, fg=MUTED, justify="left", anchor="w", wraplength=1120
        )
        self.dataset_summary_label.grid(row=0, column=0, sticky="ew", pady=(0, 10))
        self._adaptive_labels.append((self.dataset_summary_label, 0.78, 420))
        cleaner_card = tk.Frame(parent, bg=CARD_BG, highlightthickness=1, highlightbackground=BORDER, padx=10, pady=10)
        cleaner_card.grid(row=1, column=0, sticky="ew", pady=(0, 10))
        cleaner_card.columnconfigure(0, weight=1)
        tk.Label(cleaner_card, text="Latest Cleaner Summary", bg=CARD_BG, fg=TEXT, font=("Segoe UI", 10, "bold")).grid(row=0, column=0, sticky="w")
        self.cleaner_summary_label = tk.Label(cleaner_card, textvariable=self.cleaner_summary_text, bg=CARD_BG, fg=MUTED, justify="left", anchor="w", wraplength=1200)
        self.cleaner_summary_label.grid(row=1, column=0, sticky="ew", pady=(6, 0))
        self._adaptive_labels.append((self.cleaner_summary_label, 0.76, 420))
        frame = tk.Frame(parent, bg=CARD_BG, highlightthickness=1, highlightbackground=BORDER)
        frame.grid(row=2, column=0, sticky="nsew")
        frame.columnconfigure(0, weight=1)
        frame.rowconfigure(0, weight=1)
        cols = ("feature", "mean", "std", "min", "max")
        self.dataset_tree = ttk.Treeview(frame, columns=cols, show="headings")
        for c, w in zip(cols, (240, 140, 140, 140, 140)):
            self.dataset_tree.heading(c, text=c.title())
            self.dataset_tree.column(c, width=w, anchor="center")
        self.dataset_tree.grid(row=0, column=0, sticky="nsew")
        dataset_scroll = ttk.Scrollbar(frame, orient="vertical", command=self.dataset_tree.yview)
        dataset_scroll.grid(row=0, column=1, sticky="ns")
        self.dataset_tree.configure(yscrollcommand=dataset_scroll.set)
    def _build_reports_tab(self, parent):
        parent.columnconfigure(0, weight=1)
        parent.rowconfigure(0, weight=1)
        sub = ttk.Notebook(parent)
        sub.grid(row=0, column=0, sticky="nsew")
        self.prepare_tab = ttk.Frame(sub, padding=10)
        self.duel_compare_tab = ttk.Frame(sub, padding=10)
        self.rf_tab = ttk.Frame(sub, padding=10)
        self.et_tab = ttk.Frame(sub, padding=10)
        self.duel_tab = ttk.Frame(sub, padding=10)
        self.context_tab = ttk.Frame(sub, padding=10)
        self.subset_tab = ttk.Frame(sub, padding=10)
        sub.add(self.prepare_tab, text="Prepare Report")
        sub.add(self.duel_compare_tab, text="Benchmark Overview")
        sub.add(self.rf_tab, text="Random Forest Report")
        sub.add(self.et_tab, text="Extra Trees Report")
        sub.add(self.duel_tab, text="King Report")
        sub.add(self.context_tab, text="Context Report")
        sub.add(self.subset_tab, text="Subset Sweep Report")
        self._build_prepare_report_subtab(self.prepare_tab)
        self._build_compare_subtab(self.duel_compare_tab)
        self._build_single_report_subtab(self.rf_tab, prefix="rf")
        self._build_single_report_subtab(self.et_tab, prefix="et")
        self._build_single_report_subtab(self.duel_tab, prefix="duel")
        self._build_single_report_subtab(self.context_tab, prefix="context")
        self._build_subset_report_subtab(self.subset_tab)
    def _build_subset_report_subtab(self, parent):
        parent.columnconfigure(0, weight=1)
        parent.rowconfigure(1, weight=1)
        parent.rowconfigure(3, weight=1)
        self.subset_summary = tk.StringVar(value="No subset sweep report loaded.")
        tk.Label(parent, textvariable=self.subset_summary, bg=APP_BG, fg=MUTED, justify="left", anchor="w", wraplength=1120).grid(row=0, column=0, sticky="ew", pady=(0, 10))
        frame1 = tk.Frame(parent, bg=CARD_BG, highlightthickness=1, highlightbackground=BORDER)
        frame1.grid(row=1, column=0, sticky="nsew", pady=(0, 10))
        frame1.columnconfigure(0, weight=1)
        frame1.rowconfigure(0, weight=1)
        self.subset_metric_tree = ttk.Treeview(frame1, columns=("metric", "value"), show="headings")
        self.subset_metric_tree.heading("metric", text="Metric")
        self.subset_metric_tree.heading("value", text="Value")
        self.subset_metric_tree.column("metric", width=320, anchor="w")
        self.subset_metric_tree.column("value", width=260, anchor="center")
        self.subset_metric_tree.grid(row=0, column=0, sticky="nsew")
        subset_metric_scroll = ttk.Scrollbar(frame1, orient="vertical", command=self.subset_metric_tree.yview)
        subset_metric_scroll.grid(row=0, column=1, sticky="ns")
        self.subset_metric_tree.configure(yscrollcommand=subset_metric_scroll.set)
        frame2 = tk.Frame(parent, bg=CARD_BG, highlightthickness=1, highlightbackground=BORDER)
        frame2.grid(row=3, column=0, sticky="nsew")
        frame2.columnconfigure(0, weight=1)
        frame2.rowconfigure(0, weight=1)
        cols = (
            "bucket",
            "feature_count",
            "metric1_label",
            "metric1_value",
            "metric2_label",
            "metric2_value",
            "metric3_label",
            "metric3_value",
            "metric4_label",
            "metric4_value",
            "combo",
        )
        self.subset_best_tree = ttk.Treeview(frame2, columns=cols, show="headings")
        headings = {
            "bucket": "Bucket",
            "feature_count": "Count",
            "metric1_label": "Metric 1",
            "metric1_value": "Value 1",
            "metric2_label": "Metric 2",
            "metric2_value": "Value 2",
            "metric3_label": "Metric 3",
            "metric3_value": "Value 3",
            "metric4_label": "Metric 4",
            "metric4_value": "Value 4",
            "combo": "Feature combo",
        }
        widths = {
            "bucket": 220,
            "feature_count": 70,
            "metric1_label": 110,
            "metric1_value": 90,
            "metric2_label": 110,
            "metric2_value": 90,
            "metric3_label": 110,
            "metric3_value": 90,
            "metric4_label": 110,
            "metric4_value": 90,
            "combo": 600,
        }
        for c in cols:
            self.subset_best_tree.heading(c, text=headings[c])
            self.subset_best_tree.column(
                c,
                width=widths[c],
                anchor="center" if c not in {"combo", "bucket", "metric1_label", "metric2_label", "metric3_label", "metric4_label"} else "w",
            )
        self.subset_best_tree.grid(row=0, column=0, sticky="nsew")
        subset_best_v_scroll = ttk.Scrollbar(frame2, orient="vertical", command=self.subset_best_tree.yview)
        subset_best_v_scroll.grid(row=0, column=1, sticky="ns")
        subset_best_h_scroll = ttk.Scrollbar(frame2, orient="horizontal", command=self.subset_best_tree.xview)
        subset_best_h_scroll.grid(row=1, column=0, sticky="ew")
        self.subset_best_tree.configure(yscrollcommand=subset_best_v_scroll.set, xscrollcommand=subset_best_h_scroll.set)
    def _build_prepare_report_subtab(self, parent):
        parent.columnconfigure(0, weight=1)
        parent.rowconfigure(1, weight=1)
        parent.rowconfigure(3, weight=1)
        self.prepare_summary = tk.StringVar(value="No prepare report loaded.")
        tk.Label(parent, textvariable=self.prepare_summary, bg=APP_BG, fg=MUTED, justify="left").grid(row=0, column=0, sticky="w", pady=(0, 10))
        frame1 = tk.Frame(parent, bg=CARD_BG, highlightthickness=1, highlightbackground=BORDER)
        frame1.grid(row=1, column=0, sticky="nsew", pady=(0, 10))
        frame1.columnconfigure(0, weight=1)
        frame1.rowconfigure(0, weight=1)
        self.prepare_metric_tree = ttk.Treeview(frame1, columns=("metric", "value"), show="headings")
        self.prepare_metric_tree.heading("metric", text="Metric")
        self.prepare_metric_tree.heading("value", text="Value")
        self.prepare_metric_tree.column("metric", width=340, anchor="w")
        self.prepare_metric_tree.column("value", width=260, anchor="center")
        self.prepare_metric_tree.grid(row=0, column=0, sticky="nsew")
        prep_metric_scroll = ttk.Scrollbar(frame1, orient="vertical", command=self.prepare_metric_tree.yview)
        prep_metric_scroll.grid(row=0, column=1, sticky="ns")
        self.prepare_metric_tree.configure(yscrollcommand=prep_metric_scroll.set)
        frame2 = tk.Frame(parent, bg=CARD_BG, highlightthickness=1, highlightbackground=BORDER)
        frame2.grid(row=3, column=0, sticky="nsew")
        frame2.columnconfigure(0, weight=1)
        frame2.rowconfigure(0, weight=1)
        self.prepare_feature_tree = ttk.Treeview(frame2, columns=("dataset", "feature", "mean", "std", "min", "max"), show="headings")
        for c, title, width in [("dataset", "Dataset", 130), ("feature", "Feature", 260), ("mean", "Mean", 110), ("std", "Std", 110), ("min", "Min", 110), ("max", "Max", 110)]:
            self.prepare_feature_tree.heading(c, text=title)
            self.prepare_feature_tree.column(c, width=width, anchor="center" if c != "feature" else "w")
        self.prepare_feature_tree.grid(row=0, column=0, sticky="nsew")
        prep_feature_scroll = ttk.Scrollbar(frame2, orient="vertical", command=self.prepare_feature_tree.yview)
        prep_feature_scroll.grid(row=0, column=1, sticky="ns")
        self.prepare_feature_tree.configure(yscrollcommand=prep_feature_scroll.set)
    def _build_compare_subtab(self, parent):
        parent.columnconfigure(0, weight=1)
        parent.rowconfigure(1, weight=1)
        parent.rowconfigure(3, weight=1)
        self.compare_summary = tk.StringVar(value="No reports loaded.")
        tk.Label(parent, textvariable=self.compare_summary, bg=APP_BG, fg=MUTED, justify="left").grid(
            row=0, column=0, sticky="w", pady=(0, 10)
        )
        frame1 = tk.Frame(parent, bg=CARD_BG, highlightthickness=1, highlightbackground=BORDER)
        frame1.grid(row=1, column=0, sticky="nsew", pady=(0, 10))
        frame1.columnconfigure(0, weight=1)
        frame1.rowconfigure(0, weight=1)
        cols = (
            "rank",
            "model",
            "source",
            "cv_ap",
            "test_ap",
            "roc_auc",
            "acc",
            "bal_acc",
            "f1",
            "recall",
            "specificity",
            "thr",
            "val_fp",
            "val_fn",
            "test_tn",
            "test_fp",
            "test_fn",
            "test_tp",
            "nodes",
            "winner",
        )
        self.benchmark_tree = ttk.Treeview(frame1, columns=cols, show="headings")
        headings = {
            "rank": "Rank",
            "model": "Model",
            "source": "Source",
            "cv_ap": "CV AP",
            "test_ap": "Test AP",
            "roc_auc": "ROC AUC",
            "acc": "Accuracy",
            "bal_acc": "Bal Acc",
            "f1": "F1",
            "recall": "Recall",
            "specificity": "Specificity",
            "thr": "Threshold",
            "val_fp": "Val FP",
            "val_fn": "Val FN",
            "test_tn": "Test TN",
            "test_fp": "Test FP",
            "test_fn": "Test FN",
            "test_tp": "Test TP",
            "nodes": "Nodes",
            "winner": "Winner",
        }
        widths = {
            "rank": 70,
            "model": 170,
            "source": 130,
            "cv_ap": 95,
            "test_ap": 95,
            "roc_auc": 95,
            "acc": 90,
            "bal_acc": 90,
            "f1": 90,
            "recall": 90,
            "specificity": 95,
            "thr": 90,
            "val_fp": 80,
            "val_fn": 80,
            "test_tn": 80,
            "test_fp": 80,
            "test_fn": 80,
            "test_tp": 80,
            "nodes": 90,
            "winner": 90,
        }
        for c in cols:
            self.benchmark_tree.heading(c, text=headings[c])
            self.benchmark_tree.column(c, width=widths[c], anchor="center")
        self.benchmark_tree.grid(row=0, column=0, sticky="nsew")
        benchmark_v_scroll = ttk.Scrollbar(frame1, orient="vertical", command=self.benchmark_tree.yview)
        benchmark_v_scroll.grid(row=0, column=1, sticky="ns")
        benchmark_h_scroll = ttk.Scrollbar(frame1, orient="horizontal", command=self.benchmark_tree.xview)
        benchmark_h_scroll.grid(row=1, column=0, sticky="ew")
        self.benchmark_tree.configure(yscrollcommand=benchmark_v_scroll.set, xscrollcommand=benchmark_h_scroll.set)
        frame2 = tk.Frame(parent, bg=CARD_BG, highlightthickness=1, highlightbackground=BORDER)
        frame2.grid(row=3, column=0, sticky="nsew")
        frame2.columnconfigure(0, weight=1)
        frame2.rowconfigure(0, weight=1)
        cols2 = ("feature", "rf", "et", "leader")
        self.importance_tree = ttk.Treeview(frame2, columns=cols2, show="headings")
        for c, w in zip(cols2, (240, 180, 180, 160)):
            self.importance_tree.heading(c, text={"rf": "Random Forest", "et": "Extra Trees"}.get(c, c.title()))
            self.importance_tree.column(c, width=w, anchor="center")
        self.importance_tree.grid(row=0, column=0, sticky="nsew")
        importance_scroll = ttk.Scrollbar(frame2, orient="vertical", command=self.importance_tree.yview)
        importance_scroll.grid(row=0, column=1, sticky="ns")
        self.importance_tree.configure(yscrollcommand=importance_scroll.set)
    def _build_single_report_subtab(self, parent, prefix):
        parent.columnconfigure(0, weight=1)
        parent.rowconfigure(1, weight=1)
        summary_var = tk.StringVar(value="No report loaded.")
        setattr(self, f"{prefix}_summary", summary_var)
        tk.Label(parent, textvariable=summary_var, bg=APP_BG, fg=MUTED, justify="left").grid(
            row=0, column=0, sticky="w", pady=(0, 10)
        )
        notebook = ttk.Notebook(parent)
        setattr(self, f"{prefix}_notebook", notebook)
        notebook.grid(row=1, column=0, sticky="nsew")
        overview_tab = ttk.Frame(notebook, padding=10)
        class_tab = ttk.Frame(notebook, padding=10)
        cm_tab = ttk.Frame(notebook, padding=10)
        params_tab = ttk.Frame(notebook, padding=10)
        fi_tab = ttk.Frame(notebook, padding=10)
        notebook.add(overview_tab, text="Overview")
        notebook.add(class_tab, text="Classification Report")
        notebook.add(cm_tab, text="Confusion Matrix")
        notebook.add(params_tab, text="Best Params")
        notebook.add(fi_tab, text="Feature Importances")
        setattr(self, f"{prefix}_overview_tab", overview_tab)
        setattr(self, f"{prefix}_class_tab", class_tab)
        setattr(self, f"{prefix}_cm_tab", cm_tab)
        setattr(self, f"{prefix}_params_tab", params_tab)
        setattr(self, f"{prefix}_fi_tab", fi_tab)
        overview_tab.columnconfigure(0, weight=1)
        overview_tab.rowconfigure(0, weight=1)
        class_tab.columnconfigure(0, weight=1)
        class_tab.rowconfigure(0, weight=1)
        cm_tab.columnconfigure(0, weight=1)
        cm_tab.rowconfigure(0, weight=1)
        params_tab.columnconfigure(0, weight=1)
        params_tab.rowconfigure(0, weight=1)
        fi_tab.columnconfigure(0, weight=1)
        fi_tab.rowconfigure(0, weight=1)
        metric_tree = ttk.Treeview(overview_tab, columns=("metric", "value"), show="headings")
        metric_tree.heading("metric", text="Metric")
        metric_tree.heading("value", text="Value")
        metric_tree.column("metric", width=320, anchor="w")
        metric_tree.column("value", width=220, anchor="center")
        metric_tree.grid(row=0, column=0, sticky="nsew")
        metric_scroll = ttk.Scrollbar(overview_tab, orient="vertical", command=metric_tree.yview)
        metric_scroll.grid(row=0, column=1, sticky="ns")
        metric_tree.configure(yscrollcommand=metric_scroll.set)
        setattr(self, f"{prefix}_metric_tree", metric_tree)
        class_tree = ttk.Treeview(
            class_tab,
            columns=("label", "precision", "recall", "f1", "support"),
            show="headings",
        )
        for c, w in zip(("label", "precision", "recall", "f1", "support"), (180, 130, 130, 130, 130)):
            class_tree.heading(c, text={"f1": "F1-score"}.get(c, c.title()))
            class_tree.column(c, width=w, anchor="center")
        class_tree.grid(row=0, column=0, sticky="nsew")
        class_scroll = ttk.Scrollbar(class_tab, orient="vertical", command=class_tree.yview)
        class_scroll.grid(row=0, column=1, sticky="ns")
        class_tree.configure(yscrollcommand=class_scroll.set)
        setattr(self, f"{prefix}_class_tree", class_tree)
        cm_tree = ttk.Treeview(cm_tab, columns=("label", "pred0", "pred1"), show="headings")
        cm_tree.heading("label", text="Actual \\ Pred")
        cm_tree.heading("pred0", text="Pred 0")
        cm_tree.heading("pred1", text="Pred 1")
        cm_tree.column("label", width=200, anchor="w")
        cm_tree.column("pred0", width=120, anchor="center")
        cm_tree.column("pred1", width=120, anchor="center")
        cm_tree.grid(row=0, column=0, sticky="nsew")
        cm_scroll = ttk.Scrollbar(cm_tab, orient="vertical", command=cm_tree.yview)
        cm_scroll.grid(row=0, column=1, sticky="ns")
        cm_tree.configure(yscrollcommand=cm_scroll.set)
        setattr(self, f"{prefix}_cm_tree", cm_tree)
        param_tree = ttk.Treeview(params_tab, columns=("param", "value"), show="headings")
        param_tree.heading("param", text="Parameter")
        param_tree.heading("value", text="Value")
        param_tree.column("param", width=280, anchor="w")
        param_tree.column("value", width=220, anchor="center")
        param_tree.grid(row=0, column=0, sticky="nsew")
        param_scroll = ttk.Scrollbar(params_tab, orient="vertical", command=param_tree.yview)
        param_scroll.grid(row=0, column=1, sticky="ns")
        param_tree.configure(yscrollcommand=param_scroll.set)
        setattr(self, f"{prefix}_param_tree", param_tree)
        fi_tree = ttk.Treeview(fi_tab, columns=("rank", "feature", "importance"), show="headings")
        fi_tree.heading("rank", text="Rank")
        fi_tree.heading("feature", text="Feature")
        fi_tree.heading("importance", text="Importance")
        fi_tree.column("rank", width=80, anchor="center")
        fi_tree.column("feature", width=320, anchor="w")
        fi_tree.column("importance", width=160, anchor="center")
        fi_tree.grid(row=0, column=0, sticky="nsew")
        fi_scroll = ttk.Scrollbar(fi_tab, orient="vertical", command=fi_tree.yview)
        fi_scroll.grid(row=0, column=1, sticky="ns")
        fi_tree.configure(yscrollcommand=fi_scroll.set)
        setattr(self, f"{prefix}_fi_tree", fi_tree)
    def _get_int(self, value, default, minimum=None):
        try:
            if hasattr(value, "get"):
                value = value.get()
            out = int(float(str(value).strip()))
        except Exception:
            out = int(default)
        if minimum is not None:
            out = max(int(minimum), out)
        return out
    def _get_float(self, value, default, minimum=None, maximum=None):
        try:
            if hasattr(value, "get"):
                value = value.get()
            out = float(str(value).strip())
        except Exception:
            out = float(default)
        if minimum is not None:
            out = max(float(minimum), out)
        if maximum is not None:
            out = min(float(maximum), out)
        return out
    def _trainer_script_var_for_kind(self, kind):
        if kind == "rf":
            return self.rf_script
        if kind == "et":
            return self.et_script
        if kind == "context":
            return self.context_script
        if kind == "subset":
            return self.subset_script
        return self.duel_script
    def _arc_training_flags(self, n_iter):
        return [
            "--n_iter", str(int(n_iter)),
            "--min_recall", f"{self._get_float(self.min_recall_goal, 0.98, 0.0, 1.0):.6f}",
            "--min_precision", f"{self._get_float(self.min_precision_goal, 0.90, 0.0, 1.0):.6f}",
            "--max_fpr", f"{self._get_float(self.max_fpr_goal, 0.03, 0.0, 1.0):.6f}",
            "--min_threshold", f"{self._get_float(self.min_threshold_goal, 0.08, 0.0, 0.99):.6f}",
            "--arc_tolerance_mode", str((self.arc_tolerance_mode.get() or "soft_positive").strip()),
            "--pre_arc_window", str(self._get_int(self.pre_arc_window, 1, 0)),
            "--post_arc_window", str(self._get_int(self.post_arc_window, 3, 0)),
            "--soft_neighbor_weight", f"{self._get_float(self.soft_neighbor_weight, 0.30, 0.02, 1.0):.6f}",
            "--expanded_neighbor_weight", f"{self._get_float(self.expanded_neighbor_weight, 0.70, 0.10, 1.25):.6f}",
            "--hard_negative_ring", str(self._get_int(self.hard_negative_ring, 2, 0)),
            "--final_negative_ratio", f"{self._get_float(self.final_negative_ratio, 0.0, 0.0, None):.6f}",
            "--positive_oversample", f"{self._get_float(self.final_positive_oversample, 1.0, 1.0, None):.6f}",
        ]
    def _context_training_flags(self, n_iter):
        return ["--n_iter", str(int(n_iter))]
    def _subset_training_flags(self):
        task_label = (self.subset_task.get() or "Arc + Context").strip().lower()
        if "context" in task_label and "arc" in task_label:
            task = "auto"
        elif "context" in task_label:
            task = "context"
        else:
            task = "arc"
        arc_feature_min = self._get_int(self.arc_subset_feature_min, 1, 1)
        arc_feature_max = self._get_int(self.arc_subset_feature_max, len(ARC_SWEEP_FEATURES), arc_feature_min)
        arc_max_combos = self._get_int(self.arc_subset_max_combinations, 0, 0)
        ctx_feature_min = self._get_int(self.context_subset_feature_min, 1, 1)
        ctx_feature_max = self._get_int(self.context_subset_feature_max, len(CONTEXT_SWEEP_FEATURES), ctx_feature_min)
        ctx_max_combos = self._get_int(self.context_subset_max_combinations, 0, 0)
        shortlist_depth = max(1, int(round(float(SUBSET_SWEEP_SEARCH_DEPTH) * 0.25)))
        return [
            "--task", task,
            "--out_report", str(self._project_path(self.subset_report.get())),
            "--out_csv", str(self._project_path(self.subset_csv.get())),
            "--context_out_report", str(self._project_path(self.context_subset_report.get())),
            "--context_out_csv", str(self._project_path(self.context_subset_csv.get())),
            "--n_iter", str(SUBSET_SWEEP_SEARCH_DEPTH),
            "--n_jobs", str(self._get_int(self.subset_n_jobs, 1, 0)),
            "--context_repeats", str(SUBSET_SWEEP_SEARCH_DEPTH),
            "--feature_count_min", str(arc_feature_min),
            "--feature_count_max", str(arc_feature_max),
            "--max_combinations", str(arc_max_combos),
            "--arc_time_budget_minutes", f"{self._get_float(self.arc_subset_time_budget_minutes, 60.0, 0.0, None):.6f}",
            "--arc_shard_size", str(self._get_int(self.arc_subset_shard_size, 128, 1)),
            "--arc_keep_per_shard", str(self._get_int(self.arc_subset_keep_per_shard, 1, 1)),
            "--arc_shortlist_size", str(self._get_int(self.arc_subset_shortlist_size, 18, 1)),
            "--arc_finalist_count", str(self._get_int(self.arc_subset_finalist_count, 6, 1)),
            "--arc_shortlist_n_iter", str(shortlist_depth),
            "--arc_tolerance_mode", str((self.arc_tolerance_mode.get() or "soft_positive").strip()),
            "--pre_arc_window", str(self._get_int(self.pre_arc_window, 1, 0)),
            "--post_arc_window", str(self._get_int(self.post_arc_window, 3, 0)),
            "--soft_neighbor_weight", f"{self._get_float(self.soft_neighbor_weight, 0.30, 0.02, 1.0):.6f}",
            "--expanded_neighbor_weight", f"{self._get_float(self.expanded_neighbor_weight, 0.70, 0.10, 1.25):.6f}",
            "--hard_negative_ring", str(self._get_int(self.hard_negative_ring, 2, 0)),
            "--sweep_negative_ratio", f"{self._get_float(self.sweep_negative_ratio, 18.0, 0.0, None):.6f}",
            "--sweep_positive_oversample", f"{self._get_float(self.sweep_positive_oversample, 1.25, 1.0, None):.6f}",
            "--final_negative_ratio", f"{self._get_float(self.final_negative_ratio, 0.0, 0.0, None):.6f}",
            "--final_positive_oversample", f"{self._get_float(self.final_positive_oversample, 1.0, 1.0, None):.6f}",
            "--context_feature_count_min", str(ctx_feature_min),
            "--context_feature_count_max", str(ctx_feature_max),
            "--context_max_combinations", str(ctx_max_combos),
            "--min_recall", f"{self._get_float(self.min_recall_goal, 0.95, 0.0, 1.0):.6f}",
            "--min_precision", f"{self._get_float(self.min_precision_goal, 0.90, 0.0, 1.0):.6f}",
            "--max_fpr", f"{self._get_float(self.max_fpr_goal, 0.03, 0.0, 1.0):.6f}",
            "--min_threshold", f"{self._get_float(self.min_threshold_goal, 0.08, 0.0, 0.99):.6f}",
        ]
    def _winner_mode_arg(self):
        label = (self.winner_mode.get() or "Balanced").strip()
        return {
            "Arc-first": "arc_guard",
            "Balanced": "safety_composite",
            "Legacy": "legacy_cv_ap",
        }.get(label, "safety_composite")
    def _build_training_command(self, kind, n_iter):
        cmd = [
            self.python_exe.get(),
            str(self._project_path(self._trainer_script_var_for_kind(kind).get())),
        ]
        if kind == "rf":
            cmd.extend([
                "--csv", str(self._project_path(self.cleaned_csv.get())),
                "--out_report", str(self._project_path(self.rf_report.get())),
                "--feature_report", str(self._project_path(self.subset_report.get())),
            ])
            cmd.extend(self._arc_training_flags(n_iter))
            cmd.extend(self._explicit_feature_args("arc"))
            return cmd
        if kind == "et":
            cmd.extend([
                "--csv", str(self._project_path(self.cleaned_csv.get())),
                "--out_report", str(self._project_path(self.et_report.get())),
                "--feature_report", str(self._project_path(self.subset_report.get())),
            ])
            cmd.extend(self._arc_training_flags(n_iter))
            cmd.extend(self._explicit_feature_args("arc"))
            return cmd
        if kind == "duel":
            cmd.extend([
                "--csv", str(self._project_path(self.cleaned_csv.get())),
                "--out_report", str(self._project_path(self.duel_report.get())),
                "--rf_out_report", str(self._project_path(self.rf_report.get())),
                "--et_out_report", str(self._project_path(self.et_report.get())),
                "--winner_mode", self._winner_mode_arg(),
                "--feature_report", str(self._project_path(self.subset_report.get())),
            ])
            cmd.extend(self._arc_training_flags(n_iter))
            cmd.extend(self._explicit_feature_args("arc"))
            return cmd
        if kind == "context":
            cmd.extend([
                "--csv", str(self._project_path(self.context_csv.get())),
                "--out_report", str(self._project_path(self.context_report.get())),
                "--feature_report", str(self._project_path(self.context_subset_report.get())),
            ])
            cmd.extend(self._context_training_flags(n_iter))
            cmd.extend(self._explicit_feature_args("context"))
            return cmd
        if kind == "subset":
            context_features = self._selected_feature_names("context")
            if not context_features:
                raise ValueError("Select at least one context feature before starting the subset sweep.")
            cmd.extend([
                "--arc_csv", str(self._project_path(self.cleaned_csv.get())),
                "--context_csv", str(self._project_path(self.context_csv.get())),
            ])
            cmd.extend(self._subset_training_flags())
            cmd.extend(self._explicit_feature_args("arc"))
            cmd.extend(["--context_features", *context_features])
            return cmd
        return cmd
    def _load_training_report_target(self, kind):
        if kind == "rf":
            payload = self._read_json(self.rf_report) or self._get_duel_result_by_key("rf") or {}
            return payload
        if kind == "et":
            payload = self._read_json(self.et_report) or self._get_duel_result_by_key("et") or {}
            return payload
        if kind == "context":
            return self._read_json(self.context_report) or {}
        if kind == "subset":
            return self._read_json(self.subset_report) or {}
        payload = self._read_json(self.duel_report) or {}
        return payload.get("winner") or payload
    def _next_iteration_value(self, current_n_iter, max_n_iter, growth):
        nxt = int(round(float(current_n_iter) * float(growth)))
        if nxt <= int(current_n_iter):
            nxt = int(current_n_iter) + max(8, int(current_n_iter) // 2)
        return min(int(max_n_iter), max(int(current_n_iter) + 1, nxt))
    def _evaluate_escalation_need(self, kind):
        if kind not in {"rf", "et", "duel"}:
            return False, "workflow does not use auto-escalation"
        target = self._load_training_report_target(kind)
        if not target:
            return True, "no report was produced"
        reasons = []
        min_recall = self._get_float(self.min_recall_goal, 0.98, 0.0, 1.0)
        min_precision = self._get_float(self.min_precision_goal, 0.90, 0.0, 1.0)
        max_val_fn = self._get_int(self.auto_max_val_fn, 0, 0)
        max_val_fp = self._get_int(self.auto_max_val_fp, 5, 0)
        constraints_met = target.get("threshold_constraints_met")
        if constraints_met is False:
            reasons.append("threshold constraints not met")
        holdout_recall = self._get_float(target.get("holdout_validation_recall", target.get("validation_recall", 0.0)), 0.0, 0.0, 1.0)
        holdout_precision = self._get_float(target.get("holdout_validation_precision", target.get("validation_precision", 0.0)), 0.0, 0.0, 1.0)
        val = target.get("validation_threshold_result", {}) or {}
        val_fn = self._get_int(val.get("fn", 10**9), 10**9, 0)
        val_fp = self._get_int(val.get("fp", 10**9), 10**9, 0)
        if holdout_recall < min_recall:
            reasons.append(f"holdout recall {holdout_recall:.4f} < {min_recall:.4f}")
        if holdout_precision < min_precision:
            reasons.append(f"holdout precision {holdout_precision:.4f} < {min_precision:.4f}")
        if val_fn > max_val_fn:
            reasons.append(f"validation FN {val_fn} > target {max_val_fn}")
        if val_fp > max_val_fp:
            reasons.append(f"validation FP {val_fp} > target {max_val_fp}")
        return (len(reasons) > 0), "; ".join(reasons)
    def _start_training_workflow(self, kind):
        if self.proc is not None:
            messagebox.showwarning("Busy", "Another process is already running.")
            return
        base_n_iter = self._get_int(self.search_iters, 120, 1)
        max_n_iter = self._get_int(self.max_search_iters, max(base_n_iter, 240), 1)
        growth = self._get_float(self.iter_growth, 2.0, 1.1, 8.0)
        if max_n_iter < base_n_iter:
            max_n_iter = base_n_iter
        workflow = {
            "kind": kind,
            "current_n_iter": base_n_iter,
            "max_n_iter": max_n_iter,
            "growth": growth,
            "attempt": 1,
            "auto_escalate": bool(self.auto_escalate.get()) and kind in {"rf", "et", "duel"},
        }
        self.current_workflow = workflow
        self._launch_training_attempt(workflow)
    def _launch_training_attempt(self, workflow):
        kind = workflow["kind"]
        current_n_iter = workflow["current_n_iter"]
        try:
            cmd = self._build_training_command(kind, current_n_iter)
        except ValueError as exc:
            self.current_workflow = None
            self.status_text.set("Blocked")
            self.progress_text.set(str(exc))
            messagebox.showerror("Feature Selection", str(exc))
            return
        label = {
            "rf": "Random Forest",
            "et": "Extra Trees",
            "duel": "King Training",
            "context": "Context Trainer",
            "subset": "Subset Sweep",
        }.get(kind, kind)
        self.status_text.set(f"Running {label} (n_iter={current_n_iter}, attempt {workflow['attempt']})…")
        self.progress_text.set(f"Preparing {label} search with n_iter={current_n_iter}")
        self._start_subprocess(
            cmd,
            on_finish=lambda rc, wf=workflow: self._after_training_attempt(wf, rc),
        )
    def _after_training_attempt(self, workflow, rc):
        if self.current_workflow is not workflow:
            return
        if rc != 0:
            self.current_workflow = None
            return
        if not workflow.get("auto_escalate"):
            self.current_workflow = None
            return
        should_retry, reason = self._evaluate_escalation_need(workflow["kind"])
        current_n_iter = int(workflow["current_n_iter"])
        max_n_iter = int(workflow["max_n_iter"])
        if should_retry and current_n_iter < max_n_iter:
            next_n_iter = self._next_iteration_value(current_n_iter, max_n_iter, workflow["growth"])
            if next_n_iter > current_n_iter:
                workflow["current_n_iter"] = next_n_iter
                workflow["attempt"] = int(workflow.get("attempt", 1)) + 1
                self.log(
                    f"\nAuto-escalating search for {workflow['kind']} because {reason}. "
                    f"Retrying with n_iter={next_n_iter}.\n"
                )
                self.status_text.set(f"Escalating search to n_iter={next_n_iter}…")
                self.progress_text.set(f"Retrying because {reason}")
                self.after(150, lambda wf=workflow: self._launch_training_attempt(wf))
                return
        self.current_workflow = None
        if should_retry:
            self.log(
                f"\nStopped auto-escalation for {workflow['kind']} at n_iter={current_n_iter}. "
                f"Still below target because {reason}.\n"
            )
        else:
            self.log(
                f"\nAuto-escalation stop condition met for {workflow['kind']} at "
                f"n_iter={current_n_iter}.\n"
            )
    def toggle_paths(self):
        self.paths_visible = not self.paths_visible
        if self.paths_visible:
            self.paths_card.grid(row=1, column=0, sticky="ew", pady=(14, 12))
            self.paths_btn.config(text="Hide Paths")
        else:
            self.paths_card.grid_forget()
            self.paths_btn.config(text="Show Paths")
    def _browse_root(self):
        path = filedialog.askdirectory(initialdir=self.project_root.get() or os.getcwd())
        if path:
            self.project_root.set(path)
    def _browse_file(self, var):
        path = filedialog.askopenfilename(initialdir=self.project_root.get() or os.getcwd())
        if path:
            var.set(path)
    def _format_cleaner_summary(self, data):
        if not data:
            return "Cleaner summary unavailable."
        conflict = data.get("conflict_resolution", {})
        policy = data.get("conflict_policy_counts", {})
        base_after = data.get('rows_after_cleaning', 0)
        aug_rows = data.get('augmentation_rows_added', 0)
        total_after = data.get('rows_after_cleaning_with_augmentation', base_after)
        row_line = f"Rows: {data.get('rows_before_cleaning', 0)} → {base_after}  (removed {data.get('rows_removed', 0)})"
        if aug_rows:
            row_line += f" | +aug {aug_rows} => {total_after}"
        return (
            row_line + "\n"
            f"Trainable: {data.get('trainable_rows', 0)} | Trusted normal: {data.get('trusted_normal_rows', 0)} | Conflict-marked: {data.get('conflict_marked_rows', 0)}\n"
            f"Forced-normal section rows: {data.get('forced_normal_section_rows', 0)} | Unknown-division rows: {data.get('unknown_division_rows', 0)} | Division counts: {data.get('division_counts', {})}\n"
            f"Conflict groups: {conflict.get('mixed_groups', 0)} | Prefer trusted normal: {conflict.get('prefer_trusted_normal', 0)} | Prefer stronger normal: {conflict.get('prefer_more_trusted_normal', 0)} | Keep stronger arc: {conflict.get('keep_more_trusted_arc', 0)} | Dropped ambiguous: {conflict.get('drop_ambiguous', 0)}\n"
            f"Labels: {data.get('label_counts', {})} | Sources: {data.get('source_counts', {})} | Policies kept: {policy}"
        )
    def _reset_progress(self):
        self._progress_payload = None
        self._last_eta_seconds = None
        self._progress_rate_ema = None
        self._last_progress_sample = None
        self._last_progress_stage = None
        self.pb.configure(mode="determinate", maximum=100.0)
        self.pb["value"] = 0.0
        self.progress_text.set("Idle")
        self.progress_percent_text.set("0%")
        self.elapsed_text.set("Elapsed: —")
        self.eta_text.set("ETA: —")
    def _fmt_duration(self, seconds):
        try:
            seconds = int(round(float(seconds)))
        except Exception:
            return "—"
        seconds = max(0, seconds)
        h, rem = divmod(seconds, 3600)
        m, s = divmod(rem, 60)
        return f"{h:d}:{m:02d}:{s:02d}" if h else f"{m:02d}:{s:02d}"
    def _tick_runtime_clock(self):
        try:
            if self.proc is not None and self._process_started_at is not None:
                elapsed = max(0.0, time.monotonic() - self._process_started_at)
                self.elapsed_text.set(f"Elapsed: {self._fmt_duration(elapsed)}")
                if self._last_eta_seconds is not None:
                    self.eta_text.set(f"ETA: {self._fmt_duration(self._last_eta_seconds)}")
                elif self._progress_payload is None:
                    self.eta_text.set("ETA: estimating...")
            elif self.proc is None and self._process_started_at is None and self.progress_text.get() == "Idle":
                self.elapsed_text.set("Elapsed: —")
                self.eta_text.set("ETA: —")
        finally:
            self.after(500, self._tick_runtime_clock)
    def _update_progress(self, payload):
        payload = dict(payload or {})
        current = payload.get("current", 0)
        total = payload.get("total", 0)
        try:
            current = float(current)
            total = float(total)
        except Exception:
            return
        if total <= 0:
            return
        pct = max(0.0, min(100.0, (current / total) * 100.0))
        self._progress_payload = payload
        stage_key = str(payload.get("stage", "")).strip().lower()
        now = time.monotonic()
        if self._process_started_at is not None:
            elapsed = max(0.0, now - self._process_started_at)
            self.elapsed_text.set(f"Elapsed: {self._fmt_duration(elapsed)}")
            if self._last_progress_stage != stage_key:
                self._progress_rate_ema = None
                self._last_progress_sample = (now, current)
                self._last_progress_stage = stage_key
            elif self._last_progress_sample is None:
                self._last_progress_sample = (now, current)
            else:
                dt = max(0.0, now - float(self._last_progress_sample[0]))
                delta = max(0.0, current - float(self._last_progress_sample[1]))
                if dt >= 0.20 and delta > 0:
                    inst_rate = delta / dt
                    if self._progress_rate_ema is None:
                        self._progress_rate_ema = inst_rate
                    else:
                        self._progress_rate_ema = (0.65 * float(self._progress_rate_ema)) + (0.35 * inst_rate)
                    self._last_progress_sample = (now, current)
                elif dt >= 2.0:
                    self._last_progress_sample = (now, current)
            if current >= total:
                self._last_eta_seconds = 0.0
            elif self._progress_rate_ema is not None and float(self._progress_rate_ema) > 0:
                remaining = max(0.0, (total - current) / float(self._progress_rate_ema))
                self._last_eta_seconds = remaining
                self.eta_text.set(f"ETA: {self._fmt_duration(remaining)}")
            else:
                self._last_eta_seconds = None
                self.eta_text.set("ETA: estimating...")
        self.pb.configure(mode="determinate", maximum=100.0)
        self.pb["value"] = pct
        task_name = str(payload.get("task", "")).strip()
        model_name = str(payload.get("model_name", "")).strip()
        stage = str(payload.get("stage", "")).strip().replace("_", " ").title()
        message = str(payload.get("message", "")).strip()
        combo_index = payload.get("combo_index")
        combo_total = payload.get("combo_total")
        pieces = []
        if task_name:
            pieces.append(task_name.replace("_", " ").title())
        if model_name:
            pieces.append(model_name)
        if stage:
            pieces.append(stage)
        if message:
            pieces.append(message)
        if combo_index is not None and combo_total is not None:
            try:
                pieces.append(f"Combo {int(combo_index)}/{int(combo_total)}")
            except Exception:
                pass
        if not pieces:
            pieces.append(f"Step {int(current)}/{int(total)}")
        if self._last_eta_seconds is not None and current < total:
            pieces.append(f"ETA {self._fmt_duration(self._last_eta_seconds)}")
        self.progress_text.set(" • ".join(pieces))
        self.progress_percent_text.set(f"{pct:0.1f}%")
        stage_key = str(payload.get("stage", "")).strip().lower()
        if payload.get("all_models_done"):
            self.status_text.set("Finished")
        elif stage_key in {"done", "workflow_done"}:
            self.status_text.set("Finalizing…")
    def _handle_progress_line(self, msg):
        prefix = "[[PROGRESS]] "
        if not msg.startswith(prefix):
            return False
        payload = msg[len(prefix):].strip()
        try:
            data = json.loads(payload)
        except Exception:
            self.log_queue.put(msg)
            return True
        self._update_progress(data)
        return True
    def _show_cleaner_summary_dialog(self, data):
        if not data:
            return
        win = tk.Toplevel(self)
        win.title("Cleaner Summary")
        win.configure(bg=APP_BG)
        win.transient(self)
        win.grab_set()
        win.geometry("760x440")
        win.minsize(680, 380)
        outer = tk.Frame(win, bg=APP_BG, padx=16, pady=16)
        outer.pack(fill="both", expand=True)
        card = tk.Frame(
            outer,
            bg=CARD_BG,
            highlightthickness=1,
            highlightbackground=BORDER,
            padx=14,
            pady=14,
        )
        card.pack(fill="both", expand=True)
        tk.Label(
            card,
            text="Latest Cleaner Summary",
            bg=CARD_BG,
            fg=TEXT,
            font=("Segoe UI", 12, "bold"),
        ).pack(anchor="w")
        summary = tk.Text(
            card,
            wrap="word",
            font=("Segoe UI", 10),
            bg=LOG_BG,
            fg=TEXT,
            relief="flat",
            padx=12,
            pady=12,
            height=14,
        )
        summary.pack(fill="both", expand=True, pady=(10, 12))
        summary.insert("1.0", self._format_cleaner_summary(data))
        summary.configure(state="disabled")
        btn_row = tk.Frame(card, bg=CARD_BG)
        btn_row.pack(fill="x")
        ttk.Button(btn_row, text="Close", command=win.destroy).pack(side="right")
    def _handle_cleaner_summary_line(self, msg):
        prefix = "__CLEANER_SUMMARY__ "
        if not msg.startswith(prefix):
            return False
        payload = msg[len(prefix):].strip()
        try:
            data = json.loads(payload)
        except Exception as exc:
            self.cleaner_summary_text.set(f"Cleaner summary parse failed: {exc}")
            self.log_queue.put(msg)
            return True
        self._pending_cleaner_summary = data
        self.cleaner_summary_text.set(self._format_cleaner_summary(data))
        pretty = self._format_cleaner_summary(data)
        self.log_queue.put("\n[Cleaner Summary]\n" + pretty + "\n")
        return True
    def _drain_logs(self):
        try:
            while True:
                msg = self.log_queue.get_nowait()
                if self._handle_progress_line(msg):
                    continue
                if self._handle_cleaner_summary_line(msg):
                    continue
                self.log_text.insert("end", msg)
                self.log_text.see("end")
        except queue.Empty:
            pass
        self.after(120, self._drain_logs)
    def log(self, msg):
        self.log_queue.put(msg)
    def _on_window_resize(self, _event=None):
        width = max(680, self.winfo_width())
        for label, frac, floor in getattr(self, "_adaptive_labels", []):
            try:
                label.configure(wraplength=max(int(width * frac), int(floor)))
            except Exception:
                pass
    def _project_path(self, rel_or_abs):
        p = Path(rel_or_abs)
        if p.is_absolute():
            return p
        return Path(self.project_root.get()) / rel_or_abs
    def _finalize_subprocess(self, rc, cmd, on_finish=None):
        elapsed = None
        if self._process_started_at is not None:
            elapsed = max(0.0, time.monotonic() - self._process_started_at)
        self.status_text.set(f"Finished (exit code {rc})")
        if self._progress_payload is not None:
            self._update_progress(self._progress_payload)
        elif rc == 0:
            self.pb.configure(mode="determinate", maximum=100.0)
            self.pb.configure(value=100.0)
            self.progress_percent_text.set("100.0%")
            self.progress_text.set("Done")
        if elapsed is not None:
            self.log(f"Total elapsed time: {self._fmt_duration(elapsed)} ({elapsed:.2f}s)\n")
            self.elapsed_text.set(f"Elapsed: {self._fmt_duration(elapsed)}")
            self.eta_text.set("ETA: done")
        if rc == 0 and cmd and Path(cmd[1]).name == "prepare_data.py" and self._pending_cleaner_summary:
            self._show_cleaner_summary_dialog(self._pending_cleaner_summary)
        self._process_started_at = None
        self._last_eta_seconds = None
        self._progress_rate_ema = None
        self._last_progress_sample = None
        self._last_progress_stage = None
        if on_finish is not None:
            on_finish(rc)
        self.refresh_views()
    def _handle_subprocess_failure(self, message):
        self.status_text.set("Failed")
        self.progress_text.set(f"Failed: {message}")
        self._process_started_at = None
        self._last_eta_seconds = None
        self._progress_rate_ema = None
        self._last_progress_sample = None
        self._last_progress_stage = None
        self.refresh_views()
    def _start_subprocess(self, cmd, on_finish=None):
        if self.proc is not None:
            messagebox.showwarning("Busy", "Another process is already running.")
            return
        self._reset_progress()
        self._process_started_at = time.monotonic()
        self._last_progress_sample = (self._process_started_at, 0.0)
        self._last_progress_stage = ""
        self.status_text.set("Running…")
        self.progress_text.set("Starting process…")
        self.progress_percent_text.set("0%")
        if cmd and Path(cmd[1]).name == "prepare_data.py":
            self._pending_cleaner_summary = None
        self.last_command.set(" ".join(cmd))
        self.elapsed_text.set("Elapsed: 00:00")
        self.eta_text.set("ETA: estimating...")
        self.log("\n" + "=" * 100 + "\n")
        self.log("Running: " + " ".join(cmd) + "\n\n")
        def worker():
            try:
                self.proc = subprocess.Popen(
                    cmd,
                    cwd=self.project_root.get(),
                    stdout=subprocess.PIPE,
                    stderr=subprocess.STDOUT,
                    text=True,
                    bufsize=1,
                )
                for line in self.proc.stdout:
                    self.log(line)
                rc = self.proc.wait()
                self.log(f"\nProcess finished with exit code {rc}\n")
                self.after(0, lambda rc=rc, cmd=cmd, on_finish=on_finish: self._finalize_subprocess(rc, cmd, on_finish))
            except Exception as e:
                self.log(f"\nERROR: {e}\n")
                self.after(0, lambda message=str(e): self._handle_subprocess_failure(message))
            finally:
                self.proc = None
        threading.Thread(target=worker, daemon=True).start()
    def stop_running(self):
        self.current_workflow = None
        if self.proc is None:
            return
        try:
            self.proc.terminate()
            self.log("\nRequested process termination.\n")
            self.status_text.set("Stopping…")
            self.progress_text.set("Stopping process…")
        except Exception as e:
            self.log(f"\nStop failed: {e}\n")
    def run_cleaner(self):
        self.current_workflow = None
        self.workflow_scope_text.set("Running Cleaner. This workflow rebuilds the prepared arc/context datasets and cleaner report only; it does not change the selected final-training feature lists.")
        self.status_text.set("Running Cleaner…")
        self.progress_text.set("Cleaner rebuilds arc_training.csv and load_context.csv while preserving the full 16-feature logging schema.")
        self._start_subprocess([
            self.python_exe.get(),
            str(self._project_path(self.prepare_script.get())),
            "--output", str(self._project_path(self.cleaned_csv.get())),
            "--context_output", str(self._project_path(self.context_csv.get())),
            "--out_report", str(self._project_path(self.prepare_report.get())),
        ])
    def run_rf_only(self):
        self.workflow_scope_text.set("Running Random Forest on the currently selected arc base features. Export/runtime metadata will keep those base inputs explicit and append the fixed 7 context inputs automatically.")
        self._start_training_workflow("rf")
    def run_et_only(self):
        self.workflow_scope_text.set("Running Extra Trees on the currently selected arc base features. Export/runtime metadata will keep those base inputs explicit and append the fixed 7 context inputs automatically.")
        self._start_training_workflow("et")
    def run_duel(self):
        self.workflow_scope_text.set("Running King Training on the currently selected arc base features. RF and ET benchmark the same selected arc inputs, then the winner export carries explicit input metadata for firmware assembly.")
        self._start_training_workflow("duel")
    def run_context(self):
        self.workflow_scope_text.set("Running Context Trainer on the currently selected context-only features using start-data rows from load_context.csv. Exported context metadata will drive firmware-side input ordering.")
        self._start_training_workflow("context")
    def run_subset_sweep(self):
        self.workflow_scope_text.set(f"Running Subset Sweep. Arc sweep uses the locked 12-feature arc pool and evaluates RF + ET together at depth {SUBSET_SWEEP_SEARCH_DEPTH} per model. Context sweep uses the locked 10-feature start-data pool and runs separately at {SUBSET_SWEEP_SEARCH_DEPTH} repeats per combo.")
        self._start_training_workflow("subset")
    def refresh_views(self):
        self._refresh_dataset_view()
        self._refresh_prepare_report()
        self._refresh_compare_view()
        self._refresh_single_report("rf")
        self._refresh_single_report("et")
        self._refresh_single_report("duel")
        self._refresh_single_report("context")
        self._refresh_subset_report()
        self._auto_apply_feature_recommendations(force=False)
    def _refresh_dataset_view(self):
        for row in self.dataset_tree.get_children():
            self.dataset_tree.delete(row)
        if pd is None:
            self.dataset_summary.set("pandas is not installed. Dataset view unavailable.")
            return
        csv_path = self._project_path(self.cleaned_csv.get())
        if not csv_path.exists():
            self.dataset_summary.set(f"Dataset not found: {csv_path}")
            return
        try:
            df = pd.read_csv(csv_path)
        except Exception as e:
            self.dataset_summary.set(f"Failed to read dataset: {e}")
            return
        rows = len(df)
        trainable = int(df["rf_train_row"].sum()) if "rf_train_row" in df.columns else rows
        label_counts = df["label_arc"].value_counts().to_dict() if "label_arc" in df.columns else {}
        source_counts = df["source_kind"].value_counts().to_dict() if "source_kind" in df.columns else {}
        division_counts = df["division_tag"].fillna("").replace("", "unknown").value_counts().to_dict() if "division_tag" in df.columns else {}
        trusted_normal = int((df["trusted_normal_session"] == 1).sum()) if "trusted_normal_session" in df.columns else 0
        conflict_rows = int((df["label_conflict"] == 1).sum()) if "label_conflict" in df.columns else 0
        trial_groups = int(df["trial_id"].astype(str).nunique()) if "trial_id" in df.columns else 0
        section_groups = int(df["session_id"].astype(str).nunique()) if "session_id" in df.columns else 0
        conflict_policy_counts = df["conflict_policy"].value_counts().to_dict() if "conflict_policy" in df.columns else {}
        family_counts = df["device_family"].value_counts().to_dict() if "device_family" in df.columns else {}
        self.dataset_summary.set(
            f"Rows: {rows}\n"
            f"Trainable rows: {trainable}\n"
            f"Trusted normal rows: {trusted_normal}\n"
            f"Conflict-marked rows: {conflict_rows}\n"
            f"Trial groups: {trial_groups} | Section groups: {section_groups}\n"
            f"Label counts: {label_counts}\n"
            f"Division counts: {division_counts}\n"
            f"Family counts: {family_counts}\n"
            f"Source counts: {source_counts}\n"
            f"Conflict policies: {conflict_policy_counts}"
        )
        feature_cols = list(ALL_COMPUTED_FEATURES)
        present = [c for c in feature_cols if c in df.columns]
        if not present:
            return
        stats = df[present].describe().T[["mean", "std", "min", "max"]]
        for feat, row in stats.iterrows():
            self.dataset_tree.insert(
                "",
                "end",
                values=(
                    feat,
                    f"{row['mean']:.6f}",
                    f"{row['std']:.6f}",
                    f"{row['min']:.6f}",
                    f"{row['max']:.6f}",
                ),
            )
    def _read_json(self, path_var):
        path = self._project_path(path_var.get())
        if not path.exists():
            return None
        try:
            return json.loads(path.read_text(encoding="utf-8"))
        except Exception:
            return None
    def _get_duel_result_by_key(self, model_key):
        duel = self._read_json(self.duel_report) or {}
        for item in duel.get("all_results", []) or []:
            if item.get("model_key") == model_key:
                return item
        return None
    def _resolve_report_payload(self, prefix):
        if prefix == "rf":
            standalone = self._read_json(self.rf_report)
            if standalone:
                return standalone, "Standalone Random Forest report"
            duel_item = self._get_duel_result_by_key("rf")
            if duel_item:
                return duel_item, "From King training benchmark"
            return None, "No report loaded."
        if prefix == "et":
            standalone = self._read_json(self.et_report)
            if standalone:
                return standalone, "Standalone Extra Trees report"
            duel_item = self._get_duel_result_by_key("et")
            if duel_item:
                return duel_item, "From King training benchmark"
            return None, "No report loaded."
        if prefix == "context":
            report = self._read_json(self.context_report)
            if report:
                return report, "Standalone context-family report"
            return None, "No report loaded."
        duel = self._read_json(self.duel_report)
        if duel:
            return duel, "King training benchmark report"
        return None, "No report loaded."
    def _refresh_prepare_report(self):
        if not hasattr(self, "prepare_metric_tree"):
            return
        for tree in (self.prepare_metric_tree, self.prepare_feature_tree):
            for row in tree.get_children():
                tree.delete(row)
        report = self._read_json(self.prepare_report)
        if not report:
            self.prepare_summary.set("No prepare report loaded.")
            return
        summary = report.get("summary", {}) or {}
        outputs = report.get("outputs", {}) or {}
        split_checks = report.get("split_checks", {}) or {}
        timing = report.get("timing", {}) or {}
        elapsed_seconds = float(
            timing.get("duration_seconds", report.get("elapsed_seconds", summary.get("elapsed_seconds", 0.0))) or 0.0
        )
        arc_rows = outputs.get("arc_training_rows", summary.get("arc_training_rows", "â€”"))
        context_rows = outputs.get("load_context_rows", summary.get("load_context_rows", "â€”"))
        arc_divisions = {
            str(k).strip().lower()
            for k, v in (split_checks.get("arc_training_division_counts", {}) or {}).items()
            if float(v or 0) > 0.0
        }
        context_divisions = {
            str(k).strip().lower()
            for k, v in (split_checks.get("load_context_division_counts", {}) or {}).items()
            if float(v or 0) > 0.0
        }
        arc_label_counts = {
            str(k): int(v)
            for k, v in (split_checks.get("arc_training_label_counts", {}) or {}).items()
            if int(v or 0) > 0
        }
        context_family_counts = {
            str(k).strip().lower(): int(v)
            for k, v in (split_checks.get("load_context_family_counts", {}) or {}).items()
            if int(v or 0) > 0 and str(k).strip()
        }
        sanity = {
            "arc_contains_only_steady_or_arc": bool(arc_divisions) and arc_divisions.issubset({"steady", "arc"}),
            "context_contains_only_start": bool(context_divisions) and context_divisions.issubset({"start", "startup"}),
            "start_rows_excluded_from_arc_training": "start" not in arc_divisions and "startup" not in arc_divisions,
            "arc_rows_excluded_from_context": "arc" not in context_divisions,
            "arc_dataset_has_both_labels": {"0", "1"}.issubset(set(arc_label_counts.keys())),
            "context_has_known_families": any(k not in {"unknown", "nan", "none", ""} for k in context_family_counts.keys()),
        }
        self.prepare_summary.set(
            f"Source: {self._project_path(self.prepare_report.get())}\n"
            f"Rows before/after: {summary.get('rows_before_cleaning', '—')} → {summary.get('rows_after_cleaning_with_augmentation', summary.get('rows_after_cleaning', '—'))}\n"
            f"Arc rows: {summary.get('arc_training_rows', '—')} | Context rows: {summary.get('load_context_rows', '—')} | Duration: {self._fmt_duration(timing.get('duration_seconds', summary.get('duration_seconds', 0.0)))}"
        )
        self.prepare_summary.set(
            "Source: "
            + str(self._project_path(self.prepare_report.get()))
            + "\nRows before/after: "
            + str(summary.get("rows_before_cleaning", "-"))
            + " -> "
            + str(summary.get("rows_after_cleaning_with_augmentation", summary.get("rows_after_cleaning", "-")))
            + "\nArc rows: "
            + str(arc_rows)
            + " | Context rows: "
            + str(context_rows)
            + " | Duration: "
            + str(self._fmt_duration(elapsed_seconds))
        )
        metrics = [
            ("Input file count", self._pick(report, ("input_files", "count"))),
            ("Rows before cleaning", summary.get("rows_before_cleaning")),
            ("Rows after cleaning", summary.get("rows_after_cleaning")),
            ("Rows removed", summary.get("rows_removed")),
            ("Augmentation rows added", summary.get("augmentation_rows_added")),
            ("Rows after augmentation", summary.get("rows_after_cleaning_with_augmentation")),
            ("Arc training rows", summary.get("arc_training_rows")),
            ("Load context rows", summary.get("load_context_rows")),
            ("Trainable rows", summary.get("trainable_rows")),
            ("Trusted normal rows", summary.get("trusted_normal_rows")),
            ("Conflict-marked rows", summary.get("conflict_marked_rows")),
            ("Division counts (cleaned)", self._pick(report, ("division_counts", "cleaned_all"))),
            ("Division counts (arc)", self._pick(report, ("division_counts", "arc_training"))),
            ("Division counts (context)", self._pick(report, ("division_counts", "load_context"))),
            ("Arc only steady/arc", sanity.get("arc_contains_only_steady_or_arc")),
            ("Context only start", sanity.get("context_contains_only_start")),
            ("Start rows excluded from arc", sanity.get("start_rows_excluded_from_arc_training")),
            ("Arc rows excluded from context", sanity.get("arc_rows_excluded_from_context")),
            ("Arc dataset has both labels", sanity.get("arc_dataset_has_both_labels")),
            ("Context has known families", sanity.get("context_has_known_families")),
            ("Warnings", report.get("warnings", [])),
        ]
        for label, value in metrics:
            self.prepare_metric_tree.insert("", "end", values=(label, self._fmt(value)))
        feature_stats = report.get("feature_stats", {}) or {}
        for dataset_name in ("arc_training", "load_context"):
            for row in feature_stats.get(dataset_name, []) or []:
                self.prepare_feature_tree.insert(
                    "",
                    "end",
                    values=(dataset_name, row.get("feature", "—"), self._fmt(row.get("mean")), self._fmt(row.get("std")), self._fmt(row.get("min")), self._fmt(row.get("max"))),
                )
    def _refresh_compare_view(self):
        for row in self.benchmark_tree.get_children():
            self.benchmark_tree.delete(row)
        for row in self.importance_tree.get_children():
            self.importance_tree.delete(row)
        duel = self._read_json(self.duel_report) or {}
        duel_results = duel.get("all_results", []) or []
        duel_winner_name = (duel.get("winner") or {}).get("model_name")
        benchmark_rows = []
        if duel_results:
            for item in duel_results:
                benchmark_rows.append((item, "King training"))
        else:
            rf = self._read_json(self.rf_report)
            et = self._read_json(self.et_report)
            if rf:
                benchmark_rows.append((rf, "Standalone"))
            if et:
                benchmark_rows.append((et, "Standalone"))
        if not benchmark_rows:
            self.compare_summary.set("No reports loaded.")
            return
        ordered = sorted(
            benchmark_rows,
            key=lambda pair: (
                -(pair[0].get("cv_best_average_precision") or float("-inf")),
                self._pick(pair[0], ("validation_threshold_result", "fp")) if isinstance(self._pick(pair[0], ("validation_threshold_result", "fp")), int) else 10**9,
                self._pick(pair[0], ("validation_threshold_result", "fn")) if isinstance(self._pick(pair[0], ("validation_threshold_result", "fn")), int) else 10**9,
                pair[0].get("estimated_node_count") or 10**9,
            ),
        )
        self.compare_summary.set(
            f"Benchmark rows shown: {len(ordered)}\n"
            f"King report loaded: {'yes' if duel else 'no'}\n"
            f"Winner: {duel_winner_name or (ordered[0][0].get('model_name') if ordered else '—')}\n"
        )
        for idx, (report, source) in enumerate(ordered, start=1):
            cm = report.get("test_confusion_matrix", {})
            is_winner = report.get("model_name") == duel_winner_name if duel_winner_name else idx == 1
            self.benchmark_tree.insert(
                "",
                "end",
                values=(
                    idx,
                    report.get("model_name", "—"),
                    source,
                    self._fmt(report.get("cv_best_average_precision")),
                    self._fmt(report.get("test_average_precision")),
                    self._fmt(report.get("test_roc_auc")),
                    self._fmt(report.get("test_accuracy")),
                    self._fmt(report.get("test_balanced_accuracy")),
                    self._fmt(report.get("test_f1")),
                    self._fmt(report.get("test_recall")),
                    self._fmt(report.get("test_specificity")),
                    self._fmt(report.get("threshold")),
                    self._fmt(self._pick(report, ("validation_threshold_result", "fp"))),
                    self._fmt(self._pick(report, ("validation_threshold_result", "fn"))),
                    self._fmt(cm.get("tn", "—")),
                    self._fmt(cm.get("fp", "—")),
                    self._fmt(cm.get("fn", "—")),
                    self._fmt(cm.get("tp", "—")),
                    self._fmt(report.get("estimated_node_count")),
                    "YES" if is_winner else "",
                ),
            )
        rf = self._get_duel_result_by_key("rf") or self._read_json(self.rf_report) or {}
        et = self._get_duel_result_by_key("et") or self._read_json(self.et_report) or {}
        rf_imp = rf.get("feature_importances", {})
        et_imp = et.get("feature_importances", {})
        for feat in sorted(set(rf_imp) | set(et_imp)):
            rv = float(rf_imp.get(feat, 0.0) or 0.0)
            ev = float(et_imp.get(feat, 0.0) or 0.0)
            if abs(rv - ev) <= 1e-12:
                leader = "Tie"
            else:
                leader = "Random Forest" if rv > ev else "Extra Trees"
            self.importance_tree.insert(
                "",
                "end",
                values=(feat, self._fmt(rv), self._fmt(ev), leader),
            )
    def _refresh_subset_report(self):
        if not hasattr(self, "subset_metric_tree"):
            return
        for tree in (self.subset_metric_tree, self.subset_best_tree):
            for row in tree.get_children():
                tree.delete(row)
        report = self._read_json(self.subset_report)
        context_report = self._read_json(self.context_subset_report)
        if not report and not context_report:
            self.subset_summary.set("No subset sweep report loaded.")
            return
        settings = (report or {}).get("settings", {}) or {}
        context_settings = (context_report or {}).get("settings", {}) or {}
        arc_src = self._project_path(self.subset_report.get())
        ctx_src = self._project_path(self.context_subset_report.get())
        if report and context_report:
            self.subset_summary.set(
                f"Arc report: {arc_src}\n"
                f"Context report: {ctx_src}\n"
                f"Arc combinations: {report.get('total_combinations', '—')} | Context combinations: {context_report.get('total_combinations', '—')}"
            )
        elif report:
            self.subset_summary.set(
                f"Arc report: {arc_src}\n"
                f"Models: {report.get('models', '—')} | Total combinations: {report.get('total_combinations', '—')}\n"
                f"CSV: {report.get('all_results_csv', self._project_path(self.subset_csv.get()))}"
            )
        else:
            self.subset_summary.set(
                f"Context report: {ctx_src}\n"
                f"Total combinations: {context_report.get('total_combinations', '—')}\n"
                f"CSV: {context_report.get('all_results_csv', self._project_path(self.context_subset_csv.get()))}"
            )
        metrics = [
            ("Arc task", report.get("task") if report else "—"),
            ("Arc feature pool size", report.get("feature_pool_size") if report else "—"),
            ("Arc total combinations", report.get("total_combinations") if report else "—"),
            ("Arc feature count min", settings.get("feature_count_min") if report else "—"),
            ("Arc feature count max", settings.get("feature_count_max") if report else "—"),
            ("Arc max combinations", settings.get("max_combinations") if report else "—"),
            ("Arc n_iter per model", settings.get("n_iter") if report else "—"),
            ("Context task", context_report.get("task") if context_report else "—"),
            ("Context feature pool size", context_report.get("feature_pool_size") if context_report else "—"),
            ("Context total combinations", context_report.get("total_combinations") if context_report else "—"),
            ("Context repeats", context_settings.get("context_repeats") if context_report else "—"),
        ]
        for label, value in metrics:
            self.subset_metric_tree.insert("", "end", values=(label, self._fmt(value)))
        buckets = []
        if report:
            buckets.extend([
                ("Arc overall best combined", report.get("overall_best_combined_tradeoff") or {}),
                ("Arc best RF", report.get("overall_best_rf") or {}),
                ("Arc best ET", report.get("overall_best_et") or {}),
            ])
            for entry in report.get("best_by_feature_count", []) or []:
                buckets.append((f"Arc best count={entry.get('feature_count', '—')}", entry.get("best_combined") or {}))
        if context_report:
            buckets.append(("Context overall best", context_report.get("overall_best_tradeoff") or {}))
            for entry in context_report.get("best_by_feature_count", []) or []:
                buckets.append((f"Context best count={entry.get('feature_count', '—')}", entry.get("best_overall") or {}))
        for bucket, row in buckets:
            combo = row.get("feature_combo") or "|".join(row.get("features", []) or [])
            self.subset_best_tree.insert(
                "",
                "end",
                values=(
                    bucket,
                    self._fmt(row.get("feature_count")),
                    self._fmt(row.get("validation_fn", row.get("combined_validation_fn", row.get("mean_accuracy")))),
                    self._fmt(row.get("validation_fp", row.get("combined_validation_fp", row.get("mean_balanced_accuracy")))),
                    self._fmt(row.get("test_fn", row.get("combined_test_fn", row.get("mean_unknown_rate")))),
                    self._fmt(row.get("test_fp", row.get("combined_test_fp", row.get("mean_confidence")))),
                    self._fmt(row.get("validation_recall", row.get("combined_validation_recall_mean", row.get("mean_accuracy")))),
                    self._fmt(row.get("validation_precision", row.get("combined_validation_precision_mean", row.get("mean_balanced_accuracy")))),
                    combo,
                ),
            )
    def _refresh_single_report(self, prefix):
        summary_var = getattr(self, f"{prefix}_summary")
        metric_tree = getattr(self, f"{prefix}_metric_tree")
        class_tree = getattr(self, f"{prefix}_class_tree")
        cm_tree = getattr(self, f"{prefix}_cm_tree")
        param_tree = getattr(self, f"{prefix}_param_tree")
        fi_tree = getattr(self, f"{prefix}_fi_tree")
        notebook = getattr(self, f"{prefix}_notebook", None)
        for tree in (metric_tree, class_tree, cm_tree, param_tree, fi_tree):
            for row in tree.get_children():
                tree.delete(row)
        report, source_note = self._resolve_report_payload(prefix)
        if not report:
            summary_var.set(source_note)
            return
        if prefix == "duel":
            winner = report.get("winner", {})
            all_results = report.get("all_results", []) or []
            target = winner
            summary_var.set(
                f"Source: {source_note}\n"
                f"Winner: {winner.get('model_name', '—')}\n"
                f"Benchmarked models: {', '.join([r.get('model_name', '—') for r in all_results]) or '—'}"
            )
        else:
            target = report
            if prefix == "context":
                summary_var.set(
                    f"Source: {source_note}\n"
                    f"Model: {target.get('model_name', '—')}\n"
                    f"Rows: {target.get('row_count', '—')} | Train: {target.get('train_rows', '—')} | Test: {target.get('test_rows', '—')}\n"
                    f"Accuracy: {self._fmt(target.get('accuracy'))} | Balanced accuracy: {self._fmt(target.get('balanced_accuracy'))} | Mean confidence: {self._fmt(target.get('mean_confidence'))} | Unknown rate: {self._fmt(target.get('unknown_rate'))}"
                )
            else:
                summary_var.set(
                    f"Source: {source_note}\n"
                    f"Model: {target.get('model_name', '—')}\n"
                    f"Threshold: {self._fmt(target.get('threshold', '—'))}"
                )
        if prefix == "context":
            if notebook is not None:
                notebook.tab(getattr(self, f"{prefix}_class_tab"), text="Per-class Metrics")
                notebook.tab(getattr(self, f"{prefix}_cm_tab"), text="Multiclass Confusion")
                notebook.hide(getattr(self, f"{prefix}_params_tab"))
                notebook.hide(getattr(self, f"{prefix}_fi_tab"))
            context_metrics = [
                ("Row count", target.get("row_count")),
                ("Train rows", target.get("train_rows")),
                ("Test rows", target.get("test_rows")),
                ("Accuracy", target.get("accuracy")),
                ("Balanced accuracy", target.get("balanced_accuracy")),
                ("Accuracy with unknown gate", target.get("accuracy_with_unknown_gate")),
                ("Mean confidence", target.get("mean_confidence")),
                ("Unknown rate", target.get("unknown_rate")),
                ("Unknown confidence threshold", target.get("unknown_confidence_threshold")),
                ("Class distribution", target.get("class_counts")),
            ]
            for label, value in context_metrics:
                metric_tree.insert("", "end", values=(label, self._fmt(value)))
            class_tree.configure(columns=("family", "rows", "precision", "recall", "f1", "accuracy", "conf", "unknown"))
            for c, title, width in [("family", "Family", 180), ("rows", "Rows", 80), ("precision", "Precision", 100), ("recall", "Recall", 100), ("f1", "F1", 100), ("accuracy", "Accuracy", 100), ("conf", "Mean conf", 110), ("unknown", "Unknown rate", 110)]:
                class_tree.heading(c, text=title)
                class_tree.column(c, width=width, anchor="center")
            for row in target.get("per_class", []) or []:
                class_tree.insert("", "end", values=(row.get("family", "—"), self._fmt(row.get("rows")), self._fmt(row.get("precision")), self._fmt(row.get("recall")), self._fmt(row.get("f1")), self._fmt(row.get("accuracy")), self._fmt(row.get("mean_confidence")), self._fmt(row.get("unknown_rate"))))
            labels = target.get("class_labels", []) or []
            matrix = target.get("confusion_matrix", []) or []
            cm_cols = tuple(["label"] + [f"c{i}" for i in range(len(labels))])
            cm_tree.configure(columns=cm_cols)
            cm_tree.heading("label", text="Actual \\ Pred")
            cm_tree.column("label", width=180, anchor="w")
            for i, fam in enumerate(labels):
                col = f"c{i}"
                cm_tree.heading(col, text=fam)
                cm_tree.column(col, width=100, anchor="center")
            for idx, fam in enumerate(labels):
                row_vals = matrix[idx] if idx < len(matrix) else []
                values = [f"Actual {fam}"] + [int(row_vals[j]) if j < len(row_vals) else 0 for j in range(len(labels))]
                cm_tree.insert("", "end", values=tuple(values))
        else:
            if notebook is not None:
                notebook.add(getattr(self, f"{prefix}_params_tab"), text="Best Params") if str(getattr(self, f"{prefix}_params_tab")) not in notebook.tabs() else None
                notebook.add(getattr(self, f"{prefix}_fi_tab"), text="Feature Importances") if str(getattr(self, f"{prefix}_fi_tab")) not in notebook.tabs() else None
            for label, key in DISPLAY_METRICS:
                metric_tree.insert("", "end", values=(label, self._fmt(self._pick(target, key))))
            class_report = target.get("test_classification_report", {}) or {}
            for label in ["0", "1", "macro avg", "weighted avg"]:
                row = class_report.get(label)
                if isinstance(row, dict):
                    class_tree.insert(
                        "",
                        "end",
                        values=(
                            label,
                            self._fmt(row.get("precision")),
                            self._fmt(row.get("recall")),
                            self._fmt(row.get("f1-score")),
                            self._fmt(row.get("support")),
                        ),
                    )
            for row in target.get("test_per_load", []) or []:
                class_tree.insert(
                    "",
                    "end",
                    values=(
                        f"load:{row.get('group', '—')}",
                        self._fmt(row.get("precision")),
                        self._fmt(row.get("recall")),
                        self._fmt(row.get("fpr")),
                        self._fmt(row.get("rows")),
                    ),
                )
            cm = target.get("test_confusion_matrix", {}) or {}
            cm_tree.insert("", "end", values=("Actual 0", self._fmt(cm.get("tn", "—")), self._fmt(cm.get("fp", "—"))))
            cm_tree.insert("", "end", values=("Actual 1", self._fmt(cm.get("fn", "—")), self._fmt(cm.get("tp", "—"))))
            for row in target.get("test_per_load", []) or []:
                cmg = row.get("confusion_matrix", {}) or {}
                cm_tree.insert("", "end", values=(f"Load {row.get('group', '—')}", f"TN={cmg.get('tn', '—')} FP={cmg.get('fp', '—')}", f"FN={cmg.get('fn', '—')} TP={cmg.get('tp', '—')}"))
            for row in target.get("test_per_runtime_context", []) or []:
                cmg = row.get("confusion_matrix", {}) or {}
                cm_tree.insert("", "end", values=(f"Ctx {row.get('group', '—')}", f"TN={cmg.get('tn', '—')} FP={cmg.get('fp', '—')}", f"FN={cmg.get('fn', '—')} TP={cmg.get('tp', '—')}"))
        params = target.get("best_params", {}) or {}
        for name in sorted(params):
            param_tree.insert("", "end", values=(name, self._fmt(params[name])))
        fi = target.get("feature_importances", {}) or {}
        for idx, (feat, value) in enumerate(sorted(fi.items(), key=lambda kv: float(kv[1]), reverse=True), start=1):
            fi_tree.insert("", "end", values=(idx, feat, self._fmt(value)))
    def _feature_list_text(self, feature_names):
        names = [str(x).strip() for x in (feature_names or []) if str(x).strip()]
        return ", ".join(names) if names else "â€”"
    def _arc_base_features_from_payload(self, payload):
        names = self._extract_feature_list(
            payload,
            "arc_base_feature_names",
            "feature_names",
            ("winner", "arc_base_feature_names"),
            ("winner", "feature_names"),
        )
        return [name for name in names if name in ARC_SWEEP_FEATURES]
    def _arc_context_inputs_from_payload(self, payload):
        names = self._extract_feature_list(payload, "arc_context_feature_names", ("winner", "arc_context_feature_names"))
        if names:
            return names
        base_names = set(self._arc_base_features_from_payload(payload))
        all_names = self._extract_feature_list(payload, "feature_names", ("winner", "feature_names"))
        return [name for name in all_names if name not in base_names]
    def _refresh_prepare_report(self):
        if not hasattr(self, "prepare_metric_tree"):
            return
        for tree in (self.prepare_metric_tree, self.prepare_feature_tree):
            for row in tree.get_children():
                tree.delete(row)
        report = self._read_json(self.prepare_report)
        if not report:
            self.prepare_summary.set("No prepare report loaded.")
            return
        summary = report.get("summary", {}) or {}
        outputs = report.get("outputs", {}) or {}
        split_checks = report.get("split_checks", {}) or {}
        elapsed_seconds = float(report.get("elapsed_seconds", summary.get("elapsed_seconds", 0.0)) or 0.0)
        arc_rows = outputs.get("arc_training_rows", summary.get("arc_training_rows", "â€”"))
        context_rows = outputs.get("load_context_rows", summary.get("load_context_rows", "â€”"))
        self.prepare_summary.set(
            f"Source: {self._project_path(self.prepare_report.get())}\n"
            f"Rows before/after: {summary.get('rows_before_cleaning', 'â€”')} â†’ {summary.get('rows_after_cleaning_with_augmentation', summary.get('rows_after_cleaning', 'â€”'))}\n"
            f"Arc rows: {arc_rows} | Context rows: {context_rows} | Duration: {self._fmt_duration(elapsed_seconds)}\n"
            f"Arc sweep features available: {summary.get('arc_feature_count_available', 'â€”')} / {len(ARC_SWEEP_FEATURES)} | Context sweep features available: {summary.get('context_feature_count_available', 'â€”')} / {len(CONTEXT_SWEEP_FEATURES)}"
        )
        metrics = [
            ("Source CSV count", report.get("source_csv_count")),
            ("Full computed feature count", len(report.get("feature_names", []) or [])),
            ("Rows before cleaning", summary.get("rows_before_cleaning")),
            ("Rows after cleaning", summary.get("rows_after_cleaning")),
            ("Rows removed", summary.get("rows_removed")),
            ("Augmentation rows added", summary.get("augmentation_rows_added")),
            ("Rows after augmentation", summary.get("rows_after_cleaning_with_augmentation")),
            ("Arc training rows", arc_rows),
            ("Load context rows", context_rows),
            ("Trainable rows", summary.get("trainable_rows")),
            ("Trusted normal rows", summary.get("trusted_normal_rows")),
            ("Conflict-marked rows", summary.get("conflict_marked_rows")),
            ("Arc sweep pool available", summary.get("arc_feature_count_available")),
            ("Context sweep pool available", summary.get("context_feature_count_available")),
            ("Full division counts", self._pick(report, ("full_dataset", "division_counts"))),
            ("Arc division counts", split_checks.get("arc_training_division_counts")),
            ("Context division counts", split_checks.get("load_context_division_counts")),
            ("Arc label counts", split_checks.get("arc_training_label_counts")),
            ("Context family counts", split_checks.get("load_context_family_counts")),
            ("Warnings", split_checks.get("warnings", [])),
        ]
        for label, value in metrics:
            self.prepare_metric_tree.insert("", "end", values=(label, self._fmt(value)))
        feature_stats = {
            "arc_training": report.get("arc_training_feature_stats", {}) or {},
            "load_context": report.get("load_context_feature_stats", {}) or {},
        }
        for dataset_name in ("arc_training", "load_context"):
            for feature_name in ALL_COMPUTED_FEATURES:
                row = feature_stats.get(dataset_name, {}).get(feature_name) or {}
                if not row:
                    continue
                self.prepare_feature_tree.insert(
                    "",
                    "end",
                    values=(dataset_name, feature_name, self._fmt(row.get("mean")), self._fmt(row.get("std")), self._fmt(row.get("min")), self._fmt(row.get("max"))),
                )
    def _refresh_compare_view(self):
        for row in self.benchmark_tree.get_children():
            self.benchmark_tree.delete(row)
        for row in self.importance_tree.get_children():
            self.importance_tree.delete(row)
        duel = self._read_json(self.duel_report) or {}
        duel_results = duel.get("all_results", []) or []
        duel_winner_name = (duel.get("winner") or {}).get("model_name")
        benchmark_rows = []
        if duel_results:
            for item in duel_results:
                benchmark_rows.append((item, "King training"))
        else:
            rf = self._read_json(self.rf_report)
            et = self._read_json(self.et_report)
            if rf:
                benchmark_rows.append((rf, "Standalone"))
            if et:
                benchmark_rows.append((et, "Standalone"))
        if not benchmark_rows:
            self.compare_summary.set("No reports loaded.")
            return
        ordered = sorted(
            benchmark_rows,
            key=lambda pair: (
                0 if pair[0].get("threshold_constraints_met") else 1,
                -self._get_float(pair[0].get("validation_balanced_accuracy", 0.0), 0.0, 0.0, 1.0),
                self._get_float(pair[0].get("validation_fpr", 1.0), 1.0, 0.0, 1.0),
                -self._get_float(pair[0].get("holdout_validation_precision", pair[0].get("validation_precision", 0.0)), 0.0, 0.0, 1.0),
                -self._get_float(pair[0].get("holdout_validation_recall", pair[0].get("validation_recall", 0.0)), 0.0, 0.0, 1.0),
                self._get_int(self._pick(pair[0], ("validation_threshold_result", "fp")), 10**9, 0),
                self._get_int(self._pick(pair[0], ("validation_threshold_result", "fn")), 10**9, 0),
                self._get_int(self._pick(pair[0], ("test_confusion_matrix", "fn")), 10**9, 0),
                self._get_int(self._pick(pair[0], ("test_confusion_matrix", "fp")), 10**9, 0),
                -(pair[0].get("cv_best_average_precision") or float("-inf")),
                pair[0].get("estimated_node_count") or 10**9,
            ),
        )
        lead_features = self._arc_base_features_from_payload(duel.get("winner") or ordered[0][0])
        self.compare_summary.set(
            f"Benchmark rows shown: {len(ordered)}\n"
            f"King report loaded: {'yes' if duel else 'no'}\n"
            f"Winner: {duel_winner_name or (ordered[0][0].get('model_name') if ordered else 'â€”')}\n"
            f"Winner arc base features ({len(lead_features)}): {self._feature_list_text(lead_features)}"
        )
        for idx, (report, source) in enumerate(ordered, start=1):
            cm = report.get("test_confusion_matrix", {})
            is_winner = report.get("model_name") == duel_winner_name if duel_winner_name else idx == 1
            self.benchmark_tree.insert(
                "",
                "end",
                values=(
                    idx,
                    report.get("model_name", "â€”"),
                    source,
                    self._fmt(report.get("cv_best_average_precision")),
                    self._fmt(report.get("test_average_precision")),
                    self._fmt(report.get("test_roc_auc")),
                    self._fmt(report.get("test_accuracy")),
                    self._fmt(report.get("test_balanced_accuracy")),
                    self._fmt(report.get("test_f1")),
                    self._fmt(report.get("test_recall")),
                    self._fmt(report.get("test_specificity")),
                    self._fmt(report.get("threshold")),
                    self._fmt(self._pick(report, ("validation_threshold_result", "fp"))),
                    self._fmt(self._pick(report, ("validation_threshold_result", "fn"))),
                    self._fmt(cm.get("tn", "â€”")),
                    self._fmt(cm.get("fp", "â€”")),
                    self._fmt(cm.get("fn", "â€”")),
                    self._fmt(cm.get("tp", "â€”")),
                    self._fmt(report.get("estimated_node_count")),
                    "YES" if is_winner else "",
                ),
            )
        rf = self._get_duel_result_by_key("rf") or self._read_json(self.rf_report) or {}
        et = self._get_duel_result_by_key("et") or self._read_json(self.et_report) or {}
        rf_imp = rf.get("feature_importances", {})
        et_imp = et.get("feature_importances", {})
        for feat in sorted(set(rf_imp) | set(et_imp)):
            rv = float(rf_imp.get(feat, 0.0) or 0.0)
            ev = float(et_imp.get(feat, 0.0) or 0.0)
            if abs(rv - ev) <= 1e-12:
                leader = "Tie"
            else:
                leader = "Random Forest" if rv > ev else "Extra Trees"
            self.importance_tree.insert("", "end", values=(feat, self._fmt(rv), self._fmt(ev), leader))
    def _subset_metric_cells(self, role, row):
        if role == "context":
            return (
                "Mean Acc",
                self._fmt(row.get("mean_accuracy")),
                "Mean BalAcc",
                self._fmt(row.get("mean_balanced_accuracy")),
                "Unknown Rate",
                self._fmt(row.get("mean_unknown_rate")),
                "Mean Conf",
                self._fmt(row.get("mean_confidence")),
            )
        return (
            "Val Event FN",
            self._fmt(row.get("validation_event_fn", row.get("combined_validation_event_fn"))),
            "Val Row FN",
            self._fmt(row.get("validation_fn", row.get("combined_validation_fn"))),
            "Val FP",
            self._fmt(row.get("validation_fp", row.get("combined_validation_fp"))),
            "FalseAlm/Session",
            self._fmt(row.get("validation_false_alarms_per_session", row.get("combined_validation_false_alarms_per_session_mean"))),
        )
    def _refresh_subset_report(self):
        if not hasattr(self, "subset_metric_tree"):
            return
        for tree in (self.subset_metric_tree, self.subset_best_tree):
            for row in tree.get_children():
                tree.delete(row)
        arc_report = self._read_json(self.subset_report)
        context_report = self._read_json(self.context_subset_report)
        if not arc_report and not context_report:
            self.subset_summary.set("No subset sweep report loaded.")
            return
        arc_settings = (arc_report or {}).get("settings", {}) or {}
        context_settings = (context_report or {}).get("settings", {}) or {}
        arc_src = self._project_path(self.subset_report.get())
        ctx_src = self._project_path(self.context_subset_report.get())
        summary_lines = []
        if arc_report:
            summary_lines.append(f"Arc report: {arc_src}")
            summary_lines.append(
                f"Arc pool ({arc_report.get('feature_pool_size', '—')}): {self._feature_list_text(arc_report.get('feature_pool', []))}"
            )
            summary_lines.append(
                f"Arc strategy: {arc_report.get('strategy', 'dual search')} | "
                f"screened {arc_report.get('screened_combinations', arc_report.get('total_combinations', 'â€”'))}/"
                f"{arc_report.get('total_combinations', 'â€”')} combos | "
                f"budget {arc_report.get('budget_minutes', 'â€”')} min"
                + (" | budget stop hit" if arc_report.get("budget_hit") else "")
            )
            summary_lines.append(
                f"Deployment recommendation [{str(arc_report.get('recommended_arc_deployment_model', 'rf')).upper()}] "
                f"({len(arc_report.get('recommended_arc_base_features_global', []) or [])}): "
                f"{self._feature_list_text(arc_report.get('recommended_arc_base_features_global') or arc_report.get('recommended_features_global') or [])}"
            )
            summary_lines.append(
                f"FN-first recommendation: {self._feature_list_text(((arc_report.get('overall_best_fn_first') or {}).get('features', [])))}"
            )
            summary_lines.append(
                f"Accuracy-first recommendation: {self._feature_list_text(((arc_report.get('overall_best_accuracy') or {}).get('features', [])))}"
            )
        if context_report:
            summary_lines.append(f"Context report: {ctx_src}")
            summary_lines.append(
                f"Context pool ({context_report.get('feature_pool_size', '—')}): {self._feature_list_text(context_report.get('feature_pool', []))}"
            )
            summary_lines.append(
                f"Recommended context features ({len(context_report.get('recommended_context_features', []) or [])}): "
                f"{self._feature_list_text(context_report.get('recommended_context_features') or context_report.get('recommended_features') or [])}"
            )
        self.subset_summary.set("\n".join(summary_lines))
        metrics = [
            ("Arc task", arc_report.get("task") if arc_report else "—"),
            ("Arc feature pool role", arc_report.get("feature_pool_role") if arc_report else "—"),
            ("Arc total combinations", arc_report.get("total_combinations") if arc_report else "—"),
            ("Arc screened combinations", arc_report.get("screened_combinations") if arc_report else "—"),
            ("Arc feature count range", f"{arc_settings.get('feature_count_min', '—')}..{arc_settings.get('feature_count_max', '—')}" if arc_report else "—"),
            ("Arc max combinations", arc_settings.get("max_combinations") if arc_report else "—"),
            ("Arc final depth", arc_settings.get("n_iter") if arc_report else "—"),
            ("Arc ET shortlist depth", arc_settings.get("arc_shortlist_n_iter") if arc_report else "—"),
            ("Arc shard size", arc_settings.get("arc_shard_size") if arc_report else "—"),
            ("Arc keep per shard", arc_settings.get("arc_keep_per_shard") if arc_report else "—"),
            ("Arc shortlist size", arc_settings.get("arc_shortlist_size") if arc_report else "—"),
            ("Arc finalist count", arc_settings.get("arc_finalist_count") if arc_report else "—"),
            ("Arc tolerance mode", arc_settings.get("arc_tolerance_mode") if arc_report else "—"),
            ("Arc tolerance rows", f"{arc_settings.get('pre_arc_window', '—')} / {arc_settings.get('post_arc_window', '—')}" if arc_report else "—"),
            ("Arc sweep neg ratio", arc_settings.get("sweep_negative_ratio") if arc_report else "—"),
            ("Arc final neg ratio", arc_settings.get("final_negative_ratio") if arc_report else "—"),
            ("Arc budget min", arc_report.get("budget_minutes") if arc_report else "—"),
            ("Arc budget stop", arc_report.get("budget_stop_stage") if arc_report and arc_report.get("budget_hit") else "No"),
            ("Arc ranking default", (arc_report or {}).get("ranking_policy_default", "practical") if arc_report else "—"),
            ("Arc deployment model", str((arc_report or {}).get("recommended_arc_deployment_model", "rf")).upper() if arc_report else "—"),
            ("Arc fixed context inputs", self._feature_list_text((arc_report or {}).get("context_inputs_fixed", [])) if arc_report else "—"),
            ("Arc recommended base features", self._feature_list_text((arc_report or {}).get("recommended_arc_base_features_global", []) or (arc_report or {}).get("recommended_features_global", [])) if arc_report else "—"),
            ("Arc practical features", self._feature_list_text(((arc_report or {}).get("overall_best_practical") or {}).get("features", [])) if arc_report else "—"),
            ("Arc FN-first features", self._feature_list_text(((arc_report or {}).get("overall_best_fn_first") or {}).get("features", [])) if arc_report else "—"),
            ("Arc accuracy-first features", self._feature_list_text(((arc_report or {}).get("overall_best_accuracy") or {}).get("features", [])) if arc_report else "—"),
            ("Context task", context_report.get("task") if context_report else "—"),
            ("Context feature pool role", context_report.get("feature_pool_role") if context_report else "—"),
            ("Context total combinations", context_report.get("total_combinations") if context_report else "—"),
            ("Context feature count range", f"{context_settings.get('feature_count_min', '—')}..{context_settings.get('feature_count_max', '—')}" if context_report else "—"),
            ("Context max combinations", context_settings.get("max_combinations") if context_report else "—"),
            ("Context repeats", context_settings.get("context_repeats") if context_report else "—"),
            ("Context recommended features", self._feature_list_text((context_report or {}).get("recommended_context_features", []) or (context_report or {}).get("recommended_features", [])) if context_report else "—"),
        ]
        for label, value in metrics:
            self.subset_metric_tree.insert("", "end", values=(label, self._fmt(value)))
        buckets = []
        if arc_report:
            buckets.extend(
                [
                    ("Arc overall best combined", arc_report.get("overall_best_combined_tradeoff") or {}, "arc"),
                    ("Arc overall best by practical ranking", arc_report.get("overall_best_practical") or arc_report.get("overall_best_combined_tradeoff") or {}, "arc"),
                    ("Arc overall best by validation cost", arc_report.get("overall_best_validation_cost") or arc_report.get("overall_best_combined_tradeoff") or {}, "arc"),
                    ("Arc overall best by FN-first", arc_report.get("overall_best_fn_first") or {}, "arc"),
                    ("Arc overall best by accuracy", arc_report.get("overall_best_accuracy") or {}, "arc"),
                    ("Arc best RF", arc_report.get("overall_best_rf") or {}, "arc"),
                    ("Arc best ET", arc_report.get("overall_best_et") or {}, "arc"),
                ]
            )
            for entry in arc_report.get("best_by_feature_count_practical", []) or []:
                buckets.append((f"Arc practical count={entry.get('feature_count', 'â€”')}", entry.get("best_combined") or {}, "arc"))
            for entry in arc_report.get("best_by_feature_count_validation_cost", []) or arc_report.get("best_by_feature_count", []) or []:
                buckets.append((f"Arc val-cost count={entry.get('feature_count', '—')}", entry.get("best_combined") or {}, "arc"))
            for entry in arc_report.get("best_by_feature_count_fn_first", []) or []:
                buckets.append((f"Arc FN-first count={entry.get('feature_count', '—')}", entry.get("best_combined") or {}, "arc"))
            for entry in arc_report.get("best_by_feature_count_accuracy", []) or []:
                buckets.append((f"Arc accuracy count={entry.get('feature_count', '—')}", entry.get("best_combined") or {}, "arc"))
        if context_report:
            buckets.append(("Context overall best", context_report.get("overall_best_tradeoff") or {}, "context"))
            for entry in context_report.get("best_by_feature_count", []) or []:
                buckets.append((f"Context best count={entry.get('feature_count', '—')}", entry.get("best_overall") or {}, "context"))
        for bucket, row, role in buckets:
            metric_cells = self._subset_metric_cells(role, row)
            combo = row.get("feature_combo") or "|".join(row.get("features", []) or [])
            self.subset_best_tree.insert(
                "",
                "end",
                values=(
                    bucket,
                    self._fmt(row.get("feature_count")),
                    metric_cells[0],
                    metric_cells[1],
                    metric_cells[2],
                    metric_cells[3],
                    metric_cells[4],
                    metric_cells[5],
                    metric_cells[6],
                    metric_cells[7],
                    combo,
                ),
            )
    def _refresh_single_report(self, prefix):
        summary_var = getattr(self, f"{prefix}_summary")
        metric_tree = getattr(self, f"{prefix}_metric_tree")
        class_tree = getattr(self, f"{prefix}_class_tree")
        cm_tree = getattr(self, f"{prefix}_cm_tree")
        param_tree = getattr(self, f"{prefix}_param_tree")
        fi_tree = getattr(self, f"{prefix}_fi_tree")
        notebook = getattr(self, f"{prefix}_notebook", None)
        for tree in (metric_tree, class_tree, cm_tree, param_tree, fi_tree):
            for row in tree.get_children():
                tree.delete(row)
        report, source_note = self._resolve_report_payload(prefix)
        if not report:
            summary_var.set(source_note)
            return
        if prefix == "duel":
            winner = report.get("winner", {})
            all_results = report.get("all_results", []) or []
            target = winner
            arc_base = self._arc_base_features_from_payload(winner)
            summary_var.set(
                f"Source: {source_note}\n"
                f"Winner: {winner.get('model_name', 'â€”')}\n"
                f"Benchmarked models: {', '.join([r.get('model_name', 'â€”') for r in all_results]) or 'â€”'}\n"
                f"Winner arc base features ({len(arc_base)}): {self._feature_list_text(arc_base)}"
            )
        else:
            target = report
            if prefix == "context":
                feature_names = self._extract_feature_list(target, "feature_names")
                summary_var.set(
                    f"Source: {source_note}\n"
                    f"Model: {target.get('model_name', 'â€”')}\n"
                    f"Rows: {target.get('row_count', 'â€”')} | Train: {target.get('train_rows', 'â€”')} | Test: {target.get('test_rows', 'â€”')}\n"
                    f"Accuracy: {self._fmt(target.get('accuracy'))} | Balanced accuracy: {self._fmt(target.get('balanced_accuracy'))} | Mean confidence: {self._fmt(target.get('mean_confidence'))} | Unknown rate: {self._fmt(target.get('unknown_rate'))}\n"
                    f"Selected context features ({len(feature_names)}): {self._feature_list_text(feature_names)}"
                )
            else:
                arc_base = self._arc_base_features_from_payload(target)
                val = target.get("validation_threshold_result", {}) or {}
                test = target.get("test_confusion_matrix", {}) or {}
                summary_var.set(
                    f"Source: {source_note}\n"
                    f"Model: {target.get('model_name', 'â€”')} | Threshold: {self._fmt(target.get('threshold', 'â€”'))}\n"
                    f"Validation FN/FP: {self._fmt(val.get('fn', 'â€”'))} / {self._fmt(val.get('fp', 'â€”'))} | Test FN/FP: {self._fmt(test.get('fn', 'â€”'))} / {self._fmt(test.get('fp', 'â€”'))}\n"
                    f"Selected arc base features ({len(arc_base)}): {self._feature_list_text(arc_base)}"
                )
        if prefix == "context":
            if notebook is not None:
                notebook.tab(getattr(self, f"{prefix}_class_tab"), text="Per-class Metrics")
                notebook.tab(getattr(self, f"{prefix}_cm_tab"), text="Multiclass Confusion")
                notebook.hide(getattr(self, f"{prefix}_params_tab"))
                notebook.hide(getattr(self, f"{prefix}_fi_tab"))
            context_metrics = [
                ("Selected features", self._feature_list_text(target.get("feature_names", []))),
                ("Feature count", len(target.get("feature_names", []) or [])),
                ("Row count", target.get("row_count")),
                ("Train rows", target.get("train_rows")),
                ("Test rows", target.get("test_rows")),
                ("Accuracy", target.get("accuracy")),
                ("Balanced accuracy", target.get("balanced_accuracy")),
                ("Accuracy with unknown gate", target.get("accuracy_with_unknown_gate")),
                ("Mean confidence", target.get("mean_confidence")),
                ("Unknown rate", target.get("unknown_rate")),
                ("Unknown confidence threshold", target.get("unknown_confidence_threshold")),
                ("Class distribution", target.get("class_counts")),
                ("Missing classes", target.get("missing_classes")),
                ("Warnings", target.get("warnings", [])),
            ]
            for label, value in context_metrics:
                metric_tree.insert("", "end", values=(label, self._fmt(value)))
            class_tree.configure(columns=("family", "rows", "precision", "recall", "f1", "accuracy", "conf", "unknown"))
            for c, title, width in [("family", "Family", 180), ("rows", "Rows", 80), ("precision", "Precision", 100), ("recall", "Recall", 100), ("f1", "F1", 100), ("accuracy", "Accuracy", 100), ("conf", "Mean conf", 110), ("unknown", "Unknown rate", 110)]:
                class_tree.heading(c, text=title)
                class_tree.column(c, width=width, anchor="center")
            for row in target.get("per_class", []) or []:
                class_tree.insert("", "end", values=(row.get("family", "â€”"), self._fmt(row.get("rows")), self._fmt(row.get("precision")), self._fmt(row.get("recall")), self._fmt(row.get("f1")), self._fmt(row.get("accuracy")), self._fmt(row.get("mean_confidence")), self._fmt(row.get("unknown_rate"))))
            labels = target.get("class_labels", []) or []
            matrix = target.get("confusion_matrix", []) or []
            cm_cols = tuple(["label"] + [f"c{i}" for i in range(len(labels))])
            cm_tree.configure(columns=cm_cols)
            cm_tree.heading("label", text="Actual \\ Pred")
            cm_tree.column("label", width=180, anchor="w")
            for i, fam in enumerate(labels):
                col = f"c{i}"
                cm_tree.heading(col, text=fam)
                cm_tree.column(col, width=100, anchor="center")
            for idx, fam in enumerate(labels):
                row_vals = matrix[idx] if idx < len(matrix) else []
                values = [f"Actual {fam}"] + [int(row_vals[j]) if j < len(row_vals) else 0 for j in range(len(labels))]
                cm_tree.insert("", "end", values=tuple(values))
            return
        if notebook is not None:
            params_tab = getattr(self, f"{prefix}_params_tab")
            fi_tab = getattr(self, f"{prefix}_fi_tab")
            if str(params_tab) not in notebook.tabs():
                notebook.add(params_tab, text="Best Params")
            if str(fi_tab) not in notebook.tabs():
                notebook.add(fi_tab, text="Feature Importances")
        arc_base = self._arc_base_features_from_payload(target)
        arc_context = self._arc_context_inputs_from_payload(target)
        metric_tree.insert("", "end", values=("Arc base feature count", self._fmt(len(arc_base))))
        metric_tree.insert("", "end", values=("Arc base features", self._feature_list_text(arc_base)))
        metric_tree.insert("", "end", values=("Fixed context input count", self._fmt(len(arc_context))))
        metric_tree.insert("", "end", values=("Fixed context inputs", self._feature_list_text(arc_context)))
        metric_tree.insert("", "end", values=("Model input feature IDs", self._fmt(target.get("model_input_feature_ids", "â€”"))))
        for label, key in DISPLAY_METRICS:
            metric_tree.insert("", "end", values=(label, self._fmt(self._pick(target, key))))
        class_tree.configure(columns=("label", "precision", "recall", "metric3", "support"))
        for c, title, width in [("label", "Label / Group", 180), ("precision", "Precision", 120), ("recall", "Recall", 120), ("metric3", "F1 / FPR", 120), ("support", "Support / Rows", 120)]:
            class_tree.heading(c, text=title)
            class_tree.column(c, width=width, anchor="center" if c != "label" else "w")
        class_report = target.get("test_classification_report", {}) or {}
        for label in ["0", "1", "macro avg", "weighted avg"]:
            row = class_report.get(label)
            if isinstance(row, dict):
                class_tree.insert(
                    "",
                    "end",
                    values=(
                        label,
                        self._fmt(row.get("precision")),
                        self._fmt(row.get("recall")),
                        self._fmt(row.get("f1-score")),
                        self._fmt(row.get("support")),
                    ),
                )
        for row in target.get("test_per_load", []) or []:
            class_tree.insert(
                "",
                "end",
                values=(
                    f"load:{row.get('group', 'â€”')}",
                    self._fmt(row.get("precision")),
                    self._fmt(row.get("recall")),
                    f"FPR {self._fmt(row.get('fpr'))}",
                    self._fmt(row.get("rows")),
                ),
            )
        for row in target.get("test_per_runtime_context", []) or []:
            class_tree.insert(
                "",
                "end",
                values=(
                    f"ctx:{row.get('group', 'â€”')}",
                    self._fmt(row.get("precision")),
                    self._fmt(row.get("recall")),
                    f"FPR {self._fmt(row.get('fpr'))}",
                    self._fmt(row.get("rows")),
                ),
            )
        cm_tree.configure(columns=("label", "pred0", "pred1"))
        cm_tree.heading("label", text="Actual \\ Pred")
        cm_tree.heading("pred0", text="Pred 0")
        cm_tree.heading("pred1", text="Pred 1")
        cm_tree.column("label", width=220, anchor="w")
        cm_tree.column("pred0", width=140, anchor="center")
        cm_tree.column("pred1", width=140, anchor="center")
        val = target.get("validation_threshold_result", {}) or {}
        cm_tree.insert("", "end", values=("Validation Actual 0", self._fmt(val.get("tn", "â€”")), self._fmt(val.get("fp", "â€”"))))
        cm_tree.insert("", "end", values=("Validation Actual 1", self._fmt(val.get("fn", "â€”")), self._fmt(val.get("tp", "â€”"))))
        cm = target.get("test_confusion_matrix", {}) or {}
        cm_tree.insert("", "end", values=("Test Actual 0", self._fmt(cm.get("tn", "â€”")), self._fmt(cm.get("fp", "â€”"))))
        cm_tree.insert("", "end", values=("Test Actual 1", self._fmt(cm.get("fn", "â€”")), self._fmt(cm.get("tp", "â€”"))))
        for row in target.get("test_per_load", []) or []:
            cmg = row.get("confusion_matrix", {}) or {}
            cm_tree.insert("", "end", values=(f"Load {row.get('group', 'â€”')}", f"TN={cmg.get('tn', 'â€”')} FP={cmg.get('fp', 'â€”')}", f"FN={cmg.get('fn', 'â€”')} TP={cmg.get('tp', 'â€”')}"))
        for row in target.get("test_per_runtime_context", []) or []:
            cmg = row.get("confusion_matrix", {}) or {}
            cm_tree.insert("", "end", values=(f"Ctx {row.get('group', 'â€”')}", f"TN={cmg.get('tn', 'â€”')} FP={cmg.get('fp', 'â€”')}", f"FN={cmg.get('fn', 'â€”')} TP={cmg.get('tp', 'â€”')}"))
        params = target.get("best_params", {}) or {}
        for name in sorted(params):
            param_tree.insert("", "end", values=(name, self._fmt(params[name])))
        fi = target.get("feature_importances", {}) or {}
        for idx, (feat, value) in enumerate(sorted(fi.items(), key=lambda kv: float(kv[1]), reverse=True), start=1):
            fi_tree.insert("", "end", values=(idx, feat, self._fmt(value)))
    @staticmethod
    def _pick(report, key):
        if not report:
            return "—"
        if isinstance(key, tuple):
            cur = report
            for k in key:
                if not isinstance(cur, dict):
                    return "—"
                cur = cur.get(k)
            return cur if cur is not None else "—"
        return report.get(key, "—") if isinstance(report, dict) else "—"
    @staticmethod
    def _fmt(v):
        if isinstance(v, float):
            if v != v:
                return "nan"
            return f"{v:.6f}"
        return str(v)
if __name__ == "__main__":
    TinyMLTrainerGUI().mainloop()
