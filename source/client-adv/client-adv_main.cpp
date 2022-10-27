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
#include <wait.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "../include/parameter_config.h"
#include "../include/mydaemon.h"
#include "../include/parameter_process.h"
#include "../include/tcp_common.h"
#include "../include/socket_common.h"
#include "client-adv.h"
using namespace std;

extern unsigned char secret[4096];

int init_client(int devid, client_base *cb, int &port, string &ip, int &client_socket, struct sockaddr_in &remote_addr)
{
    memset(&remote_addr, 0, sizeof(remote_addr));
    remote_addr.sin_family = AF_INET;
    remote_addr.sin_port = htons(port);
    remote_addr.sin_addr.s_addr = inet_addr(ip.c_str());

    //创建客户端套接字
    if ((client_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        cb->ERR_print(devid, "创建socket失败");
        return -1;
    }

    //连接服务器
    if (connect(client_socket, (struct sockaddr *)&remote_addr, sizeof(remote_addr)) < 0)
    {
        cb->ERR_print(devid, "连接server端失败");
        return -1;
    }

    //cb->ENV_print(devid, "已连接到server端");
    return 0;
}

bool version_check(unsigned short main_ver)
{
    if (main_ver >= 2)
    {
        return true;
    }
    else
    {
        return false;
    }
}

bool time_check(unsigned int svr_time)
{
    if (svr_time>=SERVER_MIN_TIME)
    {
        return true;
    }
    else
    {
        return false;
    }
}

bool identify_check(unsigned int random_num, unsigned char identify_series[33])
{
    int pos = (random_num % 4093);
    for (int i = 0; i <32; i++)
    {
        identify_series[i] = identify_series[i] ^ secret[pos];
        ++pos;
        pos = pos%4093;
    }

    if(memcmp(identify_series, "yzmond:id*str&to!tongji@by#Auth^", 32) == 0)
    {
        return true;
    }
    return false;
}

int get_cpuinfo(int devid, client_base *cb)
{
    ifstream cpuinfo;
    cpuinfo.open("/proc/cpuinfo", ios::in);
    string word;
    int cpucount = 0;
    if (cpuinfo.is_open())
    {
        while (cpuinfo >> word)
        {
            if (word == "BogoMIPS")
            {
                cpuinfo >> word;
                cpuinfo >> word;
                cpucount = atof(word.c_str());
                cpuinfo.close();
                return cpucount;
            }
        }
        cpuinfo.close();
    }
    else
    {
        cb->ERR_print(devid, "Unable to open file cpuinfo");
        exit(-1);
    } 
    return 0;
}

int get_meminfo(int devid, client_base *cb)
{
    ifstream cpuinfo;
    cpuinfo.open("/proc/meminfo", ios::in);
    string word;
    int cpucount = 0;
    if (cpuinfo.is_open())
    {
        while (cpuinfo >> word)
        {
            if (word == "MemTotal:")
            {
                cpuinfo >> word;
                cpucount = atoi(word.c_str());
                cpucount = cpucount / 1024;
                cpuinfo.close();
                return cpucount;
            }
        }
        cpuinfo.close();
    }
    else
    {
        cb->ERR_print(devid, "Unable to open file meminfo");
        exit(-1);
    } 
    return 0;
}

int get_linux_config(int devid, client_base *cb, string path, string field)
{
    ifstream cpuinfo;
    cpuinfo.open(path, ios::in);
    string word;
    int cpucount = 0;
    if (cpuinfo.is_open())
    {
        while (cpuinfo >> word)
        {
            if (word == field)
            {
                cpuinfo >> word;
                cpucount = atoi(word.c_str());
                cpuinfo.close();
                return cpucount;
            }
        }
        cpuinfo.close();
    }
    else
    {
        cb->ERR_print(devid, "Unable to open file meminfo");
        exit(-1);
    } 
    return 0;
}

//填充报文头
void fill_head(char *buff, int head1, int head2, short head3, short head4, short head5)
{
    buff[0] = ( char)(head1);
    buff[1] = ( char)(head2);

    head3 = htons(head3);
    head4 = htons(head4);
    head5 = htons(head5);

    memcpy(buff + 2, &(head3), 2);
    memcpy(buff + 4, &(head4), 2);
    memcpy(buff + 6, &(head5), 2);
}

//主机序转网络序，填充相关报文段
void fill_context(char *buff, int pos, unsigned int num, int len)
{
    unsigned char num1 = num;
    unsigned short num2 = num;
    unsigned int num4 = num;
    if (len == 1)
        memcpy(buff + pos, &num1, len);
    else if (len == 2)
    {
        num2 = htons(num2);
        memcpy(buff + pos, &num2, len);
    }
    else if (len == 4)
    {
        num4 = htonl(num4);
        memcpy(buff + pos, &num4, len);
    }
    else
    {
        ;
    }
}

//网络序转主机序，解析相关报文段
unsigned short read_context2(char *buff)
{
    unsigned short res;
    memcpy(&res, buff, 2);
    res = ntohs(res);
    return res;
}

unsigned int read_context4(char *buff)
{
    unsigned int res;
    memcpy(&res, buff, 4);
    res = ntohl(res);
    return res;
}

//fork出的子进程
int sub_process(int devid, client_base *cb)
{
    srand((unsigned int)getpid());
    int log_term_num = 0;
    int log_screen_num = 0;
    //进程全局的大循环，每一次循环代表一次连接
    while (true)
    {
        /*开始进行tcp阻塞方式client端的编写*/
        int client_socket; //客户端套接字
        int n;
        char buff[BUF_SIZE];   //数据传输缓冲区
        struct sockaddr_in remote_addr; //服务器端网络地址

        /*client端初始化，与server端建立tcp连接*/
        init_client(devid, cb, cb->port, cb->IP, client_socket, remote_addr);

        /*60个字节的报文：server to client*/
        // read不需要构造buf，write需要

        n = cb->my_read(devid, client_socket, buff, 60);
        if (n < 0)
        {
            printf("read error: %s(errno:%d)", strerror(errno), errno);
            break;
        }

        cb->RDATA_print(devid, "收到客户端状态请求[intf=认证信息]");

        unsigned short main_ver;
        //unsigned short fail_time;
        unsigned short success_time;
        unsigned int random_num;
        unsigned int svr_time;
        unsigned char identify_series[33];
        identify_series[32] = 0;

        main_ver = read_context2(buff + 8);
        //fail_time = read_context2(buff + 12);
        success_time = read_context2(buff + 14);
        random_num = read_context4(buff + 52);
        svr_time = read_context4(buff + 56);
        memcpy(&identify_series, buff + 20, 32);

        /*memcpy(&main_ver, buff + 8, 2);
        memcpy(&fail_time, buff + 12, 2);
        memcpy(&success_time, buff + 14, 2);
        memcpy(&random_num, buff + 52, 4);
        memcpy(&svr_time, buff + 56, 4);*/
        //random_num = ntohl(random_num);
        

        if (version_check(main_ver) == false) //发送最低版本报文：client to server
        {
            memset(buff, 0, BUF_SIZE);
            buff[0] = (char)(0x91);
            buff[1] = (char)(0x00);
            /*memcpy(buff + 2, (unsigned short)(12), 2);
            memcpy(buff + 4, (unsigned short)(0x0000), 2);
            memcpy(buff + 6, (unsigned short)(4), 2);
            memcpy(buff + 8, (unsigned short)(2), 2);*/
            fill_context(buff, 2, 12, 2);
            fill_context(buff, 4, 0x0000, 2);
            fill_context(buff, 6, 4, 2);
            fill_context(buff, 8, 2, 2);
            /*buff[2] = (unsigned short)(12);
            buff[4] = (unsigned short)(0x0000);
            buff[6] = (unsigned short)(4);
            buff[8] = (unsigned short)(2);*/
            buff[10] = (char)(0);
            buff[11] = (char)(0);
            cb->SDATA_print(devid, "发送客户端状态应答[intf=低版本报文 len=12(C-0)]");
            n = cb->my_write(devid, client_socket, buff, 12);
            if (n < 0)
            {
                string str_error_x = "write error: ";
                str_error_x += strerror(errno);
                cb->ERR_print(devid, str_error_x);
            }
            cb->ENV_print(devid, "低版本");
            close(client_socket); // server主动关闭
            break;
        }
        else if (time_check(svr_time) == false) //提示“数字证书过期”，关闭连接
        {
            cb->ENV_print(devid, "数字证书过期");
            close(client_socket);
            break;
        }
        else if (identify_check(random_num, identify_series) == false) //认证串不匹配提示“认证非法”，关闭连接
        {
            cb->ENV_print(devid, "认证非法，认证串不匹配");
            close(client_socket);
            break;
        }
        else
        {
            ;
        }

        string str_idenx = "认证串解密: " ;
        str_idenx += (char*)identify_series;
        cb->RDATA_print(devid, str_idenx);

        /*设备向服务器发送认证串及基本配置信息*/
        memset(buff, 0, BUF_SIZE);
        buff[0] = ( char)(0x91);
        buff[1] = ( char)(0x01);
        fill_context(buff, 2, 116, 2);
        fill_context(buff, 4, 0x0000, 2);
        fill_context(buff, 6, 108, 2);
        fill_context(buff, 8, get_cpuinfo(devid, cb), 2);
        fill_context(buff, 10, get_meminfo(devid, cb), 2);
        fill_context(buff, 12, 270, 2);
        fill_context(buff, 14, 3072, 2);
        memcpy(buff + 16, "xiaopengfei", 11);
        memcpy(buff + 32, "xpf-2703591", 11);
        memcpy(buff + 48, "Ver:30.72", 9);
        buff[64] = ( char)((devid % 10) % 2);
        buff[65] = ( char)(0);
        buff[66] = ( char)((((devid % 100) / 10) % 3) * 8);
        buff[67] = ( char)(0);
        buff[68] = ( char)(((devid % 1000) / 100) % 2);
        buff[69] = ( char)(((devid % 10000) / 1000) % 2);
        // buff[72] = (unsigned int)(devid);
        // memcpy(buff + 72, (unsigned int)(devid), 4);
        fill_context(buff, 72, devid, 4);

        buff[76] = ( char)(1);
        memcpy(buff + 80, "yzmond:id*str&to!tongji@by#Auth^", 32);
        random_num = (unsigned int)rand();
        fill_context(buff, 112, random_num, 4);

        cb->SDATA_print(devid, "发送客户端状态应答(加密前)[intf=认证信息 len=116(C-0)]");
        cb->my_data_log(devid, buff, 116);



        int pos = (random_num % 4093);
        for (int i = 8; i < 112; i++)
        {
            buff[i] = buff[i] ^ secret[pos];
            ++pos;
            pos = pos % 4093;
        }
        // buff[112] = (unsigned int)(random_num);
        // memcpy(buff + 112, (unsigned int)(random_num), 4);
        
        cb->SDATA_print(devid, "发送客户端状态应答(加密后)[intf=认证信息 len=116(C-0)]");
        n = cb->my_write(devid, client_socket, buff, 116);
        if (n < 0)
        {
            string str_error_x = "write error: ";
                str_error_x += strerror(errno);
                cb->ERR_print(devid, str_error_x);
            break;
        }

        /*开始进入正式传递信息的循环。注意断开连接是由服务器主动发出到设备*/
        while (true)
        {
            /*服务器向设备发送各种取信息请求，所有请求均为8字节*/
            n = cb->my_read(devid, client_socket, buff, 8);
            if (n < 0)
            {
                string str_error_x = "read error: ";
                str_error_x += strerror(errno);
                cb->ERR_print(devid, str_error_x);
                break;
            }

            unsigned char inst = (unsigned char)(buff[1]);
            if (inst == 0x02) //取系统信息
            {
                cb->RDATA_print(devid, "收到客户端状态请求[intf=系统信息]");
                ifstream cpuinfo;
                cpuinfo.open("/proc/stat", ios::in);
                string word;
                int user_cputime = 0;
                int nice_cputime = 0;
                int system_cputime = 0;
                int idle_cputime = 0;
                if (cpuinfo.is_open())
                {
                    while (cpuinfo >> word)
                    {
                        if (word == "cpu")
                        {
                            cpuinfo >> word;
                            user_cputime = atoi(word.c_str());
                            cpuinfo >> word;
                            nice_cputime = atoi(word.c_str());
                            cpuinfo >> word;
                            system_cputime = atoi(word.c_str());
                            cpuinfo >> word;
                            idle_cputime = atoi(word.c_str());
                            break;
                        }
                    }
                    cpuinfo.close();
                }
                else
                {
                    cb->ERR_print(devid, "Unable to stat open file");
                    exit(-1);
                }

                int MemFree = get_linux_config(devid, cb, "/proc/meminfo", "MemFree:");
                int Buffers = get_linux_config(devid, cb, "/proc/meminfo", "Buffers:");
                int Cached = get_linux_config(devid, cb, "/proc/meminfo", "Cached:");
                int full_mem = MemFree + Buffers + Cached;
                //full_mem = full_mem / 1024;

                memset(buff, 0, BUF_SIZE);
                fill_head(buff, 0x91, 0x02, 28, 0x0000, 20);
                fill_context(buff, 8, user_cputime, 4);
                fill_context(buff, 12, nice_cputime, 4);
                fill_context(buff, 16, system_cputime, 4);
                fill_context(buff, 20, idle_cputime, 4);
                fill_context(buff, 24, full_mem, 4);
                cb->SDATA_print(devid, "发送客户端状态应答[intf=系统信息 len=28(C-0)]");
                n = cb->my_write(devid, client_socket, buff, 28);
                if (n < 0)
                {
                    string str_error_x = "write error: ";
                str_error_x += strerror(errno);
                cb->ERR_print(devid, str_error_x);
                    break;
                }
            }
            else if (inst == 0x03) //取配置信息
            {
                cb->RDATA_print(devid, "收到客户端状态请求[intf=配置信息]");
                //读取文件全部内容
                ifstream ifs("config.dat", ios::in);
                string content((istreambuf_iterator<char>(ifs)), (istreambuf_iterator<char>()));
                ifs.close();
                int data_len;
                if (content.size() > 8191)
                    data_len = 8192;
                else
                    data_len = content.size() + 1;

                memset(buff, 0, BUF_SIZE);
                fill_head(buff, 0x91, 0x03, data_len + 8, 0x0000, data_len);
                memcpy(buff + 8, content.c_str(), data_len - 1);
                cb->SDATA_print(devid, "发送客户端状态应答[intf=配置信息 len="+to_string(data_len + 8)+"(C-0)]");
                n = cb->my_write(devid, client_socket, buff, data_len + 8);
                if (n < 0)
                {
                    string str_error_x = "write error: ";
                    str_error_x += strerror(errno);
                    cb->ERR_print(devid, str_error_x);
                    break;
                }
            }
            else if (inst == 0x04) //取进程信息
            {
                cb->RDATA_print(devid, "收到客户端状态请求[intf=进程信息]");
                //读取文件全部内容
                ifstream ifs("process.dat", ios::in);
                string content((istreambuf_iterator<char>(ifs)), (istreambuf_iterator<char>()));
                ifs.close();
                int data_len;
                if (content.size() > 8191)
                    data_len = 8192;
                else
                    data_len = content.size() + 1;

                memset(buff, 0, BUF_SIZE);
                fill_head(buff, 0x91, 0x04, data_len + 8, 0x0000, data_len);
                memcpy(buff + 8, content.c_str(), data_len - 1);
                cb->SDATA_print(devid, "发送客户端状态应答[intf=进程信息 len="+to_string(data_len + 8)+"(C-0)]");
                n = cb->my_write(devid, client_socket, buff, data_len + 8);
                if (n < 0)
                {
                    string str_error_x = "write error: ";
                str_error_x += strerror(errno);
                cb->ERR_print(devid, str_error_x);
                    break;
                }
            }
            else if (inst == 0x05) //取以太口信息
            {
                unsigned short ethernet;
                ethernet = read_context2(buff + 4);
                cb->RDATA_print(devid, "收到客户端状态请求[intf=Ethernet" + to_string(ethernet) + "口信息]");
                // memcpy(&ethernet, buff + 4, 2);
                memset(buff, 0, BUF_SIZE);
                fill_head(buff, 0x91, 0x05, 132, ethernet, 124);
                buff[8] = (unsigned char)(1);
                buff[9] = (unsigned char)(1);
                buff[10] = (unsigned char)(1);

                buff[12] = (unsigned char)(0x00);
                buff[13] = (unsigned char)(0x0c);
                buff[14] = (unsigned char)(0x29);
                buff[15] = (unsigned char)(0xe3);
                buff[16] = (unsigned char)(0xde);
                buff[17] = (unsigned char)(devid % 100);
                // buff[18] = (unsigned short)(0x0000);
                // memcpy(buff + 18, (unsigned short)(0x0000), 2);
                fill_context(buff, 18, 0x0000, 2);

                unsigned int ip_fake = (unsigned int)(devid % 100) * 255 * 255 * 255 + 80 * 255 * 255 + 42 * 255;
                unsigned int mask_fake = (unsigned int)255 * 255 * 255 * 255 + 255 * 255 * 255 + 255 * 255;

                fill_context(buff, 20, ip_fake + 1, 4);
                fill_context(buff, 24, mask_fake, 4);
                fill_context(buff, 28, ip_fake + 2, 4);
                fill_context(buff, 32, mask_fake, 4);
                fill_context(buff, 36, ip_fake + 3, 4);
                fill_context(buff, 40, mask_fake, 4);
                fill_context(buff, 44, ip_fake + 4, 4);
                fill_context(buff, 48, mask_fake, 4);
                fill_context(buff, 52, ip_fake + 5, 4);
                fill_context(buff, 56, mask_fake, 4);
                fill_context(buff, 60, ip_fake + 6, 4);
                fill_context(buff, 64, mask_fake, 4);

                ifstream cpuinfo;
                cpuinfo.open("/proc/net/dev", ios::in);
                string ename;
                if (ethernet == 0)
                    ename = "enp4s0:";
                else
                    ename = "lo:";
                string word;
                if (cpuinfo.is_open())
                {
                    while (cpuinfo >> word)
                    {
                        if (word == ename)
                        {
                            for (int i = 0; i < 16; i++)
                            {
                                cpuinfo >> word;
                                long long wordx = atoll(word.c_str());
                                wordx = wordx % 0xffffffff;
                                int wordxx = (int)wordx;
                                fill_context(buff, 68 + i * 4, wordxx, 4);
                            }
                            break;
                        }
                    }
                    cpuinfo.close();
                }
                else
                {
                    cb->ERR_print(devid, "Unable to stat open file");
                    exit(-1);
                }
                cb->SDATA_print(devid, "发送客户端状态应答[intf=Ethernet" + to_string(ethernet) + "口信息 len=132(C-0)]");
                n = cb->my_write(devid, client_socket, buff, 132);
                if (n < 0)
                {
                    string str_error_x = "write error: ";
                str_error_x += strerror(errno);
                cb->ERR_print(devid, str_error_x);
                    break;
                }
            }
            else if (inst == 0x07) //取usb口信息
            {
                cb->RDATA_print(devid, "收到客户端状态请求[intf=usb口信息]");
                memset(buff, 0, BUF_SIZE);
                fill_head(buff, 0x91, 0x07, 12, 0x0000, 4);
                buff[8] = (unsigned char)((devid % 10) % 2);
                cb->SDATA_print(devid, "发送客户端状态应答[intf=usb口信息 len=12(C-0)]");
                n = cb->my_write(devid, client_socket, buff, 12);
                if (n < 0)
                {
                    string str_error_x = "write error: ";
                    str_error_x += strerror(errno);
                    cb->ERR_print(devid, str_error_x);
                    break;
                }
            }
            else if (inst == 0x0c) //取u盘上文件列表信息
            {
                cb->RDATA_print(devid, "收到客户端状态请求[intf=u盘上文件列表信息]");
                //读取文件全部内容
                ifstream ifs("usefile.dat", ios::in);
                string content((istreambuf_iterator<char>(ifs)), (istreambuf_iterator<char>()));
                ifs.close();
                int data_len;
                if (content.size() > 4095)
                    data_len = 4096;
                else
                    data_len = content.size() + 1;

                memset(buff, 0, BUF_SIZE);
                fill_head(buff, 0x91, 0x0c, data_len + 8, 0x0000, data_len);
                memcpy(buff + 8, content.c_str(), data_len - 1);
                cb->SDATA_print(devid, "发送客户端状态应答[intf=u盘上文件列表信息 len="+to_string(data_len + 8)+"(C-0)]");
                n = cb->my_write(devid, client_socket, buff, data_len + 8);
                if (n < 0)
                {
                    string str_error_x = "write error: ";
                str_error_x += strerror(errno);
                cb->ERR_print(devid, str_error_x);
                    break;
                }
            }
            else if (inst == 0x08) //取打印口信息
            {
                cb->RDATA_print(devid, "收到客户端状态请求[intf=打印口信息]");
                unsigned char service_start = ((devid % 100) / 10) % 2;
                unsigned short task_num;
                if (service_start == 1)
                    task_num = (devid % 100) % 25;
                else
                    task_num = 0;

                memset(buff, 0, BUF_SIZE);
                fill_head(buff, 0x91, 0x08, 44, 0x0000, 36);
                buff[8] = (unsigned char)(service_start);
                // buff[10] = (unsigned short)(task_num);
                fill_context(buff, 10, task_num, 2);
                memcpy(buff + 12, "PRN-xpf-3072", sizeof("PRN-xpf-3072"));
                cb->SDATA_print(devid, "发送客户端状态应答[intf=打印口信息 len=44(C-0)]");
                n = cb->my_write(devid, client_socket, buff, 44);
                if (n < 0)
                {
                    string str_error_x = "write error: ";
                str_error_x += strerror(errno);
                cb->ERR_print(devid, str_error_x);
                    break;
                }
            }
            else if (inst == 0x0d) //取打印队列信息
            {
                cb->RDATA_print(devid, "收到客户端状态请求[intf=打印队列信息]");
                memset(buff, 0, BUF_SIZE);
                fill_head(buff, 0x91, 0x0d, 9, 0x0000, 1);
                buff[8] = 0;
                cb->SDATA_print(devid, "发送客户端状态应答[intf=打印队列信息 len=9(C-0)]");
                n = cb->my_write(devid, client_socket, buff, 9);
                if (n < 0)
                {
                    string str_error_x = "write error: ";
                str_error_x += strerror(errno);
                cb->ERR_print(devid, str_error_x);
                    break;
                }
            }
            else if (inst == 0x09) //取终端服务信息
            {
                cb->RDATA_print(devid, "收到客户端状态请求[intf=TServer配置信息]");
                memset(buff, 0, BUF_SIZE);
                fill_head(buff, 0x91, 0x09, 280, 0x0000, 272);

                int range = cb->max_terminal_num - cb->min_terminal_num;
                int total = rand() % (range + 1) + cb->min_terminal_num;
                int asyncnum = (((devid % 100) / 10) % 3) * 8;
                int async_term_num;

                if (asyncnum == 0)
                {
                    async_term_num = 0;
                }
                else
                {
                    async_term_num = rand() % asyncnum + 1;
                }

                total = (async_term_num > total ? async_term_num : total);
                int ipterm_num = total - async_term_num;
                for (int i = 0; i < async_term_num; i++)
                {
                    buff[8 + i] = (unsigned char)(1);
                }
                for (int i = 0; i < ipterm_num; i++)
                {
                    buff[24 + i] = (unsigned char)(1);
                }
                unsigned int randttynum;
                randttynum = rand() % (270 - total + 1) + total;
                log_term_num = total;

                fill_context(buff, 278, randttynum, 2);
                cb->SDATA_print(devid, "发送客户端状态应答[intf=TServer配置信息 len=280(C-0)]");
                n = cb->my_write(devid, client_socket, buff, 280);
                if (n < 0)
                {
                    string str_error_x = "write error: ";
                    str_error_x += strerror(errno);
                    cb->ERR_print(devid, str_error_x);
                    break;
                }
            }
            else if (inst == 0x0a || inst == 0x0b) //取哑终端/IP终端配置信息及对应虚屏的配置信息
            {
                unsigned short tty_no;
                // memcpy(&tty_no, buff + 4, 2);
                tty_no = read_context2(buff + 4);
                int range = cb->max_screen_num - cb->min_screen_num;
                int screen_num = rand() % (range + 1) + cb->min_screen_num;
                int active_screen = rand() % screen_num;

                memset(buff, 0, BUF_SIZE);
                fill_head(buff, 0x91, inst, 96 * screen_num + 36, tty_no, 96 * screen_num + 28); // 8 7*4 24*4*screen_num

                buff[8] = (unsigned char)tty_no;
                buff[9] = (unsigned char)tty_no;
                buff[10] = (unsigned char)active_screen;
                buff[11] = (unsigned char)screen_num;

                log_screen_num += screen_num;

                string tty_type_str;

                if (inst == 0x0a) // IP : 0
                {
                    cb->RDATA_print(devid, "收到客户端状态请求[intf=串口终端配置信息]");
                    tty_type_str = "串口终端";
                    fill_context(buff, 12, 0, 4);
                }
                else // IP : 随机不重复
                {
                    cb->RDATA_print(devid, "收到客户端状态请求[intf=IP终端配置信息]");
                    tty_type_str = "IP终端";
                    fill_context(buff, 12, ((unsigned int)10 * 255 * 255 * 255 + 80 * 255 * 255 + 23 * 255 + (unsigned int)tty_no), 4);
                }

                cb->SDATA_print(devid, "终端类型: " + tty_type_str);

                memcpy(buff + 16, tty_type_str.c_str(), sizeof(tty_type_str) - 1);
                if (rand() % 2 == 1)
                {
                    memcpy(buff + 28, "正常", sizeof("正常"));
                    cb->SDATA_print(devid, tty_type_str + ",终端序号" + to_string(tty_no) + "状态: 正常");
                }
                else
                {
                    memcpy(buff + 28, "菜单", sizeof("菜单"));
                    cb->SDATA_print(devid, tty_type_str + ",终端序号" + to_string(tty_no) + "状态: 菜单");
                }

                for (int i = 1; i <= screen_num; i++)
                {
                    char temp_buf[96];
                    memset(temp_buf, 0, 96);
                    fill_context(temp_buf, 0, i, 1);
                    fill_context(temp_buf, 2, i, 2);
                    fill_context(temp_buf, 4, ((unsigned int)10 * 255 * 255 * 255 + 80 * 255 * 255 + 30 * 255 + (unsigned int)i), 4);

                    memcpy(temp_buf + 8, "SSH", sizeof("SSH"));
                    if (rand() % 2 == 1)
                    {
                        memcpy(temp_buf + 20, "开机", sizeof("开机"));
                        cb->SDATA_print(devid, tty_type_str + ",终端序号" + to_string(tty_no) + ",虚屏" + to_string(i) + "状态: 开机");
                    }
                    else
                    {
                        memcpy(temp_buf + 20, "关机", sizeof("关机"));
                        cb->SDATA_print(devid, tty_type_str + ",终端序号" + to_string(tty_no) + ",虚屏" + to_string(i) + "状态: 关机");
                    }
                    memcpy(temp_buf + 28, "储蓄系统", sizeof("储蓄系统"));
                    cb->SDATA_print(devid, tty_type_str + ",终端序号" + to_string(tty_no) + ",虚屏" + to_string(i) + "提示串: 储蓄系统");
                    memcpy(temp_buf + 52, "vt100", sizeof("vt100"));

                    int t = time(0);
                    fill_context(temp_buf, 64, t, 4);

                    fill_context(temp_buf, 68, rand()%0xffffffff, 4);
                    fill_context(temp_buf, 72, rand()%0xffffffff, 4);
                    fill_context(temp_buf, 76, rand()%0xffffffff, 4);
                    fill_context(temp_buf, 80, rand()%0xffffffff, 4);
                    
                    fill_context(temp_buf, 84, rand()%123456, 4);
                    fill_context(temp_buf, 88, rand()%123456, 4);
                    fill_context(temp_buf, 92, rand()%123456, 4);

                    memcpy(buff + 36 + (i - 1) * 96, temp_buf, 96);
                }
                cb->SDATA_print(devid, "发送客户端状态应答[intf=" + tty_type_str + " len=" + to_string(96 * screen_num + 36) + "(C-0)]");
                n = cb->my_write(devid, client_socket, buff, 96 * screen_num + 36);
                if (n < 0)
                {
                    string str_error_x = "write error: ";
                    str_error_x += strerror(errno);
                    cb->ERR_print(devid, str_error_x);
                    break;
                }
            }
            else if (inst == 0xff) //退出
            {
                cb->RDATA_print(devid, "收到客户端状态请求[intf=本次接收完成]");
                memset(buff, 0, BUF_SIZE);
                fill_head(buff, 0x91, 0xff, 8, 0x0000, 0);
                cb->ENV_print(devid, "本次接收完成");
                n = cb->my_write(devid, client_socket, buff, 8);
                break;
                /*n = cb->my_read(devid, client_socket, buff, 8);
                if (n < 0) //关闭tcp连接
                {
                    break;
                }*/
            }
        }
        close(client_socket);
        if (cb->exit_after_success == 1) //进程退出
        {
            break;
        }
        else //间隔一定时间后再次连接服务器
        {
            sleep(success_time);
            continue;
        }
    }

    cb->fill_ts_count(devid, log_term_num, log_screen_num);

    return 0;
}


int main(int argc, char **argv)
{
    if(argc != 3){
        cout<<"parameter error!"<<endl;
        return -1;
    }

    int devid = atoi(argv[1]);
    int devnum = atoi(argv[2]);
    
    para_need_config pnc;
    pnc.para_num = 10;
    pnc.note = "#";
    pnc.whe_equal = false;
    pnc.whe_group = false;
    pnc.file_path = "ts.conf";

    pnc.set_name_type(0, "服务器IP地址", "string");

    pnc.set_name_type(1, "端口号", "int");
    pnc.set_default(1, 43072, "");

    pnc.set_name_type(2, "进程接收成功后退出", "int");

    pnc.set_name_type(3, "最小配置终端数量", "int");
    pnc.set_default(3, 5, "");
    pnc.set_range(3, 1, 3, 10);

    pnc.set_name_type(4, "最大配置终端数量", "int");
    pnc.set_default(4, 28, "");
    pnc.set_range(4, 1, 10, 50);

    pnc.set_name_type(5, "每个终端最小虚屏数量", "int");
    pnc.set_default(5, 3, "");
    pnc.set_range(5, 1, 1, 3);

    pnc.set_name_type(6, "每个终端最大虚屏数量", "int");
    pnc.set_default(6, 10, "");
    pnc.set_range(6, 1, 4, 16);

    pnc.set_name_type(7, "删除日志文件", "int");

    pnc.set_name_type(8, "DEBUG设置", "string");
    pnc.set_name_type(9, "DEBUG屏幕显示", "int");
    if (pnc.parameter_process() < 0)
    {
        cout << "config error!" << endl;
        exit(-1);
    }

    client_base *cb = new client_base();
    cb->IP = pnc.para[0].para_string;
    cb->port = pnc.para[1].para_value;
    cb->exit_after_success = pnc.para[2].para_value;
    cb->min_terminal_num = pnc.para[3].para_value;
    cb->max_terminal_num = pnc.para[4].para_value;
    cb->min_screen_num = pnc.para[5].para_value;
    cb->max_screen_num = pnc.para[6].para_value;
    cb->del_log = pnc.para[7].para_value;
    cb->ENV = (int)(pnc.para[8].para_string[0] - '0');
    cb->ERR = (int)(pnc.para[8].para_string[1] - '0');
    cb->SPACK = (int)(pnc.para[8].para_string[2] - '0');
    cb->RPACK = (int)(pnc.para[8].para_string[3] - '0');
    cb->SDATA = (int)(pnc.para[8].para_string[4] - '0');
    cb->RDATA = (int)(pnc.para[8].para_string[5] - '0');
    cb->show_debug = pnc.para[9].para_value;

    if(cb->del_log == 1)//删除原来，建新日志
    {
        cb->fs.open(cb->log_path, ios::in|ios::out|ios::trunc);
    }
    else//从原来日志末尾开始写
    {
        cb->fs.open(cb->log_path, ios::in|ios::out|ios::app);
    }
    if(cb->fs.is_open() == 0)
    {
        cb->ERR_print(100000, "ts.log打开失败");
        return -1;
    }

    cb->fs_count.open(cb->count_path, ios::in|ios::out|ios::trunc);
    if(cb->fs_count.is_open() == 0)
    {
        cb->ERR_print(100000, "ts_count.xls打开失败");
        return -1;
    }

    cb->ENV_print(100000, "fork子进程,开始计时");

    const int process_max_num = 210;
    const int process_min_num = 120;
    int sum = 0;
    /*主进程中记录开始和结束时间，当所有子进程结束以后才退出并记录总时间在日志中*/
    struct timeval t1, t2;
    gettimeofday(&t1, NULL);

    int start_process_num;

    if(devnum<(process_max_num - process_min_num))
    {
        start_process_num = devnum;
    }
    else
    {
        start_process_num = process_max_num - process_min_num;
    }

    for (int i = 0; i < start_process_num; i++)
    {
        int pid = fork();
        if (pid > 0)
        {
            ;
        }
        else if (pid < 0)
        {
            cb->ERR_print(100000, "主进程分裂失败！");
            exit(1);
        }
        else
        {
            sub_process(i + devid, cb);
            exit(0);
        }
    }

    for (int i = 0; i < devnum - start_process_num; i++)
    {
        int pid = fork();
        if (pid > 0)
        {
            if (i % process_min_num == process_min_num - 1)
            {
                int status;
                for (int j = 0; j < process_min_num; j++)
                {
                    wait(&status); //及时回收，进程数量限制
                }
                // cout << "i:" << i << endl;
                sum += process_min_num;
            }
            // sleep(3);
        }
        else if (pid < 0)
        {
            exit(1);
        }
        else
        {
            sub_process(i + devid + start_process_num, cb);
            exit(0);
        }
    }

    int recive2 = devnum - sum;

    for (int i = 0; i < recive2; i++)
    {
        int status;
        wait(&status);
        sum++;
    }

    gettimeofday(&t2, NULL);
    int deltaT = (t2.tv_sec - t1.tv_sec);

    cb->ENV_print(100000, "本次所有子进程回收完成");
    string str_x1 = "time use:" + to_string(deltaT) + "s";
    cb->ENV_print(100000, str_x1);
    str_x1 = "process sum:" + to_string(devnum);
    cb->ENV_print(100000, str_x1);

    cout << "time use:" << deltaT << "s"<<endl;

    // delete时所有子进程已经返回
    cb->fs.close();
    cb->fs_count.close();
    delete cb;
    return 0;
}