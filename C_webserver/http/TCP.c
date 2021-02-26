#include "TCP.h"
#include<stdio.h>
#include<errno.h>

SockAddr_In CreateSocketAddr_IN(char *ip, unsigned short int port)
{
    SockAddr_In sockaddr;
    sockaddr.sin_family = AF_INET;              // IPV4
    sockaddr.sin_port = htons(port);
    if(ip!=NULL)
    {
        sockaddr.sin_addr.s_addr = inet_addr(ip);
    }
    else
    {
        sockaddr.sin_addr.s_addr = INADDR_ANY;
    }

    bzero(&(sockaddr.sin_zero), 8);             // 填充位清零
    return sockaddr;
}

Socket InitSocketServer(char *ip, unsigned short int port, int maxListen)
{
    Socket sock;
    // 初始化
    sock.ctx = NULL;
    sock.isHttps = false;
    sock.ssl = NULL;

    sock.sockId = socket(AF_INET, SOCK_STREAM, 0);
    if(sock.sockId < 0)
    {
#ifdef OUTPUT_ERROR_INFOR
        printf("套接字创建失败\n");
#endif
        sock.sockId = -1;
        return sock;
    }

    sock.address = CreateSocketAddr_IN(ip, port);
    sock.addressLength = sizeof(SockAddr);
    if (bind(sock.sockId, (SockAddr *)&(sock.address),sock.addressLength) < 0)
    {
#ifdef OUTPUT_ERROR_INFOR
        printf("端口绑定失败\n");
#endif
        sock.sockId = -2;
        return sock;
    }

    if(listen(sock.sockId, maxListen) < 0)
    {
#ifdef OUTPUT_ERROR_INFOR
        printf("设置监听失败\n");
#endif
        sock.sockId = -3;
        return sock;
    }

    return sock;
}

Socket AcceptClientSocket(Socket serverSocket, Bool isHttps)
{
    Socket sock;
    sock.isHttps = isHttps;
    sock.ssl = NULL; // SSL字段初始化为NULL
    sock.ctx = NULL;
    sock.addressLength = sizeof(SockAddr);

#ifdef OUTPUT_DEBUG_INFO
    printf("开始监听客户端连接\n");
#endif

    sock.sockId = accept(serverSocket.sockId, (SockAddr*)&(sock.address), &(sock.addressLength));
    if(sock.sockId<0)
    {
#ifdef OUTPUT_ERROR_INFOR
        printf("客户端连接出错，套接字返回值：%d\n", sock.sockId);
#endif
    }

    if(isHttps)
    {
        sock.ctx = InitSSL();
        sock.ssl = AttachSSLWithSocket(sock.sockId, sock.ctx);
    }

    return sock;
}

int CloseSocket(Socket sock)
{
    if(sock.isHttps)
    {
        FreeSSL(sock.ssl, sock.ctx);
        sock.ssl = NULL;
        sock.ctx = NULL;

        return 0;
    }
    else
    { 
        // http的socket
        if(sock.sockId>0)
        {
            if (close(sock.sockId) < 0)
            {
    #ifdef OUTPUT_ERROR_INFOR
                printf("关闭套接字失败，错误信息: %s\n", strerror(errno));
    #endif
                return -1;          // 失败
            }
            else
            {
                sock.sockId = -10;
                return 1;           // 成功
            }
        }
        else
        {
    #ifdef OUTPUT_ERROR_INFOR
            printf("关闭套接字不存在\n");
    #endif
            return 0;
        }
    }
}

