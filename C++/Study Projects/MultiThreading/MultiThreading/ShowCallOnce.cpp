#include <iostream>
#include <mutex>
#include <string>
#include <thread>

//�򵥵ĵ���ģʽ(�����汾 �̰߳�ȫ�汾 ����Ҫ����)
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

//�򵥵ĵ���ģʽ(�����汾 �̲߳���ȫ�汾 ��Ҫ����)
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
        if(!instance) instance = std::unique_ptr<Log1>(); //���ﵼ���̲߳���ȫ

        return *instance;
    }

    void PrintLog(std::string msg)
    {
        std::cout << __TIME__ << ": " << msg << "\n";
    }
};

//����Log1���̲߳���ȫʹ��call_once���иĽ�
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
        std::call_once(once, init); //��֤����ĺ����ڶ���߳���ֻ��ִ��һ��
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
    //����ģʽ
    Log::GetInstance().PrintLog("����ģʽ");
    //call once
    std::thread t1(print, "t1");
    std::thread t2(print, "t2");
    t1.join();
    t2.join();
}