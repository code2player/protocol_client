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

/*PRINT_ERR��ʹ�ã������ж��Ƿ���Դ�����error_ignore�У�*/
bool ignore_error = false;

/*����������*/
Socket_Base::Socket_Base()
{

}

/*�����׽���*/
int Socket_Base::create_server_socket()
{
    memset(&servaddr, 0, sizeof(servaddr));           //�ṹ���ʼ������
    servaddr.sin_family = AF_INET;                    // IPv4
    servaddr.sin_addr.s_addr = inet_addr(ip.c_str()); // INADDR_ANY����0.0.0.0
    servaddr.sin_port = htons(port);

    addr_size = sizeof(struct sockaddr_in);

    //����������׽���
    // 1. create socket
    if ((server_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("����socketʧ��, %s(errno: %d)\n", strerror(errno), errno);
        //return -1;
        exit(1);
    }

    //ʹ��SO_REUSEADDR���Զ˿ڽ�����������ʹ��
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

    //�����ͻ����׽���
    if ((client_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("����socketʧ��, %s(errno: %d)\n", strerror(errno), errno);
        exit(1);
    }
    return 0;
}

/*��д������������*/
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

/*socketͬ��(����)/�첽(������)��ʽ������*/
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

/*epoll��ͳһ���ú�������*/
int Socket_Base::set_epoll(int epfd, int op, unsigned int event_in, int fd)
{
    struct epoll_event ev;
    ev.events = event_in; //��д
    ev.data.fd = fd;
    int ret = epoll_ctl(epfd, op, fd, &ev);
    PRINT_ERR(ret, "epoll_ctl()");
    return ret;
    // type == 1 ? EPOLL_CTL_ADD : EPOLL_CTL_DEL
}

/*ͳһ��ʼ����S��*/
int Socket_Accept::init_server_socket()
{
    create_server_socket();
    bind_server_socket();
    listen_server_socket();
    return 0;
}
/*ͳһ��ʼ����C��*/
int Socket_Connect::init_client_socket()
{
    create_client_socket();
    bind_client_socket();
    connect_client_socket();
    return 0;
}

int Socket_Listen::bind_server_socket()
{
        //���׽��ֵ�ip�Ͷ˿�
    // 2.bind socket & port to server
    if (bind(server_socket, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0)
    {
        printf("bind��socket�Ͷ˿�ʧ��, %s(errno: %d)\n", strerror(errno), errno);
        exit(1);
    }

    cout << "�Ѱ�" << ip << "��" << port << "�˿�" << endl;
    return 0;
}
int Socket_Listen::listen_server_socket()
{
    //�����������󣬼������г���Ϊ10
    // 3.listen
    if (listen(server_socket, 10) < 0)
    {
        printf("����������Ч, %s(errno: %d)\n", strerror(errno), errno);
        exit(1);
    }

    printf("����ȴ�����״̬......\n");
    return 0;
}

int Socket_Accept::accept_server_socket()
{
        //�ȴ��ͻ����������󵽴�
    // 4.accept
    if ((client_socket = accept(server_socket, (struct sockaddr *)&remote_addr, &addr_size)) < 0)
    {
        printf("��������ʧ��, %s(errno: %d)\n", strerror(errno), errno);
        exit(1);
    }

    cout << "��client�˽�������, ip:" << inet_ntoa(remote_addr.sin_addr) << " port:" << htons(remote_addr.sin_port) << endl;
    return 0;
}

int Socket_Connect::bind_client_socket()
{
    // bind��ָ�������˿ںţ�һ�㲻��
    struct sockaddr_in client_addr; // client�������ַ
    memset(&client_addr, 0, sizeof(client_addr));
    client_addr.sin_family = AF_INET;
    client_addr.sin_port = htons(myport);
    client_addr.sin_addr.s_addr = INADDR_ANY;
    if (myport != -1)
    {
        if (bind(client_socket, (struct sockaddr *)&client_addr, sizeof(client_addr)) < 0)
        {
            printf("bind��socket�Ͷ˿�ʧ��, %s(errno: %d)\n", strerror(errno), errno);
            return -1;
        }
    }
    return 0;
}
int Socket_Connect::connect_client_socket()
{
    //���ӷ�����
    if (connect(client_socket, (struct sockaddr *)&remote_addr, sizeof(remote_addr)) < 0)
    {
        printf("����server��ʧ��, %s(errno: %d)\n", strerror(errno), errno);
        //return -1;
        exit(1);
    }

    printf("�����ӵ�server��\n");
    return 0;
}
