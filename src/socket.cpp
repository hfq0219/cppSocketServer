#include "socket.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include "Log.h"

const int LISTEN_NUM=1024;

int socket_bind_listen(int port)
{
    if(port<1024 || port>65535)
    {
        Loge<<"invalid port\n";
    }

    int listen_fd=0;
    listen_fd=socket(AF_INET,SOCK_STREAM,0);
    if(listen_fd==-1)
    {
        Loge<<"invalid listen_fd\n";
    }

    int optval = 1;
    if(setsockopt(listen_fd, SOL_SOCKET,  SO_REUSEADDR, &optval, sizeof(optval)) == -1) //消除time_wait状态不能再次绑定端口的错误
    {
        Loge<<"socket reuse addr error\n";
        return -1;
    }
    
    sockaddr_in server_addr;
    server_addr.sin_family=AF_INET;
    server_addr.sin_addr.s_addr=htonl(INADDR_ANY);
    server_addr.sin_port=htons((uint16_t)port);
    if(bind(listen_fd,(sockaddr*)&server_addr,sizeof(server_addr))==-1)
    {
        Loge<<"bind error\n";
        return -1;
    }

    if(listen(listen_fd,LISTEN_NUM)==-1)
    {
        Loge<<"listen error\n";
        return -1;
    }

    return listen_fd;
}
