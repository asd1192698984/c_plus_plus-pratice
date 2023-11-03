#include "zero_thread.h"
#include <iostream>

class A :public ZERO_Thread{
public:
    void run(){
        while(running_){
            std::cout<<"A"<<std::endl;
            std::cout<<CURRENT_THREADID()<<std::endl;
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    }
};
class B :public ZERO_Thread{
public:
    void run(){
        while(running_){
            std::cout<<"B"<<std::endl;
            std::cout<<CURRENT_THREADID()<<std::endl;
             std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    }
};
int main(){
    A a;
    B b;
    a.start();
    b.start();
    std::this_thread::sleep_for(std::chrono::seconds(1));
    a.stop();
    a.join();
    b.stop();
    b.join();
    // b.stop();
    return 0;
}