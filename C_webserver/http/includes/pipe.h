#ifndef _PIPE_H_
#define _PIPE_H_

#include"global.h"
#include"request.h"
#include"response.h"
#include"filesystem.h"

struct Block{
    struct FinalResponse* response;
    int connection;
    int keepalivetime;
    int chunked;
    struct FileInfo* fileinfo;
};

struct Block* init(struct Request* req);

struct Block* getfile(struct Request* req, struct Block* block);
struct Block* postfile(struct Request* req, struct Block* block);

struct Block* connectioncheck(struct Request* req, struct Block* block);

#endif
//----------------------------------------------------------------------------

/*
struct FinalResponse* constructor(struct Request* req);

struct FinalResponse* addhead(struct FinalResponse* rst,struct AdditionalResponse* ad);

struct AdditionalResponse* connectioncheck();

struct FileInfo* fileattach(char* path);

*/