#include "sync_queue2.h"
#include <iostream>
// #include "sync_queue.h"
#include <thread>
#include <iostream>
#include <mutex>
SyncQueue<int> syncQueue(5);

void PutDatas()
{
for (int i = 0; i < 20; ++i)
{
syncQueue.Put(i);
std::cout << "put"<<i<<std::endl;
}
std::cout << "PutDatas finish\n";
}


void TakeDatas()
{
int x = 0;
for (int i = 0; i < 20; ++i)
{
syncQueue.Take(x);
std::cout << "get"<<x << std::endl;
}
std::cout << "TakeDatas finish\n";
}

int main(){
    std::thread t1(PutDatas);
    std::thread t2(TakeDatas);
    t1.join();
    t2.join();
    std::cout << "main finish\n";
  return 0;
}