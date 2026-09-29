param(
    [string]$Archive = "",
    [string]$PrivateRoot = $(if ($env:MCM2_PRIVATE_ROOT) { $env:MCM2_PRIVATE_ROOT } else { Join-Path $env:LOCALAPPDATA "mcm2-private" })
)
$ErrorActionPreference = "Stop"
$RepoRoot = Split-Path -Parent $PSScriptRoot

if (-not (Get-Command python -ErrorAction SilentlyContinue)) {
    throw "python is required"
}

$args = @("$RepoRoot/tools/install_private_bundle.py", "--root", $PrivateRoot)
if ($Archive) {
    $args += @("--archive", (Resolve-Path $Archive).Path)
} else {
    if (-not $env:MCM2_PRIVATE_BUNDLE_URL) {
        throw "Pass -Archive or set MCM2_PRIVATE_BUNDLE_URL"
    }
    $args += @("--url-env", "MCM2_PRIVATE_BUNDLE_URL")
}
& python @args
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }

& python "$RepoRoot/tools/vc6_acceptance.py" --root $PrivateRoot
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }

Write-Host "VC6 private worker ready: $PrivateRoot"
Write-Host "Run commands through: python tools/with_private_env.py --root `"$PrivateRoot`" -- <command>"
