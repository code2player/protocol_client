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
using namespace std;

int init_server(int &port,string &ip,int &server_socket,int &client_socket,struct sockaddr_in servaddr,struct sockaddr_in &remote_addr,char buff[BUFSIZ],socklen_t &addr_size, int noaccept)
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
        return -1;
    }

    //使用SO_REUSEADDR来对端口进行立即重新使用
    int optval = 1;
    setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));

    //使用SO_RCVLOWAT来设置接受低潮限度
   /* int optRCVLOWAT = 20;
    setsockopt(server_socket, SOL_SOCKET, SO_RCVLOWAT, &optRCVLOWAT, sizeof(optRCVLOWAT));*/

    //绑定套接字到ip和端口
    // 2.bind socket & port to server
    if (bind(server_socket, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0)
    {
        printf("bind绑定socket和端口失败, %s(errno: %d)\n", strerror(errno), errno);
        return -1;
    }

    cout << "已绑定" << ip << "的" << port << "端口" << endl;

    //监听连接请求，监听队列长度为10
    // 3.listen
    if (listen(server_socket, 10) < 0)
    {
        printf("监听请求无效, %s(errno: %d)\n", strerror(errno), errno);
        return -1;
    }

    printf("进入等待连接状态......\n");

    /*在000113后使用，noaccept用来禁止accept的自动接受*/
    if(noaccept==1)
    return 0;

    //等待客户端连接请求到达
    // 4.accept
    if ((client_socket = accept(server_socket, (struct sockaddr *)&remote_addr, &addr_size)) < 0)
    {
        printf("建立连接失败, %s(errno: %d)", strerror(errno), errno);
        return -1;
        // continue;
    }

    cout << "与client端建立连接, ip:" << inet_ntoa(remote_addr.sin_addr) << " port:" << htons(remote_addr.sin_port) << endl;
    return 0;
}

int init_client(int &port,string &ip,int &myport,int &client_socket,char buff[BUFSIZ],struct sockaddr_in &remote_addr)
{
    memset(&remote_addr, 0, sizeof(remote_addr));
    remote_addr.sin_family = AF_INET;
    remote_addr.sin_port = htons(port);
    remote_addr.sin_addr.s_addr = inet_addr(ip.c_str());

    //创建客户端套接字
    if ((client_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("创建socket失败, %s(errno: %d)\n", strerror(errno), errno);
        return -1;
    }

    /*cout<<"port: "<<port<<endl;
    cout<<"ip: "<<ip<<endl;
    cout<<"myport: "<<myport<<endl;*/

    // bind，指定本机端口号，一般不用
    struct sockaddr_in client_addr; // client绑定网络地址
    memset(&client_addr, 0, sizeof(client_addr));
    client_addr.sin_family = AF_INET;
    client_addr.sin_port = htons(myport);
    client_addr.sin_addr.s_addr = INADDR_ANY;
    if (myport != 0)
    {
        if (bind(client_socket, (struct sockaddr *)&client_addr, sizeof(client_addr)) < 0)
        {
            printf("bind绑定socket和端口失败, %s(errno: %d)\n", strerror(errno), errno);
            return -1;
        }
    }

    //连接服务器
    if (connect(client_socket, (struct sockaddr *)&remote_addr, sizeof(remote_addr)) < 0)
    {
        printf("连接server端失败, %s(errno: %d)\n", strerror(errno), errno);
        return -1;
    }

    printf("已连接到server端\n");
    return 0;
}

int get_local_ip(string ip1)
{
    int result=0;//返回值，ip存在则为1，不存在返回0
    char *ip;
    //memset(ip, 0, sizeof(ip));
    //char dev[4]="ens";

    int fd, intrface;
    struct ifreq buf[INET_ADDRSTRLEN];  //这个结构定义在/usr/include/net/if.h，用来配置和获取ip地址，掩码，MTU等接口信息的
    struct ifconf ifc;
 
	/*1 建立socket链接，利用ioctl来自动扫描可用网卡*/
    if ((fd = socket(AF_INET, SOCK_DGRAM, 0)) >= 0)
    {
 
        ifc.ifc_len = sizeof(buf);
 
        // caddr_t,linux内核源码里定义的：typedef void *caddr_t；
        ifc.ifc_buf = (caddr_t)buf;
 
        if (!ioctl(fd, SIOCGIFCONF, (char *)&ifc))  /*2  这里获取到可用网卡清单，包括网卡ip地址，mac地址*/
        {
            intrface = ifc.ifc_len/sizeof(struct ifreq);  //计算出有效网卡的数量//  
            while (intrface-- > 0)
             {
                if (!(ioctl(fd, SIOCGIFADDR, (char *)&buf[intrface])))  /*3  遍历并索引指定网卡的地址*/
                {
						ip=(inet_ntoa(((struct sockaddr_in*)(&buf[intrface].ifr_addr))->sin_addr));
						printf("IP:%s\n", ip);
                        string str=ip;
                        if(str==ip1)
                            result = 1;
                }
             }
        }

        close(fd);
        if (ip1 == "0.0.0.0")
            result = 1;
        return result;
    }
    return -1;
}

void tcp_buf(int &server_socket, int getrbuf, int getwbuf, int setrbuf, int setwbuf, int getnrbuf, int getnwbuf)
{
    int optval;
    socklen_t len = sizeof(optval);
    if (getrbuf == 1)
    {
        getsockopt(server_socket, SOL_SOCKET, SO_RCVBUF, &optval, &len);
        cout << "getrbuf: " << optval << endl;
    }
    if (getwbuf == 1)
    {
        getsockopt(server_socket, SOL_SOCKET, SO_SNDBUF, &optval, &len);
        cout << "getwbuf: " << optval << endl;
    }

    optval = setrbuf;
    setsockopt(server_socket, SOL_SOCKET, SO_RCVBUF, &optval, sizeof(optval));
    cout << "setrbuf: " << optval << endl;
    optval = setwbuf;
    setsockopt(server_socket, SOL_SOCKET, SO_SNDBUF, &optval, sizeof(optval));
    cout << "setwbuf: " << optval << endl;

    if (getnrbuf == 1)
    {
        getsockopt(server_socket, SOL_SOCKET, SO_RCVBUF, &optval, &len);
        cout << "getnrbuf: " << optval << endl;
    }
    if (getnwbuf == 1)
    {
        getsockopt(server_socket, SOL_SOCKET, SO_SNDBUF, &optval, &len);
        cout << "getnwbuf: " << optval << endl;
    }
}

int select_set(int sock)
{
	fd_set readset;//文件集合
	FD_ZERO(&readset);//set初始化为空
	FD_SET(sock, &readset);//向set中添加socket

    /*select:无限时间非阻塞，time设为0*/
    /*timeval tout;
    tout.tv_sec = 0;
    tout.tv_usec = 0;*/

    int ret = 0;
    if ((ret = select((sock + 1), &readset, 0, 0, NULL)) <= 0)
    {
        cout << "select return: " << ret << endl;
        printf("select error: %s(errno:%d)\n", strerror(errno), errno);
    }
    else
    {
        cout << "select return: " << ret << endl;
        printf("select error: %s(errno:%d)\n", strerror(errno), errno);
    }
    return ret;
}
