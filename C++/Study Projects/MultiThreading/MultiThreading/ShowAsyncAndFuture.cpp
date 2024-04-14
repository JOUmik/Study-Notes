
#include <future>
#include <iostream>
#include <ostream>

int func01()
{
    int i = 0;
    for(i = 0; i<100; i++)
    {
        //do something
    }
    return i;
}

void func02(std::promise<int>& f)
{
    f.set_value(1000);
}
void ShowAsyncAndFuture()
{
    //async
    std::future<int> future_res = std::async(std::launch::async, func01);   //async创建后像thread一样执行
    std::cout << func01() << "\n";
    std::cout << future_res.get() <<"\n";

    //packaged_task
    std::packaged_task<int()> task(func01);
    auto task_future = task.get_future();
    std::thread t1(std::move(task));
    t1.join();
    std::cout << task_future.get() <<"\n";

    //promise
    std::promise<int> f;
    auto promise_res = f.get_future();
    std::thread t2(func02, std::ref(f));

    t2.join();

    std::cout<<promise_res.get()<<"\n";
}