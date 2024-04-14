#include <mutex>
#include <thread>

int a0 = 0;
static std::mutex mtx;
void func_guard()
{
    for(int i = 0; i<5000; i++)
    {
        std::lock_guard<std::mutex> lg(mtx);    //自动对 mtx 进行加锁，当该作用域结束后会调用lg的析构函数，析构函数中自动对 mtx 解锁
        a0++;
    }
}

void func_unique()
{
    for(int i = 0; i<5000; i++)
    {
        //只传入锁
        std::unique_lock<std::mutex> lg(mtx);    //自动对 mtx 进行加锁，当该作用域结束后会调用lg的析构函数，析构函数中自动对 mtx 解锁
        a0++;
    }
}

std::timed_mutex tmtx;  //只有使用时间互斥锁才能进行延迟加锁
void func_unique_defer_lock()
{
    for(int i = 0; i<5000; i++)
    {
        //同时传入defer_lock
        std::unique_lock<std::timed_mutex> lg(tmtx, std::defer_lock);  //使用defer_lock后，构造函数什么也不做，不会自动加锁，需要手动加锁，析构时会自动解锁
        bool bGetLock = lg.try_lock_for(std::chrono::seconds(2));   //尝试加锁，阻塞 2 秒，如果 2 秒后仍然无法加锁则返回 false 继续执行后面的程序
        if(bGetLock)
        {
            //成功获得锁执行的内容
            a0++;
        }
        else
        {
            //没有成功获得锁执行的内容
        }
        //获没获得锁都执行的内容
    }
}

void ShowLockGuardAndUniqueLock()
{
    //5.1 lock_guard
    std::thread t1(func_guard);
    std::thread t2(func_guard);

    t1.join();
    t2.join();
    //5.2 unique_lock
    //能做 lock_guard 的所有工作并可以有其他的功能，比如延迟加锁等，比 lock_guard 更强大
}