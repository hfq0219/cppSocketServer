#pragma once

#include <string>
#include <queue>
#include <memory>
#include "Timer.h"

class Task:public std::enable_shared_from_this<Task>
{
private:
    int fd_;
    uint32_t event_;
    std::string begin_;
    std::string end_;
    std::string read_buff;
    std::queue<std::string> write_buff;
    bool error_;
    bool read_close;
    std::weak_ptr<TimerNode> timer_;
public:
    bool can_read;
    bool can_write;
    Task(int fd,std::string begin,std::string end);
    ~Task();
    void run();
    void handle_read();
    void handle_write();
    void handle_error();
    int get_fd();
    void link_timer(std::shared_ptr<TimerNode> timer);
    void del_timer();
};