#include "Http.h"

#define BUFFER_SIZE 8192

int DealWithRequest(Socket sock){
    //libevent并发
    struct event_base* base = event_base_new();     //event_init();
    Eventarg* eventarg = (Eventarg*)malloc(sizeof(Eventarg));
    eventarg->base = base;
    eventarg->firsttime = true;
    eventarg->sock = &sock;
    struct event* ev_listen = event_new(base,sock.sockId,EV_READ | EV_PERSIST,accept_cb,eventarg);
    eventarg->event = ev_listen;
    struct timeval tv;
    tv.tv_sec = alivetime;
    event_add(ev_listen,&tv);
    event_base_dispatch(base);
    return 0;
}

void accept_cb(int fd, short events, void* arg){
    Eventarg* eventarg = (Eventarg*)arg;
    if(events==EV_TIMEOUT){
        if(eventarg->firsttime==true) return;
        else{CloseHttp(eventarg);}
    }else{
        eventarg->firsttime=false;
        struct Request* req = NULL;
        char buffer[BUFFER_SIZE];

        int flag = ReadHttpHeaderBytes(*(eventarg->sock), buffer, BUFFER_SIZE);
        if (flag == -1){
            //出现错误
            return;
        }
        else if (flag == -2){
            // 客户端主动断开
            CloseHttp(eventarg);
            return;
        }

        req = phaser(strlen(buffer),buffer);
        if(memcmp(req->contentLength,"\0",1)!=0){
            size_t len = atol(req->contentLength);
            req->bodyptr = (char*)malloc(len+1);
            memset(req->bodyptr,0,len+1);
            RecvDataByLength(*(eventarg->sock),req->bodyptr,len,0);
        }
        if(memcmp(req->Encoding,"chunked",7)==0) req->bodyptr = (char*)(eventarg->sock);
        printRequest(req);

        struct Block* block = init(req);

        printResponse(block);
        sendResponse(block,*(eventarg->sock));
        if(block->connection==false){
            CloseHttp(eventarg);
        }
        freeRequest(req);
        freeBlock(block);
    }
}

void freeRequest(struct Request* req){
    if(req==NULL) return;
    if(req->bodyptr!=NULL) free(req->bodyptr);
    free(req);
}

void freeResponse(struct FinalResponse* res){
    if(res==NULL) return;
    if(res->data!=NULL) free(res->data);
    free(res);
}

void freeFileinfo(struct FileInfo* fi){
    if(fi==NULL) return;
    if(fi->fpath!=NULL) free(fi->fpath);
    free(fi);
}

void freeBlock(struct Block* block){
    if(block==NULL) return;
    if(block->response!=NULL) freeResponse(block->response);
    if(block->fileinfo!=NULL) freeFileinfo(block->fileinfo);
    free(block);
}

void printRequest(struct Request* req){
    printf("\n----------------------------------------------\n[New Request]\n");
    printf("    method: %s\n",req->method);
    printf("    URL: %s\n",req->URL);
    printf("    URLlen: %ld\n",req->URLlen);
    printf("    version: %s\n",req->version);
    printf("    Connection: %s\n",req->connection);
    printf("    Accept: %s\n",req->accept);
    printf("    contentType: %s\n",req->contentType);
    printf("    contentLength: %s\n",req->contentLength);
    printf("    Encoding: %s\n",req->Encoding);
    //if(req->bodyptr!=NULL) printf("data:\n%s\n",req->bodyptr);
}

void printResponse(struct Block* block){
    printf("\n----------------------------------------------\n[Response]\n");
    printf("%s\n",block->response->data);
    if(block->fileinfo!=NULL)
        printf("path: %s\ntype: %s\n",block->fileinfo->fpath,block->fileinfo->type);
    printf("----------------------------------------------\n");
}

int ReadHttpHeaderBytes(Socket sock, char *data, int maxBuffer)
{
    int flag = RecvDataByFlag(sock, data, maxBuffer, "\r\n\r\n",0);

    if (flag == -1)
    {
#ifdef OUTPUT_ERROR_INFOR
        printf("读取http header时发生错误\n");
#endif
        *data = '\0';
        return -1;
    }
    else if (flag == -2)
    {
        return -2;
    }
    return 0;
}

void sendResponse(struct Block* block, Socket sock){
    if(block==NULL) return;
    SendData(sock,block->response->data,block->response->len,false,false);
    if(block->fileinfo!=NULL){
        SendFile(sock,block->fileinfo->fpath,block->chunked,true);
    }
}

