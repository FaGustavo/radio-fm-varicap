# Leituras e medições (27/10/2026)

Todas as tensões abaixo são **no varicap**: o firmware já multiplica por 2 a leitura do A0, por causa do divisor 100k/100k.

## Pontos de calibração gravados na EEPROM

| # | Tensão no varicap | Frequência | Inclinação até o próximo |
|---|---|---|---|
| 1 | 1,774 V | 88,3 MHz | 4,56 MHz/V |
| 2 | 3,702 V | 97,1 MHz | 3,67 MHz/V |
| 3 | 3,811 V | 97,5 MHz | 3,10 MHz/V |
| 4 | 5,166 V | 101,7 MHz | — |

A sintonia fica "mais lenta" (menos MHz por volt) conforme a tensão sobe. É o comportamento típico de varicap, e é por isso que 2 pontos não bastam.

### Teste de precisão
- Com só 97,5 e 101,7 gravados, a reta previa **91,2 MHz** para 1,774 V. A estação era **88,3 MHz**: erro de ~3 MHz.
- Com 3 pontos (88,3 / 97,5 / 101,7), o mapa estimou **97,0 MHz** numa estação que era **97,1 MHz**: erro de 0,1 MHz.

## Calibração do VREF (5V do Nano)

| Medição | Valor |
|---|---|
| 5V do Nano pela USB (multímetro) | **4,605 V** |
| A0 no multímetro | 0,865 V |
| A0 segundo o Nano, com `VREF = 5.0` | 0,96 V (erro de ~10%) |
| A0 segundo o Nano, com `VREF = 4.605` | 0,887 V (erro de ~2,5%) |

O erro que sobra vem da tolerância dos resistores, do conversor do Nano e do multímetro. Para o mapa isso não importa, porque o que conta é a leitura ser repetível.

## Estabilidade da leitura

| Situação | Leitura | Ruído (max-min em 64 amostras) |
|---|---|---|
| A0 solto | cai sozinho de 4,9 V até 2,6 V | ~3000 mV |
| Divisor ligado, rádio desligado | 0,000 V | 0 mV |
| Rádio ligado, logo após ligar | 1,4 a 1,9 V, variando | 10 a 150 mV |
| Rádio ligado, estabilizado | 1,918 a 1,927 V | 10 a 20 mV |
| Com `VREF = 4.605`, dial parado | 1,771 a 1,776 V | 9 a 27 mV |
| Estação 97,5 MHz | 3,809 a 3,813 V | 9 a 18 mV |
| Estação 101,7 MHz | 5,159 a 5,166 V | 9 a 18 mV |
| Depois de religar tudo (sem display) | 5,238 a 5,243 V | 18 a 36 mV |

Resolução: 1 passo do conversor (10 bits) vale `4,605 / 1023 × 2 ≈ 9 mV` no varicap, o que dá **~0,03 MHz**, bem menos que os 0,2 MHz entre duas estações.
