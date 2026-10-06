Param([string] $QmodPath = '')
$ErrorActionPreference = 'Stop'
$projectRoot = Split-Path $PSScriptRoot -Parent
$guard = Join-Path $PSScriptRoot 'assert-load-phase.ps1'
$template = Get-Content (Join-Path $projectRoot 'mod.template.json') -Raw | ConvertFrom-Json
$manifest = Get-Content (Join-Path $projectRoot 'mod.json') -Raw | ConvertFrom-Json
& $guard -Manifest $template -Binary '${binary}'
& $guard -Manifest $manifest

# An accidental switch back to early loading, or a duplicate in both phases,
# must fail even though both manifests remain valid QMOD JSON.
foreach ($duplicate in @($false, $true)) {
    $bad = $manifest | ConvertTo-Json -Depth 20 | ConvertFrom-Json
    $bad.modFiles = @('libvainsabers.so')
    $bad.lateModFiles = if ($duplicate) { @('libvainsabers.so') } else { @() }
    $rejected = $false
    try { & $guard -Manifest $bad } catch { $rejected = $true }
    if (-not $rejected) { throw 'Load-phase guard accepted an early/duplicate install' }
}

if ($QmodPath) {
    Add-Type -AssemblyName System.IO.Compression.FileSystem
    $zip = [IO.Compression.ZipFile]::OpenRead((Resolve-Path -LiteralPath $QmodPath).Path)
    try {
        $entry = $zip.GetEntry('mod.json')
        if (-not $entry -or -not $zip.GetEntry('libvainsabers.so')) { throw 'Incomplete QMOD' }
        $reader = [IO.StreamReader]::new($entry.Open())
        try { $packed = $reader.ReadToEnd() | ConvertFrom-Json } finally { $reader.Dispose() }
        & $guard -Manifest $packed
        if ($packed.version -ne $manifest.version) { throw 'Packaged version differs from mod.json' }
    } finally { $zip.Dispose() }
}
Write-Output 'Load-phase regression passed: template, manifest, rejected early/duplicate installs, and optional QMOD.'
