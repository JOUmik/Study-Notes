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
    //2.1 传递临时变量的问题
    //std::thread t0(foo, 1); //会报错，因为左值引用无法引用一个右值
    std::thread t0(foo1, 1);   //不会报错
    t0.join();
    int a = 1;
    std::thread t(foo, std::ref(a)); //如果函数需要传入引用类型，使用std::ref
    t.join();
    std::cout << a<< std::endl;

    //2.2 传递指针或引用指向局部变量的问题
    //test(); //会报错，因为 test 中的 a 在 test 执行后会被释放
    //th.join();

    //2.3 智能指针防止内存泄露
    
    std::shared_ptr<A> a0 = std::make_shared<A>();
    std::thread ta(&A::foo, a0);
    ta.join();

    //2.4 使用类中的私有函数
    thread_foo();
}