#ifndef INPUT_H
#define INPUT_H


#include <string>
#include <list>
#include <iostream>
#include <fstream>
#include <map>
#include <vector>
#include <memory>
#include <sstream>

#include <boost/filesystem.hpp>
#include <boost/algorithm/string.hpp>
#include <mutex>



//класс Stack будет знаниматься работой с одним входным файлом
//он сам будет следить за подгрузкой данных с диска в память

//Входные файлы должны содежать строки в формате ключ-значение, и быть лексиграфически отсортированными по ключам
//примечание : разделитель key и value символ табуляции
class Stack
{
    std::vector<char> m_buf;
    std::list<std::pair<std::string, std::string>> m_work_list;
    long long m_pointer=0;
    std::ifstream m_file;
    long long m_filesize;
    const int cash_block_size=1024*1024; //размер блока для кэширования данных, читаемых из файла
    void load_stack();
    std::string m_name;

public:
    Stack(std::string filename);
    ~Stack();
    long long find_end_of_string(std::ifstream& file, long long start); //поиск конца строки
    std::string get(); //получения ключа для операций сравнения
    std::pair<std::string, std::string> pop(); //забрать верхний элемент стэка
    bool empty();//проверка на пустоту
    long long size(); //количество байт в файле
    std::string name() {return m_name;}
};


#endif // INPUT_H
