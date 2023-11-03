#ifndef ZERO_THREAD_H
#define ZERO_THREAD_H
#include <thread>

//封装线程类thread
class ZERO_Thread{
public:
    //构造函数
    ZERO_Thread();
    //析构函数
    virtual ~ZERO_Thread();
    bool start();
    void stop();
    bool isAlive() const ; //线程是否存活
    std::thread::id id(){return th_->get_id();} //id是内部类
    void join(); //等待当前线程结束 不能在当前线程上调用
    void detach();  //能在当前线程上调用
    static size_t CURRENT_THREADID();  //获取当前运行的线程的id 因为getid返回的是一个类对象 需要把这个类对象转化为整数
protected:
    void threaadEntry();
    virtual void run()=0; //需要继承的虚函数

    bool running_; //是否在运行
    std::thread *  th_;
};

#endif