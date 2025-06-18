/*
  Kit Interativo para Ensino de Automacao em Turmas de Arquitetura baseado em Hardware Dinamicamente Reconfiguravel

  Projeto Final de Curso 2025.1 - Poli/UPE
*/

#include <Arduino.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <DNSServer.h>
#include "SPIFFS.h"
#include <OneWire.h>
#include <DallasTemperature.h>

/*
  Quando necessario, descomente uma ou mais linhas de DEBUG abaixo...
  Atencao: O uso de alguns DEBUGs pode atrasar consideravalmente o codigo
*/
#define DEBUG_WIFI
// #define DEBUG_AUTOMACOES
#define DEBUG_SISTEMA_HVAC

#define QTD_MAX_AUTOMACOES 5

#define INTERVALO_MS_ENTRE_ATUALIZACOES 100
unsigned long previousTime = 0;

#define TEMPERATURA_LIMITE 32   // Limiar de temperatura em Celcius para a automacao Sistema HVAC ligar ou desligar o Bloco Ventilador Axial

struct Automacao {
  uint8_t input_pin;
  uint8_t output_pin;
  void (*function)(uint8_t, uint8_t);
};

Automacao array_automacoes[QTD_MAX_AUTOMACOES];
int qtd_automacoes = 0;

// Objetos para o DS18B20 - serão inicializados dinamicamente
OneWire* oneWire_instances[QTD_MAX_AUTOMACOES] = {nullptr};
DallasTemperature* sensors_instances[QTD_MAX_AUTOMACOES] = {nullptr};

AsyncWebServer server(80);
DNSServer dnsServer;

// Configurações do SoftAP
const char* ssid_ap = "Kit de Automacoes";
const char* password_ap = NULL;  // Sem senha
const IPAddress local_ip(192, 168, 4, 1);
const IPAddress gateway(192, 168, 4, 1);
const IPAddress subnet(255, 255, 255, 0);

// Relacao entre os Conectores do Modulo Central com GPIOs do ESP32
// Pinos para Sensores (Analogicos/Digitais)
// Obs.: O Sensor DS18B20 é compatível apenas com os Conectores 01 e 02 por conta do OneWire ser bidirecional
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

void automacao_ilm_mod01_run(uint8_t pin_ldr, uint8_t pin_led) {
  uint8_t adc_value = analogRead(pin_ldr);

  if (adc_value < 10) {
    digitalWrite(pin_led, HIGH);
  } else {
    digitalWrite(pin_led, LOW);
  }
}

void automacao_ilm_mod02_run(uint8_t pin_pir, uint8_t pin_led) {
  digitalWrite(pin_led, digitalRead(pin_pir));
}

void automacao_hvac_mod01_run(uint8_t pin_sensor, uint8_t pin_atuador) {
  // Encontra a instância do sensor correspondente ao pino
  int sensor_index = -1;
  for (int i = 0; i < qtd_automacoes; i++) {
    if (array_automacoes[i].input_pin == pin_sensor && array_automacoes[i].function == automacao_hvac_mod01_run) {
      sensor_index = i;
      break;
    }
  }

  if (sensor_index == -1 || sensors_instances[sensor_index] == nullptr) {
    #ifdef DEBUG_SISTEMA_HVAC
    Serial.println("Erro: Sensor DS18B20 não inicializado para este pino");
    #endif
    return;
  }

  // Solicita leitura de temperatura
  sensors_instances[sensor_index]->requestTemperatures();
  
  // Lê a temperatura em Celsius
  float temp = sensors_instances[sensor_index]->getTempCByIndex(0);
  
  // Verifica se a leitura é válida
  if (temp == DEVICE_DISCONNECTED_C) {
    #ifdef DEBUG_SISTEMA_HVAC
    Serial.println("Erro: DS18B20 desconectado ou falha na leitura");
    #endif
    return;
  }

  // Controle do atuador baseado na temperatura
  if (temp >= TEMPERATURA_LIMITE) {
    digitalWrite(pin_atuador, HIGH);
  } else {
    digitalWrite(pin_atuador, LOW);
  }

  #ifdef DEBUG_SISTEMA_HVAC
  Serial.print("DS18B20 - Temperatura: ");
  Serial.print(temp);
  Serial.print(" °C | Atuador: ");
  Serial.println(digitalRead(pin_atuador) ? "Ligado" : "Desligado");
  #endif
}

