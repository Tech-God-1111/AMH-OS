param([switch]$Clean)
. "$PSScriptRoot\_common.ps1"
Assert-IdfEnvironment
Push-Location $ProjectRoot
try {
    if ($Clean) {
        idf.py fullclean
        if ($LASTEXITCODE -ne 0) { throw "idf.py fullclean failed." }
    }
    idf.py set-target esp32s3
    if ($LASTEXITCODE -ne 0) { throw "Could not configure ESP32-S3 target." }
    idf.py build
    if ($LASTEXITCODE -ne 0) { throw "AMH-OS 3.0 build failed." }
    idf.py size
    if ($LASTEXITCODE -ne 0) { throw "Firmware size report failed." }
    Write-Host "AMH-OS 3.0 build completed. No flash operation was performed." -ForegroundColor Green
} finally {
    Pop-Location
}
