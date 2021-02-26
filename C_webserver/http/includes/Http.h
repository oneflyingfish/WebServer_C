#ifndef HTTP_H
#define HTTP_H

#include<unistd.h>
#include<event2/event.h>
#include"global.h"
#include"request.h"
#include"response.h"
#include"pipe.h"
#include"filesystem.h"
#include "TCP.h"

typedef enum HttpKind
{
    Error_400,
    Error_404,
    Error_500,
    Error_501,
    Success_200
} HttpKind;

typedef struct Eventarg{
    struct event_base* base;
    struct event* event;
    Socket* sock;
    int firsttime;
} Eventarg;

void accept_cb(int fd, short events, void* arg);
void close_cb(int fd, short events, void* arg);
int DealWithRequest(Socket sock);                                               // 成功时返回0，存在错误返回-1
int ReadHttpHeaderBytes(Socket sock, char *data, int maxBuffer);

void printRequest(struct Request* req);
void printResponse(struct Block* block);

void freeRequest(struct Request* req);
void freeResponse(struct FinalResponse* res);
void freeFileinfo(struct FileInfo* fi);
void freeBlock(struct Block* block);

void sendResponse(struct Block* block, Socket sock);
void CloseHttp(Eventarg* eventarg);

int ResponseHttpWithHtml(Socket sock, HttpKind httpKind, char *htmlPath);
int ResponseHttp(Socket sock, char *contentType, char *filePath, Bool allowChunked);

#endif