#ifndef _GLOBAL_H_
#define _GLOBAL_H_

#include<stdio.h>
#include<string.h>
#include<stdlib.h>

#define dflen 100
#define dftypelen 20
#define dfpage "/index.html"
#define FILEDIR "files"
#define FILEDIR2 "files/"
#define rn "\r\n"
#define page 4096
#define chunkborder 800
#define Bool int
#define true 1
#define false 0
#define stateLen 60
#define alivetime 60

#define OK 200
#define NotFound 404
#define BadRequest 400
#define InternalServerError 500
#define MethodNotAllowed 405
#define NotAcceptable 406
#define RequestURITooLarge 414
#define HTTPVersionnotsupported 505

#define OUTPUT_ERROR_INFOR  //条件编译，决定是否输出错误信息
#define OUTPUT_DEBUG_INFO   //条件编译，决定是否打印额外执行输出

#endif
