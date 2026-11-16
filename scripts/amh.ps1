param(
    [Parameter(Position = 0)]
    [ValidateSet("help", "setup", "bootstrap", "configure", "build", "test", "flash", "serial")]
    [string]$Command = "help",

    [string]$Port,

    [switch]$ConfirmBoard
)

$ErrorActionPreference = "Stop"
$Root = (Resolve-Path (Join-Path $PSScriptRoot "..")).Path
$ToolchainRoot = Join-Path $Root "tools\xtensa-esp-elf"
$ToolchainBin = Join-Path $ToolchainRoot "bin"
$VenvPython = Join-Path $Root ".venv-windows\Scripts\python.exe"
$Bash = if ($env:CYGWIN_BASH) { $env:CYGWIN_BASH } else { "C:\cygwin64\bin\bash.exe" }

function Show-Help {
    @"
AMH-OS Windows / PyCharm commands

  .\scripts\amh.ps1 setup
  .\scripts\amh.ps1 bootstrap
  .\scripts\amh.ps1 configure
  .\scripts\amh.ps1 build
  .\scripts\amh.ps1 test
  .\scripts\amh.ps1 serial -Port COM16
  .\scripts\amh.ps1 flash -Port COM16 -ConfirmBoard

setup      Download/check the Windows Xtensa toolchain and create Python tools venv.
bootstrap  Clone Apache NuttX and NuttX Apps into third_party.
configure Configure NuttX and register the AMH app using Cygwin Bash.
build      Build NuttX using Cygwin make and the Windows Xtensa toolchain.
test       Run repository static checks with Windows Python.
serial     Open the COM port at 115200 baud.
flash      Explicitly confirm the board, then flash the built firmware.

No Linux, Ubuntu, or WSL2 installation is used. Cygwin is required because
NuttX's configure/build system uses Bash and GNU build utilities.
"@ | Write-Host
}

function Get-CygwinBash {
    if (-not (Test-Path $Bash)) {
        throw "Cygwin Bash not found at '$Bash'. Install Cygwin or set CYGWIN_BASH to the full path of bash.exe."
    }
    return (Resolve-Path $Bash).Path
}

function Invoke-Cygwin([string]$BashCommand) {
    $bashPath = Get-CygwinBash
    $cygpath = Join-Path (Split-Path $bashPath) "cygpath.exe"
    if (-not (Test-Path $cygpath)) {
        throw "cygpath.exe is missing beside Cygwin bash.exe. Install the Cygwin coreutils package."
    }
    $cygRoot = (& $cygpath -u $Root).Trim()
    if ($LASTEXITCODE -ne 0 -or -not $cygRoot) { throw "Could not convert project path for Cygwin." }
    $env:PATH = "$ToolchainBin;$env:PATH"
    & $bashPath -lc "cd '$cygRoot' && $BashCommand"
    if ($LASTEXITCODE -ne 0) { throw "Cygwin command failed with exit code $LASTEXITCODE." }
}

