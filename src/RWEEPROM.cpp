#include <Arduino.h>
#include <Wire.h>
#include <RWEEPROM.h>

//Размер страницы ЕЕПРОМ 24С32 согласно даташита
#define EEPROM__PAGE_SIZE         32

//Буфер чтения BUFFER_LENGTH, определенный в библиотеке Wire (32 байта)
//#define EEPROM__RD_BUFFER_SIZE    BUFFER_LENGTH

//Буфер чтения BUFFER_LENGTH, определенный в библиотеке Wire (128 байт)
#define EEPROM__RD_BUFFER_SIZE    I2C_BUFFER_LENGTH


//БуБуфер записи, определенный в библиотеке Wire ( -2 байта на адрес)
#define EEPROM__WR_BUFFER_SIZE    (I2C_BUFFER_LENGTH - 2)



/**
 * @brief Функция чтения из ЕЕПРОМ одного байта по указанному адресу
 * 
 * @param EEPROM_Addr - адрес ЕЕПРОМ, откуда мы будем считывать байт
 * @return byte - метод возвращает считанный из ЕЕПРОМ байт
 */

byte My_EEPROM::read_1BYTE_EEPROM_256(uint16_t EEPROM_Addr)
{
  //предварительно инициализируем значение считываемого байта единицами
  byte DATA = 0xFF;
  Wire.beginTransmission (deviceAddr);
  Wire.write((byte) (EEPROM_Addr & 0xFF00) >> 8);    // MSB
  Wire.write((byte) (EEPROM_Addr & 0x00FF));    // LSB
  Wire.endTransmission();
  Wire.requestFrom(deviceAddr, 1);
  if(Wire.available()) DATA = Wire.read();
  return DATA;
}

/**
 * @brief Функция записи в ЕЕПРОМ одного байта по указанному адресу
 * 
 * @param EEPROM_Addr  - адрес ЕЕПРОМ, куда мы будем писать байт
 * @param DATA - байт, который мы будем писать в ЕЕПРОМ
 * @return byte - метод возвращает байт, который был записан по 
 *                адрему в ЕЕПРОМ (для контроля при необходимости)
 */
byte My_EEPROM::write_1BYTE_EEPROM_256(uint16_t EEPROM_Addr, byte DATA)
{
  //минимизация перезаписывания EEPROM одними и теми же данными. Время на операции мы
  //конешно увеличим, но зато немножко сэкономится время жизни ЕЕПРОМ
  if(DATA == read_1BYTE_EEPROM_256(EEPROM_Addr)){
    return DATA;
  }
  else
  {
    //запись новых данных
    Wire.beginTransmission (deviceAddr);
    Wire.write((byte) (EEPROM_Addr & 0xFF00) >> 8);  // MSB
    Wire.write((byte) (EEPROM_Addr & 0x00FF));    // LSB
    Wire.write(DATA);
    
    Wire.endTransmission();
    delay(10); //Задержка 10мс после каждого цикла записи в соответствии с требованиями даташита
    return DATA;
  }
}


/**
 * @brief Метод записи массива байтов в EEPROM постранично. Размер страницы у
 * микросхемы 24С32 - 32 байта. Всего 256 страниц (32 килобайта с лихуем). Метод
 * пишет страницы по 30 байт, поэтому если длина записи больше, чем 30 байт,
 * данные разбиваются на несколько страниц и остаток дописывается
 * 
 * 
 * @param EEPROM_Addr -   - адрес ЕЕПРОМ, с которого мы будем писать серию байтов
 * @param BUFFER - массив байтов, из которого мы будем брать байты для записи в ЕЕПРОМ
 * @param lengthBUFFER  - длина массива байтов, который мы будем заполнять
 */
void My_EEPROM::writeBites_24c32(uint16_t EEPROM_Addr, byte *BUFFER, uint16_t lengthBUFFER)
{
  uint16_t notAlignedLength = 0;//изначально предполагаем, что страница выровнена и запись будет вестись с начала страницы
  uint16_t pageOffset = EEPROM_Addr % EEPROM__PAGE_SIZE; //Определяем, будет ли смещение адреса начала записи страницы
    
    if (pageOffset > 0)//Если начало записи не входит в длину страницы...
    {
        notAlignedLength = EEPROM__PAGE_SIZE - pageOffset; //определяем длину куска, который нужно писать до конца страницы
        write_PAGE_EEPROM(EEPROM_Addr, BUFFER, notAlignedLength); //пишем этот кусок
        lengthBUFFER -= notAlignedLength; //определяем остаточную длину записи
    }

    if (lengthBUFFER > 0)
    {
        EEPROM_Addr += notAlignedLength;
        BUFFER += notAlignedLength;

        // Write complete and aligned pages.
        byte pageCount = lengthBUFFER / EEPROM__PAGE_SIZE;
        for (byte i = 0; i < pageCount; i++)
        {
            write_PAGE_EEPROM(EEPROM_Addr, BUFFER, EEPROM__PAGE_SIZE);
            EEPROM_Addr += EEPROM__PAGE_SIZE;
            BUFFER += EEPROM__PAGE_SIZE;
            lengthBUFFER -= EEPROM__PAGE_SIZE;
        }

        if (lengthBUFFER > 0)
        {
            // Write remaining uncomplete page.
            write_PAGE_EEPROM(EEPROM_Addr, BUFFER, EEPROM__PAGE_SIZE);
        }
    }
}

