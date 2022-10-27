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


/*����Ĵ�������������*/
/*����ʹ�ú궨������ͳһ�ĳ�����Ϣ���д���*/
#define PRINT_ERR(statement, errinfo)     \
    if ((statement) < 0 && !ignore_error) \
        printf("%s error: %s(errno:%d)\n", errinfo, strerror(errno), errno);
#define IGNORE_ERR(statement) ignore_error = true, statement, ignore_error = false
extern bool ignore_error;
/*��װһ��socket�࣬��Ϊsocket��̵�ͳһʹ��*/

/*ͳһ�궨��*/
const int BUFSIZE = 10020;

/*������Socket_Base�����ڹ������֣���д�����������ã�ͬ��/�첽��ʽ�����õ�*/
class Socket_Base
{
public:
    /*socket��ػ�������*/
    int server_socket;              //������׽���
    int client_socket;              //�ͻ����׽���
    struct sockaddr_in servaddr;    //�����������ַ
    struct sockaddr_in remote_addr; ///�ͻ��˵�ַ
    char buff[BUFSIZE];             //���ݴ��ͻ�����
    // int n;
    socklen_t addr_size; //�ṹ�峤�ȣ�accept�����иò�������Ϊsocklen_t *

    /*�˿ںż���ӦIP����*/
    int port;   //�˿ں�
    string ip;  // ip��ַ
    int myport; // client��

    /*�շ���������С����*/
    int getrbuf;    //��ӡTCP�ջ�����ԭʼ��С
    int getwbuf;    //��ӡTCP��������ԭʼ��С
    int setrbuf;    //TCP�ջ�������С����
    int setwbuf;    //TCP����������С����
    int getnrbuf;   //��ӡTCP�ջ��������ú��С
    int getnwbuf;   //��ӡTCP�����������ú��С

    /*����������*/
    Socket_Base();

    /*�����׽���*/
    int create_server_socket();
    int create_client_socket();

    /*��д������������*/
    void set_TCP_buf(int &socket1);

    /*socketͬ��(����)/�첽(������)��ʽ������*/
    int server_socket_NONBLOCK(int whe_block); // 0��ͬ����1���첽
    int client_socket_NONBLOCK(int whe_block); // 0��ͬ����1���첽

    /*��ʼ���*/
    void fill_ip_port(int port1,string ip1,int myport1=-1);

    /*epoll���*/
    int set_epoll(int epfd, int op, unsigned int event_in, int fd);
};

/*��չ��Socket_Listen������server��bind��socket�ĺ�������*/
class Socket_Listen : public Socket_Base
{
public:
    int bind_server_socket();
    int listen_server_socket();

};

/*��չ��Socket_Accept������server��accept��socket�ĺ�������*/
class Socket_Accept : public Socket_Listen
{
public:
    int accept_server_socket();

    /*ͳһ��ʼ����S��*/
    int init_server_socket();
};

/*��չ��Socket_Connect������client��socket�ĺ�������*/
class Socket_Connect : public Socket_Base
{
public:
    int bind_client_socket();
    int connect_client_socket();

    /*ͳһ��ʼ����C��*/
    int init_client_socket();
};

/*����ΪTCP���socket�࣬���˽���*/
/*����Ϊ�����ٱ�UDP�̳е�������ں��������п���*/