int RecvDataByFlag(Socket sock, char *data, int maxBuffer, char *endFlag, int alarm)
{
    int startTime = time(NULL);     // 程序开始时间戳，单位为s

    memset(data, 0, maxBuffer);
    char ch = '\0', *p = data, *flag = endFlag;

    // 开始读取数据
    while (1)
    {
        if (*flag == '\0')
        {
            *p = '\0';
            return (int)(p-data); // 结束读取
        }

        int length = read_sock(sock, &ch, 1);
        if(length<0)
        {
            return -1;
        }

        if (length == 0 && (time(NULL) - startTime > alarm) && alarm>0)
        {
            // 超时未读取到数据，且已经超过最大alarm时，断开TCP连接
            CloseSocket(sock);
            return -3;
        }

        if ((p - data) >= maxBuffer - 1)
        {
            // 对于空头判断
            if(strlen(data)<=0)
            {
#ifdef OUTPUT_ERROR_INFOR
                printf("等候头部时对方主动退出\n");
#endif
                data[0] = '\0';
                return -2;
            }

#ifdef OUTPUT_ERROR_INFOR
            data[maxBuffer] = '\0';
            printf("Read()读取时缓冲区太小，未能在\"%s\"之前返回，当前buffer size为: %d, 已经读取的数据为:%s\n", endFlag, maxBuffer - 1, data);
#endif
            return -4;
        }

        *p = ch;
        p++;

        // 开始读取
        if (ch == *flag)
        {
            flag++;
        }
        else
        {
            flag = endFlag;
        }
    }
}

int RecvDataByLength(Socket sock, char *data, int length, int alarm)
{
    int startTime = time(NULL); // 程序开始时间戳，单位为s

    if(length <= 0)
    {
        return -4;
    }

    memset(data, 0, length+1);
    int recvLength = recv_sock(sock, data, length, MSG_WAITALL); // 等待所有数据到达，否则阻塞
    if (recvLength < 0)
    {
#ifdef OUTPUT_ERROR_INFOR
        printf("RecvDataByLength: Recv()发生错误\n");
#endif
        return -1;
    }
    else if (recvLength == 0 && (time(NULL) - startTime > alarm) && alarm > 0)
    {
        // 未接收到数据，且超时
        CloseSocket(sock);
        return -3;
    }
    else if (recvLength == 0)
    {
        // 对方断开
        return -2;
    }
    return recvLength;
}

int SendData(Socket sock, char *data, int dataLength, Bool allowchunked, Bool SendEndFlag)
{
    if(!allowchunked)
    {
        if (write_sock(sock, data, dataLength) < 0)
        {
            return -1;
        }
    }
    else
    {
        char *p = data;
        while (1)
        {
            if((p-data)>=dataLength)
            {
                // 发送完成
                if(SendEndFlag)
                {
                    if(SendChunkEndFlag(sock)<0)
                    {
                        return -1;
                    }
                }
                return 0;
            }

            char OctData[17]; // 块长度不应该超过8字节表示范围
            memset(OctData, 0, 17);
            if((data+dataLength-p)>=MAX_CHUNK_SIZE)
            {
                sprintf(OctData, "%lx\r\n", (unsigned long int)MAX_CHUNK_SIZE);
                write_sock(sock, OctData, strlen(OctData)); // 写入长度

                if (write_sock(sock, p, MAX_CHUNK_SIZE) < 0)
                {
                    return -1;
                }
                p += MAX_CHUNK_SIZE;
            }
            else
            {
                sprintf(OctData, "%lx\r\n", data + dataLength - p);
                write_sock(sock, OctData, strlen(OctData)); // 写入长度

                if (write_sock(sock, p, data + dataLength - p) < 0)
                {
                    return -1;
                }

                p += data + dataLength - p;
            }

            // 写入块尾部
            if (write_sock(sock, "\r\n", 2) < 0)
            {
                return -1;
            }
        }
    }
    return 0;
}

int SendFile(Socket sock, char *path, Bool allowchunked, Bool SendEndFlag)
{
    FILE *fp = fopen(path, "rb");
    if (fp == NULL)
    {
#ifdef OUTPUT_ERROR_INFOR
        printf("发送文件时，打开文件出错，文件名: %s\n", path);
#endif
        return -1;
    }

    int fileLength = GetFileLength(path);

    unsigned char buffer[MAX_CHUNK_SIZE];
    int sendedLength = 0, len=0;
    while (1)
    {
        // 判断文件结束
        if(feof(fp) || sendedLength>=fileLength)
        {
            if (allowchunked)
            {
                // 发送完成
                if (SendEndFlag)
                {
                    if (SendChunkEndFlag(sock) < 0)
                    {
                        return -1;
                    }
                }
            }
            break;
        }

        memset(buffer, 0, MAX_CHUNK_SIZE);
        if ((fileLength - sendedLength) >= MAX_CHUNK_SIZE)
        {
            fread(buffer, sizeof(unsigned char), MAX_CHUNK_SIZE, fp);
            len = MAX_CHUNK_SIZE;
        }
        else
        {
            fread(buffer,1, fileLength - sendedLength, fp);
            len = fileLength - sendedLength;
        }

        if(allowchunked)
        {
            // 打印数据长度
            char OctData[17];                               // 块长度不应该超过8字节表示范围
            memset(OctData, 0, 17);
            sprintf(OctData, "%x\r\n", len);
            write_sock(sock, OctData, strlen(OctData)); // 写入长度
            //printf("传送头部: %s", OctData );
        }

        if(len>0)
        {
            write_sock(sock, buffer, len); // 发送数据
            sendedLength += len;
            len = 0;
            if (allowchunked)
            {
                write_sock(sock, "\r\n", 2);
            }
        }

        
#ifdef OUTPUT_DEBUG_INFO
        //printf("传送正文: %s",buffer);
#endif
    }

    fclose(fp);
    return 0;
}

