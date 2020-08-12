#include "util.h"
#include <signal.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include "Log.h"

const int SIZE=1024;

int readEagain(int fd,std::string &buff)
{
    int read_sum=0;
    while(1)
    {
        char tmp[SIZE];
        int n=read(fd,tmp,SIZE);
        if(n<0)
        {
            if(errno==EAGAIN) return read_sum;
            else if(errno==EINTR) continue;
            else
            {
                Loge<<"read error\n";
                return -1;
            }
        }
        else if(n==0)
        {
            return read_sum;
        }
        read_sum+=n;
        buff+=std::string(tmp,tmp+n);
    }
    return read_sum;
}

int writeEagain(int fd,std::string &buff)
{
    int left=buff.size();
    int write_sum=0;
    const char* ptr=buff.c_str();
    while(left>0)
    {
        int n=write(fd,ptr,left);
        if(n<0)
        {
            if(errno==EAGAIN) break;
            else if(errno==EINTR) continue;
            else
            {
                Loge<<"write error\n";
                return -1;
            }
        }
        write_sum+=n;
        left-=n;
        ptr+=n;
    }
    if(left==0) buff.clear();
    else buff=buff.substr(write_sum);
    return write_sum;
}

int setSocketNonBlocking(int fd)
{
    int flag=fcntl(fd,F_GETFL,0);
    if(flag==-1) return -1;
    if(fcntl(fd,F_SETFL,flag|O_NONBLOCK)==-1) return -1;
    return 0;
}

void ignoreSigpipe()
{
    signal(SIGPIPE,SIG_IGN);
}
