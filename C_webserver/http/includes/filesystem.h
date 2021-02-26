#ifndef _FILESYSTEM_H_
#define _FILESYSTEM_H_

#include"global.h"
#include"response.h"
#include"request.h"
#include"TCP.h"

int createfile(char *name,struct Request* req);

struct FileInfo* file2body(char *path);

int GetFileLength(char* path);
/*****************************************************
 * 功能：返回一个磁盘文件的字节长度
 *
 * 返回值：成功时返回字节数，失败返回-1
*****************************************************/

int EndWithString(char *str1, char *str2);

void GetFileName(char *path, char *result);

void getType(char* file_name,char* extension);

#endif