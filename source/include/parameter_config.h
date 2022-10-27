#pragma once
#include <string>
#include <vector>
using namespace std;

//单个参数的所有属性
class st_para_config
{
public:
    string para_name;
    string para_type;
    
    int para_value;     // int型value数据
    string para_string; // string型value数据

    int para_whe_have_range; //是否有范围（只对int型有效），0：没有，1：有
    int min_range;           //最小值
    int max_range;           //最大值

    bool whe_have; //是否已经存在（多个则以第一个为准）

    st_para_config()
    {
        para_name = "";
        para_type = "int";
        // para_whe_have_default = 1;
        // para_value_default = 0;
        // para_string_default = "";
        // para_whe_have_value = 1;
        para_value = 0;
        para_string = "";
        para_whe_have_range = 0;
        min_range = 0;
        min_range = 0;
        whe_have = false;
    }
};

/*para_need为返回的参数,所有参数的集合*/
class para_need_config
{
public:
    int para_num;
    st_para_config para[20];

    string note;      //注释标识符
    bool whe_equal;   //是否需要 = 作为赋值符
    bool whe_group;   //是否有配置组名[]
    string file_path; //配置文件路径

    /*一组设定函数，num*/
    void set_name_type(int num, string para_name = "", string para_type = "int");               //基础属性
    void set_default(int num, int para_value = 0, string para_string = "");                     //缺省设定
    void set_range(int num, int para_whe_have_range = 0, int min_range = 0, int max_range = 0); // int类型值范围设定
    string del_note(string line, string note);
    bool check_group(string line);
    vector<string> split_str(string str);
    int find_name_pos(string name);
    int parameter_process();
};