int SendChunkEndFlag(Socket sock)
{
    if (write_sock(sock, "0\r\n\r\n", 5) < 0)
    {
        return - 1;
    }

    return 0;
}

char* RecvOneChunk(Socket sock,unsigned long int* dataLength)
{
    // 读取有效数据长度
    char len[18];
    memset(len, 0, 18);

    char *p = len;
    while (1)
    {
        if (read_sock(sock, p, 1) < 0)
        {
            return NULL;
        }

        if((p-len)>=18)
        {
            // 溢出
            return NULL;
        }

        if((p-len)>=1 && *p=='\n' && *(p-1)=='\r')
        {
            *(p - 1) = '\0';
            break;              //读取长度完成
        }

        p++;
    }

    sscanf(len, "%lx", dataLength);

    // 读取有效数据
    char ch = '\0';
    char *data = (char *)malloc(sizeof(char) * (*dataLength+1));
    if(data==NULL)
    {
        return NULL;
    }
    
    for (int i = 0; i < *dataLength; i++)
    {
        if(*dataLength==0)
        {
            break;
        }

        if (read_sock(sock, &ch, 1) < 0)
        {
            free(data);
            return NULL;
        }
        data[i] = ch;
    }

    data[*dataLength] = '\0';

    char flag1 = '\0', flag2 = '\0';
    if (read_sock(sock, &flag1, 1) < 0)
    {
        free(data);
        return NULL;
    }

    if (read_sock(sock, &flag2, 1) < 0)
    {
        free(data);
        return NULL;
    }

    if(!(flag1=='\r'&& flag2=='\n'))
    {
        free(data);
        return NULL;    // 块结束不正确
    }

    return data;
}

int write_sock(Socket sock, char *buffer, int bufferSize)
{
    if(sock.isHttps)
    {
        if (sock.ssl == NULL)
        {
            return -1;
        }
        return SSL_write(sock.ssl, buffer, bufferSize);
    }
    else
    {
        return write(sock.sockId, buffer, bufferSize);
    }
}

int read_sock(Socket sock, char *buffer, int bufferSize)
{
    if (sock.isHttps)
    {
        if(sock.ssl==NULL)
        {
            return -1;
        }
        return SSL_read(sock.ssl, buffer, bufferSize);
    }
    else
    {
        return read(sock.sockId, buffer, bufferSize);
    }
}

int recv_sock(Socket sock, char *buffer, int size, int flag)
{
    int MAX_WAIT_TIME = 30; // 最大阻塞30s

    if (sock.isHttps)
    {
        if (sock.ssl == NULL)
        {
            return -1;
        }

        if(flag==MSG_WAITALL)
        {
            int length = 0;
            unsigned long int startTime = time(NULL);
            while (1)
            {
                if(length>=size)
                {
                    return length;
                }

                int len = SSL_read(sock.ssl, buffer + length, size - length);
                length += len;

                if (len < 0)
                {
                    return -1;
                }
                else if(len==0 && (time(NULL)-startTime)>MAX_WAIT_TIME)
                {
                    // 超时
                    return 0;
                }
            }
        }
        else
        {
            return SSL_read(sock.ssl, buffer, size);
        }
        
    }
    else
    {
        return recv(sock.sockId, buffer, size, flag);
    }
}