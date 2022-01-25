
#include<pthread.h>
#include<vector>

class Threadpool{
private:
    pthread_mutex_t mu;
    pthread_cond_t con;
    int max_pthread;
    vector<pthread_t> threads;
    bool shutdown;
public:
    Threadpool(int max_p):max_pthread(max_p){
        shutdown = false;
        threads.resize(max_p);
        for(int i=0;i<max_pthread;i++){
            int ret = pthread_create(&threads[i], NULL,woker, NULL);
            if(ret != 0 ){
                std::cout<<"出错了"<<std::endl;
            }
        }

    }
    void *woker(void*arg){
        TaskPool* task = NULL;
        while(true){
            pthread_mutex_lock(&mu);
            while(tasks.empty()&&!shutdown){
                pthread_cond_wait(&con,&mu);
            }
            if(shutdown){
                pthread_mutex_unlock(&mu);
                break;
            }
            task  = tasks.front();
            tasks.pop_front();
            pthread_mutex_unlock(&mu);
            task->doTask();
        }
        
    }
    ~Threadpool(){

    }
    void ShutDown(){
        shutdown = true;
        pthread_cond_broadcast(&con);
    }
}