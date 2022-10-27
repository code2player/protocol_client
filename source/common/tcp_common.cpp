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
        return -1;
    }

    //ʹ��SO_REUSEADDR���Զ˿ڽ�����������ʹ��
    int optval = 1;
    setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));

    //ʹ��SO_RCVLOWAT�����ý��ܵͳ��޶�
   /* int optRCVLOWAT = 20;
    setsockopt(server_socket, SOL_SOCKET, SO_RCVLOWAT, &optRCVLOWAT, sizeof(optRCVLOWAT));*/

    //���׽��ֵ�ip�Ͷ˿�
    // 2.bind socket & port to server
    if (bind(server_socket, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0)
    {
        printf("bind��socket�Ͷ˿�ʧ��, %s(errno: %d)\n", strerror(errno), errno);
        return -1;
    }

    cout << "�Ѱ�" << ip << "��" << port << "�˿�" << endl;

    //�����������󣬼������г���Ϊ10
    // 3.listen
    if (listen(server_socket, 10) < 0)
    {
        printf("����������Ч, %s(errno: %d)\n", strerror(errno), errno);
        return -1;
    }

    printf("����ȴ�����״̬......\n");

    /*��000113��ʹ�ã�noaccept������ֹaccept���Զ�����*/
    if(noaccept==1)
    return 0;

    //�ȴ��ͻ����������󵽴�
    // 4.accept
    if ((client_socket = accept(server_socket, (struct sockaddr *)&remote_addr, &addr_size)) < 0)
    {
        printf("��������ʧ��, %s(errno: %d)", strerror(errno), errno);
        return -1;
        // continue;
    }

    cout << "��client�˽�������, ip:" << inet_ntoa(remote_addr.sin_addr) << " port:" << htons(remote_addr.sin_port) << endl;
    return 0;
}

int init_client(int &port,string &ip,int &myport,int &client_socket,char buff[BUFSIZ],struct sockaddr_in &remote_addr)
{
    memset(&remote_addr, 0, sizeof(remote_addr));
    remote_addr.sin_family = AF_INET;
    remote_addr.sin_port = htons(port);
    remote_addr.sin_addr.s_addr = inet_addr(ip.c_str());

    //�����ͻ����׽���
    if ((client_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("����socketʧ��, %s(errno: %d)\n", strerror(errno), errno);
        return -1;
    }

    /*cout<<"port: "<<port<<endl;
    cout<<"ip: "<<ip<<endl;
    cout<<"myport: "<<myport<<endl;*/

    // bind��ָ�������˿ںţ�һ�㲻��
    struct sockaddr_in client_addr; // client�������ַ
    memset(&client_addr, 0, sizeof(client_addr));
    client_addr.sin_family = AF_INET;
    client_addr.sin_port = htons(myport);
    client_addr.sin_addr.s_addr = INADDR_ANY;
    if (myport != 0)
    {
        if (bind(client_socket, (struct sockaddr *)&client_addr, sizeof(client_addr)) < 0)
        {
            printf("bind��socket�Ͷ˿�ʧ��, %s(errno: %d)\n", strerror(errno), errno);
            return -1;
        }
    }

    //���ӷ�����
    if (connect(client_socket, (struct sockaddr *)&remote_addr, sizeof(remote_addr)) < 0)
    {
        printf("����server��ʧ��, %s(errno: %d)\n", strerror(errno), errno);
        return -1;
    }

    printf("�����ӵ�server��\n");
    return 0;
}

int get_local_ip(string ip1)
{
    int result=0;//����ֵ��ip������Ϊ1�������ڷ���0
    char *ip;
    //memset(ip, 0, sizeof(ip));
    //char dev[4]="ens";

    int fd, intrface;
    struct ifreq buf[INET_ADDRSTRLEN];  //����ṹ������/usr/include/net/if.h���������úͻ�ȡip��ַ�����룬MTU�Ƚӿ���Ϣ��
    struct ifconf ifc;
 
	/*1 ����socket���ӣ�����ioctl���Զ�ɨ���������*/
    if ((fd = socket(AF_INET, SOCK_DGRAM, 0)) >= 0)
    {
 
        ifc.ifc_len = sizeof(buf);
 
        // caddr_t,linux�ں�Դ���ﶨ��ģ�typedef void *caddr_t��
        ifc.ifc_buf = (caddr_t)buf;
 
        if (!ioctl(fd, SIOCGIFCONF, (char *)&ifc))  /*2  �����ȡ�����������嵥����������ip��ַ��mac��ַ*/
        {
            intrface = ifc.ifc_len/sizeof(struct ifreq);  //�������Ч����������//  
            while (intrface-- > 0)
             {
                if (!(ioctl(fd, SIOCGIFADDR, (char *)&buf[intrface])))  /*3  ����������ָ�������ĵ�ַ*/
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
	fd_set readset;//�ļ�����
	FD_ZERO(&readset);//set��ʼ��Ϊ��
	FD_SET(sock, &readset);//��set�����socket

    /*select:����ʱ���������time��Ϊ0*/
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
