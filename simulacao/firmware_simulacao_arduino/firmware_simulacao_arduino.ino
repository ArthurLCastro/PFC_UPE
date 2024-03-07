/*
  Kit de Eletrônica para Maquetes de Arquitetura
  (Firmware adaptado para simulacao com ATmega328 ao inves de ESP32)
  
  Projeto 2EE - Microcontroladores - Poli/UPE
  Turma DS - Prof Andrea Maria

  Arthur Castro
  Daires Macedo
  Rodrigo Laurenio
*/

// Mapeamento entre os Conectores do Modulo Central com GPIOs do microcontrolador
// (prevemos que no projeto final sejam todos GPIOs analogicos para nao limitar o usuario durante as conexoes)
const uint8_t CONECTOR_01 = A0;     // GPIO Analogico
const uint8_t CONECTOR_02 = A1;     // GPIO Analogico
const uint8_t CONECTOR_03 = A2;     // GPIO Analogico
const uint8_t CONECTOR_04 = A3;     // GPIO Analogico

String conLdr1="", conLed1="", conLdr2="", conLed2="";
uint8_t pinLdr1, pinLed1, pinLdr2, pinLed2;
bool run = false;


void intrucoes(){
  Serial.println("Por Serial envie uma string do tipo \"conector_ldr1,conector_led1,conector_ldr2,conector_led2\"");
  Serial.println("Exemplo: Para um hardware onde existam \"Blocos Sensor de Luminosidade\" conectados aos \"Conectores\" 1 e 2 e \"Blocos Lâmpada\" conectados aos \"Conectores\" 3 e 4, envie: \"3,1,4,2\" ou \"4,1,3,2\" ou \"3,2,4,1\" ou \"4,2,3,1\"");
}

void automacao_ldr_config(uint8_t pin_ldr, uint8_t pin_led) {
  pinMode(pin_ldr, INPUT);
  pinMode(pin_led, OUTPUT);
}

void automacao_ldr_run(uint8_t pin_ldr, uint8_t pin_led) {
  uint8_t adc_value = analogRead(pin_ldr);

  if (adc_value < 151) {    // 151 foi o valor encontrado na metade da faixa lida pelo LDR do simulador (79 a 222)
    digitalWrite(pin_led, HIGH);
  } else {
    digitalWrite(pin_led, LOW);
  }
}

uint8_t converter_conectores_em_pinos(String substring) {
  if (substring.toInt() == 1) {
    return CONECTOR_01;
  } else if (substring.toInt() == 2) {
    return CONECTOR_02;
  } else if (substring.toInt() == 3) {
    return CONECTOR_03;
  } else if (substring.toInt() == 4) {
    return CONECTOR_04;
  } else {
    Serial.println("Valor de conector inválido! Reinicie o sistema");
    while(1){};
  }
}

void trata_configuracoes_do_usuario(){
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

  Serial.println("Definindo pinos como entradas ou saídas...");
  Serial.println("");

  automacao_ldr_config(pinLdr1, pinLed1);
  automacao_ldr_config(pinLdr2, pinLed2);

  run = true;

  intrucoes();
}

void setup() {
  // Inicializando todos os pinos como entradas por seguranca
  pinMode(CONECTOR_01, INPUT);
  pinMode(CONECTOR_02, INPUT);
  pinMode(CONECTOR_03, INPUT);
  pinMode(CONECTOR_04, INPUT);

  Serial.begin(115200);   // Para fins de DEBUG

  intrucoes();
}

void loop() {
  if (Serial.available() > 0){
    run = false;

    String rx = Serial.readString();
    Serial.println(rx);
    Serial.println("");

    conLdr1 = rx.substring(0,1);
    conLed1 = rx.substring(2,3);
    conLdr2 = rx.substring(4,5);
    conLed2 = rx.substring(6,7);

    Serial.print("conLdr1: ");
    Serial.println(conLdr1);
    Serial.print("conLed1: ");
    Serial.println(conLed1);
    Serial.print("conLdr2: ");
    Serial.println(conLdr2);
    Serial.print("conLed2: ");
    Serial.println(conLed2);
    Serial.println("");
    
    trata_configuracoes_do_usuario();
  }

  if (run) {
    automacao_ldr_run(pinLdr1, pinLed1);
    automacao_ldr_run(pinLdr2, pinLed2);
  }

  delay(100);       // Provisorio
}