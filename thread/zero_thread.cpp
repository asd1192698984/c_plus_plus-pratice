#include "zero_thread.h"
#include <sstream>
#include <iostream>
#include <exception>

ZERO_Thread::ZERO_Thread():
running_(false),th_(NULL)
{   

}

//析构函数
//主要是为了回收th的内存空间
ZERO_Thread::~ZERO_Thread(){
    if(th_!=NULL){
        if(th_->joinable()){ //还没有join 让线程detach 
            std::cout << "~ZERO_Thread detach\n";
            th_->detach();
        }
        delete th_; //回收内存
        th_=NULL;
    }
     std::cout << "~ZERO_Thread()" << std::endl;
}


//启动线程 其实就是创建线程
bool ZERO_Thread::start(){  
    if(running_){ //如果线程已经启动了 不做任何处理
        return false; //启动失败 因为已经启动了
    } 
    //创建线程 执行任务
    //放一个try catch块捕获异常
    try{
        //这里并不将running置为true
    th_=new std::thread(ZERO_Thread::threaadEntry,this);  //如果传递的是类函数 需要传递类
    }catch(...){  //捕获所有异常
        throw "[ZERO_Thread::start] thread start error]";
    }
    return true;
}

void ZERO_Thread::stop(){
    running_=false;
}

bool ZERO_Thread::isAlive() const {
    return running_;
}

void ZERO_Thread::join(){
    if(th_->joinable()){ //先判断是否能够join  已经join的不能join detach的不能join
        th_->join(); 
    }
    
}

void ZERO_Thread::detach(){
    th_->detach();
}

size_t ZERO_Thread::CURRENT_THREADID(){
    // 声明为thread_local的本地变量在线程中是持续存在的，不同于普通临时变量的生命周期，
    // 它具有static变量一样的初始化特征和生命周期，即使它不被声明为static。
    static thread_local size_t threadId=0;
    if(threadId ==0){ //如果还没有初始化
        std::stringstream ss; //将id转化为整数
        ss << std::this_thread::get_id();
        threadId = strtol(ss.str().c_str(), NULL, 0);
    }
    return threadId;
}

//当线程创建完毕后才放置running为true
void ZERO_Thread::threaadEntry(){
    running_=true;
    try{
        run(); //调用子类虚函数
    }catch(std::exception &ex){ //这里捕获异常是为了 防止发生异常 running的值没有被恢复
        running_ = false;
        throw ex; //继续外抛
    }catch(...){
        running_=false;
        throw ;
    }
}