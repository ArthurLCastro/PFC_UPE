/*
  Kit de Eletrônica para Maquetes de Arquitetura
  
  Projeto 2EE - Microcontroladores - Poli/UPE
  Turma DS - Prof Andrea Maria

  Arthur Castro
  Daires Macedo
  Rodrigo Laurenio
*/

#include <Arduino.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include "SPIFFS.h"

#include "env.h"    // Arquivo externo criado com as definicoes de ENV_WIFI_SSID e ENV_WIFI_PASSWORD

// Descomente as linhas de DEBUG abaixo relacionadas a cada parte do sistema. Atencao: O uso de alguns DEBUGs pode atrasar consideravalmente o codigo
#define DEBUG_WIFI

AsyncWebServer server(80);

const char* ssid = ENV_WIFI_SSID;
const char* password = ENV_WIFI_PASSWORD;

// Relacao entre os Conectores do Modulo Central com GPIOs do ESP32
// Pinos para Sensores (Analogicos)
const uint8_t CONECTOR_01 = 32;     // GPIO Analogico
const uint8_t CONECTOR_02 = 33;     // GPIO Analogico
const uint8_t CONECTOR_03 = 34;     // GPI Analogico
const uint8_t CONECTOR_04 = 35;     // GPI Analogico
// Pinos para Atuadores (Digitais)
const uint8_t CONECTOR_05 = 5;      // GPIO Digital
const uint8_t CONECTOR_06 = 18;     // GPIO Digital
const uint8_t CONECTOR_07 = 19;     // GPIO Digital
const uint8_t CONECTOR_08 = 21;     // GPIO Digital

String tipo_de_automacao="", modelo_da_automacao="";
uint8_t pinLDR, pinLED;
bool run = false;

void automacao_ldr_run(uint8_t pin_ldr, uint8_t pin_led) {
  uint8_t adc_value = analogRead(pin_ldr);

  if (adc_value < 10) {
    digitalWrite(pin_led, HIGH);
  } else {
    digitalWrite(pin_led, LOW);
  }
}

uint8_t converter_conectores_em_pinos(String substring) {
  if (substring == "C1") {
    return CONECTOR_01;
  } else if (substring == "C2") {
    return CONECTOR_02;
  } else if (substring == "C3") {
    return CONECTOR_03;
  } else if (substring == "C4") {
    return CONECTOR_04;
  } else if (substring == "C5") {
    return CONECTOR_05;
  } else if (substring == "C6") {
    return CONECTOR_06;
  } else if (substring == "C7") {
    return CONECTOR_07;
  } else if (substring == "C8") {
    return CONECTOR_08;
  } else {
    Serial.println("Valor de conector inválido! Reinicie o sistema");
    while(1){};
  }
}

void notFound(AsyncWebServerRequest *request) {
  request->send(404, "text/plain", "Not found");
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

  server.on("/stop", HTTP_GET, [](AsyncWebServerRequest *request){
    run = false;
    digitalWrite(pinLDR, LOW);
    digitalWrite(pinLED, LOW);

    request->send(SPIFFS, "/stop_central.html", "text/html");
  });

  // Requisicao GET para <ESP_IP>/config passando os parametros necessarios
  server.on("/config", HTTP_GET, [] (AsyncWebServerRequest *request) {
    if (request->hasParam("tipo_de_automacao") and request->hasParam("modelo_da_automacao")) {
      tipo_de_automacao = request->getParam("tipo_de_automacao")->value();
      modelo_da_automacao = request->getParam("modelo_da_automacao")->value();
      
      if (tipo_de_automacao == "ILM" and modelo_da_automacao == "M01") {
        String conLDR, conLED;
        // uint8_t pinLDR, pinLED;

        conLDR = request->getParam("conLDR")->value();
        conLED = request->getParam("conLED")->value();

        Serial.print("conLDR: "); Serial.println(conLDR);
        Serial.print("conLED: "); Serial.println(conLED);

        pinLDR = converter_conectores_em_pinos(conLDR);
        pinLED = converter_conectores_em_pinos(conLED);

        Serial.print("pinLDR: "); Serial.println(pinLDR);
        Serial.print("pinLED: "); Serial.println(pinLED);
        Serial.println("");

        run = true;
        request->send(SPIFFS, "/success.html", "text/html");

      // } else if (tipo_de_automacao == "ILM" and modelo_da_automacao == "M02") {

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

  pinMode(CONECTOR_05, OUTPUT);
  pinMode(CONECTOR_06, OUTPUT);
  pinMode(CONECTOR_07, OUTPUT);
  pinMode(CONECTOR_08, OUTPUT);

  // Inicializando saidas em nivel logico baixo
  digitalWrite(CONECTOR_05, LOW);
  digitalWrite(CONECTOR_06, LOW);
  digitalWrite(CONECTOR_07, LOW);
  digitalWrite(CONECTOR_08, LOW);

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
  Serial.println();
  Serial.print("Endereço IP: ");
  Serial.println(WiFi.localIP());

  setWebserver();
}

void loop() {
  if (run) {
    automacao_ldr_run(pinLDR, pinLED);
  }
  delay(100);       // Provisorio
}
