#include "input.h"



Stack::Stack(std::string filename) : m_name(filename)
{
    //узнаем и сохраняем размер файла
    boost::filesystem::path path(filename);
    m_filesize=boost::filesystem::file_size(path);
    //std::cout<<"@"<<filename<<" "<<m_filesize<<std::endl;

    //открываем файл и держим его открытым вплоть до срабатывания деструктора
    m_file.open(filename, std::ios::binary);
    if (!m_file) throw std::exception(); //проблеммы с файлом

    load_stack();

}

Stack::~Stack()
{
    m_file.close();
}


long long Stack::find_end_of_string(std::ifstream& file, long long start)
{
    file.seekg(start);
    if (!file.failbit) std::cout<<"error seek\n";
    for(char c='=';c!='\n';file.get(c)) {}
    return file.tellg();
};


void Stack::load_stack()
{
    //std::cout<<"load_stack\n";
    if (m_pointer>=m_filesize) return;

    long long res;
    if (m_pointer+cash_block_size<m_filesize) res= find_end_of_string(m_file, m_pointer+cash_block_size);
    else res=m_filesize;
    m_buf.resize(res-m_pointer);

    m_file.seekg(m_pointer);
    if (!m_file.failbit) std::cout<<"error seek\n";

    m_file.read(m_buf.data(), m_buf.size());

    //собственно разбор буфера с загрузкой в стэк
    std::stringstream ss(std::string(m_buf.data(), m_buf.size()));
    std::string str;
    while (std::getline(ss,str))
    {
        if (!str.empty())
        {
            if (int(*(--str.end()))<32) str.pop_back(); //удаляем непечатные символы в конце строки
            std::vector<std::string> vector;
            boost::algorithm::split(vector, str, [](char ch){return (ch=='\t');}); //табуляцию используем как служебный символ разделения key и value в строке на этапе map

            //std::cout<<"load "<<vector.front()<<" "<<vector.back()<<std::endl;
            m_work_list.emplace_back(vector.front(), vector.back());
        }

    }
    m_pointer=res;
}


std::string Stack::get()
{
    if (m_work_list.empty()) load_stack();
    if (!m_work_list.empty()) return m_work_list.front().first;
    else return "Stack empty\n";
}

std::pair<std::string, std::string> Stack::pop()
{
    if (m_work_list.empty()) load_stack();
    if (!m_work_list.empty())
    {
        auto res=m_work_list.front();
        m_work_list.pop_front();
        return res;
    }
    else return {"Stack empty\n","Stack empty\n"};
}

bool Stack::empty()
{
    if (m_work_list.empty()) load_stack();
    return m_work_list.empty();
}

long long Stack::size()
{
    return m_filesize;
}
