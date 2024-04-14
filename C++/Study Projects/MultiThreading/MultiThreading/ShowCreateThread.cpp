#include <iostream>
#include <thread>

void printMessage(std::string msg)
{
    std::cout << msg << std::endl;
}

void ShowCreateThread()
{
    std::thread thread1(printMessage, "Hello Thread");
    const bool bCanJoin = thread1.joinable(); //判断能不能调用join,比如调用过join后就不能再调用join了
    if(bCanJoin)
    {
        thread1.join();        // join会阻塞主线程，只有当线程结束时才会继续执行主线程后续任务
        //thread1.detach();    // detach不阻塞主线程的运行，当主线程结束时如果detach的线程没有结束则强制结束
    }
}