$ErrorActionPreference = "Stop"

$wordDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$runId = [guid]::NewGuid().ToString("N")
$referenceDocx = Join-Path $wordDir "reference.docx"
$seedDocx = Join-Path $wordDir ("_reference_seed_$runId.docx")
$patchedDocx = Join-Path $wordDir ("_reference_patched_$runId.docx")
$workDir = Join-Path $wordDir ("_reference_work_$runId")

if (-not (Get-Command pandoc -ErrorAction SilentlyContinue)) {
  throw "Pandoc is required to create the reference DOCX."
}

New-Item -ItemType Directory -Force -Path $workDir | Out-Null

pandoc -o $seedDocx --print-default-data-file reference.docx

Add-Type -AssemblyName System.IO.Compression.FileSystem
[System.IO.Compression.ZipFile]::ExtractToDirectory($seedDocx, $workDir)

$stylesPath = Join-Path $workDir "word\styles.xml"
$documentPath = Join-Path $workDir "word\document.xml"
$settingsPath = Join-Path $workDir "word\settings.xml"

$ns = New-Object System.Xml.XmlNamespaceManager((New-Object System.Xml.NameTable))
$ns.AddNamespace("w", "http://schemas.openxmlformats.org/wordprocessingml/2006/main")

function Set-Attr {
  param(
    [System.Xml.XmlElement]$Element,
    [string]$LocalName,
    [string]$Value
  )
  [void]$Element.SetAttribute($LocalName, "http://schemas.openxmlformats.org/wordprocessingml/2006/main", $Value)
}

function Ensure-Child {
  param(
    [System.Xml.XmlNode]$Parent,
    [string]$LocalName
  )
  $child = $Parent.SelectSingleNode("w:$LocalName", $ns)
  if ($null -eq $child) {
    $child = $Parent.OwnerDocument.CreateElement("w", $LocalName, "http://schemas.openxmlformats.org/wordprocessingml/2006/main")
    [void]$Parent.AppendChild($child)
  }
  return [System.Xml.XmlElement]$child
}

function Set-RunFont {
  param(
    [System.Xml.XmlElement]$RunPr,
    [string]$SizeHalfPoints,
    [bool]$Bold = $false,
    [bool]$Italic = $false
  )
  $rFonts = Ensure-Child $RunPr "rFonts"
  foreach ($name in @("ascii", "hAnsi", "cs", "eastAsia")) {
    Set-Attr $rFonts $name "Times New Roman"
  }

  $sz = Ensure-Child $RunPr "sz"
  Set-Attr $sz "val" $SizeHalfPoints
  $szCs = Ensure-Child $RunPr "szCs"
  Set-Attr $szCs "val" $SizeHalfPoints

  $color = Ensure-Child $RunPr "color"
  Set-Attr $color "val" "000000"
  [void]$color.RemoveAttribute("themeColor", "http://schemas.openxmlformats.org/wordprocessingml/2006/main")
  [void]$color.RemoveAttribute("themeShade", "http://schemas.openxmlformats.org/wordprocessingml/2006/main")
  [void]$color.RemoveAttribute("themeTint", "http://schemas.openxmlformats.org/wordprocessingml/2006/main")

  $existingBold = $RunPr.SelectSingleNode("w:b", $ns)
  if ($Bold -and $null -eq $existingBold) {
    [void]$RunPr.AppendChild($RunPr.OwnerDocument.CreateElement("w", "b", "http://schemas.openxmlformats.org/wordprocessingml/2006/main"))
  } elseif (-not $Bold -and $null -ne $existingBold) {
    [void]$RunPr.RemoveChild($existingBold)
  }

  $existingItalic = $RunPr.SelectSingleNode("w:i", $ns)
  if ($Italic -and $null -eq $existingItalic) {
    [void]$RunPr.AppendChild($RunPr.OwnerDocument.CreateElement("w", "i", "http://schemas.openxmlformats.org/wordprocessingml/2006/main"))
  } elseif (-not $Italic -and $null -ne $existingItalic) {
    [void]$RunPr.RemoveChild($existingItalic)
  }
}

