// 3 Sensores
const int PINO_BOTAO_RE  = 2;   // Botão para engatar/desengatar ré
const int PINO_INCLINA   = 8;   // Sensor de Inclinação (SW 200D)
const int PINO_TRIG      = 9;   // Ultrassônico Trig
const int PINO_ECHO      = 10;  // Ultrassônico Echo

// 2 Atuadores (Sonoro e Visual)
const int PINO_BUZZER    = 3;   // Piezo (Buzzer)
const int PINO_LED_AZUL  = 4;   // LED Azul -> Indica apenas inclinação
const int PINO_LED_R     = 5;   // LED RGB -> Vermelho
const int PINO_LED_G     = 6;   // LED RGB -> Verde
const int PINO_LED_B     = 7;   // LED RGB -> Azul

const int DIST_ATENCAO = 50;    // Abaixo de 50 cm: Amarelo
const int DIST_PERIGO  = 25;    // Abaixo de 25 cm: Vermelho (bipe rápido)
const int DIST_CRITICA = 10;    // Abaixo de 10 cm: Vermelho (apito contínuo)

bool reEngatada = false;
bool estadoAnteriorBotao = HIGH;
unsigned long ultimoDebounce = 0;

unsigned long tempoAnteriorBipe = 0;
bool estadoBipe = false;

void setup() {
  Serial.begin(9600);

  pinMode(PINO_BOTAO_RE, INPUT_PULLUP);
  pinMode(PINO_INCLINA, INPUT_PULLUP);
  
  pinMode(PINO_TRIG, OUTPUT);
  pinMode(PINO_ECHO, INPUT);

  pinMode(PINO_BUZZER, OUTPUT);
  pinMode(PINO_LED_AZUL, OUTPUT);
  pinMode(PINO_LED_R, OUTPUT);
  pinMode(PINO_LED_G, OUTPUT);
  pinMode(PINO_LED_B, OUTPUT);

  desligarTudo();
  Serial.println("Sistema Pronto! Pressione o botao para engatar a Re.");
}

void loop() {
  bool leituraBotao = digitalRead(PINO_BOTAO_RE);
  
  if (leituraBotao == LOW && estadoAnteriorBotao == HIGH && (millis() - ultimoDebounce > 250)) {
    ultimoDebounce = millis();
    reEngatada = !reEngatada;

    if (reEngatada) {
      Serial.println(">>> MARCHA RE ENGATADA <<<");
      tone(PINO_BUZZER, 1000, 100);
      delay(120);
    } else {
      Serial.println(">>> PONTO MORTO (DESLIGADO) <<<");
      desligarTudo();
    }
  }
  estadoAnteriorBotao = leituraBotao;

  // Se a ré não estiver engatada, mantém tudo desligado
  if (!reEngatada) {
    delay(20);
    return;
  }

  // 2. Leitura dos Sensores
  long distancia = lerDistanciaCM();
  bool estaInclinado = (digitalRead(PINO_INCLINA) == LOW);

  // 3. LED Azul: Indica APENAS se está inclinado
  digitalWrite(PINO_LED_AZUL, estaInclinado ? HIGH : LOW);

  // 4. LED RGB e Buzzer: Indicam APENAS a distância / perigo
  if (distancia > DIST_ATENCAO) {
    // Seguro (> 50 cm) -> Verde
    definirCorRGB(LOW, HIGH, LOW);
    noTone(PINO_BUZZER);
    estadoBipe = false;
  } 
  else if (distancia > DIST_PERIGO) {
    // Atenção (26 a 50 cm) -> Amarelo (Vermelho + Verde)
    definirCorRGB(HIGH, HIGH, LOW);
    gerenciarBipe(500, 1200);
  } 
  else if (distancia > DIST_CRITICA) {
    // Perigo (11 a 25 cm) -> Vermelho + Bipe rápido
    definirCorRGB(HIGH, LOW, LOW);
    gerenciarBipe(150, 1800);
  } 
  else {
    // Crítico (<= 10 cm) -> Vermelho + Apito contínuo
    definirCorRGB(HIGH, LOW, LOW);
    tone(PINO_BUZZER, 2500);
  }

  // Monitor Serial
  Serial.print("Distancia: "); Serial.print(distancia);
  Serial.print(" cm | Inclinado: "); Serial.println(estaInclinado ? "SIM" : "NAO");

  delay(60);
}

// --- Funções Auxiliares ---

long lerDistanciaCM() {
  digitalWrite(PINO_TRIG, LOW);
  delayMicroseconds(2);
  digitalWrite(PINO_TRIG, HIGH);
  delayMicroseconds(10);
  digitalWrite(PINO_TRIG, LOW);

  long duracao = pulseIn(PINO_ECHO, HIGH, 30000);
  if (duracao == 0) return 999;
  return duracao * 0.034 / 2;
}

void gerenciarBipe(unsigned long intervaloMs, int frequenciaHz) {
  unsigned long tempoAtual = millis();
  if (tempoAtual - tempoAnteriorBipe >= intervaloMs) {
    tempoAnteriorBipe = tempoAtual;
    estadoBipe = !estadoBipe;

    if (estadoBipe) {
      tone(PINO_BUZZER, frequenciaHz);
    } else {
      noTone(PINO_BUZZER);
    }
  }
}

void definirCorRGB(int r, int g, int b) {
  digitalWrite(PINO_LED_R, r);
  digitalWrite(PINO_LED_G, g);
  digitalWrite(PINO_LED_B, b);
}

void desligarTudo() {
  definirCorRGB(LOW, LOW, LOW);
  digitalWrite(PINO_LED_AZUL, LOW);
  noTone(PINO_BUZZER);
  estadoBipe = false;
}