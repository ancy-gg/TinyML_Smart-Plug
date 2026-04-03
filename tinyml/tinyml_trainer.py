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
    ("Estimated node count", "estimated_node_count"),
    ("Validation cost", ("validation_threshold_result", "cost")),
    ("Validation recall", ("validation_threshold_result", "recall")),
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
        self.geometry("1600x980")
        self.minsize(1260, 800)
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
        self.status_text = tk.StringVar(value="Ready")
        self.last_command = tk.StringVar(value="—")

        self._build_styles()
        self._build_ui()
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

        outer = ttk.Frame(self, padding=16)
        outer.grid(row=0, column=0, sticky="nsew")
        outer.columnconfigure(0, weight=1)
        outer.rowconfigure(1, weight=1)

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
        main_card.grid(row=1, column=0, sticky="nsew", pady=(14, 0))
        main_card.columnconfigure(0, weight=1)
        main_card.rowconfigure(3, weight=1)

        self._card_label(main_card, "Actions", 0)

        actions = ttk.Frame(main_card)
        actions.grid(row=1, column=0, sticky="ew", pady=(0, 12))
        for i in range(5):
            actions.columnconfigure(i, weight=1)

        self._btn(actions, "Cleaner", self.run_cleaner, 0, 0)
        self._btn(actions, "Random Forest Only", self.run_rf_only, 0, 1)
        self._btn(actions, "Extra Trees Only", self.run_et_only, 0, 2)
        self._btn(actions, "King Training", self.run_duel, 0, 3)
        self._btn(actions, "Refresh", self.refresh_views, 0, 4)

        status = tk.Frame(main_card, bg=CARD_BG)
        status.grid(row=2, column=0, sticky="ew", pady=(0, 12))
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

        tk.Label(status, text="Last command", bg=CARD_BG, fg=MUTED, font=("Segoe UI", 9)).grid(
            row=1, column=0, sticky="w", pady=(10, 0)
        )
        tk.Label(
            status,
            textvariable=self.last_command,
            bg=CARD_BG,
            fg=TEXT,
            font=("Consolas", 9),
            justify="left",
            wraplength=1120,
        ).grid(row=1, column=1, columnspan=3, sticky="w", padx=(8, 0), pady=(10, 0))

        self.pb = ttk.Progressbar(main_card, mode="indeterminate")
        self.pb.grid(row=2, column=0, sticky="sew", pady=(52, 0))

        notebook = ttk.Notebook(main_card)
        notebook.grid(row=3, column=0, sticky="nsew", pady=(12, 0))

        logs_tab = ttk.Frame(notebook, padding=10)
        data_tab = ttk.Frame(notebook, padding=10)
        reports_tab = ttk.Frame(notebook, padding=10)

        notebook.add(logs_tab, text="Live Logs")
        notebook.add(data_tab, text="Dataset")
        notebook.add(reports_tab, text="Reports")

        self._build_logs_tab(logs_tab)
        self._build_data_tab(data_tab)
        self._build_reports_tab(reports_tab)

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
        parent.rowconfigure(1, weight=1)

        self.dataset_summary = tk.StringVar(value="No dataset loaded.")
        tk.Label(
            parent, textvariable=self.dataset_summary, bg=APP_BG, fg=MUTED, justify="left"
        ).grid(row=0, column=0, sticky="w", pady=(0, 10))

        frame = tk.Frame(parent, bg=CARD_BG, highlightthickness=1, highlightbackground=BORDER)
        frame.grid(row=1, column=0, sticky="nsew")
        frame.columnconfigure(0, weight=1)
        frame.rowconfigure(0, weight=1)

        cols = ("feature", "mean", "std", "min", "max")
        self.dataset_tree = ttk.Treeview(frame, columns=cols, show="headings")
        for c, w in zip(cols, (240, 140, 140, 140, 140)):
            self.dataset_tree.heading(c, text=c.title())
            self.dataset_tree.column(c, width=w, anchor="center")
        self.dataset_tree.grid(row=0, column=0, sticky="nsew")
        ttk.Scrollbar(frame, orient="vertical", command=self.dataset_tree.yview).grid(
            row=0, column=1, sticky="ns"
        )

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
        ttk.Scrollbar(frame2, orient="vertical", command=self.importance_tree.yview).grid(
            row=0, column=1, sticky="ns"
        )

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
        ttk.Scrollbar(overview_tab, orient="vertical", command=metric_tree.yview).grid(
            row=0, column=1, sticky="ns"
        )
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
        ttk.Scrollbar(class_tab, orient="vertical", command=class_tree.yview).grid(
            row=0, column=1, sticky="ns"
        )
        setattr(self, f"{prefix}_class_tree", class_tree)

        cm_tree = ttk.Treeview(cm_tab, columns=("label", "pred0", "pred1"), show="headings")
        cm_tree.heading("label", text="Actual \\ Pred")
        cm_tree.heading("pred0", text="Pred 0")
        cm_tree.heading("pred1", text="Pred 1")
        cm_tree.column("label", width=200, anchor="w")
        cm_tree.column("pred0", width=120, anchor="center")
        cm_tree.column("pred1", width=120, anchor="center")
        cm_tree.grid(row=0, column=0, sticky="nsew")
        ttk.Scrollbar(cm_tab, orient="vertical", command=cm_tree.yview).grid(
            row=0, column=1, sticky="ns"
        )
        setattr(self, f"{prefix}_cm_tree", cm_tree)

        param_tree = ttk.Treeview(params_tab, columns=("param", "value"), show="headings")
        param_tree.heading("param", text="Parameter")
        param_tree.heading("value", text="Value")
        param_tree.column("param", width=280, anchor="w")
        param_tree.column("value", width=220, anchor="center")
        param_tree.grid(row=0, column=0, sticky="nsew")
        ttk.Scrollbar(params_tab, orient="vertical", command=param_tree.yview).grid(
            row=0, column=1, sticky="ns"
        )
        setattr(self, f"{prefix}_param_tree", param_tree)

        fi_tree = ttk.Treeview(fi_tab, columns=("rank", "feature", "importance"), show="headings")
        fi_tree.heading("rank", text="Rank")
        fi_tree.heading("feature", text="Feature")
        fi_tree.heading("importance", text="Importance")
        fi_tree.column("rank", width=80, anchor="center")
        fi_tree.column("feature", width=320, anchor="w")
        fi_tree.column("importance", width=160, anchor="center")
        fi_tree.grid(row=0, column=0, sticky="nsew")
        ttk.Scrollbar(fi_tab, orient="vertical", command=fi_tree.yview).grid(
            row=0, column=1, sticky="ns"
        )
        setattr(self, f"{prefix}_fi_tree", fi_tree)

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

    def _drain_logs(self):
        try:
            while True:
                msg = self.log_queue.get_nowait()
                self.log_text.insert("end", msg)
                self.log_text.see("end")
        except queue.Empty:
            pass
        self.after(120, self._drain_logs)

    def log(self, msg):
        self.log_queue.put(msg)

    def _project_path(self, rel_or_abs):
        p = Path(rel_or_abs)
        if p.is_absolute():
            return p
        return Path(self.project_root.get()) / rel_or_abs

    def _start_subprocess(self, cmd):
        if self.proc is not None:
            messagebox.showwarning("Busy", "Another process is already running.")
            return

        self.pb.start(10)
        self.status_text.set("Running…")
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
                self.status_text.set(f"Finished (exit code {rc})")
            except Exception as e:
                self.log(f"\nERROR: {e}\n")
                self.status_text.set("Failed")
            finally:
                self.proc = None
                self.pb.stop()
                self.refresh_views()

        threading.Thread(target=worker, daemon=True).start()

    def stop_running(self):
        if self.proc is None:
            return
        try:
            self.proc.terminate()
            self.log("\nRequested process termination.\n")
        except Exception as e:
            self.log(f"\nStop failed: {e}\n")

    def run_cleaner(self):
        self._start_subprocess([
            self.python_exe.get(),
            str(self._project_path(self.prepare_script.get())),
        ])

    def run_rf_only(self):
        self._start_subprocess([
            self.python_exe.get(),
            str(self._project_path(self.rf_script.get())),
        ])

    def run_et_only(self):
        self._start_subprocess([
            self.python_exe.get(),
            str(self._project_path(self.et_script.get())),
        ])

    def run_duel(self):
        self._start_subprocess([
            self.python_exe.get(),
            str(self._project_path(self.duel_script.get())),
        ])

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
        self.dataset_summary.set(
            f"Rows: {rows}\n"
            f"Trainable rows: {trainable}\n"
            f"Label counts: {label_counts}\n"
            f"Source counts: {source_counts}"
        )

        feature_cols = [
            "cycle_nmse", "zcv", "zc_dwell_ratio", "cycle_rms_drop_ratio",
            "peak_fluct_cv", "midband_residual_rms", "hf_band_energy_ratio",
            "spec_entropy", "neg_dip_event_ratio", "irms_drop_vs_baseline",
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
