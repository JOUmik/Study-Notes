#include <iostream>
#include <mutex>
#include <thread>
std::mutex m1, m2;

void func_1()
{
    m1.lock();
    m2.lock();
    std::cout << "func1" << std::endl;
    m1.unlock();
    m2.unlock();
}

void func_2()
{
    m2.lock();
    m1.lock();
    std::cout << "func2" << std::endl;
    m1.unlock();
    m2.unlock();
}

void func_3()
{
    m1.lock();
    m2.lock();
    std::cout << "func3" << std::endl;
    m1.unlock();
    m2.unlock();
}

void ShowMutexAndDeadlock()
{
    /*
     * 当有两个互斥锁 m1，m2,
     * func_1是先锁 m1 再锁 m2,func_2是先锁 m2 再锁 m1,
     * 那么执行这两个函数的两个线程就可能造成死锁
     */
    /*
     * 解决办法是两个函数调用锁的顺序一致，比如 func_1 和 func_3一定不会出现死锁问题
     */
    std::thread t1(func_1);
    std::thread t2(func_3);
    t1.join();
    t2.join();
}