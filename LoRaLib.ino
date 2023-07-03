/**
 * FreeRTOS
 */
#include "app.h"

unsigned long period = 20000; // Variavel apenas para tempo de envio

void setup()
{
  Serial.begin(115200);
  Serial.println("Inicializando");
  period = random(15, 25) * 1000;
  // Configura a antena na frequencia selecionada
  config(915);
  // cria a task que cuida da recepcao de dados LoRa
  xTaskCreate(
      recv, "recv_LoRa", 4096, NULL, 1, NULL);
  // Cria a task para envio das mensagens
  xTaskCreate(
      unQueueLoRa, "envia_LoRa", 4096, NULL, 1, NULL);
}

void loop()
{
  if (!msgRecv.isEmpty())
  {
    recvMessage receivedMsgLoRa = msgRecv.dequeue();
    Serial.println("Mensagem Recebida");
    Serial.println("Mensagem de: " + receivedMsgLoRa.from);
    Serial.println("Tag Projeto: " + receivedMsgLoRa.projName);
    Serial.println("Mensagem: " + receivedMsgLoRa.message);
  }

  // Exemplo de envio de mensagem
  if (millis() - lastMillis >= period)
  {
    // Funcao de envio da mensagem lora. Parametros:
    // 1 = tag projeto, 2 = mensagem
    enviarMsgLoRa("arCond", "Hello World");
    lastMillis = millis();
  }
}
