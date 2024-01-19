
#include <boost/program_options.hpp>
#include "_map.h"
#include "_shuffle.h"
#include "_reduce.h"
#include <stdlib.h>

namespace po = boost::program_options;

using namespace std;
#define TEST 0


// Самодельный фреймворк на приципах map reduce и его применение для решения задачи
// поиска минимального префикса для большого набора строк


//генерация достаточно большого файла для тестирования фреймворка
void generate_test_file()
{
    std::ofstream outfile;
    std::string str1="dst";
    std::string str2="@puf";
    std::string str3="@ase";
    std::string str4="frt";
    outfile.open("D:\\mapreduce\\mapreduce.txt");
    if (outfile.is_open())
    {
        for (int i=0; i<20000; i++)
            for (int j=0; j<20000; j++)
                if (i%2)
                {
                    auto k=(j%2)? i : i+234;
                    outfile<<str1+to_string(k)+str2+"\n";
                }
                else
                {
                    if ((i+j)%2)
                    {
                        auto k=(i%2)? j : j+24;
                        outfile<<str4+to_string(k)+str3+"\n";
                    }
                    else
                        if (j%2)
                        {
                            auto k=(i%2)? j : j+234;
                            outfile<<str4+to_string(k)+str1+to_string(2*i)+str2+"\n";
                        }
                            else
                        {
                            auto k=(i%2)? j : j+2;
                            outfile<<str1+to_string(k+4)+str4+to_string(2*i)+str2+"\n";
                        }
                }

        outfile.close();
    }
}

//проверка сортировки файлов этапа шафл
void test_file(std::string filename)
{
    std::ifstream file;
    file.open(filename);
    std::string str1, str2;
    std::getline(file, str1);
    size_t i=0;
    while(std::getline(file, str2))
    {
        if (str2<str1)
        {
            cout<<str2<<" __ "<<str1<<std::endl;
            cout<<i<<"ERROR!!!!!!!!!\n";
            i++;
        }
        str1=str2;
    }
    cout<<"OK!!!!!!!!!!";

};


//пользовательский функтор для этапа мап
void map_func(std::string s, std::map<std::string, std::string> &m)
{
    m[s]=s;
}



//пользовательский функтор этапа редюсе
class ReduceFunctor
{
    std::string strOld;
    std::string prefix;
public:

void operator()(std::pair<std::string, std::string> s, std::map<std::string, std::string> &m)
{
    std::string str=s.first; //обрабатывемая строка

    auto find_prefix=[](std::string first, std::string second)->std::pair<std::string,std::string>
    {
        std::string::iterator it1=first.begin();
        std::string::iterator it2=second.begin();
        std::string result1, result2;
        result1+=*it1;
        result2+=*it2;
        while (*it1==*it2)
        {
            it1++;
            it2++;

            if (it1!=first.end()) result1+=*it1;
            if (it2!=second.end()) result2+=*it2;
            if ((it1==first.end()) || (it2==second.end())) break;
        }

        //cout<<"% "<<first<<" "<<second<<" "<<result1<<" "<<result2<<"\n";
        return std::make_pair(result1, result2);
    };

    if (strOld.empty()) //самое первое применение функции
    {
        std::string prefix;
        prefix+=*(str.begin()); //в этом случае префикс будет первой буквой строки
    }
    else
    {
        auto pair=find_prefix(strOld,str);
        if (prefix.length()<pair.first.length()) m[pair.first]=strOld;
        else  m[prefix]=strOld;
        prefix=pair.second;
    }
        strOld=str;

}

};


