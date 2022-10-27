#pragma once
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
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/epoll.h>
using namespace std;


/*优秀的处理方法！！！！*/
/*尝试使用宏定义来对统一的出错信息进行处理*/
#define PRINT_ERR(statement, errinfo)     \
    if ((statement) < 0 && !ignore_error) \
        printf("%s error: %s(errno:%d)\n", errinfo, strerror(errno), errno);
#define IGNORE_ERR(statement) ignore_error = true, statement, ignore_error = false
extern bool ignore_error;
/*封装一组socket类，作为socket编程的统一使用*/

/*统一宏定义*/
const int BUFSIZE = 10020;

/*基础类Socket_Base，用于公共部分，读写缓冲区的设置，同步/异步方式的设置等*/
class Socket_Base
{
public:
    /*socket相关基础数据*/
    int server_socket;              //服务端套接字
    int client_socket;              //客户端套接字
    struct sockaddr_in servaddr;    //服务器网络地址
    struct sockaddr_in remote_addr; ///客户端地址
    char buff[BUFSIZE];             //数据传送缓冲区
    // int n;
    socklen_t addr_size; //结构体长度，accept函数中该参数类型为socklen_t *

    /*端口号及相应IP设置*/
    int port;   //端口号
    string ip;  // ip地址
    int myport; // client端

    /*收发缓冲区大小设置*/
    int getrbuf;    //打印TCP收缓冲区原始大小
    int getwbuf;    //打印TCP发缓冲区原始大小
    int setrbuf;    //TCP收缓冲区大小设置
    int setwbuf;    //TCP发缓冲区大小设置
    int getnrbuf;   //打印TCP收缓冲区设置后大小
    int getnwbuf;   //打印TCP发缓冲区设置后大小

    /*构造与析构*/
    Socket_Base();

    /*创建套接字*/
    int create_server_socket();
    int create_client_socket();

    /*读写缓冲区的设置*/
    void set_TCP_buf(int &socket1);

    /*socket同步(阻塞)/异步(非阻塞)方式的设置*/
    int server_socket_NONBLOCK(int whe_block); // 0：同步，1：异步
    int client_socket_NONBLOCK(int whe_block); // 0：同步，1：异步

    /*起始填充*/
    void fill_ip_port(int port1,string ip1,int myport1=-1);

    /*epoll相关*/
    int set_epoll(int epfd, int op, unsigned int event_in, int fd);
};

/*扩展类Socket_Listen，用于server端bind的socket的后续处理*/
class Socket_Listen : public Socket_Base
{
public:
    int bind_server_socket();
    int listen_server_socket();

};

/*扩展类Socket_Accept，用于server端accept的socket的后续处理*/
class Socket_Accept : public Socket_Listen
{
public:
    int accept_server_socket();

    /*统一初始化，S端*/
    int init_server_socket();
};

/*扩展类Socket_Connect，用于client端socket的后续处理*/
class Socket_Connect : public Socket_Base
{
public:
    int bind_client_socket();
    int connect_client_socket();

    /*统一初始化，C端*/
    int init_client_socket();
};

/*以上为TCP相关socket类，至此结束*/
/*以下为后续再被UDP继承的情况，在后续过程中考虑*/
