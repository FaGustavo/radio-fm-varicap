# Problemas encontrados e soluções

| Sintoma | Causa | Solução |
|---|---|---|
| PC não via o Nano (só portas Bluetooth) | Cabo USB só de carga | Trocar o cabo. Passou a aparecer como `USB-SERIAL CH340 (COM12)` |
| `avrdude: not in sync: resp=0x00` | Nano clone com bootloader antigo | Usar `arduino:avr:nano:cpu=atmega328old` (IDE: *Processor → ATmega328P (Old Bootloader)*) |
| Leitura caindo sozinha de 4,9 para 2,6 V, ruído de ~3000 mV | A0 solto (flutuando) | Ligar o divisor e o GND em comum |
| Leitura 0,000 V e ruído 0 | Rádio desligado | Ligar o rádio |
| Nano ~10% acima do multímetro | 5V da USB é 4,605 V, não 5,0 | `VREF = 4.605` |
| Leitura oscilando ~0,4 V no começo | Sintonia mexida / circuito estabilizando | Esperar; depois ficou estável em ±5 mV |
| Ao ligar o display: `Dispositivo USB Desconhecido (Falha na Solicitação de Descritor)`, depois o Nano sumiu do PC | Curto ou consumo excessivo na montagem do display (derruba o 5V) | **Em aberto.** Sem o display, o Nano volta como CH340 |
| Serial com caracteres corrompidos e A0 flutuando depois de tirar o display | Ligações soltas na protoboard | Refazer as ligações; serial e leitura voltaram ao normal |

## Em aberto: display derrubando a USB

O CH340 se apresenta ao PC antes do programa rodar, e nesse momento D2 a D8 ainda estão em INPUT. Se a USB cai com o display ligado, a causa é **física**, não o código. O que verificar (com a USB desconectada):

1. Resistência entre 5V e GND com o display montado (deve ser de vários kΩ, nunca perto de 0).
2. Display ou resistores encaixados nas trilhas de alimentação (+/−) da protoboard.
3. Algum fio do display no 5V, 3V3, GND, VIN ou RST em vez de D2 a D8.
4. Pinos extras do display (se houver mais de 7) com ligação interna de alimentação.

### Análise de 28/10/2026: o display pode atrapalhar a gravação?

Pelo circuito documentado, **não**:

- A gravação usa só **D0 (RX), D1 (TX), RST** e o chip **CH340**.
- Durante a gravação, D2 a D8 ficam em alta impedância (o bootloader não mexe neles), então nenhuma corrente passa pelos LEDs.

O erro de ontem, "Falha na Solicitação de Descritor", vem do **CH340**, antes até de o ATmega participar. Ele quase sempre indica **5 V da USB caindo** (curto ou consumo), não interferência nos dados. Como na prática o problema aconteceu, **a montagem real deve estar diferente do desenho**.

Suspeitos, do mais provável ao menos provável:
1. **Display ligado em D0 a D6 em vez de D2 a D8.** Os pinos do display são numerados de 0 a 6 e é fácil ligar o "pino 0" no D0. Isso ocupa o RX e o TX e faz a gravação falhar.
2. Fio, resistor ou perna de LED encostando no 5V, GND, RST ou nas trilhas de alimentação da protoboard.
3. Placa do display com contato metálico encostando nos pinos do Nano, ou pino extra ligado à alimentação dentro dela.
4. GND do rádio com potencial diferente do GND da USB do PC. Testar com o rádio desligado.

Como isolar: instalar o DIP switch nas 7 linhas (ver `hardware.md`).
- Grava em OFF e falha em ON → o problema está nas 7 linhas (suspeitos 1 e 2).
- Falha mesmo em OFF → o problema está fora do display (suspeitos 2, 3 e 4).

Antes de tudo, medir a resistência entre 5V e GND com a USB desligada.
