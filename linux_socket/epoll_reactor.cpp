#include <arpa/inet.h>
#include<stdio.h>
#include<iostream>
#include<sys/types.h>
#include<sys/socket.h>
#include<stdlib.h>
#include<string.h>
#include <sys/epoll.h>

#include <unistd.h>
#include <fcntl.h>

#include "wrap.h"
#define MAX_EVENTS 1024
#define MAX_LINE 10

struct my_events {
    int m_fd; //监听的文件描述符
    void *m_arg; //泛型参数
    void (*call_back)(int,my_events*); //回调函数
};
my_events event[1024];
void eventDel(int,my_events*);

void eventDel(int efd,my_events*ex){
    std::cout<<"删除节点了"<<ex->m_fd<<std::endl;
    ex->m_fd = 0;
    epoll_ctl(efd,EPOLL_CTL_DEL,ex->m_fd,nullptr);
}
void eventAdd(int efd,int fd,void (*call_back)(int,my_events*),my_events* mev){
    std::cout<<"添加节点了"<<fd<<std::endl;
    mev->m_fd = fd;
    mev->call_back = call_back;
    epoll_event eev;
    eev.data.ptr = (void*)mev;
    eev.events = EPOLLIN|EPOLLET;
    
    epoll_ctl(efd,EPOLL_CTL_ADD,fd,&eev);
}
void readData(int efd,my_events*ex){
    std::cout<<ex->m_fd<<"读数据了"<<std::endl;
    char buf[MAX_LINE];
    memset(buf,0,sizeof(buf));
    int n=0;
    while(1){
        n = Read(ex->m_fd,buf,10);
        std::cout<<"读一次->"<<buf<<"n="<<n<<std::endl;
        if(n<0){
            break;
        }else{
            if(n==0){
                std::cout<<ex->m_fd<<"断开连接了"<<std::endl;
                std::cout<<"n = "<<n<<std::endl;
                
                close(ex->m_fd);
                eventDel(efd,ex);
                break;
            }
        }
        write(ex->m_fd,buf,n);
        std::cout<<"写一次->"<<buf<<std::endl;
    }
}
void getAccept(int efd,my_events*ex){
    //接受连接
    std::cout<<"getaccrptTTTTTTTT"<<std::endl;
    struct sockaddr_in client_addr;
    socklen_t c_addrlen=sizeof(client_addr);
    int client_fd=Accept(ex->m_fd,(struct sockaddr *)&client_addr,&c_addrlen);
    char cIP[16];
    memset(cIP,0,sizeof(cIP));
    std::cout<<"client TP:"<<inet_ntop(AF_INET,&client_addr.sin_addr.s_addr,cIP,sizeof(cIP))<<" port:"<<ntohs(client_addr.sin_port)<<std::endl;;
    if(client_fd<0){
        std::cout<<"accept失败！"<<std::endl;
    }
    std::cout<<"client:"<<client_fd<<std::endl;
    int i=0;
    for(;i<MAX_EVENTS;i++){
        if(event[i].m_fd==0){
            break;
        }
    }

    //设置为非阻塞
    int flag=0;
    flag = fcntl(client_fd, F_GETFL);
	flag |= O_NONBLOCK;
	fcntl(client_fd, F_SETFL, flag);

    //添加事件
    eventAdd(efd,client_fd,readData,&event[i]);

}

int main(){
    int ser_fd=0;
    ser_fd = Socket(AF_INET,SOCK_STREAM,0);
    std::cout<<"ser_fd = "<<ser_fd<<std::endl;
    struct sockaddr_in ser_addr;
    struct sockaddr_in client_addr;
    memset(&ser_addr,0,sizeof(ser_addr));
    ser_addr.sin_family = AF_INET;
    ser_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    ser_addr.sin_port = htons(6666);
    socklen_t c_addrlen=sizeof(client_addr);
    Bind(ser_fd,(struct sockaddr *)&ser_addr,sizeof(ser_addr));
    Listen(ser_fd,128);

    //epoll
    int efd = epoll_create(10);
    eventAdd(efd,ser_fd,getAccept,&event[MAX_EVENTS-1]);

    epoll_event getevents[MAX_EVENTS];
    while(1){
        std::cout<<"epoll_wait"<<std::endl;
        int nready = epoll_wait(efd,getevents,1024,-1);
        std::cout<<nready<<std::endl;
        for(int i=0;i<nready;i++){
            my_events* tmp_ex = (my_events*)getevents[i].data.ptr;
            std::cout<<"tmp_ex->m_fd = "<<tmp_ex->m_fd<<std::endl;
            if(EPOLLIN & getevents[i].events){
                tmp_ex->call_back(efd,tmp_ex);
            }
        }
    }
    return 0;

}