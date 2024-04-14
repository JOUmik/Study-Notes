#include <iostream>
#include <mutex>
#include <string>
#include <thread>

//简单的单例模式(懒汉版本 线程安全版本 不需要加锁)
class Log
{
private:
    Log(){}
public:
    Log(const Log& log) = delete;
    Log& operator=(const Log& log) = delete;

    static Log& GetInstance()
    {
        static std::unique_ptr<Log> instance(new Log());

        return *instance;
    }

    void PrintLog(std::string msg)
    {
        std::cout << __TIME__ << ": " << msg << "\n";
    }
};

//简单的单例模式(懒汉版本 线程不安全版本 需要加锁)
class Log1
{
private:
    Log1(){}
public:
    Log1(const Log1& log) = delete;
    Log1& operator=(const Log1& log) = delete;

    static Log1& GetInstance()
    {
        static std::unique_ptr<Log1> instance;
        if(!instance) instance = std::unique_ptr<Log1>(); //这里导致线程不安全

        return *instance;
    }

    void PrintLog(std::string msg)
    {
        std::cout << __TIME__ << ": " << msg << "\n";
    }
};

//对于Log1的线程不安全使用call_once进行改进
class Log2
{
private:
    static std::unique_ptr<Log2> instance;
    static std::once_flag once;
    Log2(){}
public:
    Log2(const Log2& log) = delete;
    Log2& operator=(const Log2& log) = delete;

    static void init()
    {
        if(!instance) instance = std::unique_ptr<Log2>();
    }
    
    static Log2& GetInstance()
    {
        std::call_once(once, init); //保证传入的函数在多个线程中只被执行一次
        return *instance;
    }

    void PrintLog(std::string msg)
    {
        std::cout << __TIME__ << ": " << msg << "\n";
    }
};

std::unique_ptr<Log2> Log2::instance;
std::once_flag Log2::once;

void print(std::string msg)
{
    Log2::GetInstance().PrintLog(msg);
}

void ShowCallOnce()
{
    //单例模式
    Log::GetInstance().PrintLog("单例模式");
    //call once
    std::thread t1(print, "t1");
    std::thread t2(print, "t2");
    t1.join();
    t2.join();
}