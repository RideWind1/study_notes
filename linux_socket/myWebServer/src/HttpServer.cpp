#include"../include/HttpServer.h"
#include <sys/types.h>          
#include <sys/socket.h>

HttpServer::HttpServer(short port,const char*IP){
    int ser_fd = socket(AF_INET,SOCK_STREAM,0);
 
    struct sockaddr_in servaddr;
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_port = htons(port);
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = INADDR_ANY;
    //端口复用
    int flags = 1;
    setsockopt(ser_fd,SOL_SOCKET,SO_REUSEADDR,&flags,sizeof(flags));
    if(bind(lfd,(struct sockaddr*)&servaddr,sizeof(servaddr)) < 0){
        perror("bind err");
        exit(1);
    } 

}
HttpServer::~HttpServer(){
    close(ser_fd);
}
void HttpServer::run(){
    listen(ser_fd, 20);
    
}
