

//--------------------- НАСТРОЙКИ ----------------------
#define CH_NUM 0x60   // номер канала (должен совпадать с передатчиком)


// УРОВЕНЬ МОЩНОСТИ ПЕРЕДАТЧИКА
// На выбор RF24_PA_MIN, RF24_PA_LOW, RF24_PA_HIGH, RF24_PA_MAX
#define SIG_POWER RF24_PA_MIN

// СКОРОСТЬ ОБМЕНА
#define SIG_SPEED RF24_1MBPS

//--------------------- БИБЛИОТЕКИ ----------------------
#include <Servo.h>
#include <SPI.h>
#include "nRF24L01.h"
#include "RF24.h"


RF24 radio(9, 10);   // "создать" модуль на пинах 9 и 10 для НАНО/УНО
//--------------------- БИБЛИОТЕКИ ----------------------

//--------------------- ПЕРЕМЕННЫЕ ----------------------
byte pipeNo;
byte address[][6] = {"1Node", "2Node", "3Node", "4Node", "5Node", "6Node"}; // возможные номера труб

int recieved_data[2];         // массив принятых данных
int telemetry[2];             // массив данных телеметрии (то что шлём на передатчик)
//--------------------- ПЕРЕМЕННЫЕ ----------------------
byte pin1 = 4;
byte pin2 = 5;
byte pin3 = 6;
byte pin4 = 7;
byte servo = 3;
Servo myservo;

void setup() {
  myservo.attach(servo);
  Serial.begin(9600);
  radioSetup();
  pinMode(pin1, OUTPUT); 
  pinMode(pin2, OUTPUT); 
  pinMode(pin3, OUTPUT);
  pinMode(pin4, OUTPUT);

  
}

void loop() {
  while (radio.available(&pipeNo)) {                                  // слушаем эфир
    radio.read( &recieved_data, sizeof(recieved_data));              // чиатем входящий сигнал
  
    // формируем пакет данных телеметрии (напряжение АКБ, скорость, температура...)
    telemetry[0] = 0;
    telemetry[1] = 0;

    radio.writeAckPayload(pipeNo, &telemetry, sizeof(telemetry));    // отправляем пакет телеметрии
  }
  if(recieved_data[0]<300){
    myservo.write(0);
  }
  else if(recieved_data[0]>700){
    myservo.write(180);
  }else{
    myservo.write(90);
  }
  delay(30);
}

void radioSetup() {         // настройка радио
  radio.begin();               // активировать модуль
  radio.setAutoAck(1);         // режим подтверждения приёма, 1 вкл 0 выкл
  radio.setRetries(0, 15);     // (время между попыткой достучаться, число попыток)
  radio.enableAckPayload();    // разрешить отсылку данных в ответ на входящий сигнал
  radio.setPayloadSize(32);    // размер пакета, байт
  radio.openReadingPipe(1, address[0]);   // хотим слушать трубу 0
  radio.setChannel(CH_NUM);               // выбираем канал (в котором нет шумов!)
  radio.setPALevel(SIG_POWER);            // уровень мощности передатчика
  radio.setDataRate(SIG_SPEED);           // скорость обмена
  // должна быть одинакова на приёмнике и передатчике!
  // при самой низкой скорости имеем самую высокую чувствительность и дальность!!

  radio.powerUp();         // начать работу
  radio.startListening();  // начинаем слушать эфир, мы приёмный модуль
}