int ResponseHttpWithHtml(Socket sock, HttpKind httpKind, char *htmlPath)
{
    // 计算http 头部内容
    char headerBuffer[BUFFER_SIZE];
    memset(headerBuffer, 0, BUFFER_SIZE);

    // 计算获取正确的html path
    char path[500];
    memset(path, 0, 500);
    strcpy(path, "response/");

    // 插入http类型，并计算正确的http文件所在path
    switch (httpKind)
    {
    case Error_400:
        strcat(headerBuffer, "HTTP/1.1 400 BAD REQUEST\r\n");

        strcat(path, "400.html");
        break;
    case Error_404:
        strcat(headerBuffer, "HTTP/1.1 404 NOT FOUND\r\n");

        strcat(path, "404.html");
        break;
    case Error_500:
        strcat(headerBuffer, "HTTP/1.1 500 Internal Server Error\r\n");

        strcat(path, "500.html");
        break;
    case Error_501:
        strcat(headerBuffer, "HTTP/1.1 501 Method Not Implemented\r\n");

        strcat(path, "501.html");
        break;
    case Success_200:
        strcat(path, htmlPath);
        if (htmlPath == NULL || access(path, F_OK) != 0)
        {
#ifdef OUTPUT_ERROR_INFOR
            printf("获取返回html时为空，将自动渲染404界面\n");
#endif
            strcat(headerBuffer, "HTTP/1.1 400 BAD REQUEST\r\n");
            memset(path, 0, 500);
            strcpy(path, "response/404.html");
            httpKind = Error_404;
            break;
        }

        // 以下仅为测试代码
        strcat(headerBuffer, "HTTP/1.1 200 OK\r\n");
        break;

    default:
#ifdef OUTPUT_ERROR_INFOR
        printf("HTTP_HTML Response错误枚举\n");
#endif
        return -1;
    }

    strcat(headerBuffer, "Server: C http server 1.0\r\n");
    strcat(headerBuffer, "Connection: keep-alive\r\n");
    strcat(headerBuffer, "Content-Type: text/html\r\n");

    // 写入data长度
    int dataLength = GetFileLength(path);

    char dataLengthString[30];
    memset(dataLengthString, 0, 30);
    sprintf(dataLengthString, "Content-Length: %d\r\n", dataLength);
    strcat(headerBuffer, dataLengthString);
    //strcat(headerBuffer, "Transfer-Encoding: chunked\r\n");

    // 写入空行
    strcat(headerBuffer, "\r\n");

#ifdef OUTPUT_DEBUG_INFO
    printf("发送头部:\n%s", headerBuffer);
#endif

    SendData(sock, headerBuffer,strlen(headerBuffer),false,false); // 发送头部
    SendFile(sock, path,false,true);
    // char *d = "<html><head><title>test</title></head>\r\n<body><p>this is test</p></body></html>";
    // SendData(sock, d, strlen(d),true,true);
}

int ResponseHttp(Socket sock, char *contentType, char *filePath, Bool allowChunked)
{
    // 计算获取正确的文件path
    char path[500];
    memset(path, 0, 500);
    strcpy(path, "response/");
    strcat(path, filePath);
    
    // 判断文件是否存在
    if (access(path, F_OK) != 0)
    {
        // 资源文件不存在
        ResponseHttpWithHtml(sock, Error_404, NULL);
        return -1;
    }

    // 计算http 头部内容
    char headerBuffer[BUFFER_SIZE];
    memset(headerBuffer, 0, BUFFER_SIZE);

    strcat(headerBuffer, "HTTP/1.1 200 OK\r\n");
    strcat(headerBuffer, "Server: C http server 1.0\r\n");
    strcat(headerBuffer, "Connection: keep-alive\r\n");

    // 输入ContentType
    strcat(headerBuffer, "Content-Type: ");
    strcat(headerBuffer, contentType);
    strcat(headerBuffer, "\r\n");
    
    strcat(headerBuffer, "Content-Disposition:attachment;filename=");
    char fileName[40];
    memset(fileName, 0, 40);
    GetFileName(filePath, fileName);
    strcat(headerBuffer, fileName);
    strcat(headerBuffer, "\r\n");

    if(!allowChunked)
    {
        // 使用Content-Length的方式写入
        int dataLength = GetFileLength(path);

        char dataLengthString[30];
        memset(dataLengthString, 0, 30);
        sprintf(dataLengthString, "Content-Length: %d\r\n", dataLength);
        strcat(headerBuffer, dataLengthString);
    }
    else
    {
        // 使用chunked的方式写入
        strcat(headerBuffer, "Transfer-Encoding: chunked\r\n");
    }
    strcat(headerBuffer, "\r\n");   // 增加空行, 头部生成完毕

#ifdef OUTPUT_DEBUG_INFO
    printf("发送头部:\n%s", headerBuffer);
#endif

    SendData(sock, headerBuffer,strlen(headerBuffer),false,false); // 发送头部
    SendFile(sock, path, allowChunked,true);
    return 0;
}

void CloseHttp(Eventarg* eventarg){
    event_base_loopbreak(eventarg->base);
    event_del(eventarg->event);
    event_free(eventarg->event);
    event_base_free(eventarg->base);
    CloseSocket(*(eventarg->sock));
    free(eventarg);
    printf("[Info] HTTP/HTTPS connection close.\n");
    return;
}