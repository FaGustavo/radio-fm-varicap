// DisplayFIC.h - Biblioteca do display LED de 4 digitos (88:88) com icones
// Display charlieplex de 7 pinos, mapeado a partir do log do usuario.
//
// Numeracao dos pinos do display: 0..6 (a mesma usada no sketch mapeador).
// Cada LED fica entre um pino ANODO (HIGH) e um pino CATODO (LOW).
// Ligue cada pino do display a um pino do Arduino com resistor de 150-220 ohm.
//
// Uso:
//   #include "DisplayFIC.h"
//   DisplayFIC disp(2, 3, 4, 5, 6, 7, 8);   // pinos do Arduino para os pinos 0..6 do display
//   void setup() { disp.begin(); disp.showNumber(1234); }
//   void loop()  { disp.update(); }          // chame o mais rapido possivel
//   // no lugar de delay(ms), use disp.wait(ms)

#pragma once
#include <Arduino.h>

class DisplayFIC {
public:
  // LEDs 0..27 = segmentos: indice = digito*7 + segmento (a=0 b=1 c=2 d=3 e=4 f=5 g=6)
  // digito 0 = esquerda ... digito 3 = direita
  enum Led : uint8_t {
    DP3 = 28,     // ponto depois do 3o digito
    COLON = 29,   // os dois pontos (acendem juntos)
    PLAY = 30,
    PAUSE = 31,
    BT = 32,
    USB = 33,
    SD = 34,
    MHZ = 35,
    LED_COUNT = 36
  };

  DisplayFIC(uint8_t p0, uint8_t p1, uint8_t p2, uint8_t p3,
             uint8_t p4, uint8_t p5, uint8_t p6) {
    _pin[0] = p0; _pin[1] = p1; _pin[2] = p2; _pin[3] = p3;
    _pin[4] = p4; _pin[5] = p5; _pin[6] = p6;
    clear();
  }

  void begin(uint16_t slotMicros = 250) {
    _slotUs = slotMicros;
    allOff();
  }

  // ---------- Controle de LEDs individuais ----------
  void set(uint8_t led, bool on = true) {
    if (led >= LED_COUNT) return;
    if (on) _buf[led >> 3] |= (1 << (led & 7));
    else    _buf[led >> 3] &= ~(1 << (led & 7));
  }
  bool get(uint8_t led) const {
    if (led >= LED_COUNT) return false;
    return _buf[led >> 3] & (1 << (led & 7));
  }
  void clear() { for (uint8_t i = 0; i < sizeof(_buf); i++) _buf[i] = 0; }
  void allOn()  { for (uint8_t i = 0; i < LED_COUNT; i++) set(i, true); }

  // Segmento individual: pos 0..3, seg 'a'..'g'
  void setSegment(uint8_t pos, char seg, bool on = true) {
    if (pos > 3 || seg < 'a' || seg > 'g') return;
    set(pos * 7 + (seg - 'a'), on);
  }

  // ---------- Icones e pontos ----------
  void colon(bool on)   { set(COLON, on); }
  void dot3(bool on)    { set(DP3, on); }
  void play(bool on)    { set(PLAY, on); }
  void pause(bool on)   { set(PAUSE, on); }
  void bluetooth(bool on) { set(BT, on); }
  void usb(bool on)     { set(USB, on); }
  void sd(bool on)      { set(SD, on); }
  void mhz(bool on)     { set(MHZ, on); }

  // ---------- Texto e numeros ----------
  // mascara: bit0=a ... bit6=g
  void setDigitMask(uint8_t pos, uint8_t mask) {
    if (pos > 3) return;
    for (uint8_t s = 0; s < 7; s++) set(pos * 7 + s, (mask >> s) & 1);
  }

  void showChar(uint8_t pos, char c) { setDigitMask(pos, glyph(c)); }

  // 4 caracteres (os que faltarem viram espaco). Nao mexe nos icones.
  void showText(const char* s) {
    for (uint8_t i = 0; i < 4; i++) {
      char c = ' ';
      if (*s) c = *s++;
      showChar(i, c);
    }
  }

  // Numero de -999 a 9999, alinhado a direita
  void showNumber(int n) {
    if (n < -999 || n > 9999) { showText("----"); return; }
    char b[5] = {' ', ' ', ' ', ' ', 0};
    bool neg = n < 0;
    if (neg) n = -n;
    for (int i = 3; i >= 0; i--) {
      b[i] = '0' + (n % 10);
      n /= 10;
      if (n == 0) {
        if (neg && i > 0) b[i - 1] = '-';
        break;
      }
    }
    showText(b);
  }

