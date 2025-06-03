/*
  Kit Interativo para Ensino de Automacao em Turmas de Arquitetura baseado em Hardware Dinamicamente Reconfiguravel

  Projeto Final de Curso 2025.1 - Poli/UPE
*/

#include <Arduino.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include "SPIFFS.h"

#include "env.h"    // Arquivo externo criado com as definicoes de ENV_WIFI_SSID e ENV_WIFI_PASSWORD

/*
  Quando necessario, descomente uma ou mais linhas de DEBUG abaixo...
  Atencao: O uso de alguns DEBUGs pode atrasar consideravalmente o codigo
*/
#define DEBUG_WIFI
// #define DEBUG_AUTOMACOES

#define QTD_MAX_AUTOMACOES 4

struct Automacao {
  uint8_t input_pin;
  uint8_t output_pin;
  void (*function)(uint8_t, uint8_t);
};

Automacao array_automacoes[QTD_MAX_AUTOMACOES];
int qtd_automacoes = 0;

AsyncWebServer server(80);

const char* ssid = ENV_WIFI_SSID;
const char* password = ENV_WIFI_PASSWORD;

// Relacao entre os Conectores do Modulo Central com GPIOs do ESP32
// Pinos para Sensores (Analogicos)
const uint8_t CONECTOR_01 = 33;     // GPIO Analogico
const uint8_t CONECTOR_02 = 32;     // GPIO Analogico
const uint8_t CONECTOR_03 = 36;     // GPI Analogico
const uint8_t CONECTOR_04 = 34;     // GPI Analogico
const uint8_t CONECTOR_05 = 35;     // GPI Analogico
// Pinos para Atuadores (Digitais)
const uint8_t CONECTOR_06 = 18;     // GPIO Digital
const uint8_t CONECTOR_07 = 19;     // GPIO Digital
const uint8_t CONECTOR_08 = 21;     // GPIO Digital
const uint8_t CONECTOR_09 = 23;     // GPIO Digital
const uint8_t CONECTOR_10 = 22;     // GPIO Digital

String tipo_de_automacao="", modelo_da_automacao="";
bool run = false;

void automacao_ldr_run(uint8_t pin_ldr, uint8_t pin_led) {
  uint8_t adc_value = analogRead(pin_ldr);

  if (adc_value < 10) {
    digitalWrite(pin_led, HIGH);
  } else {
    digitalWrite(pin_led, LOW);
  }
}

void automacao_pir_run(uint8_t pin_pir, uint8_t pin_led) {  
  digitalWrite(pin_led, digitalRead(pin_pir));
}

// Função para adicionar automação às configurações. Quando o número de automações ultrapassar o limite, a primeira configurada será sobrescrita
void config_automacao(uint8_t input_pin, uint8_t output_pin, void (*function)(uint8_t, uint8_t)) {
  int idx_automacao;

  qtd_automacoes++;

  if (qtd_automacoes <= QTD_MAX_AUTOMACOES) {
    idx_automacao = qtd_automacoes - 1;
  } else {
    qtd_automacoes = QTD_MAX_AUTOMACOES;
    idx_automacao = 0;
  }

  array_automacoes[idx_automacao].input_pin = input_pin;
  array_automacoes[idx_automacao].output_pin = output_pin;
  array_automacoes[idx_automacao].function = function;
  
  #ifdef DEBUG_AUTOMACOES
  Serial.print("Configurada a automação ");
  Serial.print(idx_automacao);
  Serial.print("  |  qtd_automacoes: ");
  Serial.println(qtd_automacoes);
  #endif
}

uint8_t converter_conectores_em_pinos(String substring) {
  if (substring == "C01") {
    return CONECTOR_01;
  } else if (substring == "C02") {
    return CONECTOR_02;
  } else if (substring == "C03") {
    return CONECTOR_03;
  } else if (substring == "C04") {
    return CONECTOR_04;
  } else if (substring == "C05") {
    return CONECTOR_05;
  } else if (substring == "C06") {
    return CONECTOR_06;
  } else if (substring == "C07") {
    return CONECTOR_07;
  } else if (substring == "C08") {
    return CONECTOR_08;
  } else if (substring == "C09") {
    return CONECTOR_09;
  } else if (substring == "C10") {
    return CONECTOR_10;
  } else {
    Serial.println("Valor de conector inválido! Reinicie o sistema");
    while(1){};
  }
}

void notFound(AsyncWebServerRequest *request) {
  request->send(404, "text/plain", "Not found");
}

String processor(const String& var){
  String run_state;
  
  if(var == "RUN_STATE"){
    if(run){
      run_state = "Automações em execução";
    }
    else{
      run_state = "Automações suspensas";
    }
    return run_state;
  }
  return String();
}

