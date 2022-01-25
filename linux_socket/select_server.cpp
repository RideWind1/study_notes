#include <arpa/inet.h>
#include<stdio.h>
#include<iostream>
#include<sys/types.h>
#include<sys/socket.h>
#include<stdlib.h>
#include<string.h>
#include <unistd.h>
#include<sys/wait.h>
#include <signal.h>
#include "wrap.h"
int main(){
    int ser_fd=0;
    ser_fd = Socket(AF_INET,SOCK_STREAM,0);
    struct sockaddr_in ser_addr;
    struct sockaddr_in client_addr;
    memset(&ser_addr,0,sizeof(ser_addr));
    ser_addr.sin_family = AF_INET;
    ser_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    ser_addr.sin_port = htons(6666);
    socklen_t c_addrlen=sizeof(client_addr);
    Bind(ser_fd,(struct sockaddr *)&ser_addr,sizeof(ser_addr));
    Listen(ser_fd,128);


    int client_fd;
    pid_t pid;
    fd_set tmp_set,rset;
    FD_ZERO(&rset);
    FD_SET(ser_fd,&rset);
    int maxfd = ser_fd;
    while(1){
        //select
        tmp_set = rset;
        int nready = select(maxfd,&tmp_set,NULL,NULL,NULL);
        if(nready<0){
            perr_exit("select error");
        }
        //accept接受新的连接
        std::cout<<"等待新的连接..."<<std::endl;
        client_fd=Accept(ser_fd,(struct sockaddr *)&client_addr,&c_addrlen);
        char cIP[16];
        memset(cIP,0,sizeof(cIP));
        std::cout<<"client TP:"<<inet_ntop(AF_INET,&client_addr.sin_addr.s_addr,cIP,sizeof(cIP))<<" port:"<<ntohs(client_addr.sin_port)<<std::endl;;
        if(client_fd<0){
            std::cout<<"accept失败！"<<std::endl;
        }
        std::cout<<"client:"<<client_fd<<std::endl;

        //创建子进程
        
        pid= fork();
        if(pid==0){
            close(ser_fd);
            while(1){
                int n=0;
                char buf[1024];
                memset(buf,0,sizeof(buf));
                std::cout<<"read之前"<<std::endl;
                n= read(client_fd,buf,sizeof(buf));
                if(n<=0)
                break;
                std::cout<<"read之后"<<std::endl;
                std::cout<<buf<<std::endl;
                for(int i=0;i<n;i++){
                    buf[i]=toupper(buf[i]);
                }
                write(client_fd,buf,n);
                
            }
            close(client_fd);
            exit(0);
        }else if(pid==-1){
            std::cout<<"error"<<std::endl;
            
            exit(1);
        }else{
            close(client_fd);
        }
    }
    close(ser_fd);
    return 0;
}