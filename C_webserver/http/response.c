#include"response.h"

struct BasicResponse* responseFirstLine(int scode,char* description){
  struct BasicResponse* rst = (struct BasicResponse*)malloc(sizeof(struct BasicResponse));
  sprintf(rst->state,"HTTP/1.1 %d %s\r\n",scode,description);
  rst->len = strlen(rst->state);
  return rst;
}

struct AdditionalResponse* initResponse(char* key,char* value){
    struct AdditionalResponse* rst = (struct AdditionalResponse*)malloc(sizeof(struct AdditionalResponse));
    sprintf(rst->state,"%s: %s\r\n",key,value);
    rst->len = strlen(rst->state);
    return rst;
}

void addResponse(struct FinalResponse* res,struct AdditionalResponse* ar){
    char* tmp = res->data+res->len;
    memcpy(tmp,ar->state,ar->len);
    res->len = res->len+ar->len;
    free(ar);
}

void addBasicResponse(struct FinalResponse* res,struct BasicResponse* br){
    memcpy(res->data,br->state,br->len);
    res->len = br->len;
    free(br);
}

void addTail(struct FinalResponse* res){
    memcpy(res->data+res->len,"\r\n",2);
    res->len = res->len+2;
}