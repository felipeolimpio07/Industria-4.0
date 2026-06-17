const int pinoSensor = A0;
const int ledVerde = 8;
const int ledAmarelo = 9;
const int ledVermelho = 10;
const int pinoBuzzer = 11;

unsigned long tempoInicioEstado = 0;
unsigned long tempoVerde = 0;
unsigned long tempoAmarelo = 0;
unsigned long tempoVermelho = 0;
unsigned long ultimaVezApagouCritico = 0;
unsigned long tempoUltimoRelatorio = 0;

// Trava para não "enviar" notificações a cada segundo (Cooldown de 30 segundos no simulador)
unsigned long tempoUltimaNotificacao = 0;
const unsigned long INTERVALO_NOTIFICACAO = 30000; 

int estadoAtual = -1;

const int TAMANHO_HISTORICO = 10;
float historicoTemperaturas[TAMANHO_HISTORICO];
int indiceHistorico = 0;
bool historicoCheio = false;
int leiturasRealizadas = 0;

void setup() {
  Serial.begin(9600);
  pinMode(ledVerde, OUTPUT);
  pinMode(ledAmarelo, OUTPUT);
  pinMode(ledVermelho, OUTPUT);
  pinMode(pinoBuzzer, OUTPUT);
  
  digitalWrite(ledVerde, HIGH);
  digitalWrite(ledAmarelo, HIGH);
  digitalWrite(ledVermelho, HIGH);
  
  tempoInicioEstado = millis();
  Serial.print(">>> MONITORAMENTO INDUSTRIAL COM IA ATIVA (TINKERCAD) <<<\r\n");
}

void loop() {
  int leitura = analogRead(pinoSensor);
  float tensao = leitura * (5.0 / 1023.0);
  float temperaturaC = (tensao - 0.5) * 100.0; // Fórmula padrão do TMP36 no Tinkercad

  historicoTemperaturas[indiceHistorico] = temperaturaC;
  indiceHistorico = (indiceHistorico + 1) % TAMANHO_HISTORICO;
  if (indiceHistorico == 0) historicoCheio = true;
  if (!historicoCheio) leiturasRealizadas++;

  int novoEstado;
  if (temperaturaC < 25.0) novoEstado = 0;
  else if (temperaturaC < 40.0) novoEstado = 1;
  else novoEstado = 2;

  unsigned long tempoAgora = millis();
  unsigned long duracaoPassada = (tempoAgora - tempoInicioEstado) / 1000;
  tempoInicioEstado = tempoAgora; 

  if (estadoAtual == 0) tempoVerde += duracaoPassada;
  else if (estadoAtual == 1) tempoAmarelo += duracaoPassada;
  else if (estadoAtual == 2) tempoVermelho += duracaoPassada;

  if (novoEstado != estadoAtual) {
    if (estadoAtual == 2 && novoEstado != 2) {
      ultimaVezApagouCritico = tempoAgora / 1000;
    }

    digitalWrite(ledVerde, HIGH);
    digitalWrite(ledAmarelo, HIGH);
    digitalWrite(ledVermelho, HIGH);
    noTone(pinoBuzzer);

    if (novoEstado == 0) {
      digitalWrite(ledVerde, LOW);
      Serial.print("\r\n[STATUS] Temperatura BAIXA - LED Verde Ativo\r\n");
    }
    else if (novoEstado == 1) {
      digitalWrite(ledAmarelo, LOW);
      Serial.print("\r\n[STATUS] Temperatura NORMAL - LED Amarelo Ativo\r\n");
    }
    else if (novoEstado == 2) {
      digitalWrite(ledVermelho, LOW);
      Serial.print("\r\n[STATUS] Temperatura CRITICA - LED Vermelho Ativo\r\n");
    }

    estadoAtual = novoEstado;
  }

  // --- LÓGICA DE ALERTA E "API" ---
  if (estadoAtual == 2) {
    // Apita o buzzer após 5 segundos no vermelho
    if (tempoVermelho > 5) { 
      tone(pinoBuzzer, 1000);
    }
    
    // Gatilho da API: Se passar de 45°C e o tempo de espera da última mensagem já passou
    if (temperaturaC > 45.0 && (tempoAgora - tempoUltimaNotificacao > INTERVALO_NOTIFICACAO)) {
      simularChamadaAPI(temperaturaC);
      tempoUltimaNotificacao = tempoAgora;
    }
  }

  // Gera o relatório a cada 10 segundos no simulador para não poluir demais a tela
  if (tempoAgora - tempoUltimoRelatorio > 10000) {
    analisarIA(temperaturaC);
    gerarRelatorio(temperaturaC, tempoVerde, tempoAmarelo, tempoVermelho, ultimaVezApagouCritico);
    tempoUltimoRelatorio = tempoAgora;
  }

  delay(1000);
}

