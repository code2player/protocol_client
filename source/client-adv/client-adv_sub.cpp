#include <iostream>
#include <iomanip>
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
#include "client-adv.h"
using namespace std;

//打印buff到length中的内容，格式为16进制+字符+对应换行+记录，具体参见日志。
int client_base::my_data_log(int devid, char* buff, int length)
{
    int i;
    string line_str = "";
    for (i = 0; i < length; i++)
    {
        if(i%16 == 0)
        {
            fs <<"  "<< hex << setw(4) << setfill('0') << i << ":  ";
        }
        fs << hex << setw(2) << setfill('0') << static_cast<int>(buff[i]) << " ";
        if (buff[i] >= 33 && buff[i] <= 126)
            line_str += buff[i];
        else
            line_str += '.';

        if(i%16 == 7)
        {
            fs <<"- ";
        }
        if(i%16 == 15)
        {
            fs <<" ";
            fs<<line_str<<endl;
            line_str = "";
        }
    }
    fs <<" ";
    fs<<line_str<<endl;
    return 0;
}


/*重写read、write函数，增加*/
int client_base::my_read(int devid, int client_socket, char *buff, int readbyte)
{
    int n;
    int now_sum = 0;

    while (now_sum < readbyte) //一段数据多包设计
    {
        n = read(client_socket, buff + now_sum, readbyte - now_sum);
        if (n < 0)
        {
            return n;
        }
        
        if(RPACK == 1)//打印本次包的数据
        {
            ALL_print(devid, "读取" + to_string(n) + "字节");
            ALL_print(devid, "(读取数据为:)");
            my_data_log(devid, buff + now_sum, n);
        }
        now_sum += n;
    }
    
    /*if(RPACK == 1)//打印所有数据
    {
        ENV_print(devid, "(读取数据为:)");
        my_data_log(devid, buff, readbyte);
    }*/
    return readbyte;
}

int client_base::my_write(int devid, int client_socket, char *buff, int writebyte)
{
    int n;
    int now_sum = 0;
    
    while (now_sum < writebyte) //一段数据多包设计
    {
        n = write(client_socket, buff + now_sum, writebyte - now_sum);
        if (n < 0)
        {
            return n;
        }
        
        if(SPACK == 1)//打印本次包的数据
        {
            ALL_print(devid, "发送" + to_string(n) + "字节");
            ALL_print(devid, "(发送数据为:)");
            my_data_log(devid, buff + now_sum, n);
        }
        now_sum += n;
    }
    
    /*if(SPACK == 1)//打印所有数据
    {
        ENV_print(devid, "(发送数据为:)");
        my_data_log(devid, buff, writebyte);
    }*/
    return writebyte;
}

void client_base::ENV_print(int devid, string str)
{
    string out_print = "";
    time_t timeReal;
    time(&timeReal);
    timeReal = timeReal + 8 * 3600;//时区设定
    tm *t = gmtime(&timeReal);
    out_print = to_string(t->tm_hour) + ":" + to_string(t->tm_min) + ":" + to_string(t->tm_sec) + " [" + to_string(devid) + "] ";
    out_print += str;

    if (ENV == 1)
    {
        fs << out_print << endl;
        if (show_debug == 1)
        {
            cout << out_print << endl;
        }
    }
}

void client_base::ERR_print(int devid, string str)
{
    string out_print = "";
    time_t timeReal;
    time(&timeReal);
    timeReal = timeReal + 8 * 3600;
    tm *t = gmtime(&timeReal);
    out_print = to_string(t->tm_hour) + ":" + to_string(t->tm_min) + ":" + to_string(t->tm_sec) + " [" + to_string(devid) + "] ";
    out_print += str;
    if (ERR == 1)
    {
        fs << out_print << endl;
        if (show_debug == 1)
        {
            cout << out_print << endl;
        }
    }
}

void client_base::SDATA_print(int devid, string str)
{
    string out_print = "";
    time_t timeReal;
    time(&timeReal);
    timeReal = timeReal + 8 * 3600;
    tm *t = gmtime(&timeReal);
    out_print = to_string(t->tm_hour) + ":" + to_string(t->tm_min) + ":" + to_string(t->tm_sec) + " [" + to_string(devid) + "] ";
    out_print += str;
    if (SDATA == 1)
    {
        fs << out_print << endl;
        if (show_debug == 1)
        {
            cout << out_print << endl;
        }
    }
}

void client_base::RDATA_print(int devid, string str)
{
    string out_print = "";
    time_t timeReal;
    time(&timeReal);
    timeReal = timeReal + 8 * 3600;
    tm *t = gmtime(&timeReal);
    out_print = to_string(t->tm_hour) + ":" + to_string(t->tm_min) + ":" + to_string(t->tm_sec) + " [" + to_string(devid) + "] ";
    out_print += str;
    if (RDATA == 1)
    {
        fs << out_print << endl;
        if (show_debug == 1)
        {
            cout << out_print << endl;
        }
    }
}

void client_base::ALL_print(int devid, string str)
{
    string out_print = "";
    time_t timeReal;
    time(&timeReal);
    timeReal = timeReal + 8 * 3600;//时区设定
    tm *t = gmtime(&timeReal);
    out_print = to_string(t->tm_hour) + ":" + to_string(t->tm_min) + ":" + to_string(t->tm_sec) + " [" + to_string(devid) + "] ";
    out_print += str;

    fs << out_print << endl;
    if (show_debug == 1)
    {
        cout << out_print << endl;
    }
}

void client_base::fill_ts_count(int a, int b, int c)
{
    time_t timeReal;
    time(&timeReal);
    timeReal = timeReal + 8 * 3600; //时区设定
    tm *t = gmtime(&timeReal);
    fs_count << to_string(t->tm_hour) << ":" << to_string(t->tm_min) << ":" << to_string(t->tm_sec) << "\t" << a << "\t"
             << "1\t" << b << "\t" << c << endl;
}
