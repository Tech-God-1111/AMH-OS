$ErrorActionPreference = "Stop"
$Root = Split-Path -Parent $PSScriptRoot
Set-Location $Root
py -m unittest discover -s tests -v
if ($LASTEXITCODE -ne 0) { throw "Repository checks failed." }
Write-Host "All checks passed."
