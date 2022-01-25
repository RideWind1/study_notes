#include <signal.h>
#include<iostream>
#include <unistd.h>
void do_sigchild(int signo)
{
    //signal(signo, sigint_handler);
    for(int i=0;i<5;i++){
        std::cout<<"sigint_handler sleep "<<i<<std::endl;
        sleep(1);
    }
}
int main(){



    struct sigaction newact;
	newact.sa_handler = do_sigchild;
	sigemptyset(&newact.sa_mask);
	newact.sa_flags = 0;
	sigaction(SIGINT, &newact, NULL);
    for(int i=0;i<15;i++){
        std::cout<<"sleep "<<i<<std::endl;
        sleep(2);
    }




    return 0;
}