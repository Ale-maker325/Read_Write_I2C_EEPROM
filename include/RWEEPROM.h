#ifndef __EEPROM__
#define __EEPROM__

#include <Arduino.h>

class My_EEPROM {
    public:
        /**
         * @brief Конструктор с указанием адреса микросхемы
         * 
         * @param Addr - адрес микросхемы. Например, 0х50...
         */
        My_EEPROM(int Addr)
        {
            deviceAddr = Addr;
        };
        
        //Чтение одного байта из ЕЕПРОМ
        byte read_1BYTE_EEPROM_256(uint16_t EEPROM_Addr);
        //Запись одного байта в ЕЕПРОМ
        byte write_1BYTE_EEPROM_256(uint16_t EEPROM_Addr, byte DATA);
        //Запись байтов в виде страницы (странично) в ЕЕПРОМ
        void writeBites_24c32(uint16_t EEPROM_Addr, byte *BUFFER, uint16_t lengthBUFFER);
        //Чтение байтов в виде страницы (постранично) из ЕЕПРОМ
        void readBites_24c32(uint16_t EEPROM_Addr, byte *BUFFER, uint16_t lengthBUFFER);
        
        
        
        

        /**
         * @brief Шаблон функции для записи любого обьекта в ЕЕПРОМ. Обьект разбивается на байты и записывается
         *        в память, начиная с указанного адреса. Функция возвращает количество записанных байт. Запись
         *        осуществляется побайтно, поэтому дольше, так как после каждого байта делается пауза 10мс 
         *        согласно даташиту на 24с32. Зато метод не зависит от страниц ЕЕПРОМ и заполняет все её адреса
         *        последовательно.
         * 
         * @tparam T - тип параметра функции
         * @param ADDR - адрес, с которого мы будем писать в ЕЕПРОМ
         * @param value - объект, который мы будем писать в ЕЕПРОМ
         * @return n - возвращаемое количество байт, записанных в ЕЕПРОМ
         */
        template <class T>
        uint16_t write_Object(uint16_t ADDR, const T & value)
        {
            uint16_t n;
            const byte *p = (const byte*)(const void*)&value;
            for (n = 0; n < sizeof (T); n++)
            {
                write_1BYTE_EEPROM_256(ADDR++, *p++);
            }
            return n;

        }

        /**
         * @brief Шаблон для считывания любых данных из ЕЕПРОМ в нужном нам формате. Чтение осуществляется побайтно,
         *        поэтому дольше. Метод возвращает количество байт, которые были считаны из ЕЕПРОМ. Зато метод не зависит
         *        от страниц ЕЕПРОМ, и читает все адреса последовательно.
         * 
         * @tparam T - тип параметра функции
         * @param ADDR - адрес, с которого мы будем читать из ЕЕПРОМ
         * @param value - объект, который мы будем читать из ЕЕПРОМ
         * @return n - возвращаемое количество байт, записанных в ЕЕПРОМ 
         */
        template <class T>
        uint16_t readObject(uint16_t ADDR, const T & value)
        {
            uint16_t n;
            byte *p = (byte*)(void*)&value;
            for (n = 0; n < sizeof(value); n++)
            {
                *p++ = read_1BYTE_EEPROM_256(ADDR++);
            }
            return n;
        }


        /**
         * @brief Шаблон для записи любых данных в 24с32 ЕЕПРОМ. Данные разбиваются на байты
         * и при помощи метода writeBites_24c32 записываются постранично в ЕЕПРОМ. Метод быстрый очень,
         * так как пишет потоком. Может применяться не только к 24с32 (только необходимо отредакти-
         * ровать #define EEPROM__PAGE_SIZE, который настроен на ЕЕПРОМ с длиной страницы 32 байта).
         * Единственное что не учитывается конец ЕЕПРОМ и это необходимо контролировать.
         * 
         * @tparam T - тип данных, записываемый в ЕЕПРОМ
         * @param ADDR - адрес, с которого мы будем писать в ЕЕПРОМ
         * @param valu - объект, который мы будем писать в ЕЕПРОМ
         * @return uint16_t - возвращаемый результат: количество записаных байт
         */
        template <class T>
        uint16_t writePage(uint16_t ADDR, const T & value)
        {
            uint16_t n;
            //Создаем временный массив, в который поместим буфер записи
            byte mass[sizeof(T)];
            //Делаем указатель на начало объекта байтовый
            const byte *p = (const byte*)(const void*)&value;
            //Заполняем временный массив
            for(n = 0; n < sizeof(value); n++)
            {
                mass[n] = *p++;
            }
            //Пишем странично буфер в ЕЕПРОМ
            writeBites_24c32(ADDR, mass, sizeof(T));
            return n;
        }

        /**
         * @brief Шаблон для чтения любых данных из 24с32 ЕЕПРОМ в наш объект. Объект разбивается
         * на байты и при помощи метода readBites_24c32 считываются постранично из ЕЕПРОМ в него.
         * 
         * @tparam T тип данных, считываемый из ЕЕПРОМ
         * @param ADDR - адрес, с которого мы будем читать из ЕЕПРОМ
         * @param value - объект, который мы будем заполнять из ЕЕПРОМ
         * @return uint16_t -  возвращаемый результат: количество считанных байт
         */
        template <class T>
        uint16_t readPage(uint16_t ADDR, const T & value)
        {
            //создаём временный массив байтов размером value, куда будем считывать данные
            byte in[sizeof(value)];
            //считываем в этот массив байты страниц ЕЕПРОМ
            readBites_24c32(ADDR, in, sizeof(T));
            //преобразуем выходной объект в последовательность байтовых адресов
            byte *p = (byte*)(void*)&value;
            //записываем в выходной объект байты, считанные из ЕЕПРОМ
            uint16_t n;
            for (n = 0; n < sizeof(value); n++)
            {
                *p++ = in[n];
            }
            return n;
        }


    private:
        
        int deviceAddr; //переменная, хранящая адрес микросхемы ЕЕПРОМ
        void write_complit_Buffer(uint16_t  _address, byte *_data, uint16_t  _length);
        void write_PAGE_EEPROM(uint16_t  _address, byte *_data, uint16_t  _length);
        void read_PAGE_EEPROM(uint16_t EEPROM_Addr, byte *BUFFER, uint16_t lengthBUFFER);

};



#endif