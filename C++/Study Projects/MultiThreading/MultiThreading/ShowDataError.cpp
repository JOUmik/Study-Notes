#include <iostream>
#include <thread>
#include <memory>

void foo(int& x)
{
    x+=1;
}

void foo1(int&& x)
{
    x+=1;
}

std::thread th;
void test()
{
    int a = 1;
    th = std::thread(foo, std::ref(a));
}

class A
{
public:
    void foo()
    {
        std::cout << "A"<<std::endl;
    }
};

class B
{
private:
    void foo()
    {
        std::cout<<"B"<<std::endl;
    }

public:
    friend void thread_foo();
};

void thread_foo()
{
    std::shared_ptr<B> b = std::make_shared<B>();
    std::thread tb(&B::foo, b);
    tb.join();
}


void ShowDataError()
{
    //2.1 ������ʱ����������
    //std::thread t0(foo, 1); //�ᱨ����Ϊ��ֵ�����޷�����һ����ֵ
    std::thread t0(foo1, 1);   //���ᱨ��
    t0.join();
    int a = 1;
    std::thread t(foo, std::ref(a)); //���������Ҫ�����������ͣ�ʹ��std::ref
    t.join();
    std::cout << a<< std::endl;

    //2.2 ����ָ�������ָ��ֲ�����������
    //test(); //�ᱨ����Ϊ test �е� a �� test ִ�к�ᱻ�ͷ�
    //th.join();

    //2.3 ����ָ���ֹ�ڴ�й¶
    
    std::shared_ptr<A> a0 = std::make_shared<A>();
    std::thread ta(&A::foo, a0);
    ta.join();

    //2.4 ʹ�����е�˽�к���
    thread_foo();
}