#include "Threadpool.h"
#include "Log.h"

Threadpool::Threadpool(int th_num,int que_size):thread_num(th_num),queue_size(que_size),thread_stop(false)
{
    Logi<<"threadpool start\n";
    threads.resize(thread_num);
    for(auto &th:threads)
    {
        th=std::thread(&Threadpool::thread_fun,this);
    }
}

int Threadpool::addTask(std::shared_ptr<Task> task)
{
    Logi<<"threadpool add new task\n";
    std::unique_lock<std::mutex> l(lock_);
    if(task_queue.size()>=queue_size)
    {
        Logw<<"too much task!\n";
        return -1;
    }
    task_queue.push(task);
    cond_.notify_one();
}

void Threadpool::thread_fun()
{
    while(1)
    {
        std::shared_ptr<Task> task;
        {
            std::unique_lock<std::mutex> l(lock_);
            while(task_queue.size()<=0 && !thread_stop) cond_.wait(l);
            if(thread_stop) break;
            task=task_queue.front();
            task_queue.pop();
        }
        task->run();
    }
    Logi<<"threadpool thread exit\n";
}

void Threadpool::stopThread()
{
    std::unique_lock<std::mutex> l(lock_);
    thread_stop=true;
    cond_.notify_all();
}

Threadpool::~Threadpool()
{
    for(auto &th:threads)
    {
        th.join();
        Logi<<"threadpool thread join\n";
    }
    Logi<<"threadpool exit\n";
}