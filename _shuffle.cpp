#include "_shuffle.h"


Shuffle::Shuffle(std::list<std::string>& input_files, int r)
{
     std::cout<<"Stage shuffle\n";
    try
    {
        //создаем рабочие стэки
        for (auto x: input_files)
        {
            m_stacks.emplace_back(std::make_shared<Stack>(x));
        }

        //рассчитываем общий размер файлов для дальнейшего деления по выходным файлам
        long long size=0;

        for (auto x: m_stacks)
        {
            //std::cout<<"file: "<<x->name()<<" "<<x->size()<<'\n';
            size+=x->size();
        }
        m_out.settings(r, size);
        //std::cout<<"summarn razmer"<<size<<std::endl;
        work();
        std::cout<<"End shuffle\n";

    }  catch (std::exception) {
        std::cout<<"Shuffle error\n";
    }

}

bool Shuffle::sort_stack()
{
    if (m_stacks.size()==1)
    {
        m_min=m_stacks.front();
        return false;
    }

    if (m_stacks.front()->get()<=m_stacks.back()->get())
    {
        m_min=m_stacks.front();
        m_next=m_stacks.back();
    }
    else
    {
        m_next=m_stacks.front();
        m_min=m_stacks.back();
    }
    if (m_stacks.size()==2) return true;
        for (auto x: m_stacks)
        {
            if (x->get()<m_min->get()) m_min=x;
        }
        for (auto x: m_stacks)
        {
            if ((x->get()<m_next->get())&&(x!=m_min)) m_next=x;
        }

    //std::cout<<"sort "<<m_min->name()<<" "<<m_next->name()<<'\n';
    return true;
}


void Shuffle::one_step(bool end)
{
    //std::cout<<"onestep\n";
    if (!end) //минимум два файла в обработке
    {
        while (m_min->get()<=m_next->get())
        {
            m_out.save(m_min->pop());
            if (m_min->empty())
            {
                m_stacks.remove(m_min);
                break;
            }
        }
    }
    else //остался последний необработанный файл, просто все из него пуляем на выход
    {
        //std::cout<<"last!!!!";
        while (!m_min->empty()) m_out.save(m_min->pop());
        m_stacks.clear();
        m_out.close_file();
    }
}

void Shuffle::work()
{
   //std::cout<<"work start\n";
   while (sort_stack())
   {
       one_step(false);
   }
   one_step(true);
}


//***************************************************************************************************
Out::Out()
{
    auto out_file=prefix+std::to_string(m_courent_file_num);
    m_out_files.push_back(out_file);
    m_file.open(out_file);
    if (!m_file.is_open()) std::cout<<"Error open OUT file\n";
}

Out::~Out()
{

}


void Out::settings(int r, long long size_summary)
{
    m_size_summary=size_summary;
    m_count_reduce_thread=r;
    m_filesize=size_summary/r;

}


void Out::save(std::pair<std::string, std::string> pair)
{
    //std::cout<<"save->"<<pair.first<<" $ "<<pair.second<<'\n';
    //std::cout<<"$$$"<<m_keeper.get()<<" "<<m_current_size<<" "<<m_filesize<<" "<<m_count_reduce_thread<<'\n';
    if (m_current_size<m_filesize)
    {
        m_file<<pair.first<<'\t'<<pair.second<<std::endl;
        m_current_size+=pair.first.size();
        m_current_size+=pair.second.size();
    }
    else
    {
        m_file.close();
        auto out_file=prefix+std::to_string(++m_courent_file_num);
        m_out_files.push_back(out_file);
        m_file.open(out_file);
        if (!m_file.is_open()) std::cout<<"Error open OUT file\n";
        m_current_size=0;
    }
}

void Out::close_file()
{
    m_file.close();
}
