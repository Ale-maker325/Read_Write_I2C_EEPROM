/**
 * @file main.cpp
 * @author Alexander (deed30511@gmail.com)
 * @brief Библиотека для записи и чтения I2C ЕЕПРОМ (FRAM, 24C256, 24C512 ... и др.) 
 *   
 * @version 0.1
 * @date 2021-12-11
 * 
 * @copyright Copyright (c) 2021
 * 
 */



#include <Arduino.h>
#include <RWEEPROM.h>
#include <Wire.h>

//Создаем объект ЕЕПРОМ с адресом 0х50 - т.е. все три А0, А1, А2 подтянуты к земле (либо никуда не подключены)
My_EEPROM eeprom1(0x50);


const int PERIOD = 48;

class Records{
  public:
    double BUFFER [PERIOD];
    void recordBUFFER(double data);
};

void Records::recordBUFFER(double data)
{
  for (byte i = 0; i < PERIOD; i++)
  {
    BUFFER[i] = data;
  }
}

Records myRecord1;
Records myRecord2;
Records myRecord3;

//Указатели на класс
Records *ptr_Record1 = &myRecord1;
Records *ptr_Record2 = &myRecord2;
Records *ptr_Record3 = &myRecord3;

//Читаем из ЕЕПРОМ по адресу
void readEEPROM(uint16_t adress)
{
  Serial.println(F("Считываем массив из ЕЕПРОМ..."));
  Serial.print(F("Считали "));
  Serial.print(eeprom1.readObject(adress, ptr_Record1->BUFFER));
  Serial.println(F(" байт"));
}

//Пишем в ЕЕПРОМ по адресу
void writeEEPROM(uint16_t adress)
{
  Serial.println(F("Записываем массив в ЕЕПРОМ..."));
  Serial.print(F("Записали "));
  Serial.print(eeprom1.write_Object(adress, ptr_Record1->BUFFER));
  Serial.println(F(" байт"));
}

void printMassiv()
{
  Serial.println(F("Имеем следующий массив данных: "));
  for (byte i = 0; i < PERIOD; i++)
  {
    Serial.print(ptr_Record1->BUFFER[i]);
    Serial.print(" ");
  }
  Serial.println(" ");
}



void setup() {
  Wire.begin();
  Serial.begin(9600);
  delay(100);


  ptr_Record1->recordBUFFER(-12.22);
  
  printMassiv();
  
  writeEEPROM(700);

  readEEPROM(700);

  printMassiv();

  ptr_Record1->BUFFER[47] = 10.00;

  writeEEPROM(700);

  readEEPROM(700);

  printMassiv();

  ptr_Record1->BUFFER[46] = 10.00;

  ptr_Record1->BUFFER[47] = 11.00;

  writeEEPROM(700);

  readEEPROM(700);

  printMassiv();

}






void loop() {
  
  
  
}