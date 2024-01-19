#ifndef MAP_H
#define MAP_H

#include <string>
#include <list>
#include <iostream>
#include <fstream>
#include <map>
#include <set>
#include <boost/filesystem.hpp>
#include <sstream>
#include <thread>
#include <vector>
#include <functional>
#include <future>
#include <memory>
#include <algorithm>


//Этот класс будет отвечать за этап map
//Должен принимать в конструкторе имя файла, количество запускаемых рабочих потоков для этапа map,
//а также функтор применяемый к строкам для формирования выходных файлов
//Каждый запущенный поток читает свой кусок исходного файла и создает собственный выходной файл.
//Чтение данных из исходного файла осуществляем с кэшированием относительно большого блока файла.

//Пользовательский функтор должен сформировать выходной std::map<std::string, std::string>
//Выходной файл будет содежать строки в формате ключ-значение, который будет лексиграфически отсортирован по ключам

class Map
{
    //typedef void(*Functor)(std::string, std::map<std::string, std::string> &);
    typedef std::function<void(std::string, std::map<std::string, std::string> &)> Functor;
    std::string m_path;
    int m_thread_count;
    std::vector<std::thread> m_thead_pool;
    const int cash_block_size=1024*1024; //размер блока для кэширования данных, читаемых из файла
    const std::string prefix="D:\\mapreduce\\";
    Functor m_functor;
    std::list<std::string> m_out_files;
    long long m_bit;

    long long find_end_of_string(std::ifstream& file, long long start);
    void work(std::vector<char>& buf, std::map<std::string, std::string>& map);

public:
    Map(std::string path, int m, Functor f, std::list<std::string>& out_files);

    void threadFunction(std::string out_file, long long start_position, long long stop_position, std::shared_ptr<std::promise<bool>> promise);

    //void threadFunctionSmall(long long start_position, long long stop_poition);

};

#endif // MAP_H
