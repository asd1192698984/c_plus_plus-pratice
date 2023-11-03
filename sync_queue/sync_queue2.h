#ifndef SYNC_QUEUE_H
#define SYNC_QUEUE_H
#include <list>
#include <mutex>
#include<thread>
#include<condition_variable>
#include <iostream>


template<typename T>
class SyncQueue{
private:
    bool IsFull() const {
        return _queue.size()==_maxSize;
    }
    bool IsEmpty() const {
        return _queue.empty();
    }
public:
    SyncQueue(int maxSize):_maxSize(maxSize){
    }

    //放置一个
    void Put (const T &x){
        std::unique_lock<std::mutex> locker(_mutex);  //先拿到锁
        // _notFull.wait(_mutex); 
         _notFull.wait(locker,[this]{ return !IsFull();}); 
        // while (IsFull())
        // {
        // std::cout << "full wait..." << std::endl;
        // _notFull.wait(_mutex);  //没有这个函数了
        // }
        //放置一个
        _queue.push_back(x);
        _notEmpty.notify_one();
    }
    //拿走一个
    void Take(T& x){ //传引用进来取值
        std::unique_lock<std::mutex> locker(_mutex);  //先拿到锁
        // _notEmpty.wait(_mutex);
        // while (IsEmpty())
        // {
        // std::cout << "empty wait.." << std::endl;
        // _notEmpty.wait(_mutex);
        // } 
         _notEmpty.wait(locker,[this]{ return ! IsEmpty();});  
        //拿走一个
        x=_queue.front();
        _queue.pop_front();
        _notFull.notify_one();
    }
    //是否为空
    bool Empty(){
        std::lock_guard<std::mutex> locker(_mutex); 
        return  IsEmpty();
    }
    //是否满了
    bool Full(){
        std::lock_guard<std::mutex> locker(_mutex); 
        return IsFull();
    }   
    //返回元素个数
    size_t  Size(){
        std::lock_guard<std::mutex> locker(_mutex); 
        return _queue.size();
    }
    int Count()
    {
        return _queue.size();
    }
private :
    std::list<T> _queue; //队列
    std::mutex _mutex; //互斥量
    std::condition_variable _notEmpty; //缓冲区没空
    std::condition_variable _notFull; //缓冲区没满
    int _maxSize; //同步队列最大的size
};
#endif