param(
    [Parameter(Mandatory=$true)][string]$Port
)
. "$PSScriptRoot\_common.ps1"
Assert-IdfEnvironment
Push-Location $ProjectRoot
try {
    if (-not (Test-Path ".\build\amh_os_v3.bin")) {
        throw "Firmware binary not found. Build and inspect it first."
    }
    Write-Host "About to write AMH-OS 3.0 to $Port. This script does not erase the full flash." -ForegroundColor Yellow
    idf.py -p $Port flash
    if ($LASTEXITCODE -ne 0) { throw "Flash operation failed." }
} finally {
    Pop-Location
}
