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
using namespace std;

const int PAC_SIZE = 10000;
const int BUF_SIZE = 10020;

int init_server(int &port,string &ip,int &server_socket,int &client_socket,struct sockaddr_in servaddr,struct sockaddr_in &remote_addr,char buff[BUFSIZ],socklen_t &addr_size, int noaccept=0);
int init_client(int &port,string &ip,int &myport,int &client_socket,char buff[BUFSIZ],struct sockaddr_in &remote_addr);
int get_local_ip(string ip1);
void tcp_buf(int &server_socket, int getrbuf, int getwbuf, int setrbuf, int setwbuf, int getnrbuf, int getnwbuf);
int select_set(int sock);

