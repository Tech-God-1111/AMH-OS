$ErrorActionPreference = "Stop"
$ProjectRoot = Split-Path -Parent $PSScriptRoot

function Assert-IdfEnvironment {
    if (-not $env:IDF_PATH -or -not (Get-Command idf.py -ErrorAction SilentlyContinue)) {
        throw "ESP-IDF environment is not active. Open ESP-IDF PowerShell and run export.ps1."
    }
}