// Função para inicializar uma instância do DS18B20 em um pino específico
void init_ds18b20_instance(uint8_t pin, int index) {
  if (oneWire_instances[index] != nullptr) {
    delete oneWire_instances[index];
  }
  if (sensors_instances[index] != nullptr) {
    delete sensors_instances[index];
  }
  
  oneWire_instances[index] = new OneWire(pin);
  sensors_instances[index] = new DallasTemperature(oneWire_instances[index]);
  sensors_instances[index]->begin();
  
  #ifdef DEBUG_SISTEMA_HVAC
  Serial.print("DS18B20 inicializado no pino ");
  Serial.print(pin);
  Serial.print(" (índice ");
  Serial.print(index);
  Serial.println(")");
  #endif
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
  
  // Se for uma automação HVAC, inicializa o DS18B20
  if (function == automacao_hvac_mod01_run) {
    init_ds18b20_instance(input_pin, idx_automacao);
  }
  
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

// Redirecionamento do Captive Portal
void handleCaptivePortal(AsyncWebServerRequest *request) {
  String url = "http://" + local_ip.toString();
  request->redirect(url);
}

void setWebserver(){
  // Captive Portal - Redireciona qualquer domínio para o IP local

  server.onNotFound([](AsyncWebServerRequest *request){
    String host = request->host();
    // Se não for uma requisição para o IP local, redireciona
    if (host != local_ip.toString()) {
      handleCaptivePortal(request);
      return;
    }
    // Caso contrário, retorna 404
    notFound(request);
  });


  // Rotas específicas para detectores de Captive Portal

  server.on("/generate_204", HTTP_GET, handleCaptivePortal);              // Android
  server.on("/hotspot-detect.html", HTTP_GET, handleCaptivePortal);       // iOS
  server.on("/connectivity-check.html", HTTP_GET, handleCaptivePortal);   // Firefox
  server.on("/success.txt", HTTP_GET, handleCaptivePortal);               // Windows


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

  server.on("/hvac/escolher", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/hvac/escolher.html", "text/html");
  });

  server.on("/hvac/mod01", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/hvac/mod01.html", "text/html");
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

    // Limpa as instâncias dos sensores DS18B20
    for (int i = 0; i < qtd_automacoes; i++) {
      if (oneWire_instances[i] != nullptr) {
        delete oneWire_instances[i];
        oneWire_instances[i] = nullptr;
      }
      if (sensors_instances[i] != nullptr) {
        delete sensors_instances[i];
        sensors_instances[i] = nullptr;
      }
    }

    qtd_automacoes = 0;   // O array 'array_automacoes' ainda esta preenchido, mas sera sobrescrito na proxima configuracao

    request->send(SPIFFS, "/start_stop.html", String(), false, processor);
  });

  // Requisicao GET para <ESP_IP>/config passando os parametros necessarios
  server.on("/config", HTTP_GET, [] (AsyncWebServerRequest *request) {
    if (request->hasParam("tipo_de_automacao") and request->hasParam("modelo_da_automacao")) {
      tipo_de_automacao = request->getParam("tipo_de_automacao")->value();
      modelo_da_automacao = request->getParam("modelo_da_automacao")->value();
      
      if (tipo_de_automacao == "ILM") {
        if (modelo_da_automacao == "M01") {
          String conLDR, conLED;
          uint8_t pinLDR, pinLED;

          conLDR = request->getParam("conLDR")->value();
          conLED = request->getParam("conLED")->value();

          pinLDR = converter_conectores_em_pinos(conLDR);
          pinLED = converter_conectores_em_pinos(conLED);

          config_automacao(pinLDR, pinLED, automacao_ilm_mod01_run);
          run = true;
          request->send(SPIFFS, "/success.html", "text/html");

        } else if (modelo_da_automacao == "M02") {
          String conPIR, conLED;
          uint8_t pinPIR, pinLED;

          conPIR = request->getParam("conPIR")->value();
          conLED = request->getParam("conLED")->value();

          pinPIR = converter_conectores_em_pinos(conPIR);
          pinLED = converter_conectores_em_pinos(conLED);

          config_automacao(pinPIR, pinLED, automacao_ilm_mod02_run);
          run = true;
          request->send(SPIFFS, "/success.html", "text/html");
        }

      } else if (tipo_de_automacao == "REF") {
        if (modelo_da_automacao == "M01") {
          String conSensor, conAtuador;
          uint8_t pinSensor, pinAtuador;

          conSensor = request->getParam("conSensor")->value();
          conAtuador = request->getParam("conAtuador")->value();

          pinSensor = converter_conectores_em_pinos(conSensor);
          pinAtuador = converter_conectores_em_pinos(conAtuador);

          config_automacao(pinSensor, pinAtuador, automacao_hvac_mod01_run);
          run = true;
          request->send(SPIFFS, "/success.html", "text/html");
        }

      } else {
        request->send(SPIFFS, "/error.html", "text/html");
      }
    
    } else {
      request->send(SPIFFS, "/error.html", "text/html");
    }
  });

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

  // Configura o ESP32 como Access Point
  WiFi.mode(WIFI_AP);
  WiFi.softAPConfig(local_ip, gateway, subnet);
  WiFi.softAP(ssid_ap, password_ap);

  #ifdef DEBUG_WIFI
  Serial.println();
  Serial.println("Access Point iniciado");
  Serial.print("SSID: ");
  Serial.println(ssid_ap);
  Serial.print("IP: ");
  Serial.println(WiFi.softAPIP());
  Serial.println();
  #endif

  // Configura servidor DNS para Captive Portal
  dnsServer.start(53, "*", local_ip);

  setWebserver();
}

void loop() { 
  if (millis() - previousTime >= INTERVALO_MS_ENTRE_ATUALIZACOES) {
    previousTime = millis();

    // Processa requisições DNS para Captive Portal
    dnsServer.processNextRequest();

    #ifdef DEBUG_AUTOMACOES    
    Serial.println("Iniciando atualização de todas as automações!");
    #endif

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
        if (qtd_automacoes == (i+1)) {
          Serial.println("\n--------------------------------------------------------------------------------\n");
        }
        #endif
      }
    }

  }
}
