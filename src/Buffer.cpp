#include "Buffer.h"
#include <cstring>

Buffer::Buffer():cur(0)
{

}

Buffer::~Buffer()
{

}

void Buffer::append(std::string str)
{
    strcpy(buff+cur,str.c_str());
    cur+=str.size();
}

void Buffer::clear()
{
    cur=0;
    memset(buff,0,BUFF_SIZE);
}

int Buffer::available()
{
    return BUFF_SIZE-cur;
}

char* Buffer::data()
{
    return buff;
}