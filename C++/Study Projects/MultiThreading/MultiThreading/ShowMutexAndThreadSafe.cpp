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

    std::cout << a <<std::endl; // ��ʱ a ���ܲ��� 10000���͵��������йأ������ܵ��Եõ��Ľ������һֱ����ȷ�ģ�
    //3.1 ����
    a = 0;
    std::thread t3(func1);
    std::thread t4(func1);

    t3.join();
    t4.join();

    std::cout << a <<std::endl; // ��ʱ a һ���� 10000����Ϊʹ�û�������֤�������̲߳���ͬʱ����һ������
}