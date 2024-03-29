/**
 * FreeRTOS
 */
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/timers.h"
#include <esp_system.h>
#include <time.h>
#include <sys/time.h>
/**
 * Libs
 */
#include <WiFi.h>
#include <Wire.h>
#include <SPI.h>
#include <RH_RF95.h>
#include <RHSoftwareSPI.h>
#include "Oled_api.h"
#include <ArduinoQueue.h>

struct recvMessage
{
  String from;
  String to;
  String projName;
  String timestamp;
  String message;
};

ArduinoQueue<String> filaLoRa(10);
ArduinoQueue<recvMessage> msgRecv(10);
timeval tv;
RHSoftwareSPI spi;
RH_RF95 rf95(18, 26);

int controleTimeStamp[10];
String controleMsg[10];
String mac;
String gateway = "b8:27:eb:8e:94:f2";
unsigned long lastMillis = 0;
// static String messages[] = {"abcd", "efgh", "mnop", "wxyz", "0123", "1234", "7645",
//                            "sfas", "5426"};

struct RecvData
{
  char *buf{};
  String from{};
  String to{};
  String projName{};
  unsigned long timestamp{};
  String recvMsg;
};

void setRTC(time_t timeStamp)
{
  Serial.println("AtualizandoRTC: " + String(timeStamp));
  tv.tv_sec = timeStamp;   // coloca na variavel o timeStamp
  settimeofday(&tv, NULL); // Inseri no RTC o time Stamp certo
}

time_t getRTC()
{
  time_t timestamp = time(NULL);
  return timestamp;
}

void controleRetirarLoRa(int _timeStamp)
{

  for (int i = 0; i < 11; i++)
  {
    if (controleTimeStamp[i] == _timeStamp)
    {
      // Serial.println("Retirando Msg: " + String(controleTimeStamp[i]) +  " no Controle");
      controleTimeStamp[i] = 0;
      controleMsg[i] = "";
      return;
    }
    if (i == 10)
    {
      Serial.println("TimeStamp nao esta gravado");
    }
  }
}

void controleGravarLoRa(unsigned long _timeStamp, String msg)
{

  for (int i = 0; i < 11; i++)
  {
    if (controleTimeStamp[i] == 0)
    {
      controleTimeStamp[i] = _timeStamp;
      controleMsg[i] = msg;
      // Serial.println("Gravando Msg: " + String(controleTimeStamp[i]) + " no Controle");
      return;
    }
    if (i == 10)
    {
      Serial.println("Vetor Controle lotado");
    }
  }
}

void sendLoRa(String msg)
{
  uint8_t data[RH_RF95_MAX_MESSAGE_LEN];
  memset(data, '\0', sizeof(data));
  for (int i = 0; i < msg.length(); i++)
  {
    data[i] = (uint8_t)msg[i];
  }
  rf95.send(data, sizeof(data));
  rf95.waitPacketSent();
  Serial.println("Enviou a mensagem: ");
  Serial.println(msg);
}

void sendConfirmation(String y)
{
  mac.toLowerCase();
  String msg1 = mac + "!" + gateway + "!" + "confirm" + "!" + y + "!OK";
  delay(400);
  Serial.println("Enviando Confirmacao");
  filaLoRa.enqueue(msg1);
}

void unQueueLoRa(void *pvParameters)
{
  while (!filaLoRa.isEmpty())
  {
    Serial.println("Tirando da fila");
    String msg1 = filaLoRa.dequeue();
    sendLoRa(msg1);
  }
  vTaskDelay(pdMS_TO_TICKS(1000));
}

void enviarLoRa(unsigned long timeStamp, String projName, String msg)
{
  mac.toLowerCase();
  msg = mac + "!" + gateway + "!" + projName + "!" + String(timeStamp) + "!" + msg;
  filaLoRa.enqueue(msg);
  controleGravarLoRa(timeStamp, msg);
}

void checarControle()
{
  for (;;)
  {
    time_t timestamp = getRTC();
    unsigned long controlTimeStamp = timestamp;
    for (int i = 0; i < 11; i++)
    {
      if (controlTimeStamp - controleTimeStamp[i] > 20)
      {
        Serial.println("Re-enviando Mensagem do control");
        enviarLoRa(controleTimeStamp[i], "confirm", controleMsg[i]);
      }
    }
  }
}

void recvDataFactory(char *buf, RecvData &data)
{
  data.buf = buf;
  data.from = String(strtok(buf, "!"));
  data.to = String(strtok(nullptr, "!"));
  data.projName = String(strtok(nullptr, "!"));
  data.timestamp = atol(strtok(nullptr, "!"));
  data.recvMsg = String(strtok(nullptr, ""));
}

void recv(void *pvParameters)
{
  if (rf95.available())
  {

    Serial.println("Recebeu uma mensagem");
    // Should be a message for us now
    uint8_t buf[RH_RF95_MAX_MESSAGE_LEN];
    uint8_t len = sizeof(buf);

    memset(buf, '\0', len);
    struct RecvData data;
    if (rf95.recv(buf, &len))
    {
      // Serial.println((char *)buf);
      recvDataFactory((char *)buf, data);
      if (data.to.equals(mac))
      {
        recvMessage receivedLoRa = {
            data.from,
            data.to,
            data.projName,
            String(data.timestamp),
            data.recvMsg};

        sendConfirmation(receivedLoRa.timestamp);

        if (data.projName == "time")
        {
          setRTC(data.timestamp);
          receivedLoRa.message = "";
        }
        if (data.projName == "confirm")
        {
          controleRetirarLoRa(data.timestamp);
          receivedLoRa.message = "";
        }
        if (receivedLoRa.message)
          msgRecv.enqueue(receivedLoRa);
      }
      else
      {
        Serial.println("Msg nao e para mim");
      }
    }
  }
  vTaskDelay(pdMS_TO_TICKS(500));
}

void loraConfig(const float &frequencia)
{
  Serial.begin(115200);
  SPI.begin(5, 19, 27, 18);
  spi.setPins(19, 27, 5);
  if (!rf95.init())
  {
    Serial.println("init failed");
    while (true)
      ;
  }
  Serial.println("init success");
  rf95.setFrequency(frequencia);
}

// void oledMenu() {
//   int frequencia = 915;
//   String gateway = "b8:27:eb:8e:94:f2";
//     oledEscrever(30, 1, String(frequencia) + "Mhz");
//     oledEscrever(1, 10, "From: " + mac);
//     //oledEscrever(1, 30, "Msg: " + messages[num]);
//     oledEscrever(1, 40, "To: " + gateway);
//     oledEscrever(1, 50, "Delay: " + String(period));
// }

void config(int frequencia)
{
  loraConfig(frequencia);
  mac = WiFi.macAddress();
  // oledInit();
  // oledMenu(); //inicializa menu (opcional)
  tv.tv_sec = 1646092800;  // coloca na variavel o timeStamp
  settimeofday(&tv, NULL); // Inseri no RTC o time Stamp certo
}

void enviarMsgLoRa(String tagProj, String msg)
{
  // Pega o timestamp para enviar para a funcao de envio
  time_t timestamp = getRTC();
  // Funcao de envio da mensagem lora. Parametros:
  // 1 = timestamp, 2 = tag projeto, 3 = mensagem
  enviarLoRa(timestamp, tagProj, msg);
}
