# Lê a serial do Nano por alguns segundos e, opcionalmente, envia um comando.
# Uso:  .\scripts\serial.ps1                 (lê 6 s)
#       .\scripts\serial.ps1 -Comando 98.5   (grava a tensão atual como 98.5 MHz)
#       .\scripts\serial.ps1 -Comando l      (lista os pontos)
param(
  [string]$Comando = "",
  [int]$Segundos = 6
)

$dev = Get-CimInstance Win32_PnPEntity | Where-Object { $_.PNPDeviceID -match 'VID_1A86' } | Select-Object -First 1
if (-not $dev) { Write-Error "Nano (CH340) não encontrado."; exit 1 }
$porta = $dev.Name -replace '.*\((COM\d+)\).*', '$1'

$p = New-Object System.IO.Ports.SerialPort $porta, 115200
$p.DtrEnable = $true   # abrir a porta reinicia o Nano
$p.NewLine = "`n"
$p.Open()
Start-Sleep -Milliseconds 3000
if ($Comando) { $p.WriteLine($Comando) }
Start-Sleep -Seconds $Segundos
$p.ReadExisting()
$p.Close()
