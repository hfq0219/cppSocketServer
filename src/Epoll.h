#pragma once

#include <unistd.h>
#include <sys/epoll.h>
#include "Task.h"
#include "Threadpool.h"
#include "Timer.h"
#include <memory>
#include <string>

const int EVENT_SIZE=1024;
const int MAX_FDS=1024;

class Epoll
{
private:
    int ep_fd;
    epoll_event events[EVENT_SIZE];
    std::shared_ptr<Task> fd2task[EVENT_SIZE];
    Threadpool &thread_pool_;
    TimerManager manager;
    std::string msg_begin_flag;
    std::string msg_end_flag;
public:
    Epoll(Threadpool &thread_pool,std::string begin_flag,std::string end_flag);
    ~Epoll();
    int addEvent(int fd,std::shared_ptr<Task> task,uint32_t event);
    int modEvent(int fd,std::shared_ptr<Task> task,uint32_t event);
    int delEvent(int fd,std::shared_ptr<Task> task,uint32_t event);
    int waitEpoll(int listen_fd,int timeout);
    int getEpoll() const;
    void acceptNewConnect(int listen_fd);
    void addTimer(std::shared_ptr<Task> task,int timeout);
};