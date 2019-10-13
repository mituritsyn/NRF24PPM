/*
  DIY Arduino based RC Transmitter
  by Mikhail Turicyn, MITuritsyn@yandex.ru
  Library: TMRh20/RF24, https://github.com/tmrh20/RF24/
*/

#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <Wire.h>

//RF24 radio(9, 10);   // nRF24L01 (CE, CSN)/
RF24 radio(10, 9);
#define t2 2         // arm switch

struct MyData {
  byte throttle;
  byte pitch;
  byte roll;
  byte yaw;
  byte arm;
  byte aux1;
  byte aux2;
  byte aux3;
  byte aux4;
  byte aux5;
  byte aux6;
  byte aux7;
};

MyData data;

void setup() {
  Serial.begin(9600);
  radio.begin();
  radio.setChannel(5);                                  // Указываем канал передачи данных (от 0 до 127), 5 - значит передача данных осуществляется на частоте 2,405 ГГц (на одном канале может быть только 1 приёмник и до 6 передатчиков)
  radio.setDataRate     (RF24_1MBPS);                   // Указываем скорость передачи данных (RF24_250KBPS, RF24_1MBPS, RF24_2MBPS), RF24_1MBPS - 1Мбит/сек
  radio.setPALevel      (RF24_PA_HIGH);                 // Указываем мощность передатчика (RF24_PA_MIN=-18dBm, RF24_PA_LOW=-12dBm, RF24_PA_HIGH=-6dBm, RF24_PA_MAX=0dBm)
  radio.openWritingPipe (0x1234567890LL);               // Открываем трубу с идентификатором 0x1234567890 для передачи данных (на одном канале может быть открыто до 6 разных труб, которые должны отличаться только последним байтом идентификатора)
  pinMode(t2, INPUT_PULLUP);
}
void loop() {
  data.throttle = map(analogRead(A0), 0, 1023, 0, 255);
  data.yaw = map(analogRead(A1), 0, 1023, 0, 255);
  data.roll = map(analogRead(A2), 0, 1023, 0, 255);
  data.pitch = map(analogRead(A3), 0, 1023, 0, 255);
  data.arm = digitalRead(t2);

  radio.write(&data, sizeof(MyData));
}
