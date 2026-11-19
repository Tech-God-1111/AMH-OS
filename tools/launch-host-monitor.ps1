$ErrorActionPreference = "Stop"
$Root = Split-Path -Parent $PSScriptRoot
$Monitor = Join-Path $Root "host\amh_host_monitor.py"
$Requirements = Join-Path $Root "host\requirements.txt"

if (-not (Test-Path $Monitor)) {
    throw "Host monitor file is missing. Pull the latest AMH-OS main branch."
}

$Python = Get-Command py -ErrorAction SilentlyContinue
if (-not $Python) {
    $Python = Get-Command python -ErrorAction SilentlyContinue
}
if (-not $Python) {
    throw "Python was not found. Install Python 3 and add it to PATH."
}

& $Python.Source -c "import psutil" 2>$null
if ($LASTEXITCODE -ne 0) {
    Write-Host "Installing the optional psutil dependency..."
    & $Python.Source -m pip install -r $Requirements
    if ($LASTEXITCODE -ne 0) {
        throw "Could not install host monitor requirements."
    }
}

Write-Host "Starting optional AMH-OS Windows Host Monitor..."
Write-Host "This service is read-only and listens only on 127.0.0.1:8766."
Write-Host "Keep this PowerShell window open. Press Ctrl+C to stop."
& $Python.Source $Monitor
