#include <mutex>
#include <thread>

int a0 = 0;
static std::mutex mtx;
void func_guard()
{
    for(int i = 0; i<5000; i++)
    {
        std::lock_guard<std::mutex> lg(mtx);    //�Զ��� mtx ���м������������������������lg�����������������������Զ��� mtx ����
        a0++;
    }
}

void func_unique()
{
    for(int i = 0; i<5000; i++)
    {
        //ֻ������
        std::unique_lock<std::mutex> lg(mtx);    //�Զ��� mtx ���м������������������������lg�����������������������Զ��� mtx ����
        a0++;
    }
}

std::timed_mutex tmtx;  //ֻ��ʹ��ʱ�以�������ܽ����ӳټ���
void func_unique_defer_lock()
{
    for(int i = 0; i<5000; i++)
    {
        //ͬʱ����defer_lock
        std::unique_lock<std::timed_mutex> lg(tmtx, std::defer_lock);  //ʹ��defer_lock�󣬹��캯��ʲôҲ�����������Զ���������Ҫ�ֶ�����������ʱ���Զ�����
        bool bGetLock = lg.try_lock_for(std::chrono::seconds(2));   //���Լ��������� 2 �룬��� 2 �����Ȼ�޷������򷵻� false ����ִ�к���ĳ���
        if(bGetLock)
        {
            //�ɹ������ִ�е�����
            a0++;
        }
        else
        {
            //û�гɹ������ִ�е�����
        }
        //��û�������ִ�е�����
    }
}

void ShowLockGuardAndUniqueLock()
{
    //5.1 lock_guard
    std::thread t1(func_guard);
    std::thread t2(func_guard);

    t1.join();
    t2.join();
    //5.2 unique_lock
    //���� lock_guard �����й����������������Ĺ��ܣ������ӳټ����ȣ��� lock_guard ��ǿ��
}