switch ($Command) {
    "help" {
        Show-Help
    }
    "setup" {
        $version = "14.2.0_20241119"
        $archiveName = "xtensa-esp-elf-$version-x86_64-w64-mingw32.zip"
        $url = "https://github.com/espressif/crosstool-NG/releases/download/esp-$version/$archiveName"
        $expectedSha256 = "62ae704777d73c30689efff6e81178632a1ca44d1a2d60f4621eb997e040e028"
        $downloadDir = Join-Path $Root "downloads"
        $archive = Join-Path $downloadDir $archiveName
        New-Item -ItemType Directory -Force -Path $downloadDir | Out-Null
        New-Item -ItemType Directory -Force -Path $ToolchainRoot | Out-Null

        if (-not (Test-Path (Join-Path $ToolchainBin "xtensa-esp-elf-gcc.exe"))) {
            Write-Host "Downloading Espressif Xtensa ESP32-S3 Windows toolchain..."
            Invoke-WebRequest -Uri $url -OutFile $archive
            $actual = (Get-FileHash -Algorithm SHA256 $archive).Hash.ToLowerInvariant()
            if ($actual -ne $expectedSha256) {
                Remove-Item $archive -Force
                throw "Toolchain checksum mismatch. Download removed."
            }
            Expand-Archive -Path $archive -DestinationPath $ToolchainRoot -Force
        }

        $compiler = Get-ChildItem -Path $ToolchainRoot -Filter "xtensa-esp-elf-gcc.exe" -File -Recurse | Select-Object -First 1
        if (-not $compiler) { throw "Toolchain extraction did not contain xtensa-esp-elf-gcc.exe." }
        $ToolchainBin = $compiler.Directory.FullName
        $env:PATH = "$ToolchainBin;$env:PATH"
        & $compiler.FullName --version
        if ($LASTEXITCODE -ne 0) { throw "Xtensa compiler did not start successfully." }

        if (-not (Test-Path $VenvPython)) {
            python -m venv (Join-Path $Root ".venv-windows")
        }
        & $VenvPython -m pip install --upgrade pip
        & $VenvPython -m pip install "esptool>=4.8.1" "pyserial>=3.5"
        if ($LASTEXITCODE -ne 0) { throw "Could not install esptool and pyserial in the project virtual environment." }
        Write-Host "Windows toolchain and Python utilities are prepared."
        Write-Host "Cygwin must still be installed separately with the packages in docs/GETTING_STARTED.md."
    }
    "bootstrap" {
        $thirdParty = Join-Path $Root "third_party"
        New-Item -ItemType Directory -Force -Path $thirdParty | Out-Null
        if (-not (Test-Path (Join-Path $thirdParty "nuttx\.git"))) {
            git clone --depth 1 https://github.com/apache/nuttx.git (Join-Path $thirdParty "nuttx")
            if ($LASTEXITCODE -ne 0) { throw "NuttX clone failed." }
        } else { Write-Host "NuttX checkout already exists; leaving it unchanged." }
        if (-not (Test-Path (Join-Path $thirdParty "apps\.git"))) {
            git clone --depth 1 https://github.com/apache/nuttx-apps.git (Join-Path $thirdParty "apps")
            if ($LASTEXITCODE -ne 0) { throw "NuttX Apps clone failed." }
        } else { Write-Host "NuttX Apps checkout already exists; leaving it unchanged." }
    }
    "configure" {
        if (-not (Test-Path (Join-Path $Root "third_party\nuttx\.git"))) {
            throw "Run .\scripts\amh.ps1 bootstrap first."
        }
        Invoke-Cygwin "bash scripts/configure.sh"
    }
    "build" {
        if (-not (Test-Path (Join-Path $Root "third_party\nuttx\.config"))) {
            throw "Run .\scripts\amh.ps1 configure first."
        }
        if (-not (Test-Path (Join-Path $ToolchainBin "xtensa-esp-elf-gcc.exe"))) {
            throw "Run .\scripts\amh.ps1 setup first."
        }
        Invoke-Cygwin "bash scripts/build.sh"
    }
    "test" {
        python (Join-Path $Root "tests\check_project.py")
        if ($LASTEXITCODE -ne 0) { throw "Project checks failed." }
    }
    "serial" {
        if (-not $Port) { throw "Specify -Port COM16 (replace with the COM port shown in Device Manager)." }
        if (-not (Test-Path $VenvPython)) { throw "Run .\scripts\amh.ps1 setup first to install pyserial." }
        & $VenvPython -m serial.tools.miniterm $Port 115200
    }
    "flash" {
        if (-not $Port) { throw "Specify -Port COM16 (replace with the COM port shown in Device Manager)." }
        if (-not $ConfirmBoard) { throw "Flash refused. Add -ConfirmBoard only after verifying the exact ESP32-S3 carrier-board model and flash settings." }
        if (-not (Test-Path (Join-Path $Root "third_party\nuttx\nuttx"))) { throw "Built NuttX ELF not found. Run build first." }
        Write-Warning "Flashing replaces the firmware currently on $Port. Preserve the Wi-Fi Radar source first."
        $typed = Read-Host "Type FLASH to continue"
        if ($typed -cne "FLASH") { throw "Flash cancelled." }
        $env:AMH_BOARD_CONFIRMED = "YES"
        try {
            Invoke-Cygwin "AMH_BOARD_CONFIRMED=YES bash scripts/flash.sh $Port"
        } finally {
            Remove-Item Env:AMH_BOARD_CONFIRMED -ErrorAction SilentlyContinue
        }
    }
}
