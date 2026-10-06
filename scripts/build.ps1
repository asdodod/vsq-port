Param(
    [Parameter(Mandatory=$false)]
    [Switch] $clean,

    [Parameter(Mandatory=$false)]
    [Switch] $help
)

if ($help -eq $true) {
    Write-Output 'Build the Quest library. Use -clean to rebuild from scratch.'
    Write-Output "`n-- Arguments --`n"

    Write-Output "-Clean `t`t Deletes the `"build`" folder, so that the entire library is rebuilt"

    exit
}

# Resolve and check the directory before a requested clean build.
$projectRoot = [IO.Path]::GetFullPath((Split-Path $PSScriptRoot -Parent))
$buildPath = [IO.Path]::GetFullPath((Join-Path $projectRoot 'build'))
if ($clean.IsPresent) {
    if (Test-Path -LiteralPath $buildPath) {
        if ((Split-Path $buildPath -Parent) -ne $projectRoot) {
            throw 'Refusing to clean outside the project.'
        }
        Remove-Item -LiteralPath $buildPath -Recurse -Force
    }
}


if (($clean.IsPresent) -or (-not (Test-Path -Path "build"))) {
    new-item -Path build -ItemType Directory
} 

& cmake -G "Ninja" -DCMAKE_BUILD_TYPE="RelWithDebInfo" -B build
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }
& cmake --build ./build -j 4
exit $LASTEXITCODE
