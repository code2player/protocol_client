#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <sys/param.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
void my_daemon()
{
    int pid;
    //int i;
    pid = fork();
    if (pid > 0)
        exit(0);
    else if (pid < 0)
        exit(1);
    else
    {
        ;
    }

    //pid=0, �ǵ�һ�ӽ��̣���̨����ִ��
    setsid(); //��һ�ӽ��̳�Ϊ�µĻỰ�鳤�ͽ����鳤
    //��������ն˷���
    /*if ((pid = fork()))
        exit(0); //�ǵ�һ�ӽ��̣�������һ�ӽ���
    else if (pid < 0)
        exit(1); // forkʧ�ܣ��˳�*/
    //�ǵڶ��ӽ��̣�����
    //�ڶ��ӽ��̲����ǻỰ�鳤

    /*int i;
    for (i = 0; i < NOFILE; ++i) //�رմ򿪵��ļ���д������
        close(i);*/
    //NOFILE=3,���������ļ��ı�׼/�����������

    //chdir("/tmp"); //�˴����ı乤���ļ��е�/tmp
    //umask(0);      //�����ļ�������ģ
    
    return;
}

//������ȫ���ػ�����
void my_daemon_nodae()
{
    int pid;
    //int i;
    pid = fork();
    if (pid > 0)
        exit(0);
    else if (pid < 0)
        exit(1);
    else
    {
        ;
    }

    //pid=0, �ǵ�һ�ӽ��̣���̨����ִ��
    //setsid(); //��һ�ӽ��̳�Ϊ�µĻỰ�鳤�ͽ����鳤

    return;
}