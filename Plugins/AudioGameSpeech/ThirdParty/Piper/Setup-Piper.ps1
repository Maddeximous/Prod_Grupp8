# Downloads Piper (offline TTS) and the Swedish voice into this folder.
# Run once after cloning:  powershell -ExecutionPolicy Bypass -File Setup-Piper.ps1
# The files are git-ignored (too big / binaries). Packaged builds include them automatically.

$ErrorActionPreference = "Stop"
$ProgressPreference = "SilentlyContinue"   # much faster downloads
$Here = $PSScriptRoot

$PiperZip = "https://github.com/rhasspy/piper/releases/download/2023.11.14-2/piper_windows_amd64.zip"
$VoiceUrl = "https://huggingface.co/rhasspy/piper-voices/resolve/main/sv/sv_SE/nst/medium"
$Voice    = "sv_SE-nst-medium"

# 1. Piper program (piper.exe, DLLs, espeak-ng-data)
if (-not (Test-Path "$Here\piper.exe")) {
    Write-Host "Downloading Piper..."
    $Zip = Join-Path $env:TEMP "piper_windows_amd64.zip"
    Invoke-WebRequest $PiperZip -OutFile $Zip
    $Tmp = Join-Path $env:TEMP "piper_extract"
    if (Test-Path $Tmp) { Remove-Item $Tmp -Recurse -Force }
    Expand-Archive $Zip -DestinationPath $Tmp
    Copy-Item "$Tmp\piper\*" $Here -Recurse -Force   # zip has a "piper" folder inside
    Remove-Item $Tmp, $Zip -Recurse -Force
}

# 2. Swedish voice
New-Item -ItemType Directory -Force "$Here\voices" | Out-Null
foreach ($File in "$Voice.onnx", "$Voice.onnx.json", "MODEL_CARD") {
    $Target = "$Here\voices\$File"
    if ($File -eq "MODEL_CARD") { $Target = "$Here\voices\$Voice.MODEL_CARD.txt" }
    if (-not (Test-Path $Target)) {
        Write-Host "Downloading $File..."
        Invoke-WebRequest "$VoiceUrl/$File" -OutFile $Target
    }
}

Write-Host "Piper ready."
