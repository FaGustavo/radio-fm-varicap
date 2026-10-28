# Compila e envia um sketch para o Nano (bootloader antigo).
# Uso:  .\scripts\upload.ps1                 (envia FM_Mapeador)
#       .\scripts\upload.ps1 TesteDisplayFIC
#       .\scripts\upload.ps1 -SoCompilar
param(
  [string]$Sketch = "FM_Mapeador",
  [string]$Porta = "",
  [switch]$SoCompilar
)

$repo = Split-Path -Parent $PSScriptRoot
$cli  = "$env:LOCALAPPDATA\Programs\arduino-ide\resources\app\lib\backend\resources\arduino-cli.exe"
$fqbn = "arduino:avr:nano:cpu=atmega328old"
$dir  = Join-Path $repo "firmware\$Sketch"
$lib  = Join-Path $repo "lib\DisplayFIC"

if (-not $SoCompilar -and -not $Porta) {
  $dev = Get-CimInstance Win32_PnPEntity | Where-Object { $_.PNPDeviceID -match 'VID_1A86' } | Select-Object -First 1
  if (-not $dev) { Write-Error "Nano (CH340) não encontrado. Confira o cabo USB."; exit 1 }
  $Porta = $dev.Name -replace '.*\((COM\d+)\).*', '$1'
  Write-Host "Nano encontrado em $Porta"
}

if ($SoCompilar) {
  & $cli compile --fqbn $fqbn --library $lib $dir
} else {
  & $cli compile --fqbn $fqbn --library $lib -u -p $Porta $dir
}
