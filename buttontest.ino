



long var=1000;
long rav=1400;
long qwe=0;

#include <Arduino.h>
#include <LiquidCrystal.h>
#include <LCDKeypad.h>
LiquidCrystal lcd(8, 9, 4, 5, 6, 7);
// Выход генератора -- вывод 11 (PB3, MOSI)
#define FREQ_OUT_PIN       11


extern "C" void setup();
extern "C" void loop();

// Предварительные декларации функций.
uint32_t readFreq();
void setupFreq(uint32_t freq);

// define some values used by the panel and buttons
int lcd_key     = 0;
int adc_key_in  = 0;
#define btnRIGHT  0
#define btnUP     1
#define btnDOWN   2
#define btnLEFT   3
#define btnSELECT 4
#define btnNONE   5
 
// read the buttons
int read_LCD_buttons()
{
 adc_key_in = analogRead(0);      // read the value from the sensor
 // my buttons when read are centered at these valies: 0, 144, 329, 504, 741
 // we add approx 50 to those values and check to see if we are close
 if (adc_key_in > 1000) return btnNONE; // We make this the 1st option for speed reasons since it will be the most likely result
 if (adc_key_in < 50)   return btnRIGHT; 
 if (adc_key_in < 195)  return btnUP;
 if (adc_key_in < 380)  return btnDOWN;
 if (adc_key_in < 555)  return btnLEFT;
 if (adc_key_in < 790)  return btnSELECT;  
 return btnNONE;  // when all others fail, return this...
}
 

void setup()
{
  Serial.begin(115200);
  lcd.begin(16, 2);              // start the library
 lcd.setCursor(0,0);
 lcd.print("Push the buttons"); // print a simple message
}


void loop()
{
  
  lcd.setCursor(0,1);            // move to the begining of the second line
 lcd_key = read_LCD_buttons();  // read the buttons
 
 switch (lcd_key)               // depending on which button was pushed, we perform an action
 {
   case btnRIGHT:
     {
     lcd.print("RIGHT ");
     break;
     }
   case btnLEFT:
     {
     lcd.print("LEFT   ");
     break;
     }
   case btnUP:
     {
     lcd.print("UP    ");
     
     var=var+1000;
      
     break;
     }
   case btnDOWN:
     {
     lcd.print("DOWN  ");
     var=var-1000;
      
     break;
     }
   case btnSELECT:
     {
     lcd.print("SELECT");
     qwe=1;
     break;
     }
     case btnNONE:
     {
     lcd.print("NONE  ");
     break;
     }
 }
 
 

  
  uint32_t freq = var;
  


  // Установка частоты.
  setupFreq(freq);

}

// Чтение данных из последовательного порта
uint32_t readFreq()
{

}

static const uint16_t prescallers[] = {0, 1, 8, 32, 64, 128, 256, 1024};

// Установка частоты.
void setupFreq(uint32_t freq)
{
  freq=var;
  uint32_t ocr = 0;  // Временный буфер для хранения значения регистра OCR2A.
  uint8_t prescallerBits = 1;  // Биты предделителя.
  uint16_t prescaller = prescallers[prescallerBits]; // Значение предделителя на основе 'prescallerBits'.

  TIMSK2 = 0;   // Прерывания таймера не нужны -- отключаем.

  if (freq) {   // Если частота не '0'
      do        // Пытаемся подобрать значения регистра OCR2A и предделителя
                // для заданной частоты.
        {
          ocr = F_CPU / freq / 2 / prescaller - 1; // Вычисляем значение для регистра OCR2A.
          if (ocr < 256)  // Если значение верное (регистр 8-мибитный и
                          // всё, что меньше 256, допустимо).
            {
              break;  // Завершаем цикл.
            }
          prescallerBits += 1;  // Если значение рагистра OCR2A не допустимо (условие выше не выполняется),
                                // увеличиваем значение предделителя (понижаем частоту).
          prescaller = prescallers[prescallerBits]; // Новое значение предделителя.
        }
      while (prescallerBits < 8);  // Подбираем значение регистра и предделителя,
                                   // пока биты предделителя не выходят за допустимые пределы.
      if (ocr < 256)  // Если подобрано корректное значение регистра OCR2A.
        {
          digitalWrite(FREQ_OUT_PIN, LOW);  // Вывод 11 в '0'.
          pinMode(FREQ_OUT_PIN, OUTPUT);    // Вывод 11 как выход.
          // Устанавливаем для таймера 2 режим CTC.
          TCCR2A = (1 << COM2A0)        // Задаём смену уровня вывода 11 при совпадении
                                        // счётчика таймера и регистра OCR2A.
                      | (1 << WGM21);   // Режим CTC.
          OCR2A = ocr;   // Задаём предел счёта (регистр OCR2A).
          TCCR2B = prescallerBits; // Устанавливаем предделитель для таймера.

          freq = F_CPU / 2 / prescaller / (ocr + 1);  // Вычисляем полученную частоту
                                                      // (она может отличаться от заданной).

          Serial.print(F("freq: "));  // Сообщаем о частоте.
          Serial.println(freq);
          lcd.setCursor(9,1);            // move cursor to second line "1" and 9 spaces over
          lcd.print(freq);

        }
      else  // Если не смогли подобрать значение регистра OCR2A.
        {
          Serial.println(F("freq: error (wrong value)"));  // Сообщаем о неправильных данных.
        }
  } else {    // Если частота '0'
      TCCR2B = 0;   // Выключаем PWM на ноге 11.
      pinMode(FREQ_OUT_PIN, INPUT);     // Ногу 11 делаем входом.
      digitalWrite(FREQ_OUT_PIN, LOW);  // Отключаем поддтяжку на ноге 11 (на всякий случай).
      Serial.println(F("freq: off"));      // Сообщаем о выключении генератора.
  }
  
}
 
 
