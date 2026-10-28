# RETOMAR: onde o projeto parou

> Arquivo para continuar o projeto numa nova sessão (com o Claude ou sozinho).
> Última atualização: **27/10/2026**

## Objetivo
Ler a tensão do varicap de um rádio FM com um Arduino Nano, descobrir qual frequência está sintonizada e mostrar no display FIC (4 dígitos de LED, charlieplex de 7 pinos).

## Onde as coisas estão

| O quê | Caminho |
|---|---|
| Repositório | `C:\Users\Fantinel\Projetos\radio-fm-varicap` |
| Firmware principal | `firmware/FM_Mapeador/FM_Mapeador.ino` |
| Biblioteca do display | `lib/DisplayFIC/DisplayFIC.h` (cópia instalada em `Documentos\Arduino\libraries\DisplayFIC`) |
| Sketch original de trabalho | `Documentos\Arduino\FM_Mapeador` |
| Teste do display original | `Desktop\TesteDisplayFIC` |
| arduino-cli | `%LOCALAPPDATA%\Programs\arduino-ide\resources\app\lib\backend\resources\arduino-cli.exe` |

## Fatos importantes
- **Placa:** Nano clone, USB **CH340**, aparece como **COM12**. Usa o **bootloader antigo**: FQBN `arduino:avr:nano:cpu=atmega328old`.
- **5V real do Nano pela USB:** 4,605 V (`VREF = 4.605` no firmware).
- **Divisor:** 100k/100k no A0, fator 2. O firmware já mostra a tensão no varicap.
- **Abrir a serial reinicia o Nano** (DTR). Espere ~3 s antes de enviar comandos.
- **EEPROM:** 4 pontos gravados (1,774 V = 88,3 / 3,702 = 97,1 / 3,811 = 97,5 / 5,166 = 101,7 MHz). Também estão em `dados/calibracao.csv`.
- O firmware gravado no Nano **já inclui o display**. Sem o display ligado, ele funciona normal pela serial.

## O que já funciona
- [x] Leitura estável (±5 mV) da tensão do varicap
- [x] Calibração pela serial, salva na EEPROM
- [x] Estimativa de frequência por interpolação, erro de ~0,1 MHz
- [x] Biblioteca do display com os 36 LEDs mapeados
- [x] Firmware com display compilando e gravado

## Próximos passos (em ordem)
1. **Resolver o display derrubando a USB.** Com o display ligado, o Nano aparece como "Dispositivo USB Desconhecido" ou some. Sem ele, volta ao normal. É problema físico (curto ou consumo), não de código. Checklist em `docs/problemas.md`. Comece medindo a resistência entre 5V e GND com o display montado e a USB desligada.
2. **Testar o display** com `.\scripts\upload.ps1 TesteDisplayFIC`. Depois voltar ao `FM_Mapeador`.
3. **Completar a calibração:** gravar uma estação entre 91 e 94 MHz e uma entre 105 e 108 MHz. Use `.\scripts\serial.ps1 -Comando 93.3` com a estação sintonizada.
4. **Opcional:**
   - folga de ~20 mV no aviso "(extrapolado)" (hoje aparece mesmo 1 mV abaixo do ponto);
   - capacitor de 100 nF de A0 para o GND;
   - divisor de 1M + 1M, se o divisor estiver puxando a sintonia;
   - nomes das estações na EEPROM.

## Como retomar com o Claude
Abra o Claude Code nesta pasta e diga algo como:
> "Leia o RETOMAR.md e vamos continuar pelo passo 1."
