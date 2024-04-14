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
     * �������������� m1��m2,
     * func_1������ m1 ���� m2,func_2������ m2 ���� m1,
     * ��ôִ�������������������߳̾Ϳ����������
     */
    /*
     * ����취������������������˳��һ�£����� func_1 �� func_3һ�����������������
     */
    std::thread t1(func_1);
    std::thread t2(func_3);
    t1.join();
    t2.join();
}