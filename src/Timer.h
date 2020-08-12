#pragma once

#include <memory>
#include <queue>
#include <mutex>

class Task;

class TimerNode
{
private:
    std::shared_ptr<Task> task_;
    time_t timeout_;
    bool out_time;
public:
    TimerNode(std::shared_ptr<Task> task, int timeout);
    ~TimerNode();
    bool out_of_time();
    time_t get_time();
    void del_task();
};

struct comp
{
    bool operator()(std::shared_ptr<TimerNode> a,std::shared_ptr<TimerNode> b)
    {
        return a->get_time() > b->get_time();
    }
};

class TimerManager
{
private:
    std::mutex lock_;
    std::priority_queue<std::shared_ptr<TimerNode>,std::vector<std::shared_ptr<TimerNode>>,comp> timerQueue_;
public:
    void add_timer(std::shared_ptr<Task> task,int timeout);
    void del_timeout_timer();
};