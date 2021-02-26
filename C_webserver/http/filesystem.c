#include"filesystem.h"


int createfile(char *name,struct Request* req){
    struct FileInfo* rst;
    FILE *fp;
    int len;
    char* actpath;

    actpath = (char*)malloc(strlen(FILEDIR2)+strlen(name)+1);
    memset(actpath,0,strlen(FILEDIR2)+strlen(name)+1);
    memcpy(actpath,FILEDIR2,strlen(FILEDIR2));
    memcpy(actpath+strlen(FILEDIR2),name,strlen(name));
    
    fp = fopen(actpath,"w");
    if(fp==NULL) return false;

    if(memcmp(req->Encoding,"chunked",7)==0){
        Socket sock = *((Socket*)req->bodyptr);
        char* entry;
        size_t entrylen;
        while(1){
            entry = RecvOneChunk(sock,&entrylen);
            if(entry<=0) {free(entry);break;}
            fwrite(entry,entrylen,1,fp);
            free(entry);
        }
    }else{
        fwrite(req->contentptr,atol(req->contentLength),1,fp);
    }
    fclose(fp);
    free(actpath);
    return true;
}

struct FileInfo* file2body(char *path){
    //错误则返回NULL，可能需要响应错误码
    //记得free
    struct FileInfo* rst;
    FILE *fp;
    int len;

    //if(access(path, F_OK)!= 0) return NULL;
    
    char* actpath;
    if(strlen(path)<=1){
        actpath = (char*)malloc(strlen(FILEDIR)+strlen(dfpage)+1);
        memset(actpath,0,strlen(FILEDIR)+strlen(dfpage)+1);
        memcpy(actpath,FILEDIR,strlen(FILEDIR));
        memcpy(actpath+strlen(FILEDIR),dfpage,strlen(dfpage));
    }else{
        actpath = (char*)malloc(strlen(FILEDIR)+strlen(path)+1);
        memset(actpath,0,strlen(FILEDIR)+strlen(path)+1);
        memcpy(actpath,FILEDIR,strlen(FILEDIR));
        memcpy(actpath+strlen(FILEDIR),path,strlen(path));
    }
    
    fp = fopen(actpath,"r");
    if(fp==NULL) return NULL;
    fseek(fp,0,SEEK_END);
    len = ftell(fp);
    if(len<=0) return NULL;
    rst = (struct FileInfo*)malloc(sizeof(struct FileInfo));
    getType(actpath,rst->type);
    rst->len = len;
    if(len>=chunkborder) rst->chunked = true;
    else rst->chunked = false;
    rst->fpath = actpath;
    return rst;
}

int GetFileLength(char *path)
{
    FILE *fp = fopen(path, "rb");
    if(fp==NULL)
    {
#ifdef OUTPUT_ERROR_INFOR
        printf("打开客户端请求文件出错，文件名为%s\n",path);
#endif
        return -1;
    }

    fseek(fp, 0, SEEK_END);
    int length = ftell(fp);

    fclose(fp);
    return length;
}

int EndWithString(char *str1, char *str2)
{
	if(strlen(str1)<strlen(str2))
	{
		return 0;
	}

	char* p = str1+ (strlen(str1) - strlen(str2));

	if(strcmp(p,str2)!=0)
	{
		return 0;
	}
	return 1;
}

void GetFileName(char *path, char *result)
{
	char *p = path + strlen(path);
	while (*p != '/' && (p >= path))
	{
		p--;
	}

	p++;
	while(*p!='\0')
	{
		*result = *p;
		result++;
		p++;
	}
	*result = '\0';
}

void getType(char* file_name,char* extension){
    char* tmp = strstr(file_name,".");
    if(tmp==NULL) return;
    size_t len = strlen(file_name);
    size_t bias = tmp-file_name;
    memcpy(extension,tmp+1,len-bias);
}