#ifndef ZERO_ThreadPool_H
#define ZERO_ThreadPool_H

#include <future>
#include <functional>
#include <iostream>
#include <queue>
#include <mutex>
#include <memory>
#include <vector>


#ifdef WIN32  //区分平台
#include <windows.h>
#else
#include <sys/time.h>
#endif

using namespace std;


void getNow(timeval *tv);
int64_t getNowMs();

#define TNOW      getNow()
#define TNOWMS    getNowMs()

class ZERO_ThreadPool{
protected:
    //封装一个任务结构体，带任务和超时的时间
    struct TaskFunc
    {
        TaskFunc(uint64_t expireTime):_expireTime(expireTime)
        {}
        /* data */
        std::function<void()> _func;
        int64_t _expireTime =0;
    };
    typedef  shared_ptr<TaskFunc> TaskFuncPtr ;
public:
    ZERO_ThreadPool();
    
    virtual  ~ZERO_ThreadPool();

    /**
     * @brief 初始化
    */
    bool init(size_t num);

    /**
     * @brief 获取线程个数
    */
    // size_t 
    size_t getThreadNum(){
        //加锁
        std::unique_lock<std::mutex> locker(mutex_);
        return threadNum_;
    }
     /**
    * @brief 获取当前线程池的任务数
    *
    * @return size_t 线程池的任务数
    */
    size_t getJobNum()
    {
        std::unique_lock<std::mutex> lock(mutex_);
        return tasks_.size();
    }
    /**
    * @brief  并回收空间
    */
    void stop();
    /**
     * @brief 创建所有线程
    */
    bool start();
    /**
     * @brief 等待所有工作结束 
     * @param millsecond  等待的时间(ms)  -1会永远等待
     * @return 
    */
    bool waitForAllDown(int millsecond=-1);

    //核心的方法 添加任务 class和typename没有区别

    template<class F,class... Args>
     auto exec(F&& f,Args&&... args)->std::future<decltype(f(args...))>
     {
        return  exec(0,f,args...);
     }
        template <class F, class... Args>
    auto exec(int64_t timeoutms, F&& f, Args&&... args) -> std::future<decltype(f(args...))>
    {
        int64_t expiretime =  (timeoutms == 0 ? 0 : TNOWMS + timeoutms);  // 获取现在时间
        //定义返回值类型
        using Rettype = decltype(f(args...));  // 推导返回值
        // 封装任务
        // std::bind的返回值是可调用实体，可以直接赋给std::function。
        //作为参数直接传给packaged_task的构造函数 

        auto task = std::make_shared<std::packaged_task<Rettype()>>(std::bind(std::forward<F>(f), std::forward<Args>(args)...));
        //shared_ptr<taskfunc> 类型
        TaskFuncPtr fptr = std::make_shared<TaskFunc>(expiretime);  // 封装任务指针，设置过期时间
        fptr->_func = [task]() {  // 具体执行的函数  传递一个lambda函数给std::function<void ()> 
                                    //延迟执行
            (*task)();
        };

        std::unique_lock<std::mutex> lock(mutex_);
        tasks_.push(fptr);              // 插入任务  还没有执行
        condition_.notify_one();        // 唤醒阻塞的线程，可以考虑只有任务队列为空的情况再去notify
        return task->get_future();  //如果有返回值的需求可以通过返回值拿到
    }
   
    //template <class F, class... Args>
    //auto exec(F&& f, Args&&... args) -> std::future<decltype(f(args...))>
    //{
    //    return exec(0, f, args...);
    //}

    ///**
    //* @brief 用线程池启用任务(F是function, Args是参数)
    //*
    //* @param 超时时间 ，单位ms (为0时不做超时控制) ；若任务超时，此任务将被丢弃
    //* @param bind function                                                                                      
    //* @return 返回任务的future对象, 可以通过这个对象来获取返回值
    //*/
    ///*
    //template <class F, class... Args>
    //它是c++里新增的最强大的特性之一，它对参数进行了高度泛化，它能表示0到任意个数、任意类型的参数
    //auto exec(F &&f, Args &&... args) -> std::future<decltype(f(args...))>
    //std::future<decltype(f(args...))>：返回future，调用者可以通过future获取返回值
    //返回值后置
    //*/
    //template <class F, class... Args>
    //auto exec(int64_t timeoutMs, F&& f, Args&&... args) -> std::future<decltype(f(args...))>
    //{
    //    int64_t expireTime = (timeoutMs == 0 ? 0 : TNOWMS + timeoutMs);  // 获取现在时间
    //    //定义返回值类型
    //    using RetType = decltype(f(args...));  // 推导返回值
    //    // 封装任务
    //    auto task = std::make_shared<std::packaged_task<RetType()>>(std::bind(std::forward<F>(f), std::forward<Args>(args)...));

    //    TaskFuncPtr fPtr = std::make_shared<TaskFunc>(expireTime);  // 封装任务指针，设置过期时间
    //    fPtr->_func = [task]() {  // 具体执行的函数
    //        (*task)();
    //    };

    //    std::unique_lock<std::mutex> lock(mutex_);
    //    tasks_.push(fPtr);              // 插入任务
    //    condition_.notify_one();        // 唤醒阻塞的线程，可以考虑只有任务队列为空的情况再去notify

    //    return task->get_future();;
    //}
protected:
    /**
     * @brief 获取任务
    */
   bool get(TaskFuncPtr &task);
   /**
    * @brief 线程池是否退出
   */
  bool isTerminate(){ return bTerminate_;}
  /**
   * @brief 线程运行函数
  */
  void run();
protected:
    /**
     * 任务队列
    */
    std::queue<TaskFuncPtr> tasks_;
    /**
     * 工作线程 放线程的指针
    */
   std::vector<std::thread*> threads_;
   /**
    * 互斥量
    */
    std::mutex          mutex_;
    /**
    * 条件变量 
    */
    std::condition_variable   condition_;
    /**
     * 线程数量
    */
    size_t threadNum_ ;
    /**
     * 是否结束线程池
    */
    bool bTerminate_ ; 

    /**
     * 原子变量 判断任务是否全部完成
    */
    std::atomic<int> atomic_{0};
};

#endif