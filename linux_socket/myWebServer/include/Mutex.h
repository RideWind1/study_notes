#include<pthread.h>

class Mutex{
    private:
    pthread_mutex_t mu;
    public:
    Mutex(){
        pthread_mutex_init(&mu,NULL);
    }
    MutexLock(){
        pthread_mutex_lock(&mu);
    }
    MutexUnlock(){
        pthread_mutex_unlock(&mu);
    }
    ~Mutex(){
        pthread_mutex_destroy(&mu);
    }

}