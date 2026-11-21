$ErrorActionPreference = "Stop"
. "$PSScriptRoot\_common.ps1"
Assert-IdfEnvironment
Push-Location $ProjectRoot
try {
    Write-Host "========== PYTHON SOURCE CHECKS ==========" -ForegroundColor Cyan
    python -m py_compile .\pc-companion\amh_companion.py
    if ($LASTEXITCODE -ne 0) { throw "Companion Python syntax check failed." }
    python -m unittest discover -s .\tests -v
    if ($LASTEXITCODE -ne 0) { throw "AMH-OS V3 automated tests failed." }

    Write-Host "========== DESKTOP JAVASCRIPT CHECK ==========" -ForegroundColor Cyan
    $node = Get-Command node -ErrorAction SilentlyContinue
    if ($node) {
        $html = Get-Content ".\main\web\index.html" -Raw
        $match = [regex]::Match($html, '(?s)<script>(.*?)</script>')
        if (-not $match.Success) { throw "Desktop JavaScript block not found." }
        $jsFile = Join-Path $env:TEMP "amh-os-v3-desktop-check.js"
        [System.IO.File]::WriteAllText($jsFile, $match.Groups[1].Value, (New-Object System.Text.UTF8Encoding($false)))
        node --check $jsFile
        if ($LASTEXITCODE -ne 0) { throw "Desktop JavaScript syntax check failed." }
    } else {
        Write-Host "Node.js not installed; JavaScript syntax check will run in GitHub CI." -ForegroundColor Yellow
    }

    Write-Host "========== ESP-IDF BUILD ==========" -ForegroundColor Cyan
    idf.py set-target esp32s3
    if ($LASTEXITCODE -ne 0) { throw "ESP32-S3 target configuration failed." }
    idf.py build
    if ($LASTEXITCODE -ne 0) { throw "Firmware build failed." }

    Write-Host "========== FIRMWARE SIZE ==========" -ForegroundColor Cyan
    idf.py size
    if ($LASTEXITCODE -ne 0) { throw "Firmware size analysis failed." }

    $binary = Get-Item ".\build\amh_os_v3.bin"
    if ($binary.Length -gt 2MB) {
        throw "Firmware image exceeds the 2 MB OTA application slot."
    }
    Write-Host ("Firmware image: {0:N0} bytes" -f $binary.Length) -ForegroundColor Green
    Write-Host "ALL LOCAL CHECKS PASSED. NO FLASH OR ERASE WAS PERFORMED." -ForegroundColor Green
} finally {
    Pop-Location
}
