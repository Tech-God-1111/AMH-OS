param(
    [Parameter(Mandatory=$true)][string]$Port,
    [switch]$ConfirmFlash
)
. "$PSScriptRoot\_common.ps1"
Assert-IdfEnvironment
Push-Location $ProjectRoot
try {
    if (-not (Test-Path ".\build\amh_os_v3.bin")) {
        throw "Firmware binary not found. Build and inspect it first."
    }
    $binary = Get-Item ".\\build\\amh_os_v3.bin"
    if ($binary.Length -gt 2MB) { throw "Firmware exceeds the 2 MB OTA app slot." }
    if (-not (Test-Path ".\\build\\partition_table\\partition-table.bin")) { throw "Partition table binary is missing." }
    Write-Host "About to write AMH-OS 3.0 to $Port. This script does not erase the full flash." -ForegroundColor Yellow
    idf.py -p $Port flash
    if ($LASTEXITCODE -ne 0) { throw "Flash operation failed." }
} finally {
    Pop-Location
}
