Param(
    [Parameter(Mandatory=$true)] [object] $Manifest,
    [string] $Binary = 'libvainsabers.so'
)

if ($Manifest.modloader -ne 'Scotland2' -or
    @($Manifest.modFiles).Count -ne 0 -or
    @($Manifest.lateModFiles).Count -ne 1 -or
    $Manifest.lateModFiles[0] -ne $Binary) {
    throw 'VainSabers must be packaged only in lateModFiles for Scotland2. An early_mods install can crash during game startup.'
}
