#include <Wire.h>  //responsável pela comunicação i2c
#include <SSD1306.h> //responsável pela comunicação com o display

SSD1306 display(0x3c, 4, 15); //construtor do objeto que controlaremos o display

void oledInit() {
    pinMode(16, OUTPUT); //RST do oled
    pinMode(25, OUTPUT);

    digitalWrite(16, LOW);    // reseta o OLED
    delay(50);
    digitalWrite(16, HIGH); // enquanto o OLED estiver ligado, GPIO16 deve estar HIGH
    display.init(); //inicializa o display
    display.flipScreenVertically();
    display.setTextAlignment(TEXT_ALIGN_LEFT);

    display.setFont(ArialMT_Plain_10); //configura a fonte para um tamanho maior

}

void oledEscrever(int linha1, int linha2, String msg) {
    display.drawString(linha1, linha2, msg);
    display.display();

}

void oledLimpar() {
    display.clear();
}