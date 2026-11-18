param(
    [Parameter(Mandatory = $true)]
    [ValidatePattern("^COM[0-9]+$")]
    [string]$Port
)
$ErrorActionPreference = "Stop"
$Root = Split-Path -Parent $PSScriptRoot
$Firmware = Join-Path $Root "firmware"

Write-Host "AMH-OS Lite application deployment"
Write-Host "Port: $Port"
Write-Host "Copies application files only; does NOT erase flash or install firmware."

if (-not (Get-Command mpremote -ErrorAction SilentlyContinue)) {
    throw "mpremote not found. Install with: py -m pip install --upgrade mpremote"
}
if (-not (Test-Path (Join-Path $Firmware "main.py"))) {
    throw "Firmware application folder is incomplete."
}

Write-Host "[1/4] Copying Python package..."
& mpremote connect $Port fs cp -r (Join-Path $Firmware "amh") ":"
if ($LASTEXITCODE -ne 0) { throw "Failed to copy Python package." }

Write-Host "[2/4] Copying dashboard assets..."
& mpremote connect $Port fs cp -r (Join-Path $Firmware "web") ":"
if ($LASTEXITCODE -ne 0) { throw "Failed to copy dashboard assets." }

Write-Host "[3/4] Copying startup files..."
& mpremote connect $Port fs cp (Join-Path $Firmware "boot.py") ":boot.py"
if ($LASTEXITCODE -ne 0) { throw "Failed to copy boot.py." }
& mpremote connect $Port fs cp (Join-Path $Firmware "main.py") ":main.py"
if ($LASTEXITCODE -ne 0) { throw "Failed to copy main.py." }

Write-Host "[4/4] Resetting board..."
& mpremote connect $Port reset
if ($LASTEXITCODE -ne 0) { Write-Warning "Reset failed; reset the board manually." }
Write-Host "Deployment commands completed. Monitor serial at 115200 baud."
