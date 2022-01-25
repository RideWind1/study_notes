#include <arpa/inet.h>
#include<stdio.h>
#include<iostream>
#include<sys/types.h>
#include<sys/socket.h>
#include<stdlib.h>
#include<string.h>
#include <unistd.h>
#include<sys/wait.h>
#include "wrap.h"
#include <pthread.h>
void* do_work(void *arg)
{
    int client_fd = *((int*)arg);
    char buf[10];
	while(1){
                int n=0;
                
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
	
}
struct client_fd{
    int cfd;
    bool isTakeUp;
    client_fd(){
        cfd = -1;
        isTakeUp=false;
    }

};
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


    struct client_fd cur_cfd[1024];
    pthread_t tid;
    int tmp_client_fd=0;
    while(1){
        //accept接受新的连接
        std::cout<<"等待新的连接..."<<std::endl;
        tmp_client_fd=Accept(ser_fd,(struct sockaddr *)&client_addr,&c_addrlen);
        int i;
        for(i =0;i<1024;i++){
            if(!cur_cfd[i].isTakeUp)
            break;
        }
        cur_cfd[i].cfd = tmp_client_fd;
        cur_cfd[i].isTakeUp = true;
        char cIP[16];
        memset(cIP,0,sizeof(cIP));
        std::cout<<"client TP:"<<inet_ntop(AF_INET,&client_addr.sin_addr.s_addr,cIP,sizeof(cIP))<<" port:"<<ntohs(client_addr.sin_port)<<std::endl;;
        if(tmp_client_fd<0){
            std::cout<<"accept失败！"<<std::endl;
        }
        std::cout<<"client:"<<tmp_client_fd<<std::endl;

        //创建子线程
        pthread_create(&tid,NULL,do_work,(void*)&cur_cfd[i].cfd);
        pthread_detach(tid);
        
        
    }
    close(ser_fd);
    return 0;
}