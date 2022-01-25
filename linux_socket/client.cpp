
#include <arpa/inet.h>
#include<stdio.h>
#include<iostream>
#include<sys/types.h>
#include<sys/socket.h>
#include<stdlib.h>
#include<string.h>
#include <unistd.h>

int main(){
    int client_fd = socket(AF_INET,SOCK_STREAM,0);
    if(client_fd<0){
        std::cout<<"socks失败！"<<std::endl;
        return -1;

    }
    struct sockaddr_in ser_addr;
    ser_addr.sin_family = AF_INET;
    ser_addr.sin_port = htons(6666);
    inet_pton(AF_INET, "127.0.0.1", &ser_addr.sin_addr.s_addr);
    int ret = connect(client_fd, (struct sockaddr *)&ser_addr, sizeof(ser_addr));
    if(ret<0){
        std::cout<<"connect失败！"<<std::endl;
        return -1;
    }
    int n=0;
    char buf[256];
    char buf2[] ="z";
    while(1){
        memset(buf,0,sizeof(buf));
        std::cout<<"请输入数据"<<std::endl;
        n=read(STDIN_FILENO,buf,sizeof(buf));
        std::cout<<"发送数据...";
        write(client_fd,buf,n);
        write(client_fd,buf2,sizeof(buf2));
        std::cout<<"发送完成"<<std::endl;
        memset(buf,0,sizeof(buf));
        n= read(client_fd,buf,sizeof(buf));
        std::cout<<"收到数据"<<std::endl;
        if(n<=0)
        {
            std::cout<<"对方关闭连接或者读出错了！"<<std::endl;
        }
        std::cout<<buf<<std::endl;
    }
    close(client_fd);


    return 0;
}