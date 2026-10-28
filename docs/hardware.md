# Hardware

## Componentes
- Arduino Nano clone (ATmega328P, USB **CH340**, **bootloader antigo**)
- Rádio FM com sintonia por varicap
- 2 resistores de 100 kΩ (divisor de tensão)
- Capacitor de 100 nF (recomendado, de A0 para o GND)
- Display FIC (de aparelho de som), 4 dígitos 88:88 com ícones, 7 pinos charlieplex
- 7 resistores de 150 a 220 Ω (um em cada pino do display)

## Ligações

```
Linha de sintonia do varicap ──[100k]──┬──[100k]── GND
                                       │
                                       ├── A0 do Nano
                                       └──[100nF]── GND   (opcional)

GND do rádio ──────────────────────────── GND do Nano   (obrigatório)

Display pino 0 ──[220R]── D2
Display pino 1 ──[220R]── D3
Display pino 2 ──[220R]── D4
Display pino 3 ──[220R]── D5
Display pino 4 ──[220R]── D6
Display pino 5 ──[220R]── D7
Display pino 6 ──[220R]── D8
```

## Gravar o Nano sem desmontar o display

Enquanto a causa da queda da USB não for achada (ver `problemas.md`), o display precisa poder ser desligado do Nano sem desmontar nada. Basta um ponto de corte nas 7 linhas, entre os resistores e o Nano:

```
Display pino N ──[220R]──o/ o── DN      (uma chave por linha, 7 chaves)
```

| Opção | Como usar |
|---|---|
| **DIP switch de 8 vias** (recomendada) | Chaves em OFF para gravar e ON para usar. A 8ª chave fica livre |
| **Conector de 8 vias** (barra fêmea no lado do Nano, macho no cabo do display) | Tira o conector para gravar e encaixa de volta |

Cuidados:
- Corte **só as 7 linhas do display**. GND, 5V e A0 continuam ligados.
- Mantenha os resistores de 220 Ω **do lado do display**, para que cada linha continue protegida com o display reconectado.
- Se a USB **continuar caindo com tudo em OFF**, o problema não está nas 7 linhas: procure um fio do display encostando no 5V ou no GND.
- Dá para ligar o display com o Nano já rodando. Nos primeiros ~2 s o bootloader deixa D2 a D8 em INPUT, e depois o firmware assume os pinos.

## Cuidados
- **Divisor por 2:** permite medir até ~9,2 V no varicap (2 × 4,605 V). Se a tensão de sintonia passar disso, troque os resistores e ajuste `FATOR_DIVISOR` no firmware.
- **Carga na linha de sintonia:** 200 kΩ podem desafinar o rádio se a tensão do varicap vier de um resistor alto. Se a estação mudar ao ligar o Arduino, use 1M + 1M e o capacitor de 100 nF.
- **Terra em comum:** sem o GND do rádio ligado ao GND do Nano, o A0 flutua.
- **Não alimente o Nano pelo rádio:** entre os dois, ligue só o GND.
- **Display:** nenhum dos 7 pinos vai no 5V ou no GND, e cada um precisa do seu resistor.