//пользователская функция поиска минимального количества символов однозначно идентифицирующих любую строку в исходном файле
size_t final_work(std::list<std::string> input_files)
{
     size_t res=0;

        //создаем рабочие стэки
        std::list<std::shared_ptr<Stack>> stacks;
        std::pair<std::string, std::string> temp;
        for (auto x: input_files)
        {
            //cout<<"f: "<<x<<'\n';
            stacks.emplace_back(new Stack(x));
        }
        for (auto x: stacks)
        {
            std::string tstr;
            bool need_concatenate_files=false;

            while (!x->empty())
            {
                temp=x->pop();
                if (need_concatenate_files) //склеиваем два редюсе файла c учетом необходимости пересчета ключей первого и последнего элемента в склейваемых файлах
                {
                    ReduceFunctor func;
                    std::map<std::string, std::string> m;
                    auto p1=std::make_pair(tstr, tstr);
                    auto p2=std::make_pair(temp.second, temp.second);
                    auto p3=std::make_pair<std::string, std::string>("a", "a");
                    func(p1,m);
                    func(p2,m);
                    func(p3,m);
                    for (auto y: m) res=(y.first.length()>res)? y.first.length() : res;
                    need_concatenate_files=false;
                }
                if (!x->empty())
                {
                    res=(temp.first.length()>res)? temp.first.length() : res;
                }
                else
                {
                    tstr=temp.second;
                    need_concatenate_files=true;
                }
            }
        }
    return res;
}


int main(int argc,char *argv[])
{
    cout<<"start";
    if (TEST) generate_test_file();


    try {
        std::string filename="D:\\mapreduce\\temp.txt";
        size_t m=8, r=4;

        po::options_description desc{"Options"};
        desc.add_options()
                ("help,h", "Show this screen")
                ("filename,f", po::value<std::string>()->default_value("temp.txt"), "data file")
                ("num_m,m", po::value<size_t>()->default_value(8), "map threads number")
                ("num_r,r", po::value<size_t>()->default_value(4), "reduce threads number");
        po::variables_map vm;
        po::store(parse_command_line(argc, argv, desc), vm);
        if (vm.count("help"))
            std::cout << desc << '\n';
        else
        {
            if (vm.count("filename"))
            {
                filename=vm["filename"].as<std::string>();

            }
            if (vm.count("num_m"))
            {
                m=vm["num_m"].as<std::size_t>();

            }
            if (vm.count("num_r"))
            {
                r=vm["num_r"].as<std::size_t>();

            }



            //Выполнение  этапов map-shuffle-reduce
            std::list<std::string> map_files;
            Map(filename, m, map_func, map_files);




            //         map_files={"D:\\mapreduce\\mapfile_0",
            //                    "D:\\mapreduce\\mapfile_1",
            //                    "D:\\mapreduce\\mapfile_2",
            //                    "D:\\mapreduce\\mapfile_3",
            //                    "D:\\mapreduce\\mapfile_4",
            //                    "D:\\mapreduce\\mapfile_5",
            //                    "D:\\mapreduce\\mapfile_6",
            //                    "D:\\mapreduce\\mapfile_7"
            //                   };

            Shuffle shuffle(map_files, r);
            auto shuffle_files=shuffle.get_out_files();

            //    test_file("D:\\mapreduce\\shuffle_0");
            //    test_file("D:\\mapreduce\\shuffle_1");
            //    test_file("D:\\mapreduce\\shuffle_2");
            //    test_file("D:\\mapreduce\\shuffle_3");


            //         std::list<std::string> shuffle_files{
            //                        "D:\\mapreduce\\shuffle_0",
            //                        "D:\\mapreduce\\shuffle_1",
            //                        "D:\\mapreduce\\shuffle_2",
            //                        "D:\\mapreduce\\shuffle_3"
            //                        };


            ReduceFunctor reduce_func;
            Reduce reduce(shuffle_files, reduce_func);

            //                  std::list<std::string> reduce_files{
            //                                 "D:\\mapreduce\\reduce_0",
            //                                 "D:\\mapreduce\\reduce_1",
            //                                 "D:\\mapreduce\\reduce_2",
            //                                 "D:\\mapreduce\\reduce_3"
            //                                 };

            //финальные процедуры поиска минимального количества символов однозначно идентифицирующих любую строку в исходном файле

            auto reduce_files=reduce.get_out_files();
            size_t result=final_work(reduce_files);
            std::cout<<"The minimum prefix lenght required to uniquely inentify a line in the source file is "<<result<<endl;

        }



    }
    catch (const std::exception &e) {
        std::cout << "EXCEPTION "<<e.what() << std::endl;
    }


    return 0;
}

