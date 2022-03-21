 /**
 * FreeRTOS
 */
#include "app.h"

 

void setup() {
  Serial.begin(115200);
  Serial.println("Inicializando");
  config(915);
}

void loop() {
  recv();
  if(!receivedLoRa[0].equals("0")){
    Serial.println("Mensagem Recebida");
    Serial.println("Mensagem de: " + receivedLoRa[0]);
    Serial.println("Tipo de mensagem: " + receivedLoRa[2]);
    Serial.println("Mensagem: " + receivedLoRa[4]);
    receivedLoRa[0] = "0";
    sendConfirmation(receivedLoRa[0], receivedLoRa[3]);
    }
  if (millis() - lastMillis >= period ){
    int num = random(9);
    getRTC();
    enviarLoRa(timestamp, "arCond", messages[num]);
    lastMillis = millis();
    }
}
