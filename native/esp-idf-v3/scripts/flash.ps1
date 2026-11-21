param(
    [Parameter(Mandatory=$true)][string]$Port,
    [switch]$ConfirmFlash
)

. "$PSScriptRoot\_common.ps1"
Assert-IdfEnvironment
Push-Location $ProjectRoot
try {
    if (-not $ConfirmFlash) {
        throw "Flash is blocked. Re-run with -ConfirmFlash only after reviewing the successful verification report."
    }

    $binaryPath = ".\build\amh_os_v3.bin"
    $partitionPath = ".\build\partition_table\partition-table.bin"

    if (-not (Test-Path $binaryPath)) {
        throw "Firmware binary not found. Run scripts\verify.ps1 first."
    }
    if (-not (Test-Path $partitionPath)) {
        throw "Partition table binary is missing. Run scripts\verify.ps1 first."
    }

    $binary = Get-Item $binaryPath
    if ($binary.Length -gt 2MB) {
        throw "Firmware exceeds the 2 MB OTA application slot."
    }

    Write-Host "AMH-OS V3.0 flash target: $Port" -ForegroundColor Yellow
    Write-Host ("Firmware image: {0:N0} bytes" -f $binary.Length) -ForegroundColor Cyan
    Write-Host "No full-chip erase command will be used." -ForegroundColor Green

    idf.py -p $Port flash
    if ($LASTEXITCODE -ne 0) {
        throw "Flash operation failed."
    }

    Write-Host "Flash command completed. Run scripts\monitor.ps1 -Port $Port to inspect boot logs." -ForegroundColor Green
} finally {
    Pop-Location
}
