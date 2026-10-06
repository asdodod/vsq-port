param(
    [string]$NdkPath = $env:ANDROID_NDK_HOME,
    [string]$NodePath = 'node',
    [string]$PresetDirectory = ''
)
$ErrorActionPreference = 'Stop'
if (-not $NdkPath) { throw 'Set ANDROID_NDK_HOME or pass -NdkPath.' }
$projectRoot = [IO.Path]::GetFullPath((Split-Path $PSScriptRoot -Parent))
$outputDirectory = Join-Path $projectRoot 'build/resource-validation'
New-Item -ItemType Directory -Path $outputDirectory -Force | Out-Null
$wasm = Join-Path $outputDirectory 'resources.wasm'
$reference = Join-Path $outputDirectory 'noise-reference.bin'
$compiler = Join-Path $NdkPath 'toolchains/llvm/prebuilt/windows-x86_64/bin/clang++.exe'
& $compiler --target=wasm32 -std=c++20 -O2 -nostdlib '-Wl,--no-entry' '-Wl,--export-all' `
    "-I$projectRoot/include" (Join-Path $projectRoot 'tests/resources_regression.cpp') -o $wasm
if ($LASTEXITCODE -ne 0) { throw 'Helper regression build failed.' }
$stream = [IO.File]::Create($reference)
$writer = [IO.BinaryWriter]::new($stream)
try {
    $random = [Random]::new(12345)
    for ($index = 0; $index -lt (32 * 32 * 32 * 3); ++$index) {
        $writer.Write([double]$random.NextDouble())
    }
} finally { $writer.Dispose(); $stream.Dispose() }
& $NodePath (Join-Path $projectRoot 'tests/resources_regression.mjs') $wasm $reference $PresetDirectory
if ($LASTEXITCODE -ne 0) { throw 'Resource regression failed.' }
