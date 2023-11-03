#include "ZERO_ThreadPool.h"

//构造函数
ZERO_ThreadPool::ZERO_ThreadPool():threadNum_(1),bTerminate_(false){
}

//析构函数

ZERO_ThreadPool::~ZERO_ThreadPool(){
    stop();
}

/**
 * @brief 初始化
*/
bool ZERO_ThreadPool::init(size_t num){
    std::cout<<"threadpool init"<<std::endl;
    //这里为什么要加锁？因为牵涉到共享数据threads_的读写
    //先要判断threads是否为空 非空的情况下返回false
    std::unique_lock<std::mutex> lock(mutex_);
    if(!threads_.empty()){ 
        return false;
    }
    threadNum_=num;
    return true;
}

/**
 * @brief 启动线程 其实就是创建线程
*/
bool ZERO_ThreadPool::start(){
    std::cout<<"threadpool start"<<std::endl;
    //这里为什么要加锁？
    //先要判断threads是否为空 非空的情况下返回false
     std::unique_lock<std::mutex> lock(mutex_);

    if (!threads_.empty())
    {
        return false;
    }

    //创建线程 
    for(size_t i= 0;i<threadNum_;i++){
        //传类函数 需要传类指针
        threads_.push_back(new thread(&ZERO_ThreadPool::run,this));
    }
    return true;
}


/**
 * @brief 线程执行的函数
*/
void ZERO_ThreadPool::run(){
    //完成三件事 拿到任务 执行任务 检测任务全部运行完
    while(!isTerminate()){ //线程池没有结束就一直循环拿任务
        TaskFuncPtr task;
        bool ok=get(task);//1 拿到任务
        if(ok){ //拿到任务了 开始执行任务
            ++atomic_;
             try{ //这里需要判断一下超时任务
                if(task->_expireTime!=0&&task->_expireTime<TNOWMS){
                    //超时任务处理
                    //因为有些任务处理时间过长导致后面任务来不及处理
                }else{
                    task->_func(); //2 执行任务
                }
             }catch(...){ //异常处理
                std::cout<<"任务发生异常"<<endl;
             }
            --atomic_;
        }
        //3 判断是否所有任务都完成了 
        //拿锁 
        std::unique_lock<std::mutex> lock(mutex_);
        if(atomic_==0&&tasks_.empty()){
            condition_.notify_all();  //这里是通知waitforAllDown
        }
    }
}



/**
 * @brief 拿到任务
*/
bool ZERO_ThreadPool::get(TaskFuncPtr& task){
        //先加锁 
    std::unique_lock<std::mutex> lock(mutex_); // 也要等锁
    //然后访问任务队列
    if(tasks_.empty()){
        condition_.wait(lock,[this](){return bTerminate_||!tasks_.empty();});
    }
    //在什么情况下线程苏醒 1.线程池结束，2.任务非空
    if(bTerminate_){ //直接返回false 没有拿到任务 
        return false;
    }

    if(!tasks_.empty()){ //拿任务
        // task=tasks_.front();
        task =std::move(tasks_.front());
        tasks_.pop();
        return true;
    }
    //可能存在其他情况 任务做完但是还没有关闭线程池的情况
    return false;
}

/**
 * @brief 判断是否所有任务都做完 如果没有做完就一直阻塞主线程
 * @return 
*/
bool ZERO_ThreadPool::waitForAllDown(int  millsecond)
{
    //因为涉及到任务队列 先拿锁
    std::unique_lock<std::mutex> lock(mutex_);
    if(tasks_.empty()){ //为空 直接返回true
        return true; 
    }
    //不为空的情况
    if(millsecond<0){ //参数小于0 代表无限等待
        condition_.wait(lock,[this](){return tasks_.empty();});
    }else{
        //调用wait for 第二个参数是等待的最长时间
        condition_.wait_for(lock,std::chrono::milliseconds(millsecond),[this](){return tasks_.empty();});
    }
}

/**
 * @brief 停止线程并释放资源
*/
void ZERO_ThreadPool::stop(){
    //其实就是设置  bTerminate_为true  
    //此时可能有任务还没做完 有任务还没开始做 
    //bTerminate_为true  设置线程不再接任务 没做完的任务继续继续等待做完
    //然后释放线程资源 ，任务指针的资源因为是智能指针分配的不用自己手动释放
    {   
        //这里锁为什么放在打括号里面 
        //因为后面呢代码需要等待线程退出 调用了join 但是线程的run方法里面也需要锁
        //为了避免死锁
        std::unique_lock<std::mutex> lock(mutex_);
        bTerminate_=true;
        //这里调用notify_all 去打断waitforallDown的 wait状态
        //不过感觉不调用也行 任务结束也会调用
        condition_.notify_all();
    }
    //这里遍历所有的线程 等待所有线程结束
    for (size_t i = 0; i < threads_.size(); i++)  //释放资源
    {
        if(threads_[i]->joinable()) //没做完的任务继续等待
        {
            threads_[i]->join(); // 等线程退出
        }
        //结束完释放资源
        delete threads_[i]; //释放资源
        threads_[i] = NULL;
    }
    //最后清空数组
     std::unique_lock<std::mutex> lock(mutex_);
    threads_.clear(); //清空数组
}


//下面是获取时间的代码 因为没有牵涉到线程池的主要逻辑。自己没有花时间去研究了 
//有兴趣可以看看
int gettimeofday(struct timeval &tv)
{
#if WIN32
    time_t clock;
    struct tm tm;
    SYSTEMTIME wtm;
    GetLocalTime(&wtm);
    tm.tm_year   = wtm.wYear - 1900;
    tm.tm_mon   = wtm.wMonth - 1;
    tm.tm_mday   = wtm.wDay;
    tm.tm_hour   = wtm.wHour;
    tm.tm_min   = wtm.wMinute;
    tm.tm_sec   = wtm.wSecond;
    tm. tm_isdst  = -1;
    clock = mktime(&tm);
    tv.tv_sec = clock;
    tv.tv_usec = wtm.wMilliseconds * 1000;

    return 0;
#else
    return ::gettimeofday(&tv, 0);
#endif
}

void getNow(timeval *tv)
{
#if TARGET_PLATFORM_IOS || TARGET_PLATFORM_LINUX

    int idx = _buf_idx;
    *tv = _t[idx];
    if(fabs(_cpu_cycle - 0) < 0.0001 && _use_tsc)
    {
        addTimeOffset(*tv, idx);
    }
    else
    {
        TC_Common::gettimeofday(*tv);
    }
#else
    gettimeofday(*tv);
#endif
}

int64_t getNowMs()
{
    struct timeval tv;
    getNow(&tv);

    return tv.tv_sec * (int64_t)1000 + tv.tv_usec / 1000;
}
