#include "WebServer.h"

int RunHttpServer(char *ip, int port, int maxListener)
{
    RunWebServer(ip, port, maxListener, HTTP);
}

int RunHttpsServer(char *ip, int port, int maxListener)
{
    RunWebServer(ip, port, maxListener, HTTPS);
}

int RunWebServer(char *ip, int port, int maxListener, HTTP_KIND flag)
{
    Socket serverSocket = InitSocketServer(ip, port, maxListener);
    if (serverSocket.sockId < 0)
    {
#ifdef OUTPUT_DEBUG_INFO
        if (flag == HTTP)
        {
            printf("HTTP server Socket初始化出错\n");
        }
        else
        {
            printf("HTTPS server Socket初始化出错\n");
        }
#endif

        return -1; // 程序发生错误
    }

#ifdef OUTPUT_DEBUG_INFO
    if(flag==HTTP)
    {
        printf("HTTP server：%d 启动成功，进程pid= %d\n",port, getpid());
    }
    else
    {
        printf("HTTPS server %d 启动成功，进程pid= %d\n",port, getpid());
    }
#endif

    while (ALLOW_ACCEPT)
    {
        Socket clientSocket = AcceptClientSocket(serverSocket, flag==HTTPS?true:false);
        if (clientSocket.sockId < 0)
        {
#ifdef OUTPUT_DEBUG_INFO
            printf("客户端套接字错误\n");
#endif
            continue; //出错
        }

        if(clientSocket.isHttps && clientSocket.ssl==NULL)
        {
#ifdef OUTPUT_DEBUG_INFO
            printf("HTTPS监听Socket发生错误，未创建进程\n");
#endif
            continue;   // https初始化出错
        }

        int pid = fork();
        if (!pid)
        {
            close(serverSocket.sockId);
            // 子进程处理模块
            DealWithRequest(clientSocket);
#ifdef OUTPUT_DEBUG_INFO
            printf("子进程: %d 退出\n", getpid());
#endif
            exit(0);
        }

        // 安全代码，以防前面代码bug，额外做安全判断
        if (pid == 0)
        {
            // 这段代码理论上不会执行
            exit(0);
        }

        // 不能关闭SSL，会导致子进程SSL失效
        //close(clientSocket.sockId);
        clientSocket.ctx = NULL;
        clientSocket.ssl = NULL;

        // 父进程
#ifdef OUTPUT_DEBUG_INFO
        if (flag == HTTP)
        {
            printf("HTTP建立连接成功: ip= %s , port= %d ,创建子进程pid=%d\n", inet_ntoa(clientSocket.address.sin_addr), ntohs(clientSocket.address.sin_port), pid);
        }
        else
        {
            printf("HTTPS建立连接成功: ip= %s , port= %d ,创建子进程pid=%d\n", inet_ntoa(clientSocket.address.sin_addr), ntohs(clientSocket.address.sin_port), pid);
        }
#endif
    }
}