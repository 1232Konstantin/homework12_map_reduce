#include "_reduce.h"

Reduce::Reduce(std::list<std::string> input_files, Functor f) : m_functor(f)
{
     std::cout<<"Stage reduce\n";
    try
    {
        //std::cout<<"Reduce"<<input_files.size()<<std::endl;
        //for (auto x: input_files) std::cout<<x<<std::endl;

        //создаем рабочие стэки
        std::list<std::shared_ptr<Stack>> stacks;
        for (auto x: input_files)
        {
            stacks.emplace_back(new Stack(x));
        }


        std::list<std::shared_ptr<std::promise<bool>>> promises;

        int count=0;
        for (auto x: stacks)
        {
            std::string out_f=prefix+std::to_string(count);
            m_out_files.push_back(out_f); //сохраняем перечень файлов  редюсе для финальной обработки
            promises.emplace_back(new std::promise<bool>);
            m_thead_pool.emplace_back([this](std::string out_file, std::shared_ptr<Stack> stack,
                                      std::shared_ptr<std::promise<bool>> promise)
                                      {this->threadFunction(out_file, stack, promise);},
                                       out_f, //filename
                                       x,
                                       promises.back()
            );
            count++;
        }



        for(auto& x : m_thead_pool) x.detach(); //запускаем


        //встаем на ожидание результата или исключения
        for (auto x: promises)
        {
            auto future=x->get_future();
            future.wait();
        }
        std::cout<<"reduce end\n";

     }  catch (std::exception) {
        std::cout<<"Reduce error\n";
    }


}

void Reduce::threadFunction(std::string out_file, std::shared_ptr<Stack> stack, std::shared_ptr<std::promise<bool>> promise )
{
    try {
        //std::cout<<"#reduce"<<out_file<<"\n";

        std::map<std::string, std::string> map;

        auto functor=m_functor;
        while (!stack->empty())
        {
            functor(stack->pop(), map);
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
               // outfile<<str1<<std::endl;
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
