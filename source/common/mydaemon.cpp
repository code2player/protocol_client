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

    //pid=0, 是第一子进程，后台继续执行
    setsid(); //第一子进程成为新的会话组长和进程组长
    //并与控制终端分离
    /*if ((pid = fork()))
        exit(0); //是第一子进程，结束第一子进程
    else if (pid < 0)
        exit(1); // fork失败，退出*/
    //是第二子进程，继续
    //第二子进程不再是会话组长

    /*int i;
    for (i = 0; i < NOFILE; ++i) //关闭打开的文件描写叙述符
        close(i);*/
    //NOFILE=3,代表三个文件的标准/错误输入输出

    //chdir("/tmp"); //此处不改变工作文件夹到/tmp
    //umask(0);      //重设文件创建掩模
    
    return;
}

//不是完全的守护进程
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

    //pid=0, 是第一子进程，后台继续执行
    //setsid(); //第一子进程成为新的会话组长和进程组长

    return;
}