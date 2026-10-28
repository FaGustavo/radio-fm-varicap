/*
  FM_Mapeador - mapeia a tensão do varicap para frequência FM
  Arduino Nano, leitura em A0 através de divisor 100k / 100k.

  Ligação:
    Tensão do varicap --[100k]--+--[100k]-- GND
                                |
                                +-- A0   (+ capacitor 100nF de A0 para GND, recomendado)

  Uso (Monitor Serial, 115200, final de linha "Nova linha"):
    98.5   -> grava a tensão atual como 98.5 MHz (sintonize a estação antes)
    l      -> lista os pontos calibrados
    a 3    -> apaga o ponto 3
    z      -> apaga todos os pontos
    ?      -> ajuda

  Os pontos ficam salvos na EEPROM. Entre pontos a frequência é estimada
  por interpolação linear por trechos (fora da faixa, extrapola pelo trecho
  da ponta). Quanto mais pontos espalhados pela banda, melhor o mapa.
*/

#include <EEPROM.h>
#include <DisplayFIC.h>

// Display: pinos 0..6 do display em D2..D8 (com resistores de 150-220 ohm)
DisplayFIC disp(2, 3, 4, 5, 6, 7, 8);

const uint8_t PINO_LEITURA  = A0;
const float   VREF          = 4.605;  // 5V do Nano medido com multímetro (USB)
const float   FATOR_DIVISOR = 2.0;    // (100k + 100k) / 100k
const uint8_t AMOSTRAS      = 64;     // média para reduzir ruído
const uint8_t MAX_PONTOS    = 20;
const uint16_t MAGICO       = 0xFA02;
const unsigned long INTERVALO_MS = 250;

struct Ponto {
  float v;    // tensão no varicap (já corrigida pelo divisor)
  float mhz;
};

Ponto pontos[MAX_PONTOS];
uint8_t nPontos = 0;

char buf[24];
uint8_t bufLen = 0;
float ultimaTensao = 0;
float ultimoRuido = 0;

// ---------- EEPROM ----------
void salvar() {
  EEPROM.put(0, MAGICO);
  EEPROM.put(2, nPontos);
  EEPROM.put(3, pontos);
}

void carregar() {
  uint16_t m;
  EEPROM.get(0, m);
  if (m != MAGICO) { nPontos = 0; return; }
  EEPROM.get(2, nPontos);
  if (nPontos > MAX_PONTOS) nPontos = 0;
  EEPROM.get(3, pontos);
}

// ---------- Leitura ----------
// Retorna a tensão no varicap; ruido recebe a variação (max-min) entre as amostras.
float lerTensao(float &ruido) {
  analogRead(PINO_LEITURA);  // descarta a primeira leitura
  uint32_t soma = 0;
  uint16_t mn = 1023, mx = 0;
  for (uint8_t i = 0; i < AMOSTRAS; i++) {
    uint16_t r = analogRead(PINO_LEITURA);
    disp.update();  // mantém o display aceso durante a medição
    soma += r;
    if (r < mn) mn = r;
    if (r > mx) mx = r;
  }
  float escala = VREF / 1023.0 * FATOR_DIVISOR;
  ruido = (mx - mn) * escala;
  return (soma / (float)AMOSTRAS) * escala;
}

// ---------- Mapa ----------
float estimarMHz(float v, bool &foraDaFaixa) {
  foraDaFaixa = false;
  if (nPontos < 2) return NAN;
  uint8_t i;
  if (v <= pontos[0].v) {
    i = 0;
    foraDaFaixa = v < pontos[0].v;
  } else if (v >= pontos[nPontos - 1].v) {
    i = nPontos - 2;
    foraDaFaixa = v > pontos[nPontos - 1].v;
  } else {
    i = 0;
    while (v > pontos[i + 1].v) i++;
  }
  const Ponto &a = pontos[i];
  const Ponto &b = pontos[i + 1];
  return a.mhz + (v - a.v) * (b.mhz - a.mhz) / (b.v - a.v);
}

void adicionarPonto(float mhz) {
  float ruido;
  float v = lerTensao(ruido);

  // Mesma frequência já cadastrada -> atualiza a tensão
  for (uint8_t i = 0; i < nPontos; i++) {
    if (fabs(pontos[i].mhz - mhz) < 0.05) {
      // remove e reinsere para manter a ordem
      for (uint8_t j = i; j + 1 < nPontos; j++) pontos[j] = pontos[j + 1];
      nPontos--;
      break;
    }
  }
  // Tensão praticamente igual a outro ponto -> recusa (dividiria por zero)
  for (uint8_t i = 0; i < nPontos; i++) {
    if (fabs(pontos[i].v - v) < 0.01) {
      Serial.print(F("! Tensao igual a do ponto "));
      Serial.print(pontos[i].mhz, 1);
      Serial.println(F(" MHz. Apague-o antes (comando a N)."));
      return;
    }
  }
  if (nPontos >= MAX_PONTOS) {
    Serial.println(F("! Memoria cheia. Apague algum ponto."));
    return;
  }
  // Insere ordenado pela tensão
  uint8_t pos = nPontos;
  while (pos > 0 && pontos[pos - 1].v > v) {
    pontos[pos] = pontos[pos - 1];
    pos--;
  }
  pontos[pos].v = v;
  pontos[pos].mhz = mhz;
  nPontos++;
  salvar();

  Serial.print(F("+ Gravado: "));
  Serial.print(v, 3);
  Serial.print(F(" V = "));
  Serial.print(mhz, 1);
  Serial.println(F(" MHz"));
  if (ruido > 0.05) Serial.println(F("  (atencao: leitura ruidosa, considere o capacitor de 100nF em A0)"));
  verificarMonotonia();
}

