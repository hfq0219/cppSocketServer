#include "Timer.h"
#include <ctime>
#include "Epoll.h"
#include "Log.h"

extern Epoll epoll;

TimerNode::TimerNode(std::shared_ptr<Task> task, int timeout):task_(task),out_time(false)
{
    timeout_=time(0);
    timeout_+=timeout;
}

TimerNode::~TimerNode()
{
    if(task_)
    {
        uint32_t event=0;
        epoll.delEvent(task_->get_fd(),task_,event);
    }
}

bool TimerNode::out_of_time()
{
    time_t now=time(0);
    if(timeout_<=now)
    {
        return true;
    }
    else
    {
        return out_time;
    }
}

time_t TimerNode::get_time()
{
    return timeout_;
}

void TimerNode::del_task()
{
    if(task_)
    {
        task_.reset();
    }
    out_time=true;
}

void TimerManager::add_timer(std::shared_ptr<Task> task,int timeout)
{
    std::unique_lock<std::mutex> l(lock_);
    std::shared_ptr<TimerNode> timer(new TimerNode(task,timeout));
    timerQueue_.push(timer);
    task->link_timer(timer);
}

void TimerManager::del_timeout_timer()
{
    std::unique_lock<std::mutex> l(lock_);
    while(!timerQueue_.empty())
    {
        std::shared_ptr<TimerNode> timer=timerQueue_.top();
        if(timer->out_of_time())
        {
            timerQueue_.pop();
            Logi<<"pop timer\n";
        }
        else
        {
            break;
        }
    }
}