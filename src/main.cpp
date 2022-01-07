/**
 * 
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




void setup() {
  Wire.begin();
  Serial.begin(9600);
  
}






void loop() {
  
  
  
}