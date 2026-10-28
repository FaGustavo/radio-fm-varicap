# Display FIC: adaptação para LED de 7 segmentos

O display veio de um aparelho de som FIC. Ele tem **7 pinos** e **36 LEDs** ligados em **charlieplex**: cada LED fica entre um par de pinos, um como anodo (HIGH) e outro como catodo (LOW), e os outros 5 pinos ficam em alta impedância (INPUT).

A biblioteca `lib/DisplayFIC` acende **um LED por vez**, a cada 250 µs, e passa por todos os LEDs acesos rápido o bastante para o olho ver todos juntos. Por isso `disp.update()` precisa ser chamado o tempo todo no `loop()`, e `delay()` deve ser trocado por `disp.wait(ms)`.

## Layout

```
  D1   D2   D3 . D4        ícones: PLAY PAUSE BT USB SD MHz
  88   :    88               dois pontos (COLON) entre D2 e D3
                             ponto (DP3) depois do 3º dígito
```

Segmentos: `a` = topo, `b` = superior direito, `c` = inferior direito, `d` = base, `e` = inferior esquerdo, `f` = superior esquerdo, `g` = meio.

## Mapa dos LEDs (anodo → catodo)

Pinos numerados de 0 a 6 (pinos do display, não do Arduino).

| LED | a | b | c | d | e | f | g |
|---|---|---|---|---|---|---|---|
| Dígito 1 | 0→1 | 0→2 | 3→0 | 4→0 | 0→3 | 1→0 | 2→0 |
| Dígito 2 | 1→2 | 1→3 | 4→1 | 1→5 | 1→4 | 2→1 | 3→1 |
| Dígito 3 | 4→3 | 2→4 | 3→4 | 5→0 | 5→2 | 3→2 | 4→2 |
| Dígito 4 | 6→5 | 5→6 | 4→5 | 5→3 | 3→5 | 5→4 | 4→6 |

| Índice | Ícone | Anodo → catodo |
|---|---|---|
| 28 | DP3 (ponto após o 3º dígito) | 6→4 |
| 29 | COLON (dois pontos, 2 LEDs juntos) | 2→3 |
| 30 | PLAY | 0→5 |
| 31 | PAUSE | 5→1 |
| 32 | BT | 0→4 |
| 33 | USB | 2→5 |
| 34 | SD | 6→2 |
| 35 | MHZ | 2→6 |

Índices de 0 a 27 = `dígito × 7 + segmento` (a=0 … g=6), dígito 0 = esquerda.

## API resumida

```cpp
#include <DisplayFIC.h>
DisplayFIC disp(2, 3, 4, 5, 6, 7, 8);  // pinos do Arduino para os pinos 0..6 do display

disp.begin();
disp.showFM(97.1);        // " 97.1" + ponto + ícone MHz
disp.showNumber(1234);    // -999 a 9999, alinhado à direita
disp.showTime(12, 34);    // 12:34
disp.showText("CAL ");    // 4 caracteres
disp.play(true);          // ícones: play pause bluetooth usb sd mhz
disp.set(i, on);          // LED individual (0..35)
disp.update();            // chamar sempre no loop()
disp.wait(ms);            // no lugar de delay()
```

Limitação: o ponto só existe depois do 3º dígito, então a frequência aparece com 1 casa decimal (`101.7`, ` 97.1`).
