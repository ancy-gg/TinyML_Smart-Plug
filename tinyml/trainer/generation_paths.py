from __future__ import annotations

import json
import re
import shutil
from dataclasses import dataclass
from datetime import datetime, timezone
from pathlib import Path
from typing import Any


MAX_GENERATION_INDEX = 20
GENERATION_RE = re.compile(r"^gen(?:[0-9]|1[0-9]|20)$", re.IGNORECASE)


def normalize_generation_tag(value: str | None, *, allow_none: bool = False) -> str | None:
    if value is None:
        if allow_none:
            return None
        raise ValueError("Generation is required.")
    token = str(value or "").strip().lower()
    if not token:
        if allow_none:
            return None
        raise ValueError("Generation is required.")
    if not GENERATION_RE.fullmatch(token):
        raise ValueError(f"Invalid generation '{value}'. Use gen0..gen{MAX_GENERATION_INDEX}.")
    return token


def generation_index(generation: str) -> int:
    token = normalize_generation_tag(generation)
    return int(token[3:])


def next_generation_tag(generation: str) -> str | None:
    idx = generation_index(generation)
    if idx >= MAX_GENERATION_INDEX:
        return None
    return f"gen{idx + 1}"


def cli_flag_present(flag: str, argv: list[str] | tuple[str, ...] | None = None) -> bool:
    tokens = list(argv or [])
    for token in tokens:
        if token == flag or str(token).startswith(flag + "="):
            return True
    return False


def apply_generation_defaults(args, argv: list[str] | tuple[str, ...], assignments: dict[str, tuple[str, Any]]) -> None:
    for attr, (flag, value) in (assignments or {}).items():
        if cli_flag_present(flag, argv):
            continue
        setattr(args, attr, str(value))


def _iso_now() -> str:
    return datetime.now(timezone.utc).astimezone().isoformat(timespec="seconds")


def _jsonify(value: Any) -> Any:
    if isinstance(value, Path):
        return str(value)
    if isinstance(value, dict):
        return {str(k): _jsonify(v) for k, v in value.items()}
    if isinstance(value, (list, tuple, set)):
        return [_jsonify(v) for v in value]
    return value


def _deep_merge(base: dict[str, Any], updates: dict[str, Any]) -> dict[str, Any]:
    merged = dict(base or {})
    for key, value in (updates or {}).items():
        if isinstance(value, dict) and isinstance(merged.get(key), dict):
            merged[key] = _deep_merge(merged[key], value)
        else:
            merged[key] = value
    return merged


@dataclass(frozen=True)
class GenerationPaths:
    project_root: Path
    generation: str
    data_dir: Path
    raw_dir: Path
    arc_csv: Path
    context_csv: Path
    benchmark_dir: Path
    prepare_report: Path
    arc_subset_report: Path
    arc_subset_results_csv: Path
    context_subset_report: Path
    context_subset_results_csv: Path
    rf_report: Path
    et_report: Path
    duel_report: Path
    context_report: Path
    manifest_path: Path
    model_archive_dir: Path
    trainer_lib_archive_dir: Path
    rf_archive_header: Path
    et_archive_header: Path
    duel_archive_header: Path
    context_archive_header: Path
    rf_archive_joblib: Path
    et_archive_joblib: Path
    duel_archive_joblib: Path
    context_archive_joblib: Path
    rf_active_header: Path
    et_active_header: Path
    duel_active_header: Path
    context_active_header: Path
    rf_active_joblib: Path
    et_active_joblib: Path
    duel_active_joblib: Path
    context_active_joblib: Path

    def canonical_export_paths(self) -> dict[str, dict[str, str]]:
        return {
            "headers": {
                "rf": str(self.rf_active_header),
                "et": str(self.et_active_header),
                "duel": str(self.duel_active_header),
                "context": str(self.context_active_header),
            },
            "joblibs": {
                "rf": str(self.rf_active_joblib),
                "et": str(self.et_active_joblib),
                "duel": str(self.duel_active_joblib),
                "context": str(self.context_active_joblib),
            },
        }

    def archive_output_paths(self) -> dict[str, dict[str, str]]:
        return {
            "headers": {
                "rf": str(self.rf_archive_header),
                "et": str(self.et_archive_header),
                "duel": str(self.duel_archive_header),
                "context": str(self.context_archive_header),
            },
            "joblibs": {
                "rf": str(self.rf_archive_joblib),
                "et": str(self.et_archive_joblib),
                "duel": str(self.duel_archive_joblib),
                "context": str(self.context_archive_joblib),
            },
        }

    def report_paths(self) -> dict[str, str]:
        return {
            "prepare_data_report": str(self.prepare_report),
            "arc_subset_report": str(self.arc_subset_report),
            "arc_subset_results_csv": str(self.arc_subset_results_csv),
            "context_subset_report": str(self.context_subset_report),
            "context_subset_results_csv": str(self.context_subset_results_csv),
            "rf_report": str(self.rf_report),
            "et_report": str(self.et_report),
            "duel_report": str(self.duel_report),
            "context_report": str(self.context_report),
            "generation_manifest": str(self.manifest_path),
        }

    def gui_resolved_paths(self) -> dict[str, str]:
        return {
            "selected_generation": self.generation,
            "raw_folder": str(self.raw_dir),
            "arc_training_csv": str(self.arc_csv),
            "load_context_csv": str(self.context_csv),
            "benchmark_folder": str(self.benchmark_dir),
            "model_archive_folder": str(self.model_archive_dir),
            "canonical_headers": "\n".join([
                str(self.rf_active_header),
                str(self.et_active_header),
                str(self.duel_active_header),
                str(self.context_active_header),
            ]),
            "canonical_joblibs": "\n".join([
                str(self.rf_active_joblib),
                str(self.et_active_joblib),
                str(self.duel_active_joblib),
                str(self.context_active_joblib),
            ]),
        }


