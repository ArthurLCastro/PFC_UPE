#include <WiFi.h>
#include <WiFiClient.h>
#include <WebServer.h>
#include <ESPmDNS.h>

#include "env.h"    // Arquivo externo criado com as definicoes de ENV_WIFI_SSID e ENV_WIFI_PASSWORD

#define ALGUM_PINO_DO_ADC2  4

const char* ssid = ENV_WIFI_SSID;
const char* password = ENV_WIFI_PASSWORD;

WebServer server(80);

void enableWiFi(){
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.println("");

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Conectado a ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

void disableWiFi() {
  WiFi.disconnect(true);
  WiFi.mode(WIFI_OFF);
  Serial.println("WiFi Desabilitado");
}

void handleRoot() {
  server.send(200, "text/plain", "Hello World");
}

void handleNotFound() {
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i = 0; i < server.args(); i++) {
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
}

void setup(void) {
  pinMode(ALGUM_PINO_DO_ADC2, INPUT);

  Serial.begin(115200);

  enableWiFi();

  server.on("/", handleRoot);

  server.onNotFound(handleNotFound);

  server.begin();
  Serial.println("Servidor HTTP inicializado");
  Serial.println("Através da Serial envie \"D\" para desabilitar o WiFi ou \"H\" para habilitar o WiFi e observe o comportamento da leitura do ADC2");
}

void loop(void) {
  if (Serial.available() > 0) {
    String rx = Serial.readString();
    rx.trim();

    Serial.print("rx: ");
    Serial.println(rx);

    if (rx == "H") {
      enableWiFi();
    } else if (rx == "D") {
      disableWiFi();
    }
  }

  Serial.println(analogRead(ALGUM_PINO_DO_ADC2));

  server.handleClient();

  delay(2);
}
