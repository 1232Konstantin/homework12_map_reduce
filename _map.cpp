#include "_map.h"


long long Map::find_end_of_string(std::ifstream& file, long long start)
{
    file.seekg(start);
    if (!file.failbit) std::cout<<"error seek\n";
    for(char c='=';c!='\n';file.get(c)) {}
    return file.tellg();
};



Map::Map(std::string path, int m, Functor f, std::list<std::string>& out_files) : m_path(path), m_thread_count(m), m_functor(f)
{
    std::cout<<"Stage map\n";
    //создание и запуск потоков в detach режиме
    m_thead_pool.reserve(m_thread_count);
    long long filesize;


    try {
        boost::filesystem::path path(m_path);
        filesize=boost::filesystem::file_size(path);
        m_bit=filesize/m_thread_count; //вычислили примерный размер куска исходного файла для обработки одним потоком

    }  catch (const boost::filesystem::filesystem_error& e) {
        std::cout<<"Error"<<e.what()<<std::endl;
    }

//    auto find_end_of_string=[this](std::ifstream& file, long long start)->long long
//    {
//        file.seekg(start);
//        if (!file.failbit) std::cout<<"error seek\n";
//        //while ((file.get()!='\n') || (file.get()!=EOF)) {}
//        //for(char c='=';c!='\n';file.get(c)) {std::cout<<"find:"<<c<<" "<<file.tellg()<<'\n';}
//        for(char c='=';c!='\n';file.get(c)) {}
//        return file.tellg();
//    };


    //подготовим исходные данные для запуска рабочих потоков
    std::map<std::string, std::pair<long long, long long>> start_map;

    auto count_edge=[&](long long& start)->std::pair<long long, long long>
    {
        std::ifstream file;
        file.open(m_path, std::ios::binary);
        if (!file)
        {
            std::cout<<"wrong file!\n";
            return std::make_pair(0,0);
        }
        else
        {
            long long start_pos=start;
            //std::cout<<"start:"<<start_pos<<'\n';
            long long end_pos=(start+m_bit<=filesize)? find_end_of_string(file, start+m_bit) : filesize;
            //std::cout<<"end:"<<end_pos<<'\n';
            start=end_pos;
            file.close();
            return std::make_pair(start_pos,end_pos);
        }
    };

    int count=0;
    long long pos=0;
    while (count<m_thread_count)
    {
        std::string s=prefix+"mapfile_"+std::to_string(count);
        m_out_files.push_back(s);
        start_map[s]= count_edge(pos);
        count++;

    }


    //запускаем рабочие потоки
    std::list<std::shared_ptr<std::promise<bool>>> promises;
    try {

        //test проверка деления исходного файла на части *******************************************
//                std::ifstream file;
//                file.open(m_path, std::ios::binary);
//                if (!file)
//                {
//                    std::cout<<"wrong file!\n";
//                }
//                else
//                {

//                    for (auto x: start_map)
//                    {
//                        std::cout<<x.first<<" "<<x.second.first<<" "<<x.second.second<<std::endl;
//                        file.seekg(x.second.first);
//                        std::string temp;
//                        std::getline(file,temp);
//                        //file>>temp;
//                        std::cout<<temp<<'\n';
//                    }
//                    file.close();
//                }
//        //test работа функции threadFunction в однопоточном режиме
//        for (auto x: start_map)
//        {
//            promises.emplace_back(new std::promise<bool>);
//            threadFunction(x.first, x.second.first, x.second.second, promises.back() );
//        }
      //end test ******************************************************************************************************************
            for (auto x: start_map)
            {
                promises.emplace_back(new std::promise<bool>);
                m_thead_pool.emplace_back([this](std::string out_file,
                                          long long start_position,
                                          long long stop_poition,
                                          std::shared_ptr<std::promise<bool>> promise)
                                          {this->threadFunction(out_file, start_position, stop_poition, promise);},
                                           x.first, //filename
                                           x.second.first, //start pos
                                           x.second.second, //stop pos
                                           promises.back()
                );
            }

            for(auto& x : m_thead_pool) x.detach(); //запускаем


            //встаем на ожидание результата или исключения
            for (auto x: promises)
            {
                auto future=x->get_future();
                future.wait();
            }

            std::cout<<"map end\n";
    }  catch (std::exception) {
        std::cout<<"Some error occured\n";
    }
    out_files=m_out_files;
}



void Map::work( std::vector<char>& buf, std::map<std::string, std::string>& map)
{
    //работа с куском файла подлежащим кэшированию и обработке

        std::stringstream ss(std::string(buf.data(), buf.size()));
        std::string str;
        auto functor=m_functor;
        while (std::getline(ss,str))
        {

            //str.erase(std::remove_if(str.begin(), str.end(), [](unsigned char c){return !std::isprint(c);}));
            //str.erase(std::remove_if(str.begin(), str.end(), [](unsigned char c){return (int(c)>=32);}));

            if (int(*(--str.end()))<32) str.pop_back();

            if (!str.empty()) functor(str, map);
            //std::cout<<"work->"<<str<<std::endl;
        }
}

void Map::threadFunction(std::string out_file, long long start_position, long long stop_position, std::shared_ptr<std::promise<bool>> promise )
{
    try {
        //std::cout<<"#"<<out_file<<" "<<start_position<<" " <<stop_position<<"\n";

        std::map<std::string, std::string> map;

        std::vector<char> cash_block(cash_block_size);
        long long cur_pos=start_position;

        auto count_last_position=[this, stop_position, &cash_block](long long pos)->long long
        {
            long long res;
            std::ifstream file;
            file.open(m_path, std::ios::binary);
            if (!file) throw std::exception(); //проблеммы с файлом
            if (pos+cash_block_size<stop_position) res= find_end_of_string(file, pos+cash_block_size);
            else res=stop_position;
            cash_block.resize(stop_position-pos);

            file.seekg(pos);
            if (!file.failbit) std::cout<<"error seek\n";
            file.read(cash_block.data(), cash_block.size());
            file.close();
            return res;
        };

        while (cur_pos<stop_position)
        {
            //std::cout<<"#"<<cur_pos<<'\n';
            cur_pos=count_last_position(cur_pos);
            work(cash_block, map);//обрабатываем свой участок исходного файла
        }


        //далее создаем выходной файл из std::map

        std::ofstream outfile;
        outfile.open(out_file);
        if (outfile.is_open())
        {
            std::set<std::string> set;
            for (auto x : map) set.insert(x.first);
            for (auto x: set)
            {
                std::string str1=x;
                std::string str2=map[x];
                outfile<<str1<<'\t'<<str2<<std::endl;
            }
            outfile.close();
        }
        else throw std::exception();

        //сообщаем, что поток завершил работу
        promise->set_value(true);
    }  catch (...)
    {
        std::exception_ptr ptr;
        promise->set_exception(ptr);
    }
}