  // Hora no formato HH:MM, com ou sem os dois pontos
  void showTime(uint8_t h, uint8_t m, bool withColon = true) {
    showChar(0, '0' + (h / 10) % 10);
    showChar(1, '0' + h % 10);
    showChar(2, '0' + (m / 10) % 10);
    showChar(3, '0' + m % 10);
    colon(withColon);
  }

  // Frequencia de radio: 98.5 -> " 98.5" e 100.5 -> "100.5"
  // (o display so tem ponto depois do 3o digito) e acende o icone MHz
  void showFM(float freq) {
    showNumber((int)(freq * 10 + 0.5));
    dot3(true);
    mhz(true);
  }

  // ---------- Multiplexacao ----------
  // Chame o mais rapido possivel dentro do loop().
  void update() {
    uint32_t now = micros();
    if ((uint32_t)(now - _last) < _slotUs) return;
    _last = now;
    for (uint8_t n = 0; n < LED_COUNT; n++) {
      _cur = (_cur + 1) % LED_COUNT;
      if (get(_cur)) {
        drive(_cur);
        return;
      }
    }
    allOff();   // nada aceso
  }

  // Substitui delay(): continua atualizando o display durante 'ms'
  void wait(uint32_t ms) {
    uint32_t t = millis();
    while (millis() - t < ms) update();
  }

  void allOff() {
    for (uint8_t i = 0; i < 7; i++) pinMode(_pin[i], INPUT);
  }

private:
  uint8_t _pin[7];
  uint8_t _buf[5];
  uint8_t _cur = 0;
  uint16_t _slotUs = 250;
  uint32_t _last = 0;

  // Pares {anodo, catodo} de cada LED, medidos no mapeamento.
  // Ordem: D1 a..g, D2 a..g, D3 a..g, D4 a..g, DP3, COLON, PLAY, PAUSE, BT, USB, SD, MHZ
  static const uint8_t (*pairs())[2] {
    static const uint8_t P[LED_COUNT][2] = {
      {0,1},{0,2},{3,0},{4,0},{0,3},{1,0},{2,0},   // D1 a b c d e f g
      {1,2},{1,3},{4,1},{1,5},{1,4},{2,1},{3,1},   // D2
      {4,3},{2,4},{3,4},{5,0},{5,2},{3,2},{4,2},   // D3
      {6,5},{5,6},{4,5},{5,3},{3,5},{5,4},{4,6},   // D4
      {6,4},                                        // DP3
      {2,3},                                        // COLON (2 LEDs juntos)
      {0,5},                                        // PLAY
      {5,1},                                        // PAUSE
      {0,4},                                        // BT
      {2,5},                                        // USB
      {6,2},                                        // SD
      {2,6}                                         // MHZ
    };
    return P;
  }

  void drive(uint8_t led) {
    const uint8_t (*P)[2] = pairs();
    uint8_t a = _pin[P[led][0]];
    uint8_t k = _pin[P[led][1]];
    allOff();
    pinMode(a, OUTPUT); digitalWrite(a, HIGH);
    pinMode(k, OUTPUT); digitalWrite(k, LOW);
  }

  static uint8_t glyph(char c) {
    switch (c) {
      case '0': return 0x3F; case '1': return 0x06; case '2': return 0x5B;
      case '3': return 0x4F; case '4': return 0x66; case '5': return 0x6D;
      case '6': return 0x7D; case '7': return 0x07; case '8': return 0x7F;
      case '9': return 0x6F;
      case 'A': case 'a': return 0x77;
      case 'B': case 'b': return 0x7C;
      case 'C': return 0x39;  case 'c': return 0x58;
      case 'D': case 'd': return 0x5E;
      case 'E': case 'e': return 0x79;
      case 'F': case 'f': return 0x71;
      case 'G': case 'g': return 0x6F;
      case 'H': return 0x76;  case 'h': return 0x74;
      case 'I': case 'i': return 0x30;
      case 'J': case 'j': return 0x1E;
      case 'L': case 'l': return 0x38;
      case 'N': case 'n': return 0x54;
      case 'O': return 0x3F;  case 'o': return 0x5C;
      case 'P': case 'p': return 0x73;
      case 'R': case 'r': return 0x50;
      case 'S': case 's': return 0x6D;
      case 'T': case 't': return 0x78;
      case 'U': return 0x3E;  case 'u': return 0x1C;
      case 'Y': case 'y': return 0x6E;
      case '-': return 0x40;
      case '_': return 0x08;
      case '=': return 0x48;
      default:  return 0x00;
    }
  }
};