void setWebserver(){
  // Rotas para arquivos

  server.on("/img/maquete.ico", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/img/maquete.ico", "image/x-icon");
  });

  server.on("/img/maquete.png", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/img/maquete.png", "image/png");
  });

  server.on("/css/style.css", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/css/style.css", "text/css");
  });

  server.on("/css/bootstrap.min.css", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/css/bootstrap.min.css", "text/css");
  });

  server.on("/js/color-modes.js", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/js/color-modes.js", "text/javascript");
  });

  server.on("/js/bootstrap.bundle.min.js", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/js/bootstrap.bundle.min.js", "text/javascript");
  });


  // Rotas para paginas do sistema

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/index.html", "text/html");
  });

  server.on("/ilm/escolher", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/ilm/escolher.html", "text/html");
  });

  server.on("/ilm/mod01", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/ilm/mod01.html", "text/html");
  });

  server.on("/ilm/mod02", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/ilm/mod02.html", "text/html");
  });

  server.on("/start_stop", HTTP_GET, [](AsyncWebServerRequest *request){    
    request->send(SPIFFS, "/start_stop.html", String(), false, processor);
  });

  server.on("/start", HTTP_GET, [](AsyncWebServerRequest *request){
    run = true;
    request->send(SPIFFS, "/start_stop.html", String(), false, processor);
  });

  server.on("/stop", HTTP_GET, [](AsyncWebServerRequest *request){
    run = false;
  
    for (int i=0; i < qtd_automacoes; i++) {
      digitalWrite(array_automacoes[i].output_pin, LOW);
    }

    request->send(SPIFFS, "/start_stop.html", String(), false, processor);
  });

  server.on("/reset", HTTP_GET, [](AsyncWebServerRequest *request){
    run = false;
  
    for (int i=0; i < qtd_automacoes; i++) {
      digitalWrite(array_automacoes[i].output_pin, LOW);
    }

    qtd_automacoes = 0;   // O array 'array_automacoes' ainda esta preenchido, mas sera sobrescrito na proxima configuracao

    request->send(SPIFFS, "/start_stop.html", String(), false, processor);
  });

  // Requisicao GET para <ESP_IP>/config passando os parametros necessarios
  server.on("/config", HTTP_GET, [] (AsyncWebServerRequest *request) {
    if (request->hasParam("tipo_de_automacao") and request->hasParam("modelo_da_automacao")) {
      tipo_de_automacao = request->getParam("tipo_de_automacao")->value();
      modelo_da_automacao = request->getParam("modelo_da_automacao")->value();
      
      if (tipo_de_automacao == "ILM" and modelo_da_automacao == "M01") {
        String conLDR, conLED;
        uint8_t pinLDR, pinLED;

        conLDR = request->getParam("conLDR")->value();
        conLED = request->getParam("conLED")->value();

        pinLDR = converter_conectores_em_pinos(conLDR);
        pinLED = converter_conectores_em_pinos(conLED);

        config_automacao(pinLDR, pinLED, automacao_ldr_run);
        run = true;
        request->send(SPIFFS, "/success.html", "text/html");

      } else if (tipo_de_automacao == "ILM" and modelo_da_automacao == "M02") {
        String conPIR, conLED;
        uint8_t pinPIR, pinLED;

        conPIR = request->getParam("conPIR")->value();
        conLED = request->getParam("conLED")->value();

        pinPIR = converter_conectores_em_pinos(conPIR);
        pinLED = converter_conectores_em_pinos(conLED);

        config_automacao(pinPIR, pinLED, automacao_pir_run);
        run = true;
        request->send(SPIFFS, "/success.html", "text/html");

      } else {
        request->send(SPIFFS, "/error.html", "text/html");
      }
    
    } else {
      request->send(SPIFFS, "/error.html", "text/html");
    }
  });

  server.onNotFound(notFound);

  server.begin();
}

void setup() {
  // Configurando conectores como entradas ou saidas
  pinMode(CONECTOR_01, INPUT);
  pinMode(CONECTOR_02, INPUT);
  pinMode(CONECTOR_03, INPUT);
  pinMode(CONECTOR_04, INPUT);
  pinMode(CONECTOR_05, INPUT);

  pinMode(CONECTOR_06, OUTPUT);
  pinMode(CONECTOR_07, OUTPUT);
  pinMode(CONECTOR_08, OUTPUT);
  pinMode(CONECTOR_09, OUTPUT);
  pinMode(CONECTOR_10, OUTPUT);

  // Inicializando saidas em nivel logico baixo
  digitalWrite(CONECTOR_06, LOW);
  digitalWrite(CONECTOR_07, LOW);
  digitalWrite(CONECTOR_08, LOW);
  digitalWrite(CONECTOR_09, LOW);
  digitalWrite(CONECTOR_10, LOW);

  Serial.begin(115200);   // Para fins de DEBUG

  // Inicializa SPIFFS
  if(!SPIFFS.begin(true)){
    Serial.println("Ocorreu um erro ao montar o SPIFFS");
    return;
  }

  // Inicializa Wi-Fi
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  if (WiFi.waitForConnectResult() != WL_CONNECTED) {
    Serial.println("Falha no Wi-Fi!");
    return;
  }
  #ifdef DEBUG_WIFI
  Serial.println();
  Serial.print("Endereço IP: ");
  Serial.println(WiFi.localIP());
  #endif

  setWebserver();
}

void loop() {
  if (run) {
    // Itera sobre a matriz array_automacoes e executa as funções correspondentes para cada uma delas
    for (int i=0; i < qtd_automacoes; i++) {
      array_automacoes[i].function(array_automacoes[i].input_pin, array_automacoes[i].output_pin);

      #ifdef DEBUG_AUTOMACOES
      Serial.print("qtd_automacoes: ");
      Serial.print(qtd_automacoes);
      Serial.print("  |  Executando automação ");
      Serial.print(i);
      Serial.print("  |  Pinos: ");
      Serial.print(array_automacoes[i].input_pin);
      Serial.print(" e ");
      Serial.println(array_automacoes[i].output_pin);
      #endif
    }
  }
  delay(100);       // Provisorio
}
