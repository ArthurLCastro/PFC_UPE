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

// Relacao entre os conectores do Modulo Central com GPIOs do ESP32 (temporariamente ate escolher todos os pinos como IOs analogicos)
const uint8_t CONECTOR_01 = 32;     // GPIO Analogico
const uint8_t CONECTOR_02 = 33;     // GPIO Analogico
const uint8_t CONECTOR_03 = 34;     // GPI Analogico
const uint8_t CONECTOR_04 = 35;     // GPI Analogico

String conLdr1="", conLed1="", conLdr2="", conLed2="";
uint8_t pinLdr1, pinLed1, pinLdr2, pinLed2;
bool run = false;

void automacao_ldr_config(uint8_t pin_ldr, uint8_t pin_led) {
  pinMode(pin_ldr, INPUT);
  pinMode(pin_led, OUTPUT);
}

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
    digitalWrite(pinLed1, LOW);
    digitalWrite(pinLed2, LOW);

    request->send(SPIFFS, "/stop_central.html", "text/html");
  });

  // Requisicao GET para <ESP_IP>/get passando os parametros necessarios
  server.on("/get", HTTP_GET, [] (AsyncWebServerRequest *request) {
    if (request->hasParam("autom01_conLDR")) {
      conLdr1 = request->getParam("autom01_conLDR")->value();
    }
    if (request->hasParam("autom01_conLED")) {
      conLed1 = request->getParam("autom01_conLED")->value();
    }
    if (request->hasParam("autom02_conLDR")) {
      conLdr2 = request->getParam("autom02_conLDR")->value();
    }
    if (request->hasParam("autom02_conLED")) {
      conLed2 = request->getParam("autom02_conLED")->value();
    }

    Serial.print("conLdr1: ");
    Serial.println(conLdr1);
    Serial.print("conLed1: ");
    Serial.println(conLed1);
    Serial.print("conLdr2: ");
    Serial.println(conLdr2);
    Serial.print("conLed2: ");
    Serial.println(conLed2);
    Serial.println("");

    pinLdr1 = converter_conectores_em_pinos(conLdr1);
    pinLed1 = converter_conectores_em_pinos(conLed1);
    pinLdr2 = converter_conectores_em_pinos(conLdr2);
    pinLed2 = converter_conectores_em_pinos(conLed2);

    Serial.print("pinLdr1: ");
    Serial.println(pinLdr1);
    Serial.print("pinLed1: ");
    Serial.println(pinLed1);
    Serial.print("pinLdr2: ");
    Serial.println(pinLdr2);
    Serial.print("pinLed2: ");
    Serial.println(pinLed2);
    Serial.println("");

    automacao_ldr_config(pinLdr1, pinLed1);
    automacao_ldr_config(pinLdr2, pinLed2);

    run = true;

    request->send(SPIFFS, "/success.html", "text/html");
  });

  server.onNotFound(notFound);

  server.begin();
}

void setup() {
  // Inicializando todos os pinos como entradas por seguranca
  pinMode(CONECTOR_01, INPUT);
  pinMode(CONECTOR_02, INPUT);
  pinMode(CONECTOR_03, INPUT);
  pinMode(CONECTOR_04, INPUT);

  Serial.begin(115200);   // Para fins de DEBUG

  // Inicializa SPIFFS
  if(!SPIFFS.begin(true)){
    Serial.println("Ocorreu um erro ao montar o SPIFFS");
    return;
  }

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
    automacao_ldr_run(pinLdr1, pinLed1);
    automacao_ldr_run(pinLdr2, pinLed2);
  }
  delay(100);       // Provisorio
}