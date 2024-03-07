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

#include "env.h"    // Arquivo externo criado com as definicoes de ENV_WIFI_SSID e ENV_WIFI_PASSWORD

// Descomente as linhas de DEBUG abaixo relacionadas a cada parte do sistema. Atencao: O uso de alguns DEBUGs pode atrasar consideravalmente o codigo
#define DEBUG_WIFI

AsyncWebServer server(80);

const char* ssid = ENV_WIFI_SSID;
const char* password = ENV_WIFI_PASSWORD;

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="pt-br">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Kit de Eletrônica para Maquetes</title>
</head>
<body>
    <h1>Configurando Módulo Central</h1>

    <form action="/get">
        <h2>Automação 01</h2>

        <label for="autom01_conLDR">Conector do LDR:</label>
        <select name="autom01_conLDR" id="autom01_conLDR" required>
            <option value="">Selecione um conector...</option>
            <option value="C1">Conector 01</option>
            <option value="C2">Conector 02</option>
            <option value="C3">Conector 03</option>
            <option value="C4">Conector 04</option>
        </select>
        <br>
        <label for="autom01_conLED">Conector do LED:</label>
        <select name="autom01_conLED" id="autom01_conLED" required>
            <option value="">Selecione um conector...</option>
            <option value="C1">Conector 01</option>
            <option value="C2">Conector 02</option>
            <option value="C3">Conector 03</option>
            <option value="C4">Conector 04</option>
        </select>

        <h2>Automação 02</h2>

        <label for="autom02_conLDR">Conector do LDR:</label>
        <select name="autom02_conLDR" id="autom02_conLDR" required>
            <option value="">Selecione um conector...</option>
            <option value="C1">Conector 01</option>
            <option value="C2">Conector 02</option>
            <option value="C3">Conector 03</option>
            <option value="C4">Conector 04</option>
        </select>
        <br>
        <label for="autom02_conLED">Conector do LED:</label>
        <select name="autom02_conLED" id="autom02_conLED" required>
            <option value="">Selecione um conector...</option>
            <option value="C1">Conector 01</option>
            <option value="C2">Conector 02</option>
            <option value="C3">Conector 03</option>
            <option value="C4">Conector 04</option>
        </select>
        <br>
        <br>
        <input type="submit" value="Enviar">
    </form>
    
</body>
</html>
)rawliteral";

const char stop_central_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="pt-br">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Kit para Maquetes</title>
</head>
<body>
    <h1>Configurando Módulo Central</h1>
    
    <p>Interrompendo execução do <strong>Módulo Central</strong></p>
    
    <a href="/">
      <button>Voltar</button>
    </a>
</body>
</html>
)rawliteral";

const char success_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="pt-br">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Kit de Eletrônica para Maquetes</title>
</head>
<body>
    <h1>Configurando Módulo Central</h1>
    <p>Informações de conectores enviada para o Módulo Central!</p>
    <br>
    <a href="/">
      <button>Voltar</button>
    </a>
    <br>
    <a href="/stop">
      <button>Parar execução</button>
    </a>
</body>
</html>
)rawliteral";

/*
// Relacao entre os conectores do Modulo Central com GPIOs do ESP32 (temporariamente ate escolher todos os pinos como IOs analogicos)
const uint8_t CONECTOR_01 = 32;     // GPIO Analogico do ADC1
const uint8_t CONECTOR_02 = 33;     // GPIO Analogico do ADC1
const uint8_t CONECTOR_03 = 34;     // GPI Analogico do ADC1
const uint8_t CONECTOR_04 = 35;     // GPI Analogico do ADC1
*/
// Relacao entre os conectores do Modulo Central com GPIOs do ESP32 (temporariamente ate escolher todos os pinos como IOs analogicos)
const uint8_t CONECTOR_01 = 25;     // GPIO Analogico do ADC2
const uint8_t CONECTOR_02 = 26;     // GPIO Analogico do ADC2
const uint8_t CONECTOR_03 = 27;     // GPIO Analogico do ADC2
const uint8_t CONECTOR_04 = 14;     // GPIO Analogico do ADC2

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
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", index_html);
  });

  server.on("/stop", HTTP_GET, [](AsyncWebServerRequest *request){
    run = false;
    digitalWrite(pinLed1, LOW);
    digitalWrite(pinLed2, LOW);

    request->send_P(200, "text/html", stop_central_html);
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

    request->send(200, "text/html", success_html);

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
  });

  server.onNotFound(notFound);

  server.begin();
}

void enableWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

void disableWiFi() {
  WiFi.disconnect(true);
  WiFi.mode(WIFI_OFF);
  Serial.println("WiFi Disconnected");
}

void setup() {
  // Inicializando todos os pinos como entradas por seguranca
  pinMode(CONECTOR_01, INPUT);
  pinMode(CONECTOR_02, INPUT);
  pinMode(CONECTOR_03, INPUT);
  pinMode(CONECTOR_04, INPUT);

  Serial.begin(115200);   // Para fins de DEBUG

  enableWiFi();
  setWebserver();
}

void loop() {
  if (Serial.available() > 0) {
    int rx = Serial.read();
    Serial.print("rx: ");
    Serial.println(rx);

    if (rx == 1) {
      run = false;
      digitalWrite(pinLed1, LOW);
      digitalWrite(pinLed2, LOW);

      enableWiFi();
    } else if (rx == 0) {
      disableWiFi();
    }
  }

  if (run) {
    automacao_ldr_run(pinLdr1, pinLed1);
    automacao_ldr_run(pinLdr2, pinLed2);
  }
  delay(100);       // Provisorio
}