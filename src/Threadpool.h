#pragma once

#include <thread>
#include <vector>
#include <queue>
#include <memory>
#include <mutex>
#include <condition_variable>
#include "Task.h"

class Threadpool
{
private:
    int queue_size;
    int thread_num;
    bool thread_stop;
    std::vector<std::thread> threads;
    std::queue<std::shared_ptr<Task>> task_queue;
    std::mutex lock_;
    std::condition_variable cond_;

    void thread_fun();
public:
    Threadpool(int thread_num=64,int queue_size=1024);
    ~Threadpool();
    int addTask(std::shared_ptr<Task> task);
    void stopThread();
};