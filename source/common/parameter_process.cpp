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

//�������--���Ĳ��������ز�����,���򷵻ؿմ�
string ret_para_name(string name_str)
{
    string name = "";
    if (name_str.size() > 2 && name_str[0] == '-' && name_str[1] == '-')
    {
        name = name_str.substr(2, name_str.size() - 2);
    }
    return name;
}

//���ز����Ƿ���ֵ
int ret_para_whe_have_value(string paraname, para_need &pn)
{
    for (int i = 0; i < pn.para_num; i++)
    {
        
        if (paraname == pn.para[i].para_name)
        {
            return pn.para[i].para_whe_have_value;
        }
    }
    cout << "����" << paraname << "������" << endl;
    exit(1);
}

//����bool��string��Ӧ��intֵ
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
        cout << "����" << str << "����" << endl;
        exit(1);
    }
}

/*����������*/
int parameter_process(int argc, char **argv, para_need &pn)
{
    /*map�洢������ֵ�Ķ�Ӧ��ϵ*/
    unordered_map<string, string> map1;
    /*�������һ���������б����ֵȫ���������ɺ󣬼����ɣ��ɹ�����*/
    //����״̬��0.��Ҫ���� 1.��Ҫֵ

    int state = 0;
    string past_name;
    // argc��1��ʼ�㣨0��Ϊ��ִ���ļ�����
    for (int i = 1; i < argc; i++)
    {
        string para = argv[i];
        string paraname = ret_para_name(para);
        
        if (state == 0 && paraname == "") //��Ҫ�����Ҳ��ǲ���
        {
            cout << "�������ʹ��󣬴˴���Ҫ���������ȴ������ֵ" << endl;
            exit(1);
        }
        else if (state == 1 && paraname == "") //����Ҫ�����Ҳ��ǲ�������ֵ��
        {
            string value = argv[i];
            map1[past_name] = value;
            state = 0;
        }
        else if (state == 0 && paraname != "") //��Ҫ�������ǲ���
        {
            string value = "";
            map1[paraname] = value;
            state = ret_para_whe_have_value(paraname, pn);
        }
        else //����Ҫ�������ǲ���
        {
            cout << "����" << paraname << "���Ϸ�" << endl;
            exit(1);
        }
        past_name = paraname;
    }

    /*������ȡ���洢��ϣ��������Ƿ���Ч���Բ�ͬ�������ͽ��д���*/
    for (int i = 0; i < pn.para_num; i++)
    {
        if (map1.find(pn.para[i].para_name) == map1.end() && pn.para[i].para_whe_have_default == 0) // argv��û���Ҳ�������,ok
        {
            if (pn.para[i].para_whe_have_value == 0) //û��ֵ
            {
                pn.para[i].para_value = 0;
                pn.para[i].para_string = "no";
            }
            else //��ֵ
            {
                pn.para[i].para_value = pn.para[i].para_value_default;
                pn.para[i].para_string = pn.para[i].para_string_default;
            }
        }
        else if (map1.find(pn.para[i].para_name) == map1.end() && pn.para[i].para_whe_have_default == 1) // argv��û���ұ�����,error
        {
            cout << "ȱ�ٱ������" << pn.para[i].para_name << endl;
            exit(1);
        }
        else if (map1.find(pn.para[i].para_name) != map1.end()) // argv����,ok������������ò���default
        {
            if (pn.para[i].para_whe_have_value == 0) //û��ֵ
            {
                pn.para[i].para_value = 1;
                pn.para[i].para_string = "yes";
            }
            else //��ֵ
            {
                if (pn.para[i].para_type == "int")
                {
                    pn.para[i].para_value = atoi((map1[pn.para[i].para_name]).c_str());
                    if(pn.para[i].para_whe_have_range==1)//�з�Χ
                        if (pn.para[i].para_value < pn.para[i].min_range || pn.para[i].para_value > pn.para[i].max_range)
                        {
                            cout << pn.para[i].para_name << "�������ڷ�Χ��" << endl;
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
