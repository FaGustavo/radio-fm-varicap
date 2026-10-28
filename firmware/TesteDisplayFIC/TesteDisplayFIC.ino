// Teste da biblioteca DisplayFIC (Arduino Nano)
// Requer a biblioteca lib/DisplayFIC instalada (veja o README).
// Pinos 0..6 do display ligados em D2..D8, cada um com resistor de 150-220 ohm.

#include <DisplayFIC.h>

DisplayFIC disp(2, 3, 4, 5, 6, 7, 8);

void setup() {
  disp.begin();

  // 1) Acende todos os LEDs por 2 segundos
  disp.allOn();
  disp.wait(2000);

  // 2) Percorre os 36 LEDs um por um (confere o mapa)
  for (uint8_t i = 0; i < DisplayFIC::LED_COUNT; i++) {
    disp.clear();
    disp.set(i);
    disp.wait(300);
  }
  disp.clear();
}

void loop() {
  // 3) Radio FM com icones
  disp.clear();
  disp.showFM(98.5);
  disp.wait(1500);

  // 4) Hora com os dois pontos piscando
  disp.clear();
  for (uint8_t s = 0; s < 6; s++) {
    disp.showTime(12, 34, s % 2 == 0);
    disp.wait(500);
  }

  // 5) Contador com icones de reproducao
  disp.clear();
  disp.play(true);
  disp.usb(true);
  for (int n = 0; n <= 20; n++) {
    disp.showNumber(n);
    disp.wait(150);
  }

  // 6) Texto
  disp.clear();
  disp.bluetooth(true);
  disp.showText("HELP");  // o display tem só 4 dígitos
  disp.wait(1500);
}
