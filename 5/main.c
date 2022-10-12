#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>

char * Notes[64];
char method[0x10];
char path[0x200];
char version[0x10];

unsigned int content_length;
char content[0x200];

int listen_fd = -1;


char Base64[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

void b64encode(char*str,char*ret)
{
	char tmp[0x100];
	memset(tmp,0,sizeof(tmp));

	int in_len = strlen(str);
	int newlen = (in_len + 2) / 3 * 3;
	//extra 是需要补的字节数
	int Extra = newlen - in_len;

	memcpy(tmp, str, in_len);
	int out_len = newlen * 4 / 3;

	for (int i = 0; i < newlen / 3; i++)
	{
		char*pSrc = tmp + i * 3;
		char*pOut = ret + i * 4;
		pOut[0] = Base64[(pSrc[0] >> 2) & 0x3F];
		pOut[1] = Base64[((pSrc[0] << 4) & 0x30) | ((pSrc[1] >> 4) & 0x0F)];
		pOut[2] = Base64[((pSrc[1] << 2) & 0x3C) | ((pSrc[2] >> 6) & 0x03)];
		pOut[3] = Base64[(pSrc[2] & 0x3f)];
	}
	memset(&ret[out_len - Extra], '=', Extra);
}

void sendfile(FILE*sock,FILE*res){
    //get file size....
    size_t len = 0;
    fseek(res,0,SEEK_END);
    len = ftell(res);
    fseek(res,0,SEEK_SET);

    //send response line and headers.
    fprintf(sock,"HTTP/1.1 200\r\n");
    fprintf(sock,"Content-Length: %ld\r\n",len);
    fprintf(sock,"Connection: close\r\n");
    fprintf(sock,"\r\n");

    while(len>0){
        char buffer[0x1000];

        int part = len < 0x1000 ? len : 0x1000;
        int nRead = 0,nWrite = 0;

        nRead = fread(buffer,1,part,res);
        nWrite = fwrite(buffer,1,nRead,sock);

        if(nRead != nWrite){
            return;
        }
        len -= nWrite;
    }
}

void Response(FILE*fp,char*statu_line,char*content_type,char*content){

    fprintf(fp,"%s\r\n",statu_line);
    
    if(content && strlen(content)){
        fprintf(fp,"Content-Type: %s\r\n",content_type);
        fprintf(fp,"Content-Length: %ld\r\n",strlen(content));
    }
    
    fprintf(fp,"Connection: close\r\n");
    fprintf(fp,"\r\n");

    if(content && strlen(content)){
        fprintf(fp,"%s",content);
    }
}

void add(FILE*fp){
    int content_size = content_length;
    char*p = content;
    
    if(content_size < 4){
        goto _error;
    }

    int idx = ((int*) p)[0];
    int size = content_size - 4;

    char* data = p + sizeof(int);

    if(idx >= 64 || Notes[idx]){
        goto _error;
    }

    Notes[idx] = (char*)malloc(size);
    if(Notes[idx] == NULL){
        goto _error;
    }
    memcpy(Notes[idx],data,size);
    Response(fp,"HTTP/1.1 200","application/json","{\"statu\":\"success\"}");
    return;

_error:
    Response(fp,"HTTP/1.1 200","application/json","{\"statu\":\"error\"}");
}


void edit(FILE*fp){
    int content_size = content_length;
    char*p = content;
    
    if(content_size < 4){
        goto _error;
    }
    
    int idx = ((int*) p)[0];
    int size = content_size - 4;
    char* data = p + sizeof(int);

    if(idx >= 64 || Notes[idx] == 0){
        goto _error;
    }
    memcpy(Notes[idx],data,size);
    Response(fp,"HTTP/1.1 200","application/json","{\"statu\":\"success\"}");
    return;
_error:
    Response(fp,"HTTP/1.1 200","application/json","{\"statu\":\"error\"}");
}

void show(FILE*fp){
    int content_size = content_length;
    char*p = content;
    char result[0x200];
    char b64text[0x100];
    int idx = 0;

    if(content_size < 4){
        goto _error;
    }
    
    idx = ((int*) p)[0];

    if(idx >= 64 || Notes[idx] == 0){
        goto _error;
    }
    
    if(strlen(Notes[idx]) > 192){
	    goto _error;
    }
    memset(b64text,0,sizeof(b64text));
    memset(result,0,sizeof(result));

    b64encode(Notes[idx],b64text);

    snprintf(result,0x200,"{\"statu\":\"success\",\"content\":\"%s\"}",b64text);
    Response(fp,"HTTP/1.1 200","application/json",result);
    return;
_error:
    Response(fp,"HTTP/1.1 200","application/json","{\"statu\":\"error\"}");
}
void delete(FILE*fp){
    int content_size = content_length;
    char*p = content;
    
    if(content_size < 4){
        goto _error;
    }
    
    int idx = ((int*) p)[0];
    if(idx >= 64 || Notes[idx] == 0){
        goto _error;
    }
    free(Notes[idx]);
    Notes[idx] = 0;
    Response(fp,"HTTP/1.1 200","application/json","{\"statu\":\"success\"}");
    return;
_error:
    Response(fp,"HTTP/1.1 200","application/json","{\"statu\":\"error\"}");
}



void get_handler(FILE*fp){
    printf("GET %s\n",path);

    char*p = path;
    if(*p++ != '/'){
        return ;
    }
    if(strstr(p,"flag")){
        Response(fp,"HTTP/1.1 200","text/html","<html>What are you doing???? </html>");
        return;
    }
    if(*p == 0){
        strcat(p,"index.html");
    }
    FILE*res = fopen(p,"rb");
    if(res){
        sendfile(fp,res);
        fclose(res);
        return;
    }
    Response(fp,"HTTP/1.1 404","text/html;","<html>404 Not Found</html>");
}


void post_handler(FILE*fp){
    char*p = path;
    if(*p++ != '/'){
        return ;
    }
    if (!strcasecmp(p,"add")){
        add(fp);
    }else if(!strcasecmp(p,"show")){
        show(fp);
    }else if(!strcasecmp(p,"delete")){
        delete(fp);
    }else if(!strcasecmp(p,"edit")){
        edit(fp);
    }else{
        Response(fp,"HTTP/1.1 404","text/html;","<html>404 Not Found</html>");
    }
}

void connection_handler(int fd){
    char buff[64];
    FILE*fp = fdopen(fd,"a+");
    setvbuf(fp,NULL,_IONBF,0);

    fscanf(fp,"%16s%512s%16s",method,path,version);
    fgets(buff,64,fp);

    if(strcmp(buff,"\r\n") && strcmp(buff,"\n")){
        goto _error;
    }

    //clean
    content_length = 0;
    memset(content,0,sizeof(content));

    while(1){
        char buffer[0x200] = {0};
        fgets(buffer,0x200,fp);
        if(buffer[0] == 0){
            goto _error;               //invalid headers;
        }
        if(!strcmp(buffer,"\r\n")){     //end
            break;;
        }
        char*splitChar = strchr(buffer,':');
        if(splitChar){
            *splitChar++ = 0;
            if(!strncasecmp(buffer,"content-length",14)){
                content_length = atoi(splitChar);
            }
        }else{
            goto _error;
        }
    }

    //read content
    if(content_length > 0 && content_length < 0x200){
        char *p = content;
        size_t left = content_length;
        int nRead = 0;

        while(left > 0){
            nRead = fread(p,1,left,fp);
            left -= nRead;
            p+=nRead;
        }
    }

    printf("%s %s\n",method,path);

    if(!strcasecmp(method,"get")){
        get_handler(fp);
    }else if(!strcasecmp(method,"post")){
        post_handler(fp);
    }

_error: 
    fflush(fp);
    fclose(fp); //不需要自己调用close
}


int main(int argc,char* argv[]){
    int port = 80;
    struct sockaddr_in addr = {0};

    setbuf(stdin,0);
    setbuf(stdout,0);
    setbuf(stderr,0);

    if(argc > 1){
        port = atoi(argv[1]);
    }
    if(argc > 2){
        chdir(argv[2]);
    }
   
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = INADDR_ANY;

    listen_fd = socket(AF_INET,SOCK_STREAM,0);

    if(0 != bind(listen_fd,(struct sockaddr*)&addr,sizeof(addr))){
        fprintf(stderr,"bind failed!");
        exit(0);
    }
    if(0 != listen(listen_fd,64)){
        fprintf(stderr,"listen failed!");
        exit(0);
    }
    puts("listening....");
    while(1){
        struct sockaddr_in client_addr = {0};
        socklen_t len = sizeof(client_addr);

        int client = accept(listen_fd,(struct sockaddr*)&client_addr,&len);

        if(client >= 0){
            connection_handler(client);
        }
    }
    close(listen_fd);
    return 0;
}

