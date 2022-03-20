

// ЭТО СКЕТЧ ПРИЁМНИКА!!!

//--------------------- НАСТРОЙКИ ----------------------
#define CH_NUM 0x60   // номер канала (должен совпадать с передатчиком)


// УРОВЕНЬ МОЩНОСТИ ПЕРЕДАТЧИКА
// На выбор RF24_PA_MIN, RF24_PA_LOW, RF24_PA_HIGH, RF24_PA_MAX
#define SIG_POWER RF24_PA_MIN

// СКОРОСТЬ ОБМЕНА
// На выбор RF24_2MBPS, RF24_1MBPS, RF24_250KBPS
// должна быть одинакова на приёмнике и передатчике!
// при самой низкой скорости имеем самую высокую чувствительность и дальность!!
// ВНИМАНИЕ!!! enableAckPayload НЕ РАБОТАЕТ НА СКОРОСТИ 250 kbps!
#define SIG_SPEED RF24_1MBPS

//--------------------- БИБЛИОТЕКИ ----------------------
#include <Servo.h>
#include <SPI.h>
#include "nRF24L01.h"
#include "RF24.h"


RF24 radio(9, 10);   // "создать" модуль на пинах 9 и 10 для НАНО/УНО
//RF24 radio(9, 53); // для МЕГИ
//--------------------- БИБЛИОТЕКИ ----------------------

//--------------------- ПЕРЕМЕННЫЕ ----------------------
byte pipeNo;
byte address[][6] = {"1Node", "2Node", "3Node", "4Node", "5Node", "6Node"}; // возможные номера труб

int recieved_data[4];         // массив принятых данных
int last_recieved_data[4];     // прошлые принятые данные
boolean flag;                 
int telemetry[2];             // массив данных телеметрии (то что шлём на передатчик)

unsigned long Servo_time = 0;
//--------------------- ПЕРЕМЕННЫЕ ----------------------
byte pin1 = 4;
byte pin2 = 5;
byte pin3 = 6;
byte pin4 = 7;
byte servo = 3;
byte servo2 = 9;

Servo myservo1;
Servo myservo2;

void setup() {
  myservo1.attach(servo);
  myservo2.attach(servo2);

  myservo1.write(90);
  myservo2.write(90);
  Serial.begin(9600);
  radioSetup();
  pinMode(pin1, OUTPUT); 
  pinMode(pin2, OUTPUT); 
  pinMode(pin3, OUTPUT);
  pinMode(pin4, OUTPUT);

  

  digitalWrite(pin1,0);
  digitalWrite(pin2,0);
  digitalWrite(pin3,0);
  digitalWrite(pin4,0);
}

void loop() {
  while (radio.available(&pipeNo)) {                                  // слушаем эфир
    radio.read( &recieved_data, sizeof(recieved_data));              // чиатем входящий сигнал
  
    // формируем пакет данных телеметрии (напряжение АКБ, скорость, температура...)
    telemetry[0] = 0;
    telemetry[1] = 0;

    radio.writeAckPayload(pipeNo, &telemetry, sizeof(telemetry));    // отправляем пакет телеметрии
  }
if(millis() - Servo_time > 200){                                         // каждые 200 мс меняем положение сервопривода если есть изменение в полученных данных       
  for (int i = 0; i < 2; i++) { // в цикле от 0 до числа каналов         // все остальное время сервопривод отключен
    if (recieved_data[i] != last_recieved_data[i]) { // если есть изменения в recived_data
      flag = 1; 
      last_recieved_data[i] = recieved_data[i]; // запомнить последнее изменение
    }
  }
  
  Serial.println(recieved_data[2]);
  Serial.println(recieved_data[3]);
  if(flag == true){
  myservo1.attach(servo);
  myservo2.attach(servo2);
  myservo1.write(recieved_data[0]);
  myservo2.write(recieved_data[1]);
  flag = 0;
  if(recieved_data[2]==0 and recieved_data[3]==1){
    digitalWrite(pin1,1);
    digitalWrite(pin2,0);
    digitalWrite(pin3,1);
    digitalWrite(pin4,0);                                            
  }
  else if(recieved_data[2]==1 and recieved_data[3]==0){
    digitalWrite(pin1,0);
    digitalWrite(pin2,1);
    digitalWrite(pin3,0);
    digitalWrite(pin4,1);  
  }
  else if(recieved_data[2]==1 and recieved_data[3]==1){
    digitalWrite(pin1,0);
    digitalWrite(pin2,0);
    digitalWrite(pin3,0);
    digitalWrite(pin4,0);  
  }
  }
  else if(flag == false){
    myservo1.detach();
    myservo2.detach();
   
  }
  Servo_time = millis();
}
 
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
