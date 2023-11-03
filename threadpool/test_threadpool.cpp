#include "ZERO_ThreadPool.h"
#include <iostream>

using namespace std;
int func1(int a,int b){
    cout<<"start func1"<<endl;
    this_thread::sleep_for(std::chrono::seconds(2));
    cout<<"end func1"<<endl;
    return a +b;
}
int func2(int a,int b){
    cout<<"start func2"<<endl;
    this_thread::sleep_for(std::chrono::seconds(2));
    cout<<"end func2"<<endl;
    return a *b;
}
int func3(int a,int b){
    cout<<"start func3"<<endl;
    this_thread::sleep_for(std::chrono::seconds(2));
    cout<<"end func3"<<endl;
    return a -b;
}
using namespace std;
int main(){
    ZERO_ThreadPool pool;
    pool.init(5);
    pool.start();
    // threadpool.exec((void(*)(int, int))func2_1, 10, 20);   // 插入任务
    // threadpool.exec((int(*)(string, string))func2_1, "king", " and darren");
    auto result1=pool.exec((int(*)(int,int))func1,1,2);
    auto result2=pool.exec(func2,1,2);
    auto result3=pool.exec(func3,1,2);
    pool.waitForAllDown();
    pool.stop();
    cout<<"result1="<<result1.get()<<endl;
    cout<<"result2="<<result2.get()<<endl;
    cout<<"result3="<<result3.get()<<endl;
    // b.stop();
    return 0;
}