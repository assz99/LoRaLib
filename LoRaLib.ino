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

  if (millis() - lastMillis >= period ){
    int num = random(9);
    getRTC();
    enviarLoRa(timestamp, "arCond", messages[num]);
    lastMillis = millis();
    }
}
