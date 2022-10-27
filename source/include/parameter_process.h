#pragma once
#include <string>
using namespace std;

//单个参数的所有属性
class st_para{
public:
    string para_name;
    string para_type;

    int para_whe_have_default;//是否必须，0：不必须(有default)，1：必须
    int para_value_default;//int bool型value数据(缺省)
    string para_string_default;//string型value数据(缺省)

    int para_whe_have_value;//是否有值，0：没有，1：有
    int para_value;//int bool型value数据, 对于无值参数，是否存在，0：不存在，1：存在
    string para_string;//string型value数据

    int para_whe_have_range;//是否有范围（只对int型有效），0：没有，1：有
    int min_range;//最小值
    int max_range;//最大值

    st_para(){
        para_name="";
        para_type="int";
        para_whe_have_default=1;
        para_value_default=0;
        para_string_default="";
        para_whe_have_value=1;
        para_value=0;
        para_string="";
        para_whe_have_range=0;
        min_range=0;
        min_range=0;
    }
};

/*para_need为返回的参数,所有参数的集合*/
class para_need{
public:
    int para_num;
    /*string para_name[3];
    int para[3];*/
    st_para para[20];

    /*一组设定函数，num*/
    void set_name_type(int num, string para_name = "", string para_type = "int");
    void set_default(int num, int para_whe_have_default = 1, int para_value_default = 0, string para_string_default = "");
    void set_value(int num, int para_whe_have_value = 1, int para_value = 0, string para_string = "");
    void set_range(int num, int para_whe_have_range = 0, int min_range = 0, int max_range = 0);
};
int parameter_process(int argc, char **argv, para_need &pn);