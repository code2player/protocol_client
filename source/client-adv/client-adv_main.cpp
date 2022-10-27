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

    //�����ͻ����׽���
    if ((client_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        cb->ERR_print(devid, "����socketʧ��");
        return -1;
    }

    //���ӷ�����
    if (connect(client_socket, (struct sockaddr *)&remote_addr, sizeof(remote_addr)) < 0)
    {
        cb->ERR_print(devid, "����server��ʧ��");
        return -1;
    }

    //cb->ENV_print(devid, "�����ӵ�server��");
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

//��䱨��ͷ
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

//������ת�����������ر��Ķ�
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

//������ת�����򣬽�����ر��Ķ�
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

//fork�����ӽ���
int sub_process(int devid, client_base *cb)
{
    srand((unsigned int)getpid());
    int log_term_num = 0;
    int log_screen_num = 0;
    //����ȫ�ֵĴ�ѭ����ÿһ��ѭ������һ������
    while (true)
    {
        /*��ʼ����tcp������ʽclient�˵ı�д*/
        int client_socket; //�ͻ����׽���
        int n;
        char buff[BUF_SIZE];   //���ݴ��仺����
        struct sockaddr_in remote_addr; //�������������ַ

        /*client�˳�ʼ������server�˽���tcp����*/
        init_client(devid, cb, cb->port, cb->IP, client_socket, remote_addr);

        /*60���ֽڵı��ģ�server to client*/
        // read����Ҫ����buf��write��Ҫ

        n = cb->my_read(devid, client_socket, buff, 60);
        if (n < 0)
        {
            printf("read error: %s(errno:%d)", strerror(errno), errno);
            break;
        }

        cb->RDATA_print(devid, "�յ��ͻ���״̬����[intf=��֤��Ϣ]");

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
        

        if (version_check(main_ver) == false) //������Ͱ汾���ģ�client to server
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
            cb->SDATA_print(devid, "���Ϳͻ���״̬Ӧ��[intf=�Ͱ汾���� len=12(C-0)]");
            n = cb->my_write(devid, client_socket, buff, 12);
            if (n < 0)
            {
                string str_error_x = "write error: ";
                str_error_x += strerror(errno);
                cb->ERR_print(devid, str_error_x);
            }
            cb->ENV_print(devid, "�Ͱ汾");
            close(client_socket); // server�����ر�
            break;
        }
        else if (time_check(svr_time) == false) //��ʾ������֤����ڡ����ر�����
        {
            cb->ENV_print(devid, "����֤�����");
            close(client_socket);
            break;
        }
        else if (identify_check(random_num, identify_series) == false) //��֤����ƥ����ʾ����֤�Ƿ������ر�����
        {
            cb->ENV_print(devid, "��֤�Ƿ�����֤����ƥ��");
            close(client_socket);
            break;
        }
        else
        {
            ;
        }

        string str_idenx = "��֤������: " ;
        str_idenx += (char*)identify_series;
        cb->RDATA_print(devid, str_idenx);

        /*�豸�������������֤��������������Ϣ*/
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

        cb->SDATA_print(devid, "���Ϳͻ���״̬Ӧ��(����ǰ)[intf=��֤��Ϣ len=116(C-0)]");
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
        
        cb->SDATA_print(devid, "���Ϳͻ���״̬Ӧ��(���ܺ�)[intf=��֤��Ϣ len=116(C-0)]");
        n = cb->my_write(devid, client_socket, buff, 116);
        if (n < 0)
        {
            string str_error_x = "write error: ";
                str_error_x += strerror(errno);
                cb->ERR_print(devid, str_error_x);
            break;
        }

        /*��ʼ������ʽ������Ϣ��ѭ����ע��Ͽ��������ɷ����������������豸*/
        while (true)
        {
            /*���������豸���͸���ȡ��Ϣ�������������Ϊ8�ֽ�*/
            n = cb->my_read(devid, client_socket, buff, 8);
            if (n < 0)
            {
                string str_error_x = "read error: ";
                str_error_x += strerror(errno);
                cb->ERR_print(devid, str_error_x);
                break;
            }

            unsigned char inst = (unsigned char)(buff[1]);
            if (inst == 0x02) //ȡϵͳ��Ϣ
            {
                cb->RDATA_print(devid, "�յ��ͻ���״̬����[intf=ϵͳ��Ϣ]");
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
                cb->SDATA_print(devid, "���Ϳͻ���״̬Ӧ��[intf=ϵͳ��Ϣ len=28(C-0)]");
                n = cb->my_write(devid, client_socket, buff, 28);
                if (n < 0)
                {
                    string str_error_x = "write error: ";
                str_error_x += strerror(errno);
                cb->ERR_print(devid, str_error_x);
                    break;
                }
            }
            else if (inst == 0x03) //ȡ������Ϣ
            {
                cb->RDATA_print(devid, "�յ��ͻ���״̬����[intf=������Ϣ]");
                //��ȡ�ļ�ȫ������
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
                cb->SDATA_print(devid, "���Ϳͻ���״̬Ӧ��[intf=������Ϣ len="+to_string(data_len + 8)+"(C-0)]");
                n = cb->my_write(devid, client_socket, buff, data_len + 8);
                if (n < 0)
                {
                    string str_error_x = "write error: ";
                    str_error_x += strerror(errno);
                    cb->ERR_print(devid, str_error_x);
                    break;
                }
            }
            else if (inst == 0x04) //ȡ������Ϣ
            {
                cb->RDATA_print(devid, "�յ��ͻ���״̬����[intf=������Ϣ]");
                //��ȡ�ļ�ȫ������
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
                cb->SDATA_print(devid, "���Ϳͻ���״̬Ӧ��[intf=������Ϣ len="+to_string(data_len + 8)+"(C-0)]");
                n = cb->my_write(devid, client_socket, buff, data_len + 8);
                if (n < 0)
                {
                    string str_error_x = "write error: ";
                str_error_x += strerror(errno);
                cb->ERR_print(devid, str_error_x);
                    break;
                }
            }
            else if (inst == 0x05) //ȡ��̫����Ϣ
            {
                unsigned short ethernet;
                ethernet = read_context2(buff + 4);
                cb->RDATA_print(devid, "�յ��ͻ���״̬����[intf=Ethernet" + to_string(ethernet) + "����Ϣ]");
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
                cb->SDATA_print(devid, "���Ϳͻ���״̬Ӧ��[intf=Ethernet" + to_string(ethernet) + "����Ϣ len=132(C-0)]");
                n = cb->my_write(devid, client_socket, buff, 132);
                if (n < 0)
                {
                    string str_error_x = "write error: ";
                str_error_x += strerror(errno);
                cb->ERR_print(devid, str_error_x);
                    break;
                }
            }
            else if (inst == 0x07) //ȡusb����Ϣ
            {
                cb->RDATA_print(devid, "�յ��ͻ���״̬����[intf=usb����Ϣ]");
                memset(buff, 0, BUF_SIZE);
                fill_head(buff, 0x91, 0x07, 12, 0x0000, 4);
                buff[8] = (unsigned char)((devid % 10) % 2);
                cb->SDATA_print(devid, "���Ϳͻ���״̬Ӧ��[intf=usb����Ϣ len=12(C-0)]");
                n = cb->my_write(devid, client_socket, buff, 12);
                if (n < 0)
                {
                    string str_error_x = "write error: ";
                    str_error_x += strerror(errno);
                    cb->ERR_print(devid, str_error_x);
                    break;
                }
            }
            else if (inst == 0x0c) //ȡu�����ļ��б���Ϣ
            {
                cb->RDATA_print(devid, "�յ��ͻ���״̬����[intf=u�����ļ��б���Ϣ]");
                //��ȡ�ļ�ȫ������
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
                cb->SDATA_print(devid, "���Ϳͻ���״̬Ӧ��[intf=u�����ļ��б���Ϣ len="+to_string(data_len + 8)+"(C-0)]");
                n = cb->my_write(devid, client_socket, buff, data_len + 8);
                if (n < 0)
                {
                    string str_error_x = "write error: ";
                str_error_x += strerror(errno);
                cb->ERR_print(devid, str_error_x);
                    break;
                }
            }
            else if (inst == 0x08) //ȡ��ӡ����Ϣ
            {
                cb->RDATA_print(devid, "�յ��ͻ���״̬����[intf=��ӡ����Ϣ]");
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
                cb->SDATA_print(devid, "���Ϳͻ���״̬Ӧ��[intf=��ӡ����Ϣ len=44(C-0)]");
                n = cb->my_write(devid, client_socket, buff, 44);
                if (n < 0)
                {
                    string str_error_x = "write error: ";
                str_error_x += strerror(errno);
                cb->ERR_print(devid, str_error_x);
                    break;
                }
            }
            else if (inst == 0x0d) //ȡ��ӡ������Ϣ
            {
                cb->RDATA_print(devid, "�յ��ͻ���״̬����[intf=��ӡ������Ϣ]");
                memset(buff, 0, BUF_SIZE);
                fill_head(buff, 0x91, 0x0d, 9, 0x0000, 1);
                buff[8] = 0;
                cb->SDATA_print(devid, "���Ϳͻ���״̬Ӧ��[intf=��ӡ������Ϣ len=9(C-0)]");
                n = cb->my_write(devid, client_socket, buff, 9);
                if (n < 0)
                {
                    string str_error_x = "write error: ";
                str_error_x += strerror(errno);
                cb->ERR_print(devid, str_error_x);
                    break;
                }
            }
            else if (inst == 0x09) //ȡ�ն˷�����Ϣ
            {
                cb->RDATA_print(devid, "�յ��ͻ���״̬����[intf=TServer������Ϣ]");
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
                cb->SDATA_print(devid, "���Ϳͻ���״̬Ӧ��[intf=TServer������Ϣ len=280(C-0)]");
                n = cb->my_write(devid, client_socket, buff, 280);
                if (n < 0)
                {
                    string str_error_x = "write error: ";
                    str_error_x += strerror(errno);
                    cb->ERR_print(devid, str_error_x);
                    break;
                }
            }
            else if (inst == 0x0a || inst == 0x0b) //ȡ���ն�/IP�ն�������Ϣ����Ӧ������������Ϣ
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
                    cb->RDATA_print(devid, "�յ��ͻ���״̬����[intf=�����ն�������Ϣ]");
                    tty_type_str = "�����ն�";
                    fill_context(buff, 12, 0, 4);
                }
                else // IP : ������ظ�
                {
                    cb->RDATA_print(devid, "�յ��ͻ���״̬����[intf=IP�ն�������Ϣ]");
                    tty_type_str = "IP�ն�";
                    fill_context(buff, 12, ((unsigned int)10 * 255 * 255 * 255 + 80 * 255 * 255 + 23 * 255 + (unsigned int)tty_no), 4);
                }

                cb->SDATA_print(devid, "�ն�����: " + tty_type_str);

                memcpy(buff + 16, tty_type_str.c_str(), sizeof(tty_type_str) - 1);
                if (rand() % 2 == 1)
                {
                    memcpy(buff + 28, "����", sizeof("����"));
                    cb->SDATA_print(devid, tty_type_str + ",�ն����" + to_string(tty_no) + "״̬: ����");
                }
                else
                {
                    memcpy(buff + 28, "�˵�", sizeof("�˵�"));
                    cb->SDATA_print(devid, tty_type_str + ",�ն����" + to_string(tty_no) + "״̬: �˵�");
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
                        memcpy(temp_buf + 20, "����", sizeof("����"));
                        cb->SDATA_print(devid, tty_type_str + ",�ն����" + to_string(tty_no) + ",����" + to_string(i) + "״̬: ����");
                    }
                    else
                    {
                        memcpy(temp_buf + 20, "�ػ�", sizeof("�ػ�"));
                        cb->SDATA_print(devid, tty_type_str + ",�ն����" + to_string(tty_no) + ",����" + to_string(i) + "״̬: �ػ�");
                    }
                    memcpy(temp_buf + 28, "����ϵͳ", sizeof("����ϵͳ"));
                    cb->SDATA_print(devid, tty_type_str + ",�ն����" + to_string(tty_no) + ",����" + to_string(i) + "��ʾ��: ����ϵͳ");
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
                cb->SDATA_print(devid, "���Ϳͻ���״̬Ӧ��[intf=" + tty_type_str + " len=" + to_string(96 * screen_num + 36) + "(C-0)]");
                n = cb->my_write(devid, client_socket, buff, 96 * screen_num + 36);
                if (n < 0)
                {
                    string str_error_x = "write error: ";
                    str_error_x += strerror(errno);
                    cb->ERR_print(devid, str_error_x);
                    break;
                }
            }
            else if (inst == 0xff) //�˳�
            {
                cb->RDATA_print(devid, "�յ��ͻ���״̬����[intf=���ν������]");
                memset(buff, 0, BUF_SIZE);
                fill_head(buff, 0x91, 0xff, 8, 0x0000, 0);
                cb->ENV_print(devid, "���ν������");
                n = cb->my_write(devid, client_socket, buff, 8);
                break;
                /*n = cb->my_read(devid, client_socket, buff, 8);
                if (n < 0) //�ر�tcp����
                {
                    break;
                }*/
            }
        }
        close(client_socket);
        if (cb->exit_after_success == 1) //�����˳�
        {
            break;
        }
        else //���һ��ʱ����ٴ����ӷ�����
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

    pnc.set_name_type(0, "������IP��ַ", "string");

    pnc.set_name_type(1, "�˿ں�", "int");
    pnc.set_default(1, 43072, "");

    pnc.set_name_type(2, "���̽��ճɹ����˳�", "int");

    pnc.set_name_type(3, "��С�����ն�����", "int");
    pnc.set_default(3, 5, "");
    pnc.set_range(3, 1, 3, 10);

    pnc.set_name_type(4, "��������ն�����", "int");
    pnc.set_default(4, 28, "");
    pnc.set_range(4, 1, 10, 50);

    pnc.set_name_type(5, "ÿ���ն���С��������", "int");
    pnc.set_default(5, 3, "");
    pnc.set_range(5, 1, 1, 3);

    pnc.set_name_type(6, "ÿ���ն������������", "int");
    pnc.set_default(6, 10, "");
    pnc.set_range(6, 1, 4, 16);

    pnc.set_name_type(7, "ɾ����־�ļ�", "int");

    pnc.set_name_type(8, "DEBUG����", "string");
    pnc.set_name_type(9, "DEBUG��Ļ��ʾ", "int");
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

    if(cb->del_log == 1)//ɾ��ԭ����������־
    {
        cb->fs.open(cb->log_path, ios::in|ios::out|ios::trunc);
    }
    else//��ԭ����־ĩβ��ʼд
    {
        cb->fs.open(cb->log_path, ios::in|ios::out|ios::app);
    }
    if(cb->fs.is_open() == 0)
    {
        cb->ERR_print(100000, "ts.log��ʧ��");
        return -1;
    }

    cb->fs_count.open(cb->count_path, ios::in|ios::out|ios::trunc);
    if(cb->fs_count.is_open() == 0)
    {
        cb->ERR_print(100000, "ts_count.xls��ʧ��");
        return -1;
    }

    cb->ENV_print(100000, "fork�ӽ���,��ʼ��ʱ");

    const int process_max_num = 210;
    const int process_min_num = 120;
    int sum = 0;
    /*�������м�¼��ʼ�ͽ���ʱ�䣬�������ӽ��̽����Ժ���˳�����¼��ʱ������־��*/
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
            cb->ERR_print(100000, "�����̷���ʧ�ܣ�");
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
                    wait(&status); //��ʱ���գ�������������
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

    cb->ENV_print(100000, "���������ӽ��̻������");
    string str_x1 = "time use:" + to_string(deltaT) + "s";
    cb->ENV_print(100000, str_x1);
    str_x1 = "process sum:" + to_string(devnum);
    cb->ENV_print(100000, str_x1);

    cout << "time use:" << deltaT << "s"<<endl;

    // deleteʱ�����ӽ����Ѿ�����
    cb->fs.close();
    cb->fs_count.close();
    delete cb;
    return 0;
}