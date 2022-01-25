#include <sys/epoll.h>

struct my_events {
    int m_fd; //监听的文件描述符
    void *m_arg; //泛型参数
    void (*call_back)(int,my_events*); //回调函数
};
class Epoll{
private:
    int efd;
    public:
    Epoll(){
        efd = epoll_create(4);
    }
    
}