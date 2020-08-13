#include "Epoll.h"
#include <sys/epoll.h>
#include <assert.h>
#include <netinet/in.h>
#include <cstring>
#include <arpa/inet.h>
#include "util.h"
#include "Log.h"
#include "AsyncLog.h"

extern AsyncLog log;

Epoll::Epoll(Threadpool &thread_pool,std::string begin_flag,std::string end_flag):ep_fd(epoll_create(1)),thread_pool_(thread_pool),msg_begin_flag(begin_flag),msg_end_flag(end_flag)
{
    assert(ep_fd>0);
    Logi<<"Epoll create\n";
}

Epoll::~Epoll()
{
    close(ep_fd);
    thread_pool_.stopThread();
    log.thread_exit();
    Logi<<"Epoll exit\n";
}

int Epoll::addEvent(int fd,std::shared_ptr<Task> task,uint32_t event)
{
    Logi<<"Epoll add event fd: "<<fd<<"\n";
    epoll_event ev;
    ev.events=event;
    ev.data.fd=fd;
    fd2task[fd]=task;
    int res=epoll_ctl(ep_fd,EPOLL_CTL_ADD,fd,&ev);
    if(res==-1)
    {
        fd2task[fd].reset();
        Loge<<"Epoll add event error\n";
    }
    return res;
}

int Epoll::modEvent(int fd,std::shared_ptr<Task> task,uint32_t event)
{
    Logi<<"Epoll mod event fd: "<<fd<<"\n";
    epoll_event ev;
    ev.events=event;
    ev.data.fd=fd;
    fd2task[fd]=task;
    int res=epoll_ctl(ep_fd,EPOLL_CTL_MOD,fd,&ev);
    if(res==-1)
    {
        fd2task[fd].reset();
        Loge<<"Epoll mod event error\n";
    }
    return res;
}

int Epoll::delEvent(int fd,std::shared_ptr<Task> task,uint32_t event)
{
    Logi<<"Epoll del event fd: "<<fd<<"\n";
    epoll_event ev;
    ev.events=event;
    ev.data.fd=fd;
    fd2task[fd].reset();
    int res=epoll_ctl(ep_fd,EPOLL_CTL_DEL,fd,&ev);
    if(res==-1)
    {
        Loge<<"Epoll del event error\n";
    }
    return res;
}

int Epoll::waitEpoll(int listen_fd,int timeout)
{
    int res=epoll_wait(ep_fd,events,EVENT_SIZE,timeout);
    if(res<0) Loge<<"epoll wait error\n";
    for(int i=0;i<res;++i)
    {
        epoll_event ev=events[i];
        if(ev.data.fd==listen_fd)
        {
            acceptNewConnect(listen_fd); //接收新连接，加入Epoll监听
        }
        else
        {
            if(ev.events&EPOLLERR || ev.events&EPOLLHUP)
            {
                Loge<<"error event\n";
                fd2task[ev.data.fd].reset();
                continue;
            }
            std::shared_ptr<Task> task=fd2task[ev.data.fd];
            if(task)
            {
                if ((ev.events & EPOLLIN) || (ev.events & EPOLLPRI))
                    task->can_read=true;
                if (ev.events & EPOLLOUT)
                    task->can_write=true;
                Logi<<"data coming, add fd: "<<ev.data.fd<<" to threadpool.\n";
                fd2task[ev.data.fd].reset(); //将任务移除，防止多个线程同时操作一个任务
                task->del_timer(); //将任务从计时器移除，防止超时被关闭
                thread_pool_.addTask(task); //任务加入线程池
            }
            else
            {
                Logi<<"task fd: "<<ev.data.fd<<" is in process\n";
            }
        }
    }
    manager.del_timeout_timer(); //处理超时连接
}

int Epoll::getEpoll() const
{
    return ep_fd;
}

void Epoll::acceptNewConnect(int listen_fd)
{
    sockaddr_in client_addr;
    memset(&client_addr,0,sizeof(client_addr));
    int accept_fd=0;
    socklen_t len=sizeof(client_addr);
    while((accept_fd=accept(listen_fd,(sockaddr*)&client_addr,&len))>0) //循环处理新连接
    {
        Logi<<"new connect: "<<inet_ntoa(client_addr.sin_addr)<<":"<<ntohs(client_addr.sin_port)<<", fd: "<<accept_fd<<"\n";
        if(accept_fd>MAX_FDS)
        {
            Logw<<"too much connection\n";
            close(accept_fd);
            continue;
        }
        
        if(setSocketNonBlocking(accept_fd)<0)
        {
            Loge<<"set socket non blocking error\n";
            return;
        }
        int timeout=2*60; //长连接2分钟
        std::shared_ptr<Task> task(new Task(accept_fd,msg_begin_flag,msg_end_flag));
        addEvent(accept_fd,task,EPOLLIN|EPOLLET);
        addTimer(task,timeout);
    }
    if(accept_fd==-1)
    {
        Logi<<"accept eagain\n";
    }
}

void Epoll::addTimer(std::shared_ptr<Task> task,int timeout)
{
    manager.add_timer(task,timeout);
}