$ErrorActionPreference = "Stop"
$Root = Split-Path -Parent $PSScriptRoot
$Desktop = Join-Path $Root "desktop"
if (-not (Test-Path (Join-Path $Desktop "index.html"))) {
    throw "Desktop files are missing. Pull the latest AMH-OS repository first."
}
$Python = Get-Command py -ErrorAction SilentlyContinue
if (-not $Python) { $Python = Get-Command python -ErrorAction SilentlyContinue }
if (-not $Python) { throw "Python was not found. Install Python 3 or add it to PATH." }
Write-Host "Starting AMH-OS Desktop at http://localhost:8765"
Write-Host "Keep this PowerShell window open. Press Ctrl+C to stop the local server."
& $Python.Source -m http.server 8765 --bind 127.0.0.1 --directory $Desktop
