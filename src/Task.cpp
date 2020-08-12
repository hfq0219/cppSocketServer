#include "Task.h"
#include <unistd.h>
#include "util.h"
#include "Log.h"
#include "Epoll.h"

extern Epoll epoll;

Task::Task(int fd,std::string begin,std::string end):fd_(fd),begin_(begin),end_(end),error_(false)
{
    Logi<<"new task fd: "<<fd_<<"\n";
}

Task::~Task()
{
    close(fd_);
    Logi<<"task fd: "<<fd_<<" closed\n";
}

void Task::run()
{
    Logi<<"task fd: "<<fd_<<" handle new data\n";
    if(can_read)
    {
        int nread=readEagain(fd_,read_buff);
        if(nread<0)
        {
            Loge<<"read error\n";
            handle_error();
            error_=true;
        }
        else if(nread==0)
        {
            Logi<<"task fd: "<<fd_<<" read num 0\n";
            return;
        }
        if(!error_)
        {
            handle_read();
        }
    }
    if(can_write)
    {
        handle_write();
    }
    if(!error_)
    {
        can_read=false;
        can_write=false;
        event_=EPOLLIN|EPOLLET;
        if(write_buff.size()>0)
        {
            event_|=EPOLLOUT;
        }
        epoll.addTimer(shared_from_this(),2*60); //设置新的计时器，2分钟超时
        epoll.modEvent(fd_,shared_from_this(),event_);
    }
}

void Task::handle_read() //根据begin_和end_设定的标志取出一条完整的信息
{
    int pos_begin=read_buff.find(begin_);
    int pos_end=read_buff.find(end_);
    if(pos_begin==read_buff.npos)
    {
        Logi<<"no begin find\n";
        handle_error();
        error_=true;
        return;
    }
    if(pos_end==read_buff.npos)
    {
        Logi<<"no end find, continue read\n";
        return;
    }
    if(pos_begin>pos_end)
    {
        Logi<<"begin is behind end, discard end~begin\n";
        read_buff=read_buff.substr(pos_begin);
        return;
    }
    write_buff=read_buff.substr(pos_begin+begin_.size(),pos_end-pos_begin-begin_.size());
    read_buff=read_buff.substr(pos_end+end_.size());
}

void Task::handle_write()
{
    int nwrite=writeEagain(fd_,write_buff);
    if(nwrite<0)
    {
        Loge<<"write error\n";
        error_=true;
    }
}

void Task::handle_error()
{
    std::string tmp("wrong message!\n");
    writeEagain(fd_,tmp);
}

int Task::get_fd()
{
    return fd_;
}

void Task::link_timer(std::shared_ptr<TimerNode> timer)
{
    timer_=timer;
    Logi<<"fd: "<<fd_<<" link timer\n";
}

void Task::del_timer()
{
    if(timer_.lock())
    {
        std::shared_ptr<TimerNode> timer=timer_.lock();
        timer->del_task();
        timer_.reset();
        Logi<<"fd: "<<fd_<<" del timer\n";
    }
}