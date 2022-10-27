#include <iostream>
#include <fstream>
#include <sstream>
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
#include "../include/parameter_config.h"
using namespace std;

void para_need_config::set_name_type(int num, string para_name, string para_type)
{
    para[num].para_name = para_name;
    para[num].para_type = para_type;
}

void para_need_config::set_default(int num, int para_value, string para_string)
{
    para[num].para_value = para_value;
    para[num].para_string = para_string;
}
void para_need_config::set_range(int num, int para_whe_have_range, int min_range, int max_range)
{
    para[num].para_whe_have_range = para_whe_have_range;
    para[num].min_range = min_range;
    para[num].max_range = max_range;
}

//����ɾ��ע�ͺ��string
string para_need_config::del_note(string line, string note)
{
    size_t pos = line.find(note, 0);
    if (pos == line.npos)
    {
        return line;
    }
    else
    {
        return line.substr(0, pos);
    }
}

//�ж��Ƿ�����[]����ʱע����ɾ��
bool para_need_config::check_group(string line)
{
    if (line.size() == 0)
        return false;

    if (line[0] == '[' && line[line.size() - 1] == ']')
        return true;
    return false;
}

//��ֲ���(Ĭ��=�м��м��)
vector<string> para_need_config::split_str(string str)
{
    string buf;
    stringstream ss(str);
    vector<string> v;
    // �ַ���ss
    while (ss >> buf)
    {
        v.push_back(buf);
    }
    return v;
}

//������Ѱ��������ͬ��
int para_need_config::find_name_pos(string name)
{
    for (int i = 0; i < para_num; i++)
    {
        
        if (name == para[i].para_name)
        {
            return i;
        }
    }
    return -1;
}

//�������������趨ֵ֮��ִ�У��������ļ��ж�ȡ����
/*����˼·�����ж�ȡ��Ȼ�����*/
int para_need_config::parameter_process()
{
    ifstream ifs(file_path);
    if (!ifs.is_open())
    {
        return -1;
    }
    for (string line; getline(ifs, line);)
    {
        line = del_note(line, note);

        string this_name;
        string this_value_string;

        //�ո� tab������
        vector<string> split_line = split_str(line);
        if(split_line.size() == 0){
            continue;
        }
        if (check_group(split_line[0]) == true)
        {
            continue;
        }
        if (split_line.size() == 2 && whe_equal == false)
        {
            this_name = split_line[0];
            this_value_string = split_line[1];
        }
        else if (split_line.size() == 3 && whe_equal == true)
        {
            this_name = split_line[0];
            this_value_string = split_line[2];
        }
        else // error or block continue
        {
            continue;
            // cout<<"config error"<<endl;
            // return -1;
        }

        int pos = find_name_pos(this_name);

        if (para[pos].whe_have == true) //�Ѿ���ȡ��
        {
            continue;
        }
        para[pos].whe_have = true;

        /*������Ƿ������Ĭ�ϺϷ��������Ϸ�ֱ�Ӽģ�*/
        if (para[pos].para_type == "string")
        {
            para[pos].para_string = this_value_string;
        }
        else
        {
            int this_value_int = atoi(this_value_string.c_str());
            if (para[pos].para_whe_have_range == true)
            {
                if (this_value_int >= para[pos].min_range && this_value_int <= para[pos].max_range)
                {
                    para[pos].para_value = this_value_int;
                }
            }
            else
            {
                para[pos].para_value = this_value_int;
            }
        }
    }
    ifs.close();
    return 0;
}
