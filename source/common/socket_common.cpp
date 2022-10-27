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
#include <sys/ioctl.h>
#include <net/if.h>
#include "../include/socket_common.h"
using namespace std;

/*PRINT_ERR中使用，用来判断是否忽略错误（在error_ignore中）*/
bool ignore_error = false;

/*构造与析构*/
Socket_Base::Socket_Base()
{

}

/*创建套接字*/
int Socket_Base::create_server_socket()
{
    memset(&servaddr, 0, sizeof(servaddr));           //结构体初始化清零
    servaddr.sin_family = AF_INET;                    // IPv4
    servaddr.sin_addr.s_addr = inet_addr(ip.c_str()); // INADDR_ANY代表0.0.0.0
    servaddr.sin_port = htons(port);

    addr_size = sizeof(struct sockaddr_in);

    //创建服务端套接字
    // 1. create socket
    if ((server_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("创建socket失败, %s(errno: %d)\n", strerror(errno), errno);
        //return -1;
        exit(1);
    }

    //使用SO_REUSEADDR来对端口进行立即重新使用
    int optval = 1;
    setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));
    return 0;
}
int Socket_Base::create_client_socket()
{
    memset(&remote_addr, 0, sizeof(remote_addr));
    remote_addr.sin_family = AF_INET;
    remote_addr.sin_port = htons(port);
    remote_addr.sin_addr.s_addr = inet_addr(ip.c_str());

    //创建客户端套接字
    if ((client_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("创建socket失败, %s(errno: %d)\n", strerror(errno), errno);
        exit(1);
    }
    return 0;
}

/*读写缓冲区的设置*/
void Socket_Base::set_TCP_buf(int &socket1)
{
    int optval;
    socklen_t len = sizeof(optval);
    if (getrbuf == 1)
    {
        getsockopt(socket1, SOL_SOCKET, SO_RCVBUF, &optval, &len);
        cout << "getrbuf: " << optval << endl;
    }
    if (getwbuf == 1)
    {
        getsockopt(socket1, SOL_SOCKET, SO_SNDBUF, &optval, &len);
        cout << "getwbuf: " << optval << endl;
    }

    optval = setrbuf;
    setsockopt(socket1, SOL_SOCKET, SO_RCVBUF, &optval, sizeof(optval));
    cout << "setrbuf: " << optval << endl;
    optval = setwbuf;
    setsockopt(socket1, SOL_SOCKET, SO_SNDBUF, &optval, sizeof(optval));
    cout << "setwbuf: " << optval << endl;

    if (getnrbuf == 1)
    {
        getsockopt(socket1, SOL_SOCKET, SO_RCVBUF, &optval, &len);
        cout << "getnrbuf: " << optval << endl;
    }
    if (getnwbuf == 1)
    {
        getsockopt(socket1, SOL_SOCKET, SO_SNDBUF, &optval, &len);
        cout << "getnwbuf: " << optval << endl;
    }
}

/*socket同步(阻塞)/异步(非阻塞)方式的设置*/
int Socket_Base::server_socket_NONBLOCK(int whe_block)
{
    if (whe_block == 1)
    {
        int flags = fcntl(server_socket, F_GETFL, 0);
        fcntl(server_socket, F_SETFL, flags | O_NONBLOCK);
    }
    else
    {
        int flags = fcntl(server_socket, F_GETFL, 0);
        fcntl(server_socket, F_SETFL, flags & ~O_NONBLOCK);
    }
    return 0;
}
int Socket_Base::client_socket_NONBLOCK(int whe_block)
{
    if (whe_block == 1)
    {
        int flags = fcntl(client_socket, F_GETFL, 0);
        fcntl(client_socket, F_SETFL, flags | O_NONBLOCK);
    }
    else
    {
        int flags = fcntl(client_socket, F_GETFL, 0);
        fcntl(client_socket, F_SETFL, flags & ~O_NONBLOCK);
    }
    return 0;
}

void Socket_Base::fill_ip_port(int port1, string ip1, int myport1)
{
    port = port1;
    ip = ip1;
    myport = myport1;
}

/*epoll的统一设置函数，有*/
int Socket_Base::set_epoll(int epfd, int op, unsigned int event_in, int fd)
{
    struct epoll_event ev;
    ev.events = event_in; //可写
    ev.data.fd = fd;
    int ret = epoll_ctl(epfd, op, fd, &ev);
    PRINT_ERR(ret, "epoll_ctl()");
    return ret;
    // type == 1 ? EPOLL_CTL_ADD : EPOLL_CTL_DEL
}

/*统一初始化，S端*/
int Socket_Accept::init_server_socket()
{
    create_server_socket();
    bind_server_socket();
    listen_server_socket();
    return 0;
}
/*统一初始化，C端*/
int Socket_Connect::init_client_socket()
{
    create_client_socket();
    bind_client_socket();
    connect_client_socket();
    return 0;
}

int Socket_Listen::bind_server_socket()
{
        //绑定套接字到ip和端口
    // 2.bind socket & port to server
    if (bind(server_socket, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0)
    {
        printf("bind绑定socket和端口失败, %s(errno: %d)\n", strerror(errno), errno);
        exit(1);
    }

    cout << "已绑定" << ip << "的" << port << "端口" << endl;
    return 0;
}
int Socket_Listen::listen_server_socket()
{
    //监听连接请求，监听队列长度为10
    // 3.listen
    if (listen(server_socket, 10) < 0)
    {
        printf("监听请求无效, %s(errno: %d)\n", strerror(errno), errno);
        exit(1);
    }

    printf("进入等待连接状态......\n");
    return 0;
}

int Socket_Accept::accept_server_socket()
{
        //等待客户端连接请求到达
    // 4.accept
    if ((client_socket = accept(server_socket, (struct sockaddr *)&remote_addr, &addr_size)) < 0)
    {
        printf("建立连接失败, %s(errno: %d)\n", strerror(errno), errno);
        exit(1);
    }

    cout << "与client端建立连接, ip:" << inet_ntoa(remote_addr.sin_addr) << " port:" << htons(remote_addr.sin_port) << endl;
    return 0;
}

int Socket_Connect::bind_client_socket()
{
    // bind，指定本机端口号，一般不用
    struct sockaddr_in client_addr; // client绑定网络地址
    memset(&client_addr, 0, sizeof(client_addr));
    client_addr.sin_family = AF_INET;
    client_addr.sin_port = htons(myport);
    client_addr.sin_addr.s_addr = INADDR_ANY;
    if (myport != -1)
    {
        if (bind(client_socket, (struct sockaddr *)&client_addr, sizeof(client_addr)) < 0)
        {
            printf("bind绑定socket和端口失败, %s(errno: %d)\n", strerror(errno), errno);
            return -1;
        }
    }
    return 0;
}
int Socket_Connect::connect_client_socket()
{
    //连接服务器
    if (connect(client_socket, (struct sockaddr *)&remote_addr, sizeof(remote_addr)) < 0)
    {
        printf("连接server端失败, %s(errno: %d)\n", strerror(errno), errno);
        //return -1;
        exit(1);
    }

    printf("已连接到server端\n");
    return 0;
}