/**
 * @brief Метод чтения из ЕЕПРОМ байтов постранично в массив байтов.
 * 
 * @param EEPROM_Addr - начальный адрес данных
 * @param BUFFER - байтовый буфер
 * @param lengthBUFFER - длина буфера
 */
void My_EEPROM::readBites_24c32(uint16_t EEPROM_Addr, byte *BUFFER, uint16_t lengthBUFFER)
{
    uint16_t bufferCount = lengthBUFFER / EEPROM__RD_BUFFER_SIZE;
    for (byte i = 0; i < bufferCount; i++)
    {
        word offset = i * EEPROM__RD_BUFFER_SIZE;
        read_PAGE_EEPROM(EEPROM_Addr + offset, BUFFER + offset, EEPROM__RD_BUFFER_SIZE);
    }

    byte remainingBytes = lengthBUFFER % EEPROM__RD_BUFFER_SIZE;
    word offset = lengthBUFFER - remainingBytes;
    read_PAGE_EEPROM(EEPROM_Addr + offset, BUFFER + offset, remainingBytes);
}





/****************************************** ПРИВАТНЫЕ МЕТОДЫ ***************************************************/






/**
 * @brief Метод записи массива байтов в EEPROM. 
 * 
 * 
 * @param _address -   - адрес ЕЕПРОМ, с которого мы будем писать серию байтов
 * @param _data - массив байтов, из которого мы будем брать байты для записи в ЕЕПРОМ
 * @param _length  - длина массива байтов, который мы будем заполнять
 */
void My_EEPROM::write_PAGE_EEPROM(uint16_t _address, byte *_data, uint16_t _length)
{
  //определяем количество целых страниц, которые необходимо записать в ЕЕПРОМ
  byte bufferCount = _length / EEPROM__WR_BUFFER_SIZE;
  //начинаем записывать эти страницы одна за другой
  for (byte i = 0; i < bufferCount; i++)
  {
    //определяем смещения страниц
    byte offset = i * EEPROM__WR_BUFFER_SIZE;
    //пишем по 30 байт на каждое смещение
    write_complit_Buffer(_address + offset, _data + offset, EEPROM__WR_BUFFER_SIZE);
  }
  //Дописываем оставшиеся остатки байтов, не вошедшие в целый буфер
  byte remainingBytes = _length % EEPROM__WR_BUFFER_SIZE;
  byte offset = _length - remainingBytes;
  write_complit_Buffer(_address + offset, _data + offset, remainingBytes);
  
}

/**
 * @brief Метод записи в ЕЕПРОМ данных, размером в буфер АЙТУСИ, т.е. 30 байт не более. Метод внутренний,
 * определенный как 'private'. Данные пишутся побайтно!!! Следовательно перед записью данных, не соот-
 * ветствующих байтовому формату, необходимо их преобразовывать в байты!!
 * 
 * @param _address - адрес, с которого пишем
 * @param _data - данные, которые пишем (не более 30 байт)
 * @param _length - длина буфера
 */
void My_EEPROM:: write_complit_Buffer(uint16_t  _address, byte *_data, uint16_t  _length)
{
    Wire.beginTransmission(deviceAddr);
    Wire.write((byte)((_address & 0xFF00) >> 8)); // MSB
    Wire.write((byte)(_address & 0x00FF)); // LSB
    for (byte i = 0; i < _length; i++)
    {
        Wire.write(_data[i]);
    }
    Wire.endTransmission();
    //Задержка после окончания записи в ЕЕПРОМ согласно даташиту
    delay(10);
}



/**
 * @brief Метод считывания байтов из ЕЕПРОМ постранично. Возможно, 
 * что читать более 32 байт за раз не стоит, так как библиотека Wire 
 * имеет буфер размером 32 байта.
 * 
 * @param EEPROM_Addr  - адрес ЕЕПРОМ, с которого мы будем считывать серию байтов
 * @param BUFFER - массив байтов, в который мы считаем серию байтов
 * @param lengthBUFFER - длина массива байтов, который мы будем заполнять
 */
void My_EEPROM::read_PAGE_EEPROM(uint16_t EEPROM_Addr, byte *BUFFER, uint16_t lengthBUFFER)
{
  Wire.beginTransmission(deviceAddr);
  Wire.write((byte)(EEPROM_Addr & 0xFF00) >> 8); // MSB
  Wire.write((byte)(EEPROM_Addr & 0x00FF)); // LSB
  Wire.endTransmission();
  Wire.requestFrom(deviceAddr, lengthBUFFER);
  for (byte i = 0; i < lengthBUFFER; i++)
  {
      if (Wire.available()) BUFFER[i] = Wire.read();
  }
}