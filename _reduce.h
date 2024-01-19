#ifndef _REDUCE_H
#define _REDUCE_H

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
#include "input.h"

//класс Reduce будет отвечать за этап reduce
//Должен принимать в конструкторе список файлов с этапа reduce, а также пользовательский функтор для этапа reduce



class Reduce
{
    //typedef void(*Functor)(std::pair<std::string, std::string>, std::map<std::string, std::string> &);
    typedef std::function<void(std::pair<std::string, std::string>, std::map<std::string, std::string> &)> Functor;

    std::vector<std::thread> m_thead_pool;

    const std::string prefix="reduce_";
    Functor m_functor;


    long long find_end_of_string(std::ifstream& file, long long start);
    void work(std::vector<char>& buf, std::map<std::string, std::string>& map);
    std::list<std::string>    m_out_files;

public:
    Reduce(std::list<std::string> input_files, Functor f);

    void threadFunction(std::string out_file, std::shared_ptr<Stack> stack, std::shared_ptr<std::promise<bool>> promise);

     std::list<std::string> get_out_files() {return m_out_files;}
};

#endif // _REDUCE_H
