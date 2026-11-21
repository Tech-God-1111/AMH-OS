param(
    [string]$IdfPath = "D:\Espressif\.espressif\v5.5.6\esp-idf",
    [string]$IdfToolsPath = "D:\Espressif",
    [string]$PythonEnvPath = "D:\Espressif\tools\python\v5.5.6\venv"
)

$ErrorActionPreference = "Stop"

Write-Host "========== AMH-OS V3.0 ESP-IDF SETUP ==========" -ForegroundColor Cyan

if (-not (Test-Path (Join-Path $IdfPath "export.ps1"))) {
    throw "ESP-IDF export.ps1 not found at: $IdfPath. Pass the correct -IdfPath."
}
if (-not (Test-Path $PythonEnvPath)) {
    throw "ESP-IDF Python environment not found at: $PythonEnvPath."
}

$env:IDF_PATH = $IdfPath
$env:IDF_TOOLS_PATH = $IdfToolsPath
$env:IDF_PYTHON_ENV_PATH = $PythonEnvPath

. (Join-Path $IdfPath "export.ps1")

if (-not (Get-Command idf.py -ErrorAction SilentlyContinue)) {
    throw "idf.py is not available after ESP-IDF activation."
}

idf.py --version
if ($LASTEXITCODE -ne 0) {
    throw "ESP-IDF version check failed."
}

Write-Host "ESP-IDF environment is ready for AMH-OS V3.0." -ForegroundColor Green
Write-Host "Next: .\scripts\verify.ps1" -ForegroundColor Green
Write-Host "No build, flash, or erase operation was performed."
