String rx;
uint8_t pin_led, pin_but;

void setup() {
  Serial.begin(115200);
  Serial.println("Por Serial envie uma string do tipo \"numero_do_pino_1,numero_do_pino_2\" (onde cada numero tem dois digitos):");
  Serial.println("Exemplo: Para um hardware onde existam LEDs conectados aos pinos 19 e 22 e botoes conectados aos pinos 21 e 23, enviar \"19,21\" ou \"22,21\" ou \"19,23\" ou \"22,23\"");
}

void loop() {
  if (Serial.available() > 0){
    rx = Serial.readString();
    Serial.println(rx);

    pin_led = rx.substring(0,2).toInt();
    pin_but = rx.substring(3,5).toInt();

    Serial.print("PIN_LED: ");
    Serial.println(pin_led);
    Serial.print("PIN_BUT: ");
    Serial.println(pin_but);

    pinMode(pin_led, OUTPUT);
    pinMode(pin_but, INPUT);
  }

  if (digitalRead(pin_but)) {
    digitalWrite(pin_led, HIGH);
  } else {
    digitalWrite(pin_led, LOW);
  }

  delay(100);
}