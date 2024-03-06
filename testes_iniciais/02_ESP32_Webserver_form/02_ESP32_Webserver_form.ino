#include <Arduino.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>

#include "env.h"    // Arquivo externo criado com as definicoes de ENV_WIFI_SSID e ENV_WIFI_PASSWORD

AsyncWebServer server(80);

const char* ssid = ENV_WIFI_SSID;
const char* password = ENV_WIFI_PASSWORD;

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html><head>
  <title>Kit para Maquetes</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  </head><body>
  <h1>Configurando Modulo Central</h1>

  <form action="/get">
    <h2>Automacao 01</h2>
    Conector do LDR: <input type="text" name="autom01_conLDR">
    <br>
    Conector do LED: <input type="text" name="autom01_conLED">

    <h2>Automacao 02</h2>
    Conector do LDR: <input type="text" name="autom02_conLDR">
    <br>
    Conector do LED: <input type="text" name="autom02_conLED">
    <br>
    <br>
    <input type="submit" value="Enviar">
  </form>
</body></html>)rawliteral";

void notFound(AsyncWebServerRequest *request) {
  request->send(404, "text/plain", "Not found");
}

void setup() {
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  if (WiFi.waitForConnectResult() != WL_CONNECTED) {
    Serial.println("WiFi Failed!");
    return;
  }
  Serial.println();
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", index_html);
  });

  server.on("/get", HTTP_GET, [] (AsyncWebServerRequest *request) {
    String inputMessage = "";

    if (request->hasParam("autom01_conLDR")) {
      inputMessage += request->getParam("autom01_conLDR")->value();
    }
    if (request->hasParam("autom01_conLED")) {
      inputMessage += request->getParam("autom01_conLED")->value();
    }
    if (request->hasParam("autom02_conLDR")) {
      inputMessage += request->getParam("autom02_conLDR")->value();
    }
    if (request->hasParam("autom02_conLED")) {
      inputMessage += request->getParam("autom02_conLED")->value();
    }

    Serial.println(inputMessage);
    request->send(200, "text/html", "Solicitacao HTTP GET enviada ao ESP32 com conteudo de \"inputMessage\" igual a " + inputMessage + "<br><a href=\"/\">Voltar para a pagina inicial</a>");
  });
  server.onNotFound(notFound);
  server.begin();
}

void loop() { 
}