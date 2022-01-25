#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/epoll.h>
 
#define _BUF_LEN_  1024
#define _EVENT_SIZE_ 1024
 
//全局epoll树的根
int gepfd = 0;
 
void readData(int fd,int events,void *arg);
//事件驱动结构体
typedef struct xx_event{
    int fd;
    int events;
    void (*call_back)(int fd,int events,void *arg);
    void *arg;
    char buf[1024];
    int buflen;
    int epfd;
}xevent;
 
xevent myevents[_EVENT_SIZE_];
//添加事件
//eventadd(lfd,EPOLLIN,initAccept,&myevents[_EVENT_SIZE_-1],&myevents[_EVENT_SIZE_-1]);
void eventadd(int fd,int events,void (*call_back)(int ,int ,void *),void *arg,xevent *ev)
{
    ev->fd = fd;
    ev->events = events;
    ev->arg = arg;//代表结构体自己,可以通过arg得到结构体的所有信息
    ev->call_back = call_back;
    struct epoll_event epv;
    epv.events = events;
    epv.data.ptr = ev;//核心
    epoll_ctl(gepfd,EPOLL_CTL_ADD,fd,&epv);//上树
}
//修改事件
//eventset(fd,EPOLLOUT,senddata,arg,ev);
void eventset(int fd,int events,void (*call_back)(int ,int ,void *),void *arg,xevent *ev)
{
    ev->fd = fd;
    ev->events = events;
    ev->arg = arg;
    ev->call_back = call_back;
    struct epoll_event epv;
    epv.events = events;
    epv.data.ptr = ev;
    epoll_ctl(gepfd,EPOLL_CTL_MOD,fd,&epv);//修改
}
//删除事件
void eventdel(xevent *ev,int fd,int events)
{
    printf("begin call %s\n",__FUNCTION__);
    ev->fd = 0;
    ev->events = 0;
    ev->call_back = NULL;
    memset(ev->buf,0x00,sizeof(ev->buf));
    ev->buflen = 0;
    struct epoll_event epv;
    epv.data.ptr = NULL;
    epv.events = events;
    epoll_ctl(gepfd,EPOLL_CTL_DEL,fd,&epv);//下树
}
//发送数据
void senddata(int fd,int events,void *arg)
{
    printf("begin call %s\n",__FUNCTION__);
    xevent *ev = arg;
    write(fd,ev->buf,ev->buflen);
    eventset(fd,EPOLLIN,readData,arg,ev);
}
//读数据
void readData(int fd,int events,void *arg)
{
    printf("begin call %s\n",__FUNCTION__);
    xevent *ev = arg;
 
    ev->buflen = read(fd,ev->buf,sizeof(ev->buf));
    if(ev->buflen > 0){//读到数据
        //void eventset(int fd,int events,void (*call_back)(int ,int ,void *),void *arg,xevent *ev)
        eventset(fd,EPOLLOUT,senddata,arg,ev);
 
    }else if(ev->buflen == 0){//对方关闭
        close(fd);
        eventdel(ev,fd,EPOLLIN);
    }
 
}
//新连接处理
void initAccept(int fd,int events,void *arg)
{
    printf("begin call %s,gepfd =%d\n",__FUNCTION__,gepfd);//__FUNCTION__ 函数名
    struct sockaddr_in addr;
    socklen_t len = sizeof(addr);
    int cfd = accept(fd,(struct sockaddr*)&addr,&len);
    int i = 0;
    for(i = 0 ; i < _EVENT_SIZE_; i ++){
        if(myevents[i].fd == 0){
            //可以使用的
            break;
        }
    }
    //设置读事件
    eventadd(cfd,EPOLLIN,readData,&myevents[i],&myevents[i]);
}
 
int main(int argc,char *argv[])
{
    int lfd = socket(AF_INET,SOCK_STREAM,0);
 
    struct sockaddr_in servaddr,clientaddr;
    servaddr.sin_addr.s_addr = 0x00;
    servaddr.sin_port = htons(8888);
    servaddr.sin_family = AF_INET;
    //端口复用
    int flags = 1;
    setsockopt(lfd,SOL_SOCKET,SO_REUSEADDR,&flags,sizeof(flags));
    if(bind(lfd,(struct sockaddr*)&servaddr,sizeof(servaddr)) < 0){
        perror("bind err");
        exit(1);
    }
     
    listen(lfd,120);
    gepfd = epoll_create(1024);//创建根节点
    printf("gepfd === %d\n",gepfd);
    struct epoll_event events[1024];
    //添加最初始事件，将侦听的描述符上树
    eventadd(lfd,EPOLLIN,initAccept,&myevents[_EVENT_SIZE_-1],&myevents[_EVENT_SIZE_-1]);
 
    while(1){
        int ret = epoll_wait(gepfd,events,1024,-1);
        //printf("ret == %d\n",ret);
        if(ret > 0){
            int i = 0;
             
            for(i = 0;i < ret ; i ++){
                xevent *xe = events[i].data.ptr;//取ptr 指向结构体地址
                printf("fd=%d\n",xe->fd);
                if(xe->events & events[i].events){
                    xe->call_back(xe->fd,xe->events,xe);//调用事件对应的回调
                }
            }
        }
       // sleep(1);
    }
 
    return 0;
}