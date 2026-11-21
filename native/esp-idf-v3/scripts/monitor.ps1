param([Parameter(Mandatory=$true)][string]$Port)
. "$PSScriptRoot\_common.ps1"
Assert-IdfEnvironment
Push-Location $ProjectRoot
try {
    idf.py -p $Port monitor
} finally {
    Pop-Location
}
