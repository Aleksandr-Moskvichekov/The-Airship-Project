Скетч Пульта
//--------------------- НАСТРОЙКИ ----------------------
#define CH_NUM 0x60   // номер канала (должен совпадать с приёмником)
//--------------------- НАСТРОЙКИ ----------------------

//--------------------- ДЛЯ РАЗРАБОТЧИКОВ -----------------------
// УРОВЕНЬ МОЩНОСТИ ПЕРЕДАТЧИКА
// На выбор RF24_PA_MIN, RF24_PA_LOW, RF24_PA_HIGH, RF24_PA_MAX
#define SIG_POWER RF24_PA_LOW

// СКОРОСТЬ ОБМЕНА
// На выбор RF24_2MBPS, RF24_1MBPS, RF24_250KBPS
// должна быть одинакова на приёмнике и передатчике!
// при самой низкой скорости имеем самую высокую чувствительность и дальность!!
// ВНИМАНИЕ!!! enableAckPayload НЕ РАБОТАЕТ НА СКОРОСТИ 250 kbps!
#define SIG_SPEED RF24_1MBPS
//--------------------- ДЛЯ РАЗРАБОТЧИКОВ -----------------------

//--------------------- БИБЛИОТЕКИ ----------------------
#include <SPI.h>
#include "nRF24L01.h"
#include "RF24.h"
RF24 radio(9, 10); // "создать" модуль на пинах 9 и 10 Для Уно

#include <SPI.h>
#include <Wire.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_GFX.h>

#define SCREEN_WIDHT 128
#define SCREEN_HEIGHT 32

#define OLED_RESET -1
#define SCREEN_ADDRESS 0x3C

Adafruit_SSD1306 display(SCREEN_WIDHT, SCREEN_HEIGHT, &Wire, OLED_RESET);


//--------------------- БИБЛИОТЕКИ ----------------------

//--------------------- ПЕРЕМЕННЫЕ ----------------------
byte address[][6] = {"1Node", "2Node", "3Node", "4Node", "5Node", "6Node"}; // возможные номера труб

int transmit_data[2];          // массив пересылаемых данных
int telemetry[2];              // массив принятых от приёмника данных телеметрии
byte latest_data[2];
byte rssi;
int trnsmtd_pack = 1, failed_pack;
unsigned long RSSI_timer;
//--------------------- ПЕРЕМЕННЫЕ ----------------------

#define JpinX A2
#define JpinY A1

void setup() {
  
  Serial.begin(9600); // открываем порт для связи с ПК
  radioSetup();
  pinMode(JpinX,INPUT);
  pinMode(JpinY,INPUT);

   if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); 
  }
 display.display();
 delay(2000);
 display.clearDisplay();

}

void loop() {
  // забиваем transmit_data данными, для примера
  
  transmit_data[0] = analogRead(JpinX);
  transmit_data[1] = analogRead(JpinY);
  
  if (radio.write(&transmit_data, sizeof(transmit_data))) {    // отправка пакета transmit_data
    trnsmtd_pack++;
    if (!radio.available()) {                                  // если получаем пустой ответ
    } else {
      while (radio.available() ) {                    // если в ответе что-то есть
        radio.read(&telemetry, sizeof(telemetry));    // читаем
        // получили забитый данными массив telemetry ответа от приёмника
      }
    }
  } else {
    failed_pack++;
  }
  

  if (millis() - RSSI_timer > 1000) {    // таймер RSSI
    
    // расчёт качества связи (0 - 100%) на основе числа ошибок и числа успешных передач
    rssi = (1 - ((float)failed_pack / trnsmtd_pack)) * 100;

    // сбросить значения
    failed_pack = 0;
    trnsmtd_pack = 0;
    RSSI_timer = millis();
  }
  display.clearDisplay();
  
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(8,0);
  display.println("DATA1:");
  display.setCursor(50,0);
  display.println(telemetry[0]);
  display.setCursor(8,8);
  display.println("DATA2:");
  display.setCursor(50,8);
  display.println(telemetry[1]);
  display.setCursor(50,16);
  display.println(rssi);

  display.display();
  delay(20);
  
}

void radioSetup() {
  radio.begin();              // активировать модуль
  radio.setAutoAck(1);        // режим подтверждения приёма, 1 вкл 0 выкл
  radio.setRetries(0, 15);    // (время между попыткой достучаться, число попыток)
  radio.enableAckPayload();   // разрешить отсылку данных в ответ на входящий сигнал
  radio.setPayloadSize(32);   // размер пакета, в байтах
  radio.openWritingPipe(address[0]);   // мы - труба 0, открываем канал для передачи данных
  radio.setChannel(CH_NUM);            // выбираем канал (в котором нет шумов!)
  radio.setPALevel(SIG_POWER);         // уровень мощности передатчика
  radio.setDataRate(SIG_SPEED);        // скорость обмена
  // должна быть одинакова на приёмнике и передатчике!
  // при самой низкой скорости имеем самую высокую чувствительность и дальность!!

  radio.powerUp();         // начать работу
  radio.stopListening();   // не слушаем радиоэфир, мы передатчик
}
