#ifndef _SHUFFLE_H
#define _SHUFFLE_H


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
#include "input.h"



////Этот класс будет знаниматься работой с одним входным файлом
////он сам будет следить за подгрузкой данных с диска в память
//class Stack
//{
//    std::vector<char> m_buf;
//    std::list<std::pair<std::string, std::string>> m_work_list;
//    long long m_pointer=0;
//    std::ifstream m_file;
//    long long m_filesize;
//    const int cash_block_size=1024*1024; //размер блока для кэширования данных, читаемых из файла
//    void load_stack();

//public:
//    Stack(std::string filename);
//    ~Stack();
//    long long find_end_of_string(std::ifstream& file, long long start); //поиск конца строки
//    std::string get(); //получения ключа для операций сравнения
//    std::pair<std::string, std::string> pop(); //забрать верхний элемент стэка
//    bool empty();//проверка на пустоту
//    long long size(); //количество байт в файле
//};


//Этот класс будет знаниматься работой с выходными файлами
//он сам будет следить за заполнением файлов и делением выходных данных между ними
class Out
{
    int m_count_reduce_thread;
    long long m_size_summary;
    std::ofstream m_file;
    long long m_current_size=0;
    long long m_filesize=0;
    std::list<std::string> m_out_files;
    int m_courent_file_num=0;
    std::string prefix="D:\\mapreduce\\shuffle_";

public:
    Out();
    ~Out();
    void settings(int r, long long size_summary);
    void save(std::pair<std::string, std::string> pair);
    std::list<std::string> get_out_files() {return m_out_files;}
    void close_file();

};



//класс Shuffle будет отвечать за этап shuffle
//Должен принимать в конструкторе список файлов с этапа map, а также количество потоков на этапе reduce
//Входные файлы должны содежать строки в формате ключ-значение, и быть лексиграфически отсортированными по ключам
//примечание : разделитель key и value символ табуляции
//Поскольку Shuffle работает с большими данными он не должен грузить входные файлы в память полностью
//Для простоты Shuffle работает в однопоточном режиме
class Shuffle
{
    std::list<std::shared_ptr<Stack>> m_stacks;

    std::shared_ptr<Stack> m_min; //стэк с наименьшим верхним значением ключа
    std::shared_ptr<Stack> m_next; //стэк со следущим наименьшим значением
    Out m_out; //это выход

    bool sort_stack(); //функция поиска  m_min и  m_next ->выдает false если работа кончилась
    void one_step(bool end); //обработка m_min и  m_next
    void work(); //весь алгоритм работы



public:
    Shuffle(std::list<std::string>& input_files, int r);

    std::list<std::string> get_out_files() {return m_out.get_out_files();}

};

#endif // _SHUFFLE_H
