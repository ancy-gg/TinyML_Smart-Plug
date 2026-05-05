$ErrorActionPreference = "Stop"

$root = Split-Path -Parent $PSScriptRoot
Set-Location $root

$env:Path = "C:\Strawberry\perl\bin;C:\Strawberry\c\bin;$env:Path"

New-Item -ItemType Directory -Force -Path "pdf", "logs" | Out-Null

$commonArgs = @(
  "-xelatex",
  "-interaction=nonstopmode",
  "-halt-on-error",
  "-auxdir=logs",
  "-outdir=pdf"
)

$sectionBuilds = @(
  "build/misc/build_title.tex",
  "build/misc/build_approval_form.tex",
  "build/misc/build_abstract.tex",
  "build/misc/build_acknowledgement.tex",
  "build/misc/build_table_of_contents.tex",
  "build/misc/build_list_of_tables.tex",
  "build/misc/build_list_of_figures.tex",
  "build/misc/build_references.tex",
  "build/chapters/build_chapter_1.tex",
  "build/chapters/build_chapter_2.tex",
  "build/chapters/build_chapter_3.tex",
  "build/chapters/build_chapter_4.tex",
  "build/chapters/build_chapter_5.tex",
  "build/appendices/build_appendix_a.tex",
  "build/appendices/build_appendix_b.tex",
  "build/appendices/build_appendix_c.tex",
  "build/appendices/build_appendix_d.tex",
  "build/appendices/build_appendix_e.tex",
  "build/appendices/build_appendix_f.tex",
  "build/appendices/build_appendix_g.tex",
  "build/appendices/build_appendix_h.tex",
  "build/appendices/build_appendix_i.tex",
  "build/chapters/chapter_3/build_ch3_01_system_model.tex",
  "build/chapters/chapter_3/build_ch3_02_program_flow.tex",
  "build/chapters/chapter_3/build_ch3_03_physical_setup.tex",
  "build/chapters/chapter_3/build_ch3_04_pwa_monitoring.tex",
  "build/chapters/chapter_3/build_ch3_05_tinyml_workflow.tex",
  "build/chapters/chapter_3/build_ch3_06_sensor_calibration.tex",
  "build/chapters/chapter_3/build_ch3_07_waveform_features.tex",
  "build/chapters/chapter_3/build_ch3_08_protection_logic.tex",
  "build/chapters/chapter_3/build_ch3_09_test_evaluation.tex",
  "build/chapters/chapter_4/build_ch4_01_sensor_validation.tex",
  "build/chapters/chapter_4/build_ch4_02_overload_protection.tex",
  "build/chapters/chapter_4/build_ch4_03_socket_characterization.tex",
  "build/chapters/chapter_4/build_ch4_04_socket_protection.tex",
  "build/chapters/chapter_4/build_ch4_05_arc_detection.tex",
  "build/chapters/chapter_4/build_ch4_06_tinyml_metrics.tex",
  "build/chapters/chapter_4/build_ch4_07_feasibility.tex"
)

# Build the full manuscript first so standalone wrappers can import current labels and lists.
latexmk @commonArgs "main.tex"

foreach ($file in $sectionBuilds) {
  latexmk @commonArgs $file
}

# Rebuild the full manuscript last after all section checks.
latexmk @commonArgs "main.tex"
