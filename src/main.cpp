#include "Epoll.h"
#include "Log.h"
#include "util.h"
#include "socket.h"
#include "Task.h"
#include "Threadpool.h"
#include <memory>

#define BEGIN_FLAG "*-begin-*" //接收信息起止标志符
#define END_FLAG "*-end-*"

Threadpool thread_pool; //开启线程池
Epoll epoll(thread_pool,BEGIN_FLAG,END_FLAG); //创建epoll

int main()
{
    Log::log_level=Log_Level::INFO; //设置日志级别
    Logi<<"start\n";
    ignoreSigpipe(); //忽略SIGPIPE信号，防止客户端关闭后，服务器继续写导致服务器退出
    int listen_fd=socket_bind_listen(6666); //开启socket监听
    if(listen_fd<0)
    {
        Loge<<"socket error\n";
        return -1;
    }
    Logi<<"listen fd: "<<listen_fd<<"\n";
    if(setSocketNonBlocking(listen_fd)<0) //设置非阻塞模式
    {
        Loge<<"set socket non blocking error\n";
        return -1;
    }
    std::shared_ptr<Task> task(new Task(listen_fd,BEGIN_FLAG,END_FLAG)); //新建监听任务
    epoll.addEvent(listen_fd,task,EPOLLIN|EPOLLET); //加入epoll
    while(1)
    {
        epoll.waitEpoll(listen_fd,2*60*1000); //epoll wait循环，2分钟唤醒处理超时任务
    }
    Logi<<"exit\n";
    return 0;
}