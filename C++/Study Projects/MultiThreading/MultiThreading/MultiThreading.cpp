#include <iostream>
#include <thread>
#include <memory>
#include <mutex>
#include <queue>

void ShowCreateThread();

void ShowDataError();

void ShowMutexAndThreadSafe();

void ShowMutexAndDeadlock();

void ShowLockGuardAndUniqueLock();

void ShowCallOnce();

void ShowConditionVariable();

void ShowThreadPool();

void ShowAsyncAndFuture();

int main(int argc, char* argv[])
{
    
    //1.创建线程
    ShowCreateThread();

    //2.数据未定义错误
    ShowDataError();

    //3.互斥量解决多线程数据共享问题
    ShowMutexAndThreadSafe();

    //4.互斥量死锁
    ShowMutexAndDeadlock();

    //5.lock_guard 和 std::unique_lock
    ShowLockGuardAndUniqueLock();

    //6.call_once及其使用场景（单例模式）
    ShowCallOnce();

    //7.condition_variable及其使用场景
    ShowConditionVariable();

    //8.线程池
    ShowThreadPool();

    //9.async和future
    ShowAsyncAndFuture();

    
    return 0;
}