def resolve_generation_paths(project_root: str | Path, generation: str) -> GenerationPaths:
    root = Path(project_root).resolve()
    gen = normalize_generation_tag(generation)

    tinyml_dir = root / "tinyml"
    data_dir = tinyml_dir / "data" / gen
    benchmark_dir = tinyml_dir / "benchmark" / gen
    model_dir = tinyml_dir / "model"
    trainer_lib_dir = tinyml_dir / "trainer" / "lib"
    archive_model_dir = model_dir / "archive" / gen
    archive_lib_dir = trainer_lib_dir / "archive" / gen

    return GenerationPaths(
        project_root=root,
        generation=gen,
        data_dir=data_dir,
        raw_dir=data_dir / "raw",
        arc_csv=data_dir / "arc_training.csv",
        context_csv=data_dir / "load_context.csv",
        benchmark_dir=benchmark_dir,
        prepare_report=benchmark_dir / "prepare_data_report.json",
        arc_subset_report=benchmark_dir / "TinyMLFeatureSubsetSweep_report.json",
        arc_subset_results_csv=benchmark_dir / "TinyMLFeatureSubsetSweep_results.csv",
        context_subset_report=benchmark_dir / "TinyMLContextFeatureSubsetSweep_report.json",
        context_subset_results_csv=benchmark_dir / "TinyMLContextFeatureSubsetSweep_results.csv",
        rf_report=benchmark_dir / "TinyMLTreeEnsemble_RF_report.json",
        et_report=benchmark_dir / "TinyMLTreeEnsemble_ET_report.json",
        duel_report=benchmark_dir / "benchmark_report.json",
        context_report=benchmark_dir / "TinyMLContextModel_report.json",
        manifest_path=benchmark_dir / "generation_manifest.json",
        model_archive_dir=archive_model_dir,
        trainer_lib_archive_dir=archive_lib_dir,
        rf_archive_header=archive_model_dir / "TinyMLTreeEnsemble_RF.h",
        et_archive_header=archive_model_dir / "TinyMLTreeEnsemble_ET.h",
        duel_archive_header=archive_model_dir / "TinyMLTreeEnsemble.h",
        context_archive_header=archive_model_dir / "TinyMLContextModel.h",
        rf_archive_joblib=archive_lib_dir / "TinyMLTreeEnsemble_RF.joblib",
        et_archive_joblib=archive_lib_dir / "TinyMLTreeEnsemble_ET.joblib",
        duel_archive_joblib=archive_lib_dir / "TinyMLTreeEnsemble.joblib",
        context_archive_joblib=archive_lib_dir / "TinyMLContextModel.joblib",
        rf_active_header=model_dir / "TinyMLTreeEnsemble_RF.h",
        et_active_header=model_dir / "TinyMLTreeEnsemble_ET.h",
        duel_active_header=model_dir / "TinyMLTreeEnsemble.h",
        context_active_header=model_dir / "TinyMLContextModel.h",
        rf_active_joblib=trainer_lib_dir / "TinyMLTreeEnsemble_RF.joblib",
        et_active_joblib=trainer_lib_dir / "TinyMLTreeEnsemble_ET.joblib",
        duel_active_joblib=trainer_lib_dir / "TinyMLTreeEnsemble.joblib",
        context_active_joblib=trainer_lib_dir / "TinyMLContextModel.joblib",
    )


def ensure_generation_dirs(paths: GenerationPaths) -> None:
    for folder in (
        paths.data_dir,
        paths.raw_dir,
        paths.benchmark_dir,
        paths.model_archive_dir,
        paths.trainer_lib_archive_dir,
    ):
        folder.mkdir(parents=True, exist_ok=True)


