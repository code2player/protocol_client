#include <iostream>
#include <string>
#include <vector>
#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <sys/param.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unordered_map>
#include <fcntl.h>
#include "../include/parameter_process.h"
using namespace std;

//传入带“--”的参数，返回参数名,否则返回空串
string ret_para_name(string name_str)
{
    string name = "";
    if (name_str.size() > 2 && name_str[0] == '-' && name_str[1] == '-')
    {
        name = name_str.substr(2, name_str.size() - 2);
    }
    return name;
}

//返回参数是否有值
int ret_para_whe_have_value(string paraname, para_need &pn)
{
    for (int i = 0; i < pn.para_num; i++)
    {
        
        if (paraname == pn.para[i].para_name)
        {
            return pn.para[i].para_whe_have_value;
        }
    }
    cout << "参数" << paraname << "不存在" << endl;
    exit(1);
}

//返回bool型string对应的int值
int ret_bool_int(string str)
{
    if (str == "yes" || str == "Yes")
    {
        return 1;
    }
    else if (str == "no" || str == "No")
    {
        return 0;
    }
    else
    {
        cout << "参数" << str << "错误" << endl;
        exit(1);
    }
}

/*参数处理函数*/
int parameter_process(int argc, char **argv, para_need &pn)
{
    /*map存储参数与值的对应关系*/
    unordered_map<string, string> map1;
    /*读到最后一个，当所有必须的值全部被填充完成后，检测完成，成功结束*/
    //两个状态：0.需要参数 1.需要值

    int state = 0;
    string past_name;
    // argc从1开始算（0处为可执行文件名）
    for (int i = 1; i < argc; i++)
    {
        string para = argv[i];
        string paraname = ret_para_name(para);
        
        if (state == 0 && paraname == "") //需要参数且不是参数
        {
            cout << "参数类型错误，此处需要输入参数但却输入了值" << endl;
            exit(1);
        }
        else if (state == 1 && paraname == "") //不需要参数且不是参数（是值）
        {
            string value = argv[i];
            map1[past_name] = value;
            state = 0;
        }
        else if (state == 0 && paraname != "") //需要参数且是参数
        {
            string value = "";
            map1[paraname] = value;
            state = ret_para_whe_have_value(paraname, pn);
        }
        else //不需要参数且是参数
        {
            cout << "参数" << paraname << "不合法" << endl;
            exit(1);
        }
        past_name = paraname;
    }

    /*参数读取、存储完毕，检测参数是否有效，对不同数据类型进行处理*/
    for (int i = 0; i < pn.para_num; i++)
    {
        if (map1.find(pn.para[i].para_name) == map1.end() && pn.para[i].para_whe_have_default == 0) // argv中没有且不必须有,ok
        {
            if (pn.para[i].para_whe_have_value == 0) //没有值
            {
                pn.para[i].para_value = 0;
                pn.para[i].para_string = "no";
            }
            else //有值
            {
                pn.para[i].para_value = pn.para[i].para_value_default;
                pn.para[i].para_string = pn.para[i].para_string_default;
            }
        }
        else if (map1.find(pn.para[i].para_name) == map1.end() && pn.para[i].para_whe_have_default == 1) // argv中没有且必须有,error
        {
            cout << "缺少必须参数" << pn.para[i].para_name << endl;
            exit(1);
        }
        else if (map1.find(pn.para[i].para_name) != map1.end()) // argv中有,ok，这种情况下用不到default
        {
            if (pn.para[i].para_whe_have_value == 0) //没有值
            {
                pn.para[i].para_value = 1;
                pn.para[i].para_string = "yes";
            }
            else //有值
            {
                if (pn.para[i].para_type == "int")
                {
                    pn.para[i].para_value = atoi((map1[pn.para[i].para_name]).c_str());
                    if(pn.para[i].para_whe_have_range==1)//有范围
                        if (pn.para[i].para_value < pn.para[i].min_range || pn.para[i].para_value > pn.para[i].max_range)
                        {
                            cout << pn.para[i].para_name << "参数不在范围内" << endl;
                            exit(1);
                        }
                }
                else if (pn.para[i].para_type == "bool")
                    pn.para[i].para_value = ret_bool_int(map1[pn.para[i].para_name]);
                else if (pn.para[i].para_type == "string")
                    pn.para[i].para_string = map1[pn.para[i].para_name];
                else
                {
                    ;
                }
            }
        }
        else //
        {
            ;
        }
    }
    return 1;
}

void para_need::set_name_type(int num,string para_name,string para_type)
{
    para[num].para_name=para_name;
    para[num].para_type=para_type;
}

void para_need::set_default(int num,int para_whe_have_default,int para_value_default,string para_string_default)
{
    para[num].para_whe_have_default=para_whe_have_default;
    para[num].para_value_default=para_value_default;
    para[num].para_string_default=para_string_default;
}
void para_need::set_value(int num,int para_whe_have_value,int para_value,string para_string)
{
    para[num].para_whe_have_value=para_whe_have_value;
    para[num].para_value=para_value;
    para[num].para_string=para_string;
}
void para_need::set_range(int num,int para_whe_have_range,int min_range,int max_range)
{
    para[num].para_whe_have_range=para_whe_have_range;
    para[num].min_range=min_range;
    para[num].max_range=max_range;
}
