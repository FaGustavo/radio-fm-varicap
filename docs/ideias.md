# Plano: modo Bluetooth e aviso de bateria

Definido em 28/10/2026, ainda não implementado. Aparelho: **FIC CS 2002**.

O relógio foi descartado.

## Funcionamento

1. **Alimentação pelo ACC.** O Nano, o display e o rádio ligam juntos quando a chave vai para ACC. Desligou a chave, tudo apaga. O 12 V de memória não é usado.
2. **Modo FM** (normal): o display mostra a frequência, como já funciona hoje.
3. **Modo Bluetooth** (chave de faixa em AM):
   - um sinal do rádio, que só existe em AM, chega a um pino do Nano;
   - o Nano liga o **MH-M18**;
   - o display mostra `  bt` e acende o ícone **BT** (`disp.bluetooth(true)`).
4. **Aviso de bateria:** um segundo divisor de tensão mede a bateria. Abaixo de **11,5 V**, o display mostra a tensão (` 11.4`).

## Pinos

| Função | Pino do Nano | Situação |
|---|---|---|
| Varicap (FM) | A0 | já existe |
| Display | D2 a D8 | já existe |
| Sinal de AM | D9 | novo |
| Liga o M18 | D10 | novo |
| Tensão da bateria | A1 | novo |

Ficam livres D11, D12, D13 e A2 a A7.

## Sinal de AM → D9

Ainda falta achar o sinal no rádio. Pode ser positivo (por exemplo, a alimentação do circuito de AM) ou negativo (um contato da chave indo ao GND).

- **Usar um optoacoplador PC817** em qualquer um dos casos. Ele isola o rádio do Nano e aguenta os 12 V:
  ```
  sinal + ──[1k5]── PC817 pino 1 (anodo)
  GND do rádio ──── PC817 pino 2 (catodo)
  PC817 pino 4 (coletor) ── D9 (INPUT_PULLUP)
  PC817 pino 3 (emissor) ── GND do Nano
  ```
  Com AM ativo, D9 vai a LOW. Se o sinal achado for negativo (chave ao GND), o LED do opto vai entre +12 V (pelo 1k5) e o fio que vai ao GND.
- Antes de ligar, **medir com o multímetro** a tensão do sinal em AM e em FM.
- No firmware, a polaridade fica numa constante (`AM_ATIVO = LOW`) e a leitura tem debounce de ~100 ms.

## Ligar o M18 → D10

O pino do Nano não deve alimentar o M18 direto, porque o consumo de pico do módulo passa do que o pino aguenta.

- **Chave do lado positivo (high-side):** P-MOSFET (ex.: AO3401 ou IRF9540) no 5 V do M18, comandado por um NPN (BC547) no D10.
  - Não cortar o GND (low-side), porque o GND do M18 é o mesmo do áudio. Cortar ali causa ruído e zumbido.
- **Alternativa sem o Nano:** se o sinal de AM do rádio já for uma alimentação com corrente suficiente, um step-down tirado dele liga o M18 sozinho. O Nano só lê o D9 para mostrar `bt`.
- A saída de áudio do M18 entra no rádio no lugar do áudio de AM. O ponto exato ainda falta achar.

## Tensão da bateria → A1

```
+12 V (ACC) ──[10k]──┬──[3k3]── GND
                     ├── A1
                     └──[100nF]── GND
```

- Fator do divisor: (10k + 3k3) / 3k3 = **4,03**. Com 16 V na entrada, chegam ~4 V no A1, ainda dentro do limite.
- Opcional: zener de 5,1 V do A1 para o GND, contra picos do carro.
- **VREF muda** quando o Nano passa a ser alimentado pelo step-down em vez da USB. Medir o 5 V real de novo e ajustar `VREF` no firmware, porque isso também afeta a leitura do varicap.
- Média de várias leituras e **histerese**: mostra o aviso abaixo de 11,5 V e só tira acima de 11,8 V. Assim o display não fica piscando com o valor no limite.
- Ignorar os primeiros segundos depois de ligar e durante a partida, quando a tensão cai bastante.
- Referência: motor ligado ≈ 13,8 a 14,5 V. Abaixo de 11,5 V com o motor ligado indica problema no alternador.

**Decidido:** com a bateria baixa, o display **alterna a cada ~2 s** entre a tensão (` 11.4`) e o normal (frequência ou `  bt`).

## Sobre o MH-M18 (pesquisa de 28/10/2026)

- Receptor Bluetooth 4.2 (A2DP/AVRCP). **Não tem serial nem comandos AT documentados.** O fabricante (MH-ET LIVE) diz que os 4 pinos extras são de teste de fábrica.
- Não informa título, tempo nem posição da música.
- Algumas versões têm o pino **MUTE** (alto = pausado/mudo, baixo = tocando) e o pino **KEY** (botão play/pause/próxima). Conferir se a placa comprada tem esses pinos.
- LED azul: pisca rápido = desconectado, aceso = conectado, pisca devagar = tocando.
- Um ESP32 clássico (WROOM-32) + DAC PCM5102A com a biblioteca `ESP32-A2DP` daria título e tempo, mas o usuário achou que **não vale a pena** para este projeto.

## Descartado

- **Relógio no display**: sem DS3231, o Nano teria que ficar ligado no 12 V de memória (~10 a 25 mA, bateria fraca em 6 a 8 semanas de carro parado) e atrasaria/adiantaria. Com DS3231 seria viável, mas a ideia foi deixada de lado.

## Alimentação do Nano

- **Step-down de 12 V para 5 V** (mini-buck, ex.: MP1584) a partir do ACC. Evitar o VIN direto, porque o regulador da placa esquenta e sofre com os picos do carro.
- O GND do rádio, do Nano e do M18 tudo em comum.
