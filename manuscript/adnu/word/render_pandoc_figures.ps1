param(
  [switch]$GenerateOnly
)

$ErrorActionPreference = "Stop"

$root = Split-Path -Parent (Split-Path -Parent $MyInvocation.MyCommand.Path)
$outDir = Join-Path $root "word\rendered_figures"
New-Item -ItemType Directory -Force -Path $outDir | Out-Null

$labels = @(
  "fig:ch3-pwa-block",
  "fig:ch3-pwa-dashboard",
  "fig:ch3-pwa-history",
  "fig:ch3-full-schematic",
  "fig:ch3-data-labeling-deploy",
  "fig:ch3-tinyml-sequence",
  "fig:ch3-voltage-regression",
  "fig:ch3-current-regression",
  "fig:ch3-ntc-cal-setup",
  "fig:ch3-overload-block",
  "fig:ch3-heating-block",
  "fig:ch3-arc-block",
  "fig:ch4-char-water-heater",
  "fig:ch4-char-one-lamp",
  "fig:ch4-char-two-lamps",
  "fig:ch4-char-three-lamps",
  "fig:ch4-char-four-lamps",
  "fig:ch4-protect-water-heater",
  "fig:ch4-protect-one-lamp",
  "fig:ch4-protect-two-lamps",
  "fig:ch4-protect-three-lamps",
  "fig:ch4-protect-four-lamps",
  "fig:ch4-group-metrics",
  "fig:ch4-setup-outcomes",
  "fig:ch4-family-outcomes",
  "fig:app-h-gantt-chart"
)

function Convert-LabelToName {
  param([string]$Label)
  return ($Label -replace "[^A-Za-z0-9]+", "_").Trim("_")
}

function Remove-LaTeXCaption {
  param([string]$Text)

  $patterns = @(
    "\\caption\[[^\]]*\]\{[^{}]*(?:\{[^{}]*\}[^{}]*)*\}",
    "\\caption\{[^{}]*(?:\{[^{}]*\}[^{}]*)*\}"
  )
  foreach ($pattern in $patterns) {
    $Text = [regex]::Replace($Text, $pattern, "", "Singleline")
  }
  return $Text
}

$figureBlocks = @{}
$sourceFiles = Get-ChildItem -LiteralPath (Join-Path $root "chapters"), (Join-Path $root "appendices") -Recurse -Filter "*.tex"
foreach ($file in $sourceFiles) {
  $content = Get-Content -LiteralPath $file.FullName -Raw
  foreach ($match in [regex]::Matches($content, "\\begin\{figure\}(?:\[[^\]]*\])?(.*?)\\end\{figure\}", "Singleline")) {
    $block = $match.Groups[1].Value
    $labelMatch = [regex]::Match($block, "\\label\{([^}]+)\}")
    if ($labelMatch.Success) {
      $figureBlocks[$labelMatch.Groups[1].Value] = $block
    }
  }
}

$preamble = @"
\documentclass[border=6pt]{standalone}
\usepackage{fontspec}
\setmainfont{Times New Roman}
\usepackage{graphicx}
\usepackage{tikz}
\usetikzlibrary{arrows.meta,calc,fit,positioning,shapes.geometric}
\usepackage{pgfplots}
\pgfplotsset{compat=1.18}
\usepackage{amsmath,amssymb}
\usepackage{array}
\usepackage{booktabs}
\usepackage{xcolor}
\graphicspath{{figures/}{figures/ch3/}{figures/ch4/}{figures/apx/}}
\newcommand{\degreeC}{\ensuremath{^\circ\mathrm{C}}}
\newcommand{\figsource}[1]{}
\tikzset{
  thesis box/.style={
    draw,
    rounded corners=1pt,
    align=center,
    inner sep=4pt,
    minimum height=0.36in,
    font=\small
  },
  thesis process/.style={
    thesis box,
    fill=gray!8
  },
  thesis decision/.style={
    diamond,
    draw,
    aspect=2,
    align=center,
    inner sep=2pt,
    font=\small,
    fill=gray!8
  },
  thesis arrow/.style={
    -{Latex[length=2.2mm]},
    line width=0.65pt
  },
  thesis bus/.style={
    line width=0.8pt
  }
}
\input{figures/ch4/socket_plots.tex}
"@

foreach ($label in $labels) {
  if (-not $figureBlocks.ContainsKey($label)) {
    throw "Could not find source figure block for $label."
  }

  $name = Convert-LabelToName $label
  $body = $figureBlocks[$label]
  $body = Remove-LaTeXCaption $body
  $body = [regex]::Replace($body, "\\label\{[^}]+\}", "")
  $body = [regex]::Replace($body, "\\figsource\{[^}]*\}", "")
  if ($label -eq "fig:ch3-full-schematic") {
    $body = "\centering`n\includegraphics[angle=90,width=7.5in,keepaspectratio,trim=0.15in 0.50in 0.15in 0.20in,clip]{ch3/full_schematic.pdf}"
  }

  $texPath = Join-Path $outDir "$name.tex"
  $tex = @"
$preamble
\begin{document}
$body
\end{document}
"@
  Set-Content -LiteralPath $texPath -Value $tex -Encoding UTF8
  if ($GenerateOnly) {
    continue
  }
  Start-Sleep -Milliseconds 500

  $relativeOutDir = "word\rendered_figures"
  $relativeTexPath = Join-Path $relativeOutDir "$name.tex"
  xelatex -interaction=nonstopmode -halt-on-error -output-directory=$relativeOutDir $relativeTexPath | Out-Null
  if ($LASTEXITCODE -ne 0) {
    throw "xelatex failed while rendering $label."
  }

  $pdfPath = Join-Path $outDir "$name.pdf"
  for ($i = 0; $i -lt 10 -and -not (Test-Path $pdfPath); $i++) {
    Start-Sleep -Milliseconds 300
  }
  if (-not (Test-Path $pdfPath)) {
    xelatex -interaction=nonstopmode -halt-on-error -output-directory=$relativeOutDir $relativeTexPath | Out-Null
    for ($i = 0; $i -lt 10 -and -not (Test-Path $pdfPath); $i++) {
      Start-Sleep -Milliseconds 300
    }
  }
  if (-not (Test-Path $pdfPath)) {
    throw "xelatex did not produce $pdfPath while rendering $label."
  }

  $pngStem = Join-Path $outDir $name
  pdftocairo -png -singlefile -r 220 $pdfPath $pngStem | Out-Null
  if ($LASTEXITCODE -ne 0) {
    throw "pdftocairo failed while rendering $label."
  }
}

Write-Host "Rendered $($labels.Count) LaTeX figures into $outDir"
