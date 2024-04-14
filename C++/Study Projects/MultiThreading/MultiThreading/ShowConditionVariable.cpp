
#include <condition_variable>
#include <iostream>
#include <queue>
#include <thread>

//������-������ģ��
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
                //��������
                task.push(i);
                std::cout << "Produced " << i << "\n";
                
                //֪ͨ����������
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
            cv.wait(ul, [this](){return !task.empty();});  //�������Ϊ�վ͵ȴ�

            //��������
            int val = task.front();
            std::cout << "Consumed " << i << "\n";
            task.pop();

            //֪ͨ����������
            cv.notify_all();
        }
    }
private:
    std::queue<int> task; //�洢��Ҫ��ɵ�����
    const unsigned int capacity = 10;
    std::mutex mtx;
    std::condition_variable cv;
};

void ShowConditionVariable()
{
    //������-������ģ��
    ProducerConsumer pc;
    std::thread producerThread(&ProducerConsumer::producer, &pc, 10);
    std::thread consumerThread(&ProducerConsumer::consumer, &pc, 10);

    producerThread.join();
    consumerThread.join();
}