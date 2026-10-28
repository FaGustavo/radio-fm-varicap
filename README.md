# Rádio FM: varicap → frequência + display FIC

**Adaptação do display FIC LCD para LED de 7 segmentos. A tensão do varicap informa ao Arduino qual frequência está sintonizada.**

Um Arduino Nano lê a tensão de sintonia do varicap de um rádio FM (pelo A0, com um divisor 100k/100k), converte em frequência usando um mapa calibrado com estações reais e mostra o resultado na serial e num display de 4 dígitos reaproveitado de um aparelho de som FIC.

> Para retomar o projeto, comece por **[RETOMAR.md](RETOMAR.md)**.

## Estado (27/10/2026)

| Parte | Estado |
|---|---|
| Leitura da tensão do varicap (A0) | ✅ Funcionando, estável em ±5 mV |
| Mapa tensão → frequência com calibração pela serial | ✅ Funcionando, 4 estações gravadas, erro de ~0,1 MHz |
| Biblioteca do display FIC (charlieplex, 36 LEDs) | ✅ Mapeada e escrita |
| Firmware com display integrado | ✅ Compila e está gravado no Nano |
| Display ligado ao Nano | ⚠️ A montagem derruba a USB (provável curto), **em aberto** |

## Estrutura

```
firmware/
  FM_Mapeador/        firmware principal: leitura, calibração, mapa e display
  TesteDisplayFIC/    teste do display (todos os LEDs, um por um, textos)
lib/
  DisplayFIC/         biblioteca do display (charlieplex de 7 pinos)
dados/
  calibracao.csv      pontos tensão × frequência gravados
  leituras.md         todas as medições feitas
docs/
  hardware.md         componentes, ligações e cuidados
  display_fic.md      mapa dos 36 LEDs e API da biblioteca
  problemas.md        problemas encontrados e soluções
scripts/
  upload.ps1          compila e envia para o Nano (bootloader antigo)
  serial.ps1          lê a serial e envia comandos
RETOMAR.md            resumo para continuar de onde parou
```

## Como funciona

1. **Leitura.** O divisor 100k/100k reduz a tensão do varicap pela metade. O Nano faz a média de 64 leituras do A0 e multiplica por `VREF / 1023 × 2`. Com `VREF = 4,605 V` (o 5V real da USB), cada passo vale ~9 mV.
2. **Calibração.** Você sintoniza uma estação conhecida e digita a frequência na serial (ex.: `97.1`). O Nano grava o par tensão × frequência na EEPROM, em ordem de tensão.
3. **Mapa.** Entre dois pontos, a frequência é calculada por interpolação linear. Fora da faixa calibrada, extrapola pelo trecho da ponta e avisa.
4. **Display.** Mostra `CAL` com menos de 2 pontos. Depois mostra a frequência (` 97.1`, `101.7`) com o ícone MHz. O PLAY acende quando a sintonia está numa estação gravada, e o MHz pisca quando o valor é extrapolado. Uma histerese de 0,07 MHz evita que o último dígito fique trocando de valor.

### Por que não bastam 2 ou 3 pontos
A capacitância do varicap não é linear com a tensão, então a curva tensão × frequência também não é. Neste rádio a inclinação cai de **4,6 MHz/V** (88–97 MHz) para **3,1 MHz/V** (97,5–101,7 MHz). Com 2 pontos, a reta errou **3 MHz** na ponta de baixo. Com 3 pontos, o erro no meio caiu para **0,1 MHz**. O ideal são 5 ou 6 estações espalhadas pela banda.

## Calibração atual

| Tensão no varicap | Frequência |
|---|---|
| 1,774 V | 88,3 MHz |
| 3,702 V | 97,1 MHz |
| 3,811 V | 97,5 MHz |
| 5,166 V | 101,7 MHz |

Faltam pontos entre **91 e 94 MHz** e entre **105 e 108 MHz**. Detalhes em [dados/leituras.md](dados/leituras.md).

## Uso

### Instalar
1. Arduino IDE 2 com o core `arduino:avr`.
2. Copie `lib/DisplayFIC` para `Documentos\Arduino\libraries\DisplayFIC`. Os scripts fazem isso sozinhos com `--library`.
3. Placa: **Arduino Nano**, processador **ATmega328P (Old Bootloader)**.

### Enviar
```powershell
.\scripts\upload.ps1                   # FM_Mapeador
.\scripts\upload.ps1 TesteDisplayFIC   # teste do display
```

### Comandos na serial (115200, final de linha "Nova linha")

| Comando | Ação |
|---|---|
| `98.5` (ou `98,5`) | grava a tensão atual como 98,5 MHz |
| `l` | lista os pontos |
| `a 3` | apaga o ponto 3 |
| `z` | apaga todos os pontos |
| `?` | ajuda |

Saída a cada 250 ms:
```
V=3.701 V (ruido 9 mV)  | ~97.1 MHz  <- estacao 97.1
```

### Ajustes no firmware (`FM_Mapeador.ino`)

| Constante | Valor | Para quê |
|---|---|---|
| `VREF` | `4.605` | 5V real do Nano, medido com multímetro |
| `FATOR_DIVISOR` | `2.0` | (R1 + R2) / R2 |
| `AMOSTRAS` | `64` | média por leitura |
| `MAX_PONTOS` | `20` | estações na EEPROM |
| `INTERVALO_MS` | `250` | intervalo entre leituras |

Se mudar o formato dos dados na EEPROM, troque `MAGICO` para o Nano ignorar os dados antigos.

## Hardware
Resumo: varicap → 100k → **A0** → 100k → GND, GND do rádio no GND do Nano, display nos pinos **D2 a D8** com 220 Ω em cada um. Detalhes e cuidados em [docs/hardware.md](docs/hardware.md).
