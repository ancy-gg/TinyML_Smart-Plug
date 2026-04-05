import json
import os
import queue
import subprocess
import sys
import threading
from pathlib import Path
import tkinter as tk
from tkinter import filedialog, messagebox, ttk

try:
    import pandas as pd
except Exception:
    pd = None


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
        self.cleaned_csv = tk.StringVar(value="tinyml/data/cleaned_data.csv")
        self.rf_report = tk.StringVar(value="tinyml/benchmark/TinyMLTreeEnsemble_RF_report.json")
        self.et_report = tk.StringVar(value="tinyml/benchmark/TinyMLTreeEnsemble_ET_report.json")
        self.duel_report = tk.StringVar(value="tinyml/benchmark/benchmark_report.json")
        self.search_iters = tk.StringVar(value="120")
        self.max_search_iters = tk.StringVar(value="240")
        self.iter_growth = tk.StringVar(value="2.0")
        self.min_recall_goal = tk.StringVar(value="0.98")
        self.min_precision_goal = tk.StringVar(value="0.90")
        self.max_fpr_goal = tk.StringVar(value="0.03")
        self.min_threshold_goal = tk.StringVar(value="0.08")
        self.auto_max_val_fn = tk.StringVar(value="0")
        self.auto_max_val_fp = tk.StringVar(value="5")
        self.winner_mode = tk.StringVar(value="Arc-first")
        self.auto_escalate = tk.BooleanVar(value=True)
        self.current_workflow = None
        self.status_text = tk.StringVar(value="Ready")
        self.last_command = tk.StringVar(value="—")
        self.progress_text = tk.StringVar(value="Idle")
        self.progress_percent_text = tk.StringVar(value="0%")
        self.cleaner_summary_text = tk.StringVar(value="Cleaner summary will appear here after running Cleaner.")
        self._pending_cleaner_summary = None
        self._progress_payload = None
        self._root_canvas = None
        self._root_canvas_window = None

        self._adaptive_labels = []
        self._build_styles()
        self._build_ui()
        self.bind("<Configure>", self._on_window_resize)
        self.after(120, self._drain_logs)
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
        self._path_row(self.paths_card, 7, "Cleaned CSV", self.cleaned_csv)
        self._path_row(self.paths_card, 8, "Random Forest report", self.rf_report)
        self._path_row(self.paths_card, 9, "Extra Trees report", self.et_report)
        self._path_row(self.paths_card, 10, "Duel report", self.duel_report)

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
        main_card.rowconfigure(5, weight=1)

        self._card_label(main_card, "Actions", 0)

        actions = ttk.Frame(main_card)
        actions.grid(row=1, column=0, sticky="ew", pady=(0, 12))
        for i in range(6):
            actions.columnconfigure(i, weight=1)

        self._btn(actions, "Cleaner", self.run_cleaner, 0, 0)
        self._btn(actions, "Random Forest Only", self.run_rf_only, 0, 1)
        self._btn(actions, "Extra Trees Only", self.run_et_only, 0, 2)
        self._btn(actions, "King Training", self.run_duel, 0, 3)
        self._btn(actions, "Refresh", self.refresh_views, 0, 4)
        self._btn(actions, "Clear Output", self.clear_output, 0, 5)

        options = tk.Frame(main_card, bg=CARD_BG_2, highlightthickness=1, highlightbackground=BORDER, padx=10, pady=10)
        options.grid(row=2, column=0, sticky="ew", pady=(0, 12))
        for i in range(5):
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

        status = tk.Frame(main_card, bg=CARD_BG)
        status.grid(row=3, column=0, sticky="ew", pady=(0, 8))
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

        tk.Label(status, text="Last command", bg=CARD_BG, fg=MUTED, font=("Segoe UI", 9)).grid(
            row=2, column=0, sticky="w", pady=(10, 0)
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
        self.command_label.grid(row=2, column=1, columnspan=3, sticky="ew", padx=(8, 0), pady=(10, 0))
        self._adaptive_labels.append((self.command_label, 0.74, 380))

        self.pb = ttk.Progressbar(main_card, mode="determinate", maximum=100.0, value=0.0)
        self.pb.grid(row=4, column=0, sticky="ew", pady=(0, 12))

        notebook = ttk.Notebook(main_card)
        notebook.grid(row=5, column=0, sticky="nsew", pady=(0, 0))

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

        self.duel_compare_tab = ttk.Frame(sub, padding=10)
        self.rf_tab = ttk.Frame(sub, padding=10)
        self.et_tab = ttk.Frame(sub, padding=10)
        self.duel_tab = ttk.Frame(sub, padding=10)

        sub.add(self.duel_compare_tab, text="Benchmark Overview")
        sub.add(self.rf_tab, text="Random Forest Report")
        sub.add(self.et_tab, text="Extra Trees Report")
        sub.add(self.duel_tab, text="King Report")

        self._build_compare_subtab(self.duel_compare_tab)
        self._build_single_report_subtab(self.rf_tab, prefix="rf")
        self._build_single_report_subtab(self.et_tab, prefix="et")
        self._build_single_report_subtab(self.duel_tab, prefix="duel")

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
        return self.duel_script

    def _common_training_flags(self, n_iter):
        return [
            "--n_iter", str(int(n_iter)),
            "--min_recall", f"{self._get_float(self.min_recall_goal, 0.98, 0.0, 1.0):.6f}",
            "--min_precision", f"{self._get_float(self.min_precision_goal, 0.90, 0.0, 1.0):.6f}",
            "--max_fpr", f"{self._get_float(self.max_fpr_goal, 0.03, 0.0, 1.0):.6f}",
            "--min_threshold", f"{self._get_float(self.min_threshold_goal, 0.08, 0.0, 0.99):.6f}",
        ]


    def _winner_mode_arg(self):
        label = (self.winner_mode.get() or "Arc-first").strip()
        return {
            "Arc-first": "arc_guard",
            "Balanced": "safety_composite",
            "Legacy": "legacy_cv_ap",
        }.get(label, "arc_guard")

    def _build_training_command(self, kind, n_iter):
        cmd = [
            self.python_exe.get(),
            str(self._project_path(self._trainer_script_var_for_kind(kind).get())),
        ]
        cmd.extend(self._common_training_flags(n_iter))
        if kind == "duel":
            cmd.extend(["--winner_mode", self._winner_mode_arg()])
        return cmd

    def _load_training_report_target(self, kind):
        if kind == "rf":
            payload = self._read_json(self.rf_report) or self._get_duel_result_by_key("rf") or {}
            return payload
        if kind == "et":
            payload = self._read_json(self.et_report) or self._get_duel_result_by_key("et") or {}
            return payload
        payload = self._read_json(self.duel_report) or {}
        return payload.get("winner") or payload

    def _next_iteration_value(self, current_n_iter, max_n_iter, growth):
        nxt = int(round(float(current_n_iter) * float(growth)))
        if nxt <= int(current_n_iter):
            nxt = int(current_n_iter) + max(8, int(current_n_iter) // 2)
        return min(int(max_n_iter), max(int(current_n_iter) + 1, nxt))

    def _evaluate_escalation_need(self, kind):
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
        cmd = self._build_training_command(kind, current_n_iter)
        label = {
            "rf": "Random Forest",
            "et": "Extra Trees",
            "duel": "King Training",
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
        return (
            f"Rows: {data.get('rows_before_cleaning', 0)} → {data.get('rows_after_cleaning', 0)}  "
            f"(removed {data.get('rows_removed', 0)})\n"
            f"Trainable: {data.get('trainable_rows', 0)} | Trusted normal: {data.get('trusted_normal_rows', 0)} | Conflict-marked: {data.get('conflict_marked_rows', 0)}\n"
            f"Forced-normal section rows: {data.get('forced_normal_section_rows', 0)} | Unknown-division rows: {data.get('unknown_division_rows', 0)} | Division counts: {data.get('division_counts', {})}\n"
            f"Conflict groups: {conflict.get('mixed_groups', 0)} | Prefer trusted normal: {conflict.get('prefer_trusted_normal', 0)} | Prefer stronger normal: {conflict.get('prefer_more_trusted_normal', 0)} | Keep stronger arc: {conflict.get('keep_more_trusted_arc', 0)} | Dropped ambiguous: {conflict.get('drop_ambiguous', 0)}\n"
            f"Labels: {data.get('label_counts', {})} | Sources: {data.get('source_counts', {})} | Policies kept: {policy}"
        )



    def _reset_progress(self):
        self._progress_payload = None
        self.pb.configure(mode="determinate", maximum=100.0)
        self.pb["value"] = 0.0
        self.progress_text.set("Idle")
        self.progress_percent_text.set("0%")

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
        self.pb.configure(mode="determinate", maximum=100.0)
        self.pb["value"] = pct

        model_name = str(payload.get("model_name", "")).strip()
        stage = str(payload.get("stage", "")).strip().replace("_", " ").title()
        message = str(payload.get("message", "")).strip()

        pieces = []
        if model_name:
            pieces.append(model_name)
        if stage:
            pieces.append(stage)
        if message:
            pieces.append(message)
        if not pieces:
            pieces.append(f"Step {int(current)}/{int(total)}")

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
        self.status_text.set(f"Finished (exit code {rc})")
        if self._progress_payload is not None:
            self._update_progress(self._progress_payload)
        elif rc == 0:
            self.pb.configure(mode="determinate", maximum=100.0)
            self.pb.configure(value=100.0)
            self.progress_percent_text.set("100.0%")
            self.progress_text.set("Done")
        if rc == 0 and cmd and Path(cmd[1]).name == "prepare_data.py" and self._pending_cleaner_summary:
            self._show_cleaner_summary_dialog(self._pending_cleaner_summary)
        if on_finish is not None:
            on_finish(rc)
        self.refresh_views()

    def _handle_subprocess_failure(self, message):
        self.status_text.set("Failed")
        self.progress_text.set(f"Failed: {message}")
        self.refresh_views()

    def _start_subprocess(self, cmd, on_finish=None):
        if self.proc is not None:
            messagebox.showwarning("Busy", "Another process is already running.")
            return

        self._reset_progress()
        self.status_text.set("Running…")
        self.progress_text.set("Starting process…")
        self.progress_percent_text.set("0%")
        if cmd and Path(cmd[1]).name == "prepare_data.py":
            self._pending_cleaner_summary = None
        self.last_command.set(" ".join(cmd))
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
        self._start_subprocess([
            self.python_exe.get(),
            str(self._project_path(self.prepare_script.get())),
        ])

    def run_rf_only(self):
        self._start_training_workflow("rf")

    def run_et_only(self):
        self._start_training_workflow("et")

    def run_duel(self):
        self._start_training_workflow("duel")

    def refresh_views(self):
        self._refresh_dataset_view()
        self._refresh_compare_view()
        self._refresh_single_report("rf")
        self._refresh_single_report("et")
        self._refresh_single_report("duel")

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
        self.dataset_summary.set(
            f"Rows: {rows}\n"
            f"Trainable rows: {trainable}\n"
            f"Trusted normal rows: {trusted_normal}\n"
            f"Conflict-marked rows: {conflict_rows}\n"
            f"Trial groups: {trial_groups} | Section groups: {section_groups}\n"
            f"Label counts: {label_counts}\n"
            f"Division counts: {division_counts}\n"
            f"Source counts: {source_counts}\n"
            f"Conflict policies: {conflict_policy_counts}"
        )


        feature_cols = [
            "spectral_flux_midhf", "residual_crest_factor", "edge_spike_ratio", "midband_residual_ratio",
            "cycle_nmse", "peak_fluct_cv", "thd_i", "hf_energy_delta", "zcv", "abs_irms_zscore_vs_baseline",
        ]
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

        duel = self._read_json(self.duel_report)
        if duel:
            return duel, "King training benchmark report"
        return None, "No report loaded."

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

    def _refresh_single_report(self, prefix):
        summary_var = getattr(self, f"{prefix}_summary")
        metric_tree = getattr(self, f"{prefix}_metric_tree")
        class_tree = getattr(self, f"{prefix}_class_tree")
        cm_tree = getattr(self, f"{prefix}_cm_tree")
        param_tree = getattr(self, f"{prefix}_param_tree")
        fi_tree = getattr(self, f"{prefix}_fi_tree")

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
            summary_var.set(
                f"Source: {source_note}\n"
                f"Model: {target.get('model_name', '—')}\n"
                f"Threshold: {self._fmt(target.get('threshold', '—'))}"
            )

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

        cm = target.get("test_confusion_matrix", {}) or {}
        cm_tree.insert("", "end", values=("Actual 0", self._fmt(cm.get("tn", "—")), self._fmt(cm.get("fp", "—"))))
        cm_tree.insert("", "end", values=("Actual 1", self._fmt(cm.get("fn", "—")), self._fmt(cm.get("tp", "—"))))

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