function Set-PageBreakBefore {
  param(
    [System.Xml.XmlDocument]$Doc,
    [string]$StyleId
  )

  $style = $Doc.SelectSingleNode("//w:style[@w:styleId='$StyleId']", $ns)
  if ($null -eq $style) {
    return
  }

  $pPr = Ensure-Child $style "pPr"
  $pageBreak = $pPr.SelectSingleNode("w:pageBreakBefore", $ns)
  if ($null -eq $pageBreak) {
    [void]$pPr.AppendChild($pPr.OwnerDocument.CreateElement("w", "pageBreakBefore", "http://schemas.openxmlformats.org/wordprocessingml/2006/main"))
  }
}

function Set-HangingStyle {
  param(
    [System.Xml.XmlDocument]$Doc,
    [string]$StyleId,
    [string]$Left = "720",
    [string]$Hanging = "720",
    [string]$Line = "360",
    [string]$SizeHalfPoints = "24"
  )

  $style = $Doc.SelectSingleNode("//w:style[@w:styleId='$StyleId']", $ns)
  if ($null -eq $style) {
    return
  }

  $pPr = Ensure-Child $style "pPr"
  $jc = Ensure-Child $pPr "jc"
  Set-Attr $jc "val" "both"

  $spacing = Ensure-Child $pPr "spacing"
  Set-Attr $spacing "before" "0"
  Set-Attr $spacing "after" "0"
  Set-Attr $spacing "line" $Line
  Set-Attr $spacing "lineRule" "auto"

  $ind = Ensure-Child $pPr "ind"
  Set-Attr $ind "left" $Left
  Set-Attr $ind "hanging" $Hanging
  [void]$ind.RemoveAttribute("firstLine", "http://schemas.openxmlformats.org/wordprocessingml/2006/main")

  $rPr = Ensure-Child $style "rPr"
  Set-RunFont $rPr $SizeHalfPoints $false $false
}

function Set-ParaStyle {
  param(
    [System.Xml.XmlDocument]$Doc,
    [string]$StyleId,
    [string]$Justification = "both",
    [string]$Line = "480",
    [string]$FirstLine = "851",
    [string]$Before = "0",
    [string]$After = "0",
    [string]$SizeHalfPoints = "24",
    [bool]$Bold = $false,
    [bool]$Italic = $false
  )

  $style = $Doc.SelectSingleNode("//w:style[@w:styleId='$StyleId']", $ns)
  if ($null -eq $style) {
    return
  }

  $pPr = Ensure-Child $style "pPr"
  $jc = Ensure-Child $pPr "jc"
  Set-Attr $jc "val" $Justification

  $spacing = Ensure-Child $pPr "spacing"
  Set-Attr $spacing "before" $Before
  Set-Attr $spacing "after" $After
  Set-Attr $spacing "line" $Line
  Set-Attr $spacing "lineRule" "auto"

  $ind = Ensure-Child $pPr "ind"
  Set-Attr $ind "firstLine" $FirstLine

  $rPr = Ensure-Child $style "rPr"
  Set-RunFont $rPr $SizeHalfPoints $Bold $Italic
}

[xml]$styles = Get-Content -LiteralPath $stylesPath -Raw

$docDefaults = $styles.SelectSingleNode("//w:docDefaults", $ns)
if ($null -ne $docDefaults) {
  $rPrDefault = Ensure-Child (Ensure-Child $docDefaults "rPrDefault") "rPr"
  Set-RunFont $rPrDefault "24"

  $pPrDefault = Ensure-Child (Ensure-Child $docDefaults "pPrDefault") "pPr"
  $jc = Ensure-Child $pPrDefault "jc"
  Set-Attr $jc "val" "both"
  $spacing = Ensure-Child $pPrDefault "spacing"
  Set-Attr $spacing "before" "0"
  Set-Attr $spacing "after" "0"
  Set-Attr $spacing "line" "480"
  Set-Attr $spacing "lineRule" "auto"
  $ind = Ensure-Child $pPrDefault "ind"
  Set-Attr $ind "firstLine" "851"
}

foreach ($styleId in @("Normal", "BodyText", "FirstParagraph", "Compact")) {
  Set-ParaStyle $styles $styleId -Justification "both" -Line "480" -FirstLine "851" -SizeHalfPoints "24"
}

