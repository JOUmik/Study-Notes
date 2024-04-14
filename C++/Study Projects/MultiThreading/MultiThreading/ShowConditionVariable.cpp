
#include <condition_variable>
#include <iostream>
#include <queue>
#include <thread>

//生产者-消费者模型
class ProducerConsumer
{
public:
    void producer(int count)
    {
        for(int i = 0; i<count; i++)
        {
            {
                std::unique_lock<std::mutex> ul(mtx);
                cv.wait(ul, [this]()
                {
                    return task.size() < capacity;
                });
                //生产数据
                task.push(i);
                std::cout << "Produced " << i << "\n";
                
                //通知消费者消费
                cv.notify_all();
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    }

    void consumer(int count)
    {
        for(int i = 0; i<count; i++)
        {
            std::unique_lock<std::mutex> ul(mtx);
            cv.wait(ul, [this](){return !task.empty();});  //如果队列为空就等待

            //消费数据
            int val = task.front();
            std::cout << "Consumed " << i << "\n";
            task.pop();

            //通知生产者生产
            cv.notify_all();
        }
    }
private:
    std::queue<int> task; //存储需要完成的任务
    const unsigned int capacity = 10;
    std::mutex mtx;
    std::condition_variable cv;
};

void ShowConditionVariable()
{
    //生产者-消费者模型
    ProducerConsumer pc;
    std::thread producerThread(&ProducerConsumer::producer, &pc, 10);
    std::thread consumerThread(&ProducerConsumer::consumer, &pc, 10);

    producerThread.join();
    consumerThread.join();
}