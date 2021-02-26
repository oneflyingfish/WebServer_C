#ifndef _REQUEST_H_
#define _REQUEST_H_

#include"global.h"

#define typenums 5

typedef struct Request{
  char method[8];		            //only GET or POST
  char URL[dflen];
  size_t URLlen;
  char version[16];

  char connection[12];          //Connection: close or Keep-Alive //or Upgrade
  char accept[2*dflen];
  char Encoding[12];
  //char accept[2*dflen];
  //char authorization[2*dflen];
  //--------------------------------------------------------------
  char contentLength[16];         //Content-Length: num
  char contentType[16];
  char* bodyptr;
  char* contentptr;
} Request;

int headlize(char* ptr);
int headhandler(struct Request* req,size_t slen,char* ptr);
struct Request* phaser(size_t len,char* buffer);

#endif