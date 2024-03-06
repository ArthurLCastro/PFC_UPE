#include <ArduinoJson.h>

char json[] = "{\"automacoes\":[{\"nome\":\"Banheiro\",\"tipo\":\"Iluminação\",\"blocos\":[{\"tipo\":\"Sensor de Presença\",\"conector\":\"01\"},{\"tipo\":\"Lâmpada\",\"conector\":\"02\"}]},{\"nome\":\"Área Externa\",\"tipo\":\"Iluminação\",\"blocos\":[{\"tipo\":\"Sensor de Luminosidade\",\"conector\":\"03\"},{\"tipo\":\"Lâmpada\",\"conector\":\"04\"},{\"tipo\":\"Lâmpada\",\"conector\":\"05\"}]}";
JsonDocument doc;

void setup() {
  deserializeJson(doc, json);

  const char* automacoes = doc["automacoes"];
  const char* nome = doc["automacoes"][0]["nome"];
  const char* conector = doc["automacoes"][0]["blocos"][1]["conector"];

  Serial.begin(115200);
  
  Serial.print("automacoes: ");
  Serial.println(automacoes);
  Serial.print("nome: ");
  Serial.println(nome);
  Serial.print("conector: ");
  Serial.println(conector);
}

void loop() {
  // put your main code here, to run repeatedly:

}
