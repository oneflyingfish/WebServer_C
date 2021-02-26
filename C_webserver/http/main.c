#include "TCP.h"
#include "Http.h"
#include"global.h"
#include "WebServer.h"

#define HTTP_PORT 12345
#define HTTPS_PORT 54321
#define MaxListener 10
#define Max_TRY_TIMES 10

int main()
{
    int pid = fork();
    if(pid!=0)
    {
        // 父进程, 运行Https服务器
        for (int i = 0; i < Max_TRY_TIMES;i++)
        {
            RunHttpsServer(NULL, HTTPS_PORT, MaxListener);
        }
#ifdef OUTPUT_DEBUG_INFO
        printf("HTTPS server尝试启动%d次后仍旧失败，放弃启动\n", Max_TRY_TIMES);
#endif
    }
    else
    {
        // 子进程，运行http服务器
        for (int i = 0; i < Max_TRY_TIMES; i++)
        {
            RunHttpServer(NULL, HTTP_PORT, MaxListener);
        }
#ifdef OUTPUT_DEBUG_INFO
        printf("HTTP server尝试启动%d次后仍旧失败，放弃启动\n", Max_TRY_TIMES);
#endif
    }

    while(1);
}