#include "Task.h"
#include <unistd.h>
#include "util.h"
#include "Log.h"
#include "Epoll.h"

extern Epoll epoll;

Task::Task(int fd,std::string begin,std::string end):fd_(fd),begin_(begin),end_(end),error_(false),read_close(false)
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
    if(can_read) //读新信息
    {
        int nread=readEagain(fd_,read_buff);
        if(nread<0) //读出错
        {
            Loge<<"read error\n";
            handle_error();
            error_=true;
        }
        else if(nread==0) //客户端可能已经关闭，所以读返回0
        {
            read_close=true;
            Logi<<"task fd: "<<fd_<<" read num 0\n";
        }
    }
    Logi<<read_buff<<"\n";
    if(read_buff.size()>0) //从读缓冲区取出完整的信息到write_buff
    {
        handle_read(); //没读到开始标志会使error_为true，信息错误，否则取出每一条完整信息放入write_buff，下次可写
    }
    if(can_write) //write_buff有数据可写
    {
        handle_write();
    }
    if(!error_ && !read_close)
    {
        can_read=false;
        can_write=false;
        event_=EPOLLIN|EPOLLET;
        if(write_buff.size()>0)
        {
            event_|=EPOLLOUT;
        }
        int timeout=2*60;
        epoll.addTimer(shared_from_this(),timeout); //设置新的计时器，2分钟超时
        epoll.modEvent(fd_,shared_from_this(),event_); //修改epoll监听
    }
}

void Task::handle_read() //根据begin_和end_设定的标志取出每一条完整的信息
{
    if(read_buff.size()<begin_.size()) return;
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
        handle_read();
    }
    else
    {
        write_buff.push(read_buff.substr(pos_begin+begin_.size(),pos_end-pos_begin-begin_.size())); //得到一条完整信息
        read_buff=read_buff.substr(pos_end+end_.size()); //
        handle_read();
    }
}

void Task::handle_write()
{
    while(!write_buff.empty())
    {
        std::string &msg=write_buff.front(); //从队列头部取信息写
        int nwrite=writeEagain(fd_,msg);
        if(nwrite<0) //写出错，结束
        {
            Loge<<"write error\n";
            error_=true;
            return;
        }
        if(msg.empty()) //写完该条信息，从buff中推出，继续写下一条信息
        {
            write_buff.pop();
        }
        else //该条信息没写完，可能缓冲区已满，结束写
        {
            break;
        }
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