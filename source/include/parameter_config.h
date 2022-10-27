#pragma once
#include <string>
#include <vector>
using namespace std;

//������������������
class st_para_config
{
public:
    string para_name;
    string para_type;
    
    int para_value;     // int��value����
    string para_string; // string��value����

    int para_whe_have_range; //�Ƿ��з�Χ��ֻ��int����Ч����0��û�У�1����
    int min_range;           //��Сֵ
    int max_range;           //���ֵ

    bool whe_have; //�Ƿ��Ѿ����ڣ�������Ե�һ��Ϊ׼��

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

/*para_needΪ���صĲ���,���в����ļ���*/
class para_need_config
{
public:
    int para_num;
    st_para_config para[20];

    string note;      //ע�ͱ�ʶ��
    bool whe_equal;   //�Ƿ���Ҫ = ��Ϊ��ֵ��
    bool whe_group;   //�Ƿ�����������[]
    string file_path; //�����ļ�·��

    /*һ���趨������num*/
    void set_name_type(int num, string para_name = "", string para_type = "int");               //��������
    void set_default(int num, int para_value = 0, string para_string = "");                     //ȱʡ�趨
    void set_range(int num, int para_whe_have_range = 0, int min_range = 0, int max_range = 0); // int����ֵ��Χ�趨
    string del_note(string line, string note);
    bool check_group(string line);
    vector<string> split_str(string str);
    int find_name_pos(string name);
    int parameter_process();
};
