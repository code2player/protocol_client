#pragma once
#include <string>
#include <vector>
#include <fstream>
using namespace std;

#define BUF_SIZE 10000
#define SERVER_MIN_TIME 100312000

/*从配置文件中解析出来的各项内容*/
class client_base{
public:
    /*配置文件*/
    string IP;
    int port;

    int exit_after_success;
    int min_terminal_num;
    int max_terminal_num;
    int min_screen_num;
    int max_screen_num;

    int del_log;
    
    int ENV;
    int ERR;
    int SPACK;
    int RPACK;
    int SDATA;
    int RDATA;

    int show_debug;

    /*日志设定，多个子进程复用*/
    

    string count_path = "ts_count.xls";
    fstream fs_count;

    string log_path = "ts.log";
    fstream fs;
    

    /*依赖于配置文件的公共函数，有部分重写*/
    int my_read(int devid, int client_socket, char* buff, int readbyte);
    int my_write(int devid, int client_socket, char* buff, int writebyte);
    int my_data_log(int devid, char* buff, int length);
    void ENV_print(int devid, string str);
    void ERR_print(int devid, string str);
    void SDATA_print(int devid, string str);
    void RDATA_print(int devid, string str);
    void ALL_print(int devid, string str);
    void fill_ts_count(int a, int b, int c);

};







