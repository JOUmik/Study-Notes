
#include <functional>
#include <iostream>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>


class ThreadPool
{
public:
    ThreadPool(int ThreadNum) : stop(false)
    {
        for(int i = 0; i<ThreadNum; i++)
        {
            threads.emplace_back([this]()
            {
                while(1)
                {
                    std::function<void()> task;
                    {
                        std::unique_lock<std::mutex> ul(this->mtx);
                        cv.wait(ul, [this]()
                        {
                            return !tasks.empty() || stop;
                        });

                        if(stop && tasks.empty())
                        {
                            return;
                        }

                        task = std::move(tasks.front());  //ʹ��move�����ٶ���Ŀ�������
                        tasks.pop();
                    }
                    task();
                }
            });
        }
    }

    ~ThreadPool()
    {
        {
            std::unique_lock<std::mutex> ul(mtx);
            stop = true;
        }
        cv.notify_all();
        for(auto& t:threads)
        {
            t.join();
        }
    }

    //�������
    template<typename F, typename ...Args>
    void enqueue(F &&f, Args&& ... args)
    {
        //����ת��
        auto task = std::make_shared<std::function<void()>>(std::bind(std::forward<F>(f), std::forward<Args>(args) ...));
        {
            std::unique_lock<std::mutex> ul(mtx);
            tasks.emplace([task](){(*task)();});
            
        }
        cv.notify_all();
    }
    
private:
    std::vector<std::thread> threads;   //�̳߳��е��߳�
    std::queue<std::function<void()>> tasks;    //�������

    std::mutex mtx;
    std::condition_variable cv;

    bool stop;  //�̳߳�ʲôʱ����ֹ
};

void ShowThreadPool()
{
    ThreadPool tp(3);
    for(int i = 0; i<10; i++)
    {
        tp.enqueue([i]()
        {
            std::cout << "Thread Pool Task: "<<i<<"\n";
            std::this_thread::sleep_for(std::chrono::microseconds(500));
        });
        
    }
}