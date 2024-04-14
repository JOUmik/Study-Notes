#include <iostream>
#include <thread>
#include <memory>
#include <mutex>
#include <queue>

void ShowCreateThread();

void ShowDataError();

void ShowMutexAndThreadSafe();

void ShowMutexAndDeadlock();

void ShowLockGuardAndUniqueLock();

void ShowCallOnce();

void ShowConditionVariable();

void ShowThreadPool();

void ShowAsyncAndFuture();

int main(int argc, char* argv[])
{
    
    //1.�����߳�
    ShowCreateThread();

    //2.����δ�������
    ShowDataError();

    //3.������������߳����ݹ�������
    ShowMutexAndThreadSafe();

    //4.����������
    ShowMutexAndDeadlock();

    //5.lock_guard �� std::unique_lock
    ShowLockGuardAndUniqueLock();

    //6.call_once����ʹ�ó���������ģʽ��
    ShowCallOnce();

    //7.condition_variable����ʹ�ó���
    ShowConditionVariable();

    //8.�̳߳�
    ShowThreadPool();

    //9.async��future
    ShowAsyncAndFuture();

    
    return 0;
}
