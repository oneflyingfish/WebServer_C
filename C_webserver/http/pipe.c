#include"pipe.h"

struct Block* init(struct Request* req){
    if(req==NULL) return NULL;
    struct BasicResponse* br = NULL;
    struct AdditionalResponse* ar =NULL;
    struct Block* rst = NULL;

    rst = (struct Block*)malloc(sizeof(struct Block));
    memset(rst,0,sizeof(struct Block));
    rst->response = (struct FinalResponse*)malloc(sizeof(struct FinalResponse));
    memset(rst->response,0,sizeof(struct FinalResponse));
    rst->response->data = (char*)malloc(page);
    memset(rst->response->data,0,page);

    if(memcmp(req->version,"HTTP/1.1",8)!=0){
        //版本不支持
        br = responseFirstLine(HTTPVersionnotsupported,"HTTP Version not supported");
        addBasicResponse(rst->response,br);
        addTail(rst->response);
        return rst;
    }else if(memcmp(req->method,"GET",3)==0){
        //GET
        rst = getfile(req,rst);
        addTail(rst->response);
        return rst;
    }else if(memcmp(req->method,"POST",4)==0){
        //POST
        rst = postfile(req,rst);
        addTail(rst->response);
        return rst;
    }else{
        //不支持的请求方法+Allow字段
        br = responseFirstLine(MethodNotAllowed,"Method Not Allowed");
        ar = initResponse("Allow","GET,POST");
        addBasicResponse(rst->response,br);
        addResponse(rst->response,ar);
        addTail(rst->response);
        return rst;
    }
    return NULL;
}

struct Block* getfile(struct Request* req, struct Block* block){
    if(req==NULL||block==NULL) return NULL;
    struct FileInfo* fi = file2body(req->URL);
    struct BasicResponse* br = NULL;
    struct AdditionalResponse* ar = NULL;
    if(fi!=NULL){
        block->fileinfo = fi;
        block->chunked = fi->chunked;
        br = responseFirstLine(OK,"OK");
        addBasicResponse(block->response,br);
        if(fi->chunked==false){
            ar = initResponse("Content-Type",fi->type);
            char tmpnum[20];
            sprintf(tmpnum,"%ld",fi->len);
            struct AdditionalResponse* ar1 = initResponse("Content-Length",tmpnum);
            addResponse(block->response,ar);
            addResponse(block->response,ar1);
        }else{
            ar = initResponse("Transfer-Encoding","chunked");
            addResponse(block->response,ar);
        }
    }else{
        br = responseFirstLine(NotFound,"Not Found");
        ar = initResponse("Content-Length","0");
        addBasicResponse(block->response,br);
        addResponse(block->response,ar);
    }
    connectioncheck(req,block);
    return block;
}

struct Block* postfile(struct Request* req, struct Block* block){
    if(req==NULL||block==NULL) return NULL;
    struct BasicResponse* br = NULL;
    struct AdditionalResponse* ar = NULL;
    char *tmp,*fname;
    if(memcmp(req->Encoding,"chunked",7)!=0){
        tmp = req->bodyptr;
        fname = strstr(tmp,"filename=")+10;
        tmp = strstr(tmp,"\r\n\r\n");tmp = tmp+4;
        req->contentptr = tmp;
        size_t actlen = tmp - req->bodyptr;
        actlen += 46;
        actlen = atol(req->contentLength)-actlen;
        sprintf(req->contentLength,"%ld",actlen);
        tmp = strstr(fname,"\"");
        *tmp = '\0';
        printf("[FileName] %s\n",fname);
    }

    if(createfile(fname,req)==true){
        br = responseFirstLine(OK,"OK");
        addBasicResponse(block->response,br);
        ar = initResponse("Content-Type","text");
        addResponse(block->response,ar);
        ar = initResponse("Content-Length","0");
        addResponse(block->response,ar);
    }else{
        br = responseFirstLine(NotAcceptable,"Not Acceptable");
        addBasicResponse(block->response,br);
    }
    connectioncheck(req,block);
    return block;
}

struct Block* connectioncheck(struct Request* req, struct Block* block){
    if(req==NULL||block==NULL) return NULL;
    struct AdditionalResponse* ar;
    if(memcmp(req->URL,"/favicon.ico",12)==0){
        block->connection = false;
        block->keepalivetime = 0;
        ar = initResponse("Connection","close");
        addResponse(block->response,ar);
    }else if((memcmp(req->connection,"keep-alive",10)==0)||(memcmp(req->connection,"Keep-Alive",10)==0)){
        //keep alive
        block->connection = true;
        block->keepalivetime = alivetime;
        ar = initResponse("Connection","keep-alive");
        addResponse(block->response,ar);
        char tmp[6];
        sprintf(tmp,"%d",alivetime);
        ar = initResponse("Keep-Alive",tmp);
        addResponse(block->response,ar);
    }else{
        block->connection = false;
        block->keepalivetime = 0;
        ar = initResponse("Connection","close");
        addResponse(block->response,ar);
    }
    return block;
}

//--------------------------------------------------------------------------------
/*
struct FinalResponse* constructor(struct Request* req){
    if(req==NULL) return NULL;
    FinalResponse* rst = NULL;
    BasicResponse* brep = NULL;
    AdditionalResponse* addq =NULL;
    if(memcmp(req->version,"HTTP/1.1",8)!=0){
        //版本不支持
        brep = responseFirstLine(NotAcceptable,"Not Acceptable");
        rst = (FinalResponse*)malloc(sizeof(struct FinalResponse));
        rst->len = brep->len+2;
        rst->data = (char*)malloc(rst->len);
        memcpy(rst->data,brep->state,brep->len);
        memcpy((rst->data)+brep->len,"\r\n",2);
        free(brep);
        return rst;
    }
    if(memcmp(req->method,"GET",3)==0){
        //GET
    }else if(memcmp(req->method,"POST",4)==0){
        //POST
    }else{
        //不支持的请求方法+Allow字段
        brep = responseFirstLine(MethodNotAllowed,"Method Not Allowed");
        rst = (FinalResponse*)malloc(sizeof(struct FinalResponse));
        addq = addResponse("Allow","GET,POST");
        rst->data = (char*)malloc(page);
        rst->len = brep->len;
        memcpy(rst->data,brep->state,brep->len);
    }
    return NULL;
}

struct FinalResponse* addhead(struct FinalResponse* rst,struct AdditionalResponse* ad){
    if(rst==NULL) return NULL;
    if(ad==NULL) return NULL;
    char* tmp = rst->data + rst->len;
    memcpy(tmp,add->state,add->len);
    rst->len = rst->len+ad->len;
    return rst;
}

struct AdditionalResponse* connectioncheck();

struct FileInfo* fileattach(char* path){
    struct FinalResponse* file = body2file(path);
    if(file==NULL) return NULL;
    struct FileInfo* rst = (struct FileInfo*)malloc(sizeof(struct FileInfo));
    rst->fi = file;
    char numlen[20];
    itoa(file->len,numlen,10);
    rst->ad = addResponse("Content-Length",&numlen);
    return rst;
}
*/