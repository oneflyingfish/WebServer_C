#ifndef TCP_H
#define TCP_H

#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <openssl/ssl.h>
#include <openssl/err.h>

#include "SSL.h"
#include"global.h"
#include"filesystem.h"

#define MAX_CHUNK_SIZE 1024

// 定义别名
typedef struct sockaddr SockAddr;
typedef struct sockaddr_in SockAddr_In;

// 套接字结构体
typedef struct Socket
{
    int sockId;                         // socket id
    struct sockaddr_in address;         // socket 地址
    int addressLength;                  // 一般等于sizeof(SockAddr)，由accept函数赋值
    SSL* ssl;                           // 非NULL时指向SSL相关内容
    SSL_CTX *ctx;
    Bool isHttps;
} Socket;

/********************************************************************************
 * 功能：创建一个套接字地址
 * 
 * ip：C语言类型数组，当指定为NULL时表示INADDR_ANY
 * port: 端口号
 * 返回值: 套接字地址
 ************************************************************************* ******/
SockAddr_In CreateSocketAddr_IN(char *ip, unsigned short int port);

/********************************************************************************
 * 功能：初始化一个Socket Server
 * ip: C语言类型数组，当指定为NULL时表示INADDR_ANY
 * port: 端口号
 * maxListen: 最大监听数目
 * 返回值: 监听Socket结构。套接字创建失败：Socket.sockId=-1; 端口绑定失败：Socket.sockId=-2; 设置监听失败：Socket.sockId=-2
 *  ************************************************************************* ***/
Socket InitSocketServer(char* ip, unsigned short int port, int maxListen);  // 创建监听套接字并绑定端口

/********************************************************************************
 * 功能：接受客户端连接，内部会阻塞
 * 
 * serverSocket: 监听Socket结构
 * isHttps: 是否是Https服务器，true/false
 * 返回值：客户端Socket结构
 *  ************************************************************************* ***/
Socket AcceptClientSocket(Socket serverSocket, Bool isHttps);

/********************************************************************************
 * 功能：关闭套接字
 * 
 * sock: 等待关闭的Socket
 * 返回值: 成功:1 , 套接字已过期：0 , 失败：-1
 *  ************************************************************************* ***/
int CloseSocket(Socket sock);

/********************************************************************************
 * 功能：基于TCP读取字节流，会在末尾额外添加'\0'，但不保证有用，通过特定字符串判断结束
 * 
 * sock: 客户端Socket
 * data: 存放数据的缓冲区
 * maxBuffer: 缓冲区大小，应额外多出一个字节
 * endFlag: C语言字符串，结束标志
 * alarm: TCP超时未返回数据，且超过alarm时，自动关闭套接字，返回-3。alarm<=0时无效
 * 返回值: 成功：有效数据长度；读取发生错误：-1  客户端主动断开连接：-2  超时：-3 缓冲区溢出：-4
 *  ************************************************************************* ***/
int RecvDataByFlag(Socket sock, char *data, int maxBuffer,char *endFlag, int alarm);

/********************************************************************************
 * 功能：基于TCP读取指定长度字节流
 * 
 * sock: 客户端Socket
 * data: 存放数据的缓冲区
 * length: 期望读取的数据长度
 * alarm: TCP超时未返回数据，且超过alarm时，自动关闭套接字，返回-3。alarm<=0时无效
 * 返回值: 成功：有效数据长度； 读取发生错误：-1  客户端主动断开连接：-2  超时：-3 读取长度无效：-4
 *  ************************************************************************* ***/
int RecvDataByLength(Socket sock, char *data, int length, int alarm);

/********************************************************************************
 * 功能：基于TCP在chunked模式下，从Socket缓冲区读取一个块的有效内容，为malloc分配的内存，应!!显式释放！！尾部额外添加'\0'，但不保证有用
 * sock: 客户端Socket
 * dataLength: 返回有效数据长度
 * 返回值: 成功：堆区指针，指向有效数据 出错：NULL
 *  ************************************************************************* ***/
char *RecvOneChunk(Socket sock, unsigned long int *dataLength);

/********************************************************************************
 * 功能：基于TCP发送字节数组

 * sock: 客户端Socket
 * data: 数组首地址
 * datalength：数据长度
 * allowChunk：是否使用分块，使用true/false
 * sendEndFlag：结尾是否发送分块结束标志"0\r\n\r\n",使用true/false
 * 返回值: 成功：0 出错：-1
 *  ************************************************************************* ***/
int SendData(Socket sock, char *data, int dataLength, Bool allowchunked, Bool SendEndFlag);

/********************************************************************************
 * 功能：基于TCP以二进制读取方式，将磁盘文件发送到套接字内核缓冲区
 * 
 * sock: 客户端Socket
 * data: C字符串，文件路径
 * allowChunk：是否使用分块，使用true/false
 * sendEndFlag：结尾是否发送分块结束标志"0\r\n\r\n",使用true/false
 * 返回值: 成功：0 出错：-1
************************************************************************* ***/
int SendFile(Socket sock, char *path, Bool allowchunked, Bool SendEndFlag);

/********************************************************************************
 * 功能：在分块传输时，用于在结尾发送结束标志"0\r\n\r\n"
 * 
 * sock: 客户端Socket
 * 返回值: 成功：0 出错：-1
 ************************************************************************* ***/
int SendChunkEndFlag(Socket sock);

/********************************************************************************
 * 功能：对socket write的封装，自适应HTTP/HTTPS
 ************************************************************************* ***/
int write_sock(Socket sock,char* buffer, int bufferSize);

/********************************************************************************
 * 功能：对socket recv的封装，自适应HTTP/HTTPS
 * flag: 仅实现了MSG_WAITALL和0的支持
 ************************************************************************* ***/
int recv_sock(Socket sock, char *buffer, int size, int flag);

/********************************************************************************
 * 功能：对socket read的封装，自适应HTTP/HTTPS
 ************************************************************************* ***/
int read_sock(Socket sock, char *buffer, int bufferSize);

#endif

/*************************************
struct sockaddr
{
    unsigned short int sa_family;
    char sa_data[14];
};

struct socketaddr_in
{
    unsigned short int sin_family;
    uint16_t sin_port;
    struct in_addr sin_addr;
    unsigned char sin_zero[8];
};
***************************************/