def init_generation_folders(project_root: str | Path, *, start: int = 0, end: int = MAX_GENERATION_INDEX) -> list[str]:
    created = []
    root = Path(project_root).resolve()
    for idx in range(int(start), int(end) + 1):
        gp = resolve_generation_paths(root, f"gen{idx}")
        for folder in (gp.data_dir, gp.raw_dir, gp.benchmark_dir, gp.model_archive_dir, gp.trainer_lib_archive_dir):
            if not folder.exists():
                folder.mkdir(parents=True, exist_ok=True)
                created.append(str(folder))
    return created


def validate_generation_workspace(
    paths: GenerationPaths,
    *,
    require_raw_folder: bool = False,
    require_raw_csv: bool = False,
    require_arc_csv: bool = False,
    require_context_csv: bool = False,
    require_nonempty_arc_csv: bool = False,
    require_nonempty_context_csv: bool = False,
) -> list[str]:
    errors: list[str] = []
    if require_raw_folder and not paths.raw_dir.is_dir():
        errors.append(f"Raw folder does not exist: {paths.raw_dir}")
    if require_raw_csv:
        if not paths.raw_dir.is_dir():
            errors.append(f"Raw folder does not exist: {paths.raw_dir}")
        elif not any(paths.raw_dir.rglob("*.csv")):
            errors.append(f"Raw folder is empty: {paths.raw_dir}")

    def _csv_has_rows(path: Path) -> bool:
        if not path.is_file():
            return False
        try:
            with path.open("r", encoding="utf-8", errors="ignore") as handle:
                next(handle, None)
                for line in handle:
                    if str(line).strip():
                        return True
        except Exception:
            return False
        return False

    if require_arc_csv and not paths.arc_csv.is_file():
        errors.append(f"Arc training CSV not found: {paths.arc_csv}")
    elif require_nonempty_arc_csv and not _csv_has_rows(paths.arc_csv):
        errors.append(f"Arc training CSV is empty for {paths.generation}: {paths.arc_csv}")

    if require_context_csv and not paths.context_csv.is_file():
        errors.append(f"Load-context CSV not found: {paths.context_csv}")
    elif require_nonempty_context_csv and not _csv_has_rows(paths.context_csv):
        errors.append(f"Load-context CSV is empty for {paths.generation}: {paths.context_csv}")
    return errors


def copy_if_exists(src: str | Path, dst: str | Path) -> str | None:
    source = Path(src)
    if not source.is_file():
        return None
    target = Path(dst)
    target.parent.mkdir(parents=True, exist_ok=True)
    shutil.copy2(source, target)
    return str(target)


def archive_generation_exports(paths: GenerationPaths, kinds: list[str] | tuple[str, ...] | None = None) -> dict[str, dict[str, str]]:
    requested = set(str(k).strip().lower() for k in (kinds or ("rf", "et", "duel", "context")))
    copies = {"headers": {}, "joblibs": {}}
    mapping = {
        "rf": ((paths.rf_active_header, paths.rf_archive_header), (paths.rf_active_joblib, paths.rf_archive_joblib)),
        "et": ((paths.et_active_header, paths.et_archive_header), (paths.et_active_joblib, paths.et_archive_joblib)),
        "duel": ((paths.duel_active_header, paths.duel_archive_header), (paths.duel_active_joblib, paths.duel_archive_joblib)),
        "context": ((paths.context_active_header, paths.context_archive_header), (paths.context_active_joblib, paths.context_archive_joblib)),
    }
    for kind, ((src_header, dst_header), (src_joblib, dst_joblib)) in mapping.items():
        if kind not in requested:
            continue
        copied_header = copy_if_exists(src_header, dst_header)
        copied_joblib = copy_if_exists(src_joblib, dst_joblib)
        if copied_header:
            copies["headers"][kind] = copied_header
        if copied_joblib:
            copies["joblibs"][kind] = copied_joblib
    return copies


def load_generation_manifest(paths: GenerationPaths) -> dict[str, Any]:
    if not paths.manifest_path.is_file():
        return {}
    try:
        payload = json.loads(paths.manifest_path.read_text(encoding="utf-8"))
    except Exception:
        return {}
    return payload if isinstance(payload, dict) else {}


def update_generation_manifest(paths: GenerationPaths, updates: dict[str, Any] | None = None) -> dict[str, Any]:
    ensure_generation_dirs(paths)
    base = {
        "generation": paths.generation,
        "raw_folder": str(paths.raw_dir),
        "arc_training_csv": str(paths.arc_csv),
        "load_context_csv": str(paths.context_csv),
        "report_paths": paths.report_paths(),
        "model_archive_paths": paths.archive_output_paths(),
        "canonical_export_paths": paths.canonical_export_paths(),
        "timestamp": _iso_now(),
    }
    merged = _deep_merge(load_generation_manifest(paths), base)
    merged = _deep_merge(merged, _jsonify(updates or {}))
    merged["timestamp"] = _iso_now()
    paths.manifest_path.parent.mkdir(parents=True, exist_ok=True)
    paths.manifest_path.write_text(json.dumps(merged, indent=2), encoding="utf-8")
    return merged
