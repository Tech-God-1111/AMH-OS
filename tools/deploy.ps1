param(
    [Parameter(Mandatory = $true)]
    [ValidatePattern("^COM[0-9]+$")]
    [string]$Port
)
$ErrorActionPreference = "Stop"
$Root = Split-Path -Parent $PSScriptRoot
$Firmware = Join-Path $Root "firmware"

Write-Host "AMH-OS 0.3 application deployment"
Write-Host "Port: $Port"
Write-Host "Copies application files only; this does NOT erase flash or install MicroPython."

if (-not (Test-Path (Join-Path $Firmware "main.py"))) {
    throw "Firmware application folder is incomplete."
}

$Mpremote = Get-Command mpremote -ErrorAction SilentlyContinue
$Python = Get-Command python -ErrorAction SilentlyContinue
$Py = Get-Command py -ErrorAction SilentlyContinue
if (-not $Mpremote -and -not $Python -and -not $Py) {
    throw "Python was not found. Install Python and mpremote: py -m pip install --upgrade mpremote"
}

function Invoke-Mpremote {
    param([Parameter(ValueFromRemainingArguments = $true)][string[]]$Arguments)
    if ($Mpremote) {
        & $Mpremote.Source @Arguments
    }
    elseif ($Python) {
        & $Python.Source -m mpremote @Arguments
    }
    else {
        & $Py.Source -m mpremote @Arguments
    }
    if ($LASTEXITCODE -ne 0) {
        throw "mpremote command failed: $($Arguments -join ' ')"
    }
}

Write-Host "[1/4] Copying Python package..."
Invoke-Mpremote connect $Port fs cp -r (Join-Path $Firmware "amh") ":"

Write-Host "[2/4] Copying dashboard assets..."
Invoke-Mpremote connect $Port fs cp -r (Join-Path $Firmware "web") ":"

Write-Host "[3/4] Copying startup files..."
Invoke-Mpremote connect $Port fs cp (Join-Path $Firmware "boot.py") ":boot.py"
Invoke-Mpremote connect $Port fs cp (Join-Path $Firmware "main.py") ":main.py"

Write-Host "[4/4] Resetting board..."
Invoke-Mpremote connect $Port reset
Write-Host "Deployment commands completed. USB serial protocol uses 115200 baud."
