#include <iostream>
#include <mutex>
#include <thread>

int a= 0;
static std::mutex mtx;

void func()
{
    for(int i = 0; i<5000; i++)
    {
        a+=1;
    }
}

void func1()
{
    for(int i = 0; i<5000; i++)
    {
        mtx.lock();
        a+=1;
        mtx.unlock();
    }
}

void ShowMutexAndThreadSafe()
{
    std::thread t1(func);
    std::thread t2(func);

    t1.join();
    t2.join();

    std::cout << a <<std::endl; // 此时 a 可能不是 10000（和电脑性能有关，高性能电脑得到的结果可能一直是正确的）
    //3.1 加锁
    a = 0;
    std::thread t3(func1);
    std::thread t4(func1);

    t3.join();
    t4.join();

    std::cout << a <<std::endl; // 此时 a 一定是 10000，因为使用互斥锁保证了两个线程不会同时操作一个变量
}