Set-ParaStyle $styles "Title" -Justification "center" -Line "240" -FirstLine "0" -After "0" -SizeHalfPoints "24" -Bold $true
Set-ParaStyle $styles "Subtitle" -Justification "center" -Line "240" -FirstLine "0" -After "0" -SizeHalfPoints "24"
Set-ParaStyle $styles "Heading1" -Justification "center" -Line "312" -FirstLine "0" -Before "0" -After "180" -SizeHalfPoints "26" -Bold $true
Set-ParaStyle $styles "Heading2" -Justification "left" -Line "312" -FirstLine "0" -Before "360" -After "120" -SizeHalfPoints "26" -Bold $true
Set-ParaStyle $styles "Heading3" -Justification "left" -Line "288" -FirstLine "0" -Before "240" -After "120" -SizeHalfPoints "24" -Bold $true
Set-ParaStyle $styles "Heading4" -Justification "left" -Line "288" -FirstLine "0" -Before "240" -After "120" -SizeHalfPoints "24"
Set-ParaStyle $styles "Caption" -Justification "center" -Line "240" -FirstLine "0" -Before "0" -After "120" -SizeHalfPoints "24"
Set-ParaStyle $styles "TableCaption" -Justification "center" -Line "240" -FirstLine "0" -Before "0" -After "120" -SizeHalfPoints "24" -Bold $true
Set-ParaStyle $styles "FigureCaption" -Justification "center" -Line "240" -FirstLine "0" -Before "0" -After "120" -SizeHalfPoints "24" -Bold $true
Set-ParaStyle $styles "TOCHeading" -Justification "center" -Line "312" -FirstLine "0" -Before "0" -After "180" -SizeHalfPoints "26" -Bold $true
Set-ParaStyle $styles "TOC1" -Justification "left" -Line "240" -FirstLine "0" -Before "0" -After "0" -SizeHalfPoints "24"
Set-ParaStyle $styles "TOC2" -Justification "left" -Line "240" -FirstLine "0" -Before "0" -After "0" -SizeHalfPoints "24"
Set-ParaStyle $styles "TOC3" -Justification "left" -Line "240" -FirstLine "0" -Before "0" -After "0" -SizeHalfPoints "24"
Set-HangingStyle $styles "Bibliography"
Set-PageBreakBefore $styles "Heading1"

$styles.Save($stylesPath)

[xml]$document = Get-Content -LiteralPath $documentPath -Raw
foreach ($sectPr in $document.SelectNodes("//w:sectPr", $ns)) {
  $pgSz = Ensure-Child $sectPr "pgSz"
  Set-Attr $pgSz "w" "11906"
  Set-Attr $pgSz "h" "16838"

  $pgMar = Ensure-Child $sectPr "pgMar"
  Set-Attr $pgMar "top" "1440"
  Set-Attr $pgMar "bottom" "1440"
  Set-Attr $pgMar "left" "1800"
  Set-Attr $pgMar "right" "1440"
  Set-Attr $pgMar "header" "720"
  Set-Attr $pgMar "footer" "720"
  Set-Attr $pgMar "gutter" "0"
}
$document.Save($documentPath)

[xml]$settings = Get-Content -LiteralPath $settingsPath -Raw
$updateFields = $settings.SelectSingleNode("//w:updateFields", $ns)
if ($null -eq $updateFields) {
  $updateFields = $settings.CreateElement("w", "updateFields", "http://schemas.openxmlformats.org/wordprocessingml/2006/main")
  [void]$settings.DocumentElement.AppendChild($updateFields)
}
Set-Attr $updateFields "val" "true"
$settings.Save($settingsPath)

[System.IO.Compression.ZipFile]::CreateFromDirectory($workDir, $patchedDocx)
Copy-Item -LiteralPath $patchedDocx -Destination $referenceDocx -Force

try {
  [System.IO.File]::Delete($seedDocx)
  [System.IO.File]::Delete($patchedDocx)
  [System.IO.Directory]::Delete($workDir, $true)
} catch {
  Write-Warning "Reference DOCX was created, but temporary files could not be removed: $($_.Exception.Message)"
}

Write-Host "Created $referenceDocx"
