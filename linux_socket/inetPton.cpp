#include <arpa/inet.h>
#include<stdio.h>
#include<iostream>
#include<sys/types.h>
#include<sys/socket.h>
#include<stdlib.h>
#include<string.h>
#include <unistd.h>
int main(){
    int ser_fd=0;
    ser_fd = socket(AF_INET,SOCK_STREAM,0);//创建socket,得到监听文件描述符
    if(ser_fd<0){
        std::cout<<"创建socket失败！"<<std::endl;
        return -1;
    }
    struct sockaddr_in ser_addr;
    struct sockaddr_in client_addr;
    memset(&ser_addr,0,sizeof(ser_addr));
    ser_addr.sin_family = AF_INET;
    ser_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    ser_addr.sin_port = htons(6666);
    socklen_t c_addrlen=sizeof(client_addr);
    int rsl = bind(ser_fd,(struct sockaddr *)&ser_addr,sizeof(ser_addr));//将ip和端口号绑定到监听文件描述符
    if(rsl <0){
        std::cout<<"bind失败！"<<std::endl;
        return -1;
    }
    rsl = listen(ser_fd,128);
    if(rsl <0){
        std::cout<<"listen失败！"<<std::endl;
        return -1;
    }
    int client_fd=accept(ser_fd,(struct sockaddr *)&client_addr,&c_addrlen);
    char cIP[16];
    memset(cIP,0,sizeof(cIP));
    std::cout<<"client TP:"<<inet_ntop(AF_INET,&client_addr.sin_addr.s_addr,cIP,sizeof(cIP))<<" port:"<<ntohs(client_addr.sin_port)<<std::endl;;
    if(client_fd<0){
        std::cout<<"accept失败！"<<std::endl;
    }
    std::cout<<"client:"<<client_fd<<std::endl;
    int n=0;
    char buf[1024];
    while(1){
        //读数据
        memset(buf,0,sizeof(buf));
        std::cout<<"read之前"<<std::endl;
        n= read(client_fd,buf,sizeof(buf));
        if(n<0)
        break;
        std::cout<<"read之后"<<std::endl;
        std::cout<<buf<<std::endl;
        for(int i=0;i<n;i++){
            buf[i]=toupper(buf[i]);
        }
        write(client_fd,buf,n);
        sleep(10);
    }
    close(ser_fd);
    close(client_fd);
    return 0;
}