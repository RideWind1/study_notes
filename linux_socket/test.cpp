#include <event.h>
#include <stdio.h>
#include<iostream>
#include <unistd.h>
#include <pthread.h>
pthread_mutex_t mu;
pthread_cond_t con;
bool flag = true;

void* woker(void*arg){
    int n = *((int*)arg);
    
    std::cout<<"子线程"<<n<<" 来了"<<std::endl;
    while(true){
        pthread_mutex_lock(&mu);
        for(int i=0;i<n;i++){
            sleep(1);
            std::cout<<"子线程"<<n<<" 正在睡眠"<<std::endl;
        }
        while(flag==true){
            pthread_cond_wait(&con,&mu);
            for(int i =0;i<n;i++){
                sleep(1);
                std::cout<<"子线程"<<n<<" 正在睡眠"<<std::endl;
            }
        }
        if(flag==false){
            pthread_mutex_unlock(&mu);
            break;
        }
        pthread_mutex_unlock(&mu);
        
    }
    std::cout<<"子线程"<<n<<"退出循环"<<std::endl;
}
int main()
{
    pthread_t threads[4];
    pthread_mutex_init(&mu,NULL);
    pthread_cond_init(&con,NULL);
    int arr[4] = {2,4,6,8};
    for(int i=0;i<4;i++){
        pthread_create(&threads[i],NULL,woker,(void*)&arr[i]);
        sleep(1);
    }
    sleep(30);
    std::cout<<"主线程要解锁了！！"<<std::endl;
    flag = false;
    pthread_cond_broadcast(&con);
    pthread_exit(0);

}