// Num varicap a frequência deve sempre subir (ou sempre descer) com a tensão.
void verificarMonotonia() {
  if (nPontos < 3) return;
  bool sobe = pontos[1].mhz > pontos[0].mhz;
  for (uint8_t i = 1; i + 1 < nPontos; i++) {
    if ((pontos[i + 1].mhz > pontos[i].mhz) != sobe) {
      Serial.println(F("! Mapa incoerente: a frequencia nao varia num so sentido."));
      Serial.println(F("  Algum ponto deve estar errado - confira com 'l'."));
      return;
    }
  }
}

void listar() {
  if (nPontos == 0) { Serial.println(F("Nenhum ponto calibrado.")); return; }
  Serial.println(F("#   Tensao(V)   MHz"));
  for (uint8_t i = 0; i < nPontos; i++) {
    Serial.print(i + 1);
    Serial.print(F("   "));
    Serial.print(pontos[i].v, 3);
    Serial.print(F("      "));
    Serial.println(pontos[i].mhz, 1);
  }
}

void ajuda() {
  Serial.println(F("--- FM Mapeador ---"));
  Serial.println(F("98.5  grava a tensao atual como 98.5 MHz"));
  Serial.println(F("l     lista pontos"));
  Serial.println(F("a N   apaga o ponto N"));
  Serial.println(F("z     apaga tudo"));
  Serial.println(F("?     ajuda"));
}

void processarComando(char *cmd) {
  while (*cmd == ' ') cmd++;
  if (*cmd == 0) return;

  if (cmd[0] == 'l' || cmd[0] == 'L') { listar(); return; }
  if (cmd[0] == '?' || cmd[0] == 'h') { ajuda(); return; }
  if (cmd[0] == 'z' || cmd[0] == 'Z') {
    nPontos = 0; salvar();
    Serial.println(F("Todos os pontos apagados."));
    return;
  }
  if (cmd[0] == 'a' || cmd[0] == 'A') {
    int n = atoi(cmd + 1);
    if (n < 1 || n > nPontos) { Serial.println(F("! Numero invalido.")); return; }
    for (uint8_t j = n - 1; j + 1 < nPontos; j++) pontos[j] = pontos[j + 1];
    nPontos--; salvar();
    Serial.println(F("Ponto apagado."));
    return;
  }
  float mhz = atof(cmd);
  if (mhz >= 60.0 && mhz <= 120.0) { adicionarPonto(mhz); return; }
  Serial.println(F("! Comando nao reconhecido. Digite ? para ajuda."));
}

void lerSerial() {
  while (Serial.available()) {
    char c = Serial.read();
    if (c == '\n' || c == '\r') {
      buf[bufLen] = 0;
      if (bufLen > 0) processarComando(buf);
      bufLen = 0;
    } else if (bufLen < sizeof(buf) - 1) {
      buf[bufLen++] = (c == ',') ? '.' : c;  // aceita 98,5 também
    }
  }
}

// ---------- Display ----------
// Mostra a frequência com o ícone MHz. O ponto do PLAY acende quando a
// sintonia está numa estação calibrada; o MHz pisca quando extrapolado.
float freqMostrada = 0;

void atualizarDisplay(float f, bool fora, bool naEstacao, float fEstacao) {
  disp.clear();
  if (isnan(f)) {
    disp.showText("CAL ");
    return;
  }
  float alvo = naEstacao ? fEstacao : f;
  // histerese: evita o último dígito ficar trocando entre dois valores
  if (naEstacao || fabs(alvo - freqMostrada) > 0.07) freqMostrada = alvo;
  if (freqMostrada < 10.0 || freqMostrada > 999.9) {
    disp.showText("----");
    return;
  }
  disp.showFM(freqMostrada);
  disp.play(naEstacao);
  if (fora && (millis() / 500) % 2) disp.mhz(false);
}

void mostrarLeitura() {
  float ruido;
  float v = lerTensao(ruido);

  Serial.print(F("V="));
  Serial.print(v, 3);
  Serial.print(F(" V (ruido "));
  Serial.print(ruido * 1000, 0);
  Serial.print(F(" mV)"));

  bool fora;
  float f = estimarMHz(v, fora);
  if (isnan(f)) {
    Serial.println(F("  | calibre ao menos 2 estacoes"));
    atualizarDisplay(f, false, false, 0);
    return;
  }
  Serial.print(F("  | ~"));
  Serial.print(f, 1);
  Serial.print(F(" MHz"));
  if (fora) Serial.print(F(" (extrapolado)"));

  // Estação calibrada mais próxima
  uint8_t melhor = 0;
  for (uint8_t i = 1; i < nPontos; i++)
    if (fabs(pontos[i].mhz - f) < fabs(pontos[melhor].mhz - f)) melhor = i;
  bool naEstacao = fabs(pontos[melhor].mhz - f) < 0.15;
  if (naEstacao) {
    Serial.print(F("  <- estacao "));
    Serial.print(pontos[melhor].mhz, 1);
  }
  Serial.println();
  atualizarDisplay(f, fora, naEstacao, pontos[melhor].mhz);
}

void setup() {
  Serial.begin(115200);
  analogReference(DEFAULT);
  disp.begin();
  disp.showText("FM  ");
  disp.wait(800);
  carregar();
  ajuda();
  Serial.print(nPontos);
  Serial.println(F(" ponto(s) carregado(s) da EEPROM."));
}

void loop() {
  disp.update();
  lerSerial();
  static unsigned long ultimo = 0;
  if (millis() - ultimo >= INTERVALO_MS) {
    ultimo = millis();
    mostrarLeitura();
  }
}