void simularChamadaAPI(float temp) {
  // Esta função simula o que aconteceria no backend caso houvesse internet
  Serial.println();
  Serial.print("\r\n======================================================\r\n");
  Serial.print("   [ MOCK API ] SIMULANDO ENVIO DE NOTIFICACAO\r\n");
  Serial.print("======================================================\r\n");
  Serial.print("Destino: WhatsApp / E-mail\r\n");
  Serial.print("Payload JSON Gerado:\r\n");
  Serial.print("{\r\n");
  Serial.print("  \"alerta\": \"CRITICO\",\r\n");
  Serial.print("  \"mensagem\": \"Risco de superaquecimento no motor!\",\r\n");
  Serial.print("  \"temperatura_registrada\": "); Serial.print(temp); Serial.print(",\r\n");
  Serial.print("  \"timestamp_ms\": "); Serial.print(millis()); Serial.print("\r\n");
  Serial.print("}\r\n");
  Serial.print("Status: [200 OK] - Mensagem 'enviada' com sucesso.\r\n");
  Serial.print("======================================================\r\n");
}

void analisarIA(float tempAtual) {
  if (!historicoCheio) {
    Serial.print("\r\n[IA] Coletando dados iniciais para analise de tendencia...\r\n");
    return;
  }

  float somaPrimeiraMetade = 0;
  float somaSegundaMetade = 0;
  int metade = TAMANHO_HISTORICO / 2;

  for (int i = 0; i < metade; i++) {
    int idx1 = (indiceHistorico + i) % TAMANHO_HISTORICO;
    int idx2 = (indiceHistorico + metade + i) % TAMANHO_HISTORICO;
    somaPrimeiraMetade += historicoTemperaturas[idx1];
    somaSegundaMetade += historicoTemperaturas[idx2];
  }

  float mediaInicio = somaPrimeiraMetade / metade;
  float mediaFim = somaSegundaMetade / metade;

  Serial.print("\r\n[IA - ANALISE DE TENDENCIA]\r\n");
  if (mediaFim > mediaInicio + 1.0) {
    Serial.print(">>> AVISO IA: Tendencia de AUMENTO de temperatura detectada!\r\n");
  } else if (mediaFim < mediaInicio - 1.0) {
    Serial.print(">>> AVISO IA: Tendencia de QUEDA de temperatura detectada!\r\n");
  } else {
    Serial.print(">>> INFO IA: Temperatura estavel.\r\n");
  }
}

void gerarRelatorio(float temp, unsigned long v, unsigned long a, unsigned long r, unsigned long ultima) {
  Serial.println(); 
  Serial.print("\r\n------------------------------------------------------\r\n");
  Serial.print("           RELATORIO DE TELEMETRIA INDUSTRIAL           \r\n");
  Serial.print("Temperatura Atual: "); Serial.print(temp); Serial.print(" C\r\n");
  Serial.print("Tempo em Estado BAIXO (Verde): "); Serial.print(v); Serial.print("s\r\n");
  Serial.print("Tempo em Estado NORMAL (Amarelo): "); Serial.print(a); Serial.print("s\r\n");
  Serial.print("Tempo em Estado CRITICO (Vermelho): "); Serial.print(r); Serial.print("s\r\n");
  
  if(ultima > 0) {
    Serial.print("Ultimo Incidente Resolvido aos: "); Serial.print(ultima); Serial.print("s\r\n");
  }
  
  float soma = 0;
  float maxT = -100;
  float minT = 200;
  
  int limiteLoop = historicoCheio ? TAMANHO_HISTORICO : leiturasRealizadas;
  
  for(int i=0; i<limiteLoop; i++) {
    if(historicoTemperaturas[i] > maxT) maxT = historicoTemperaturas[i];
    if(historicoTemperaturas[i] < minT) minT = historicoTemperaturas[i];
    soma += historicoTemperaturas[i];
  }
  
  Serial.print("--- Estatisticas IA (Ultimas leituras validas) ---\r\n");
  Serial.print("Media:  "); Serial.print(soma/limiteLoop); Serial.print(" C\r\n");
  Serial.print("Maxima: "); Serial.print(maxT); Serial.print(" C\r\n");
  Serial.print("Minima: "); Serial.print(minT); Serial.print(" C\r\n");
  Serial.print("------------------------------------------------------\r\n");
}
