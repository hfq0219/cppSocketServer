#include "Log.h"
#include <ctime>
#include "AsyncLog.h"
#include <iostream>

AsyncLog log("server.log");

Log_Level Log::log_level=Log_Level::INFO;

Log::Log(char* file_name,int line)
{
    time_t t=time(0);
    char time_str[64];
    strftime(time_str,sizeof(time_str),"%Y-%m-%d %H:%M:%S ",localtime(&t)); //添加时间信息
    buff=std::string(time_str)+std::string(file_name)+"_"+std::to_string(line); //添加文件信息
}

Log::~Log()
{
    std::cout<<buff;
    if(str_level>=log_level)
    {
        log.append(buff);
    }
}
    
Log& Log::info()
{
    str_level=Log_Level::INFO;
    buff+=" info: "; //日志级别
    return *this;
}

Log& Log::debug()
{
    str_level=Log_Level::DEBUG;
    buff+=" debug: "; //日志级别
    return *this;
}

Log& Log::warn()
{
    str_level=Log_Level::WARN;
    buff+=" warn: "; //日志级别
    return *this;
}

Log& Log::error()
{
    str_level=Log_Level::ERROR;
    buff+=" error: "; //日志级别
    return *this;
}

Log& Log::operator<<(std::string str)
{
    buff+=str;
    return *this;
}

Log& Log::operator<<(char* str)
{
    buff+=str;
    return *this;
}

Log& Log::operator<<(int num)
{
    buff+=std::to_string(num);
    return *this;
}

Log& Log::operator<<(double num)
{
    buff+=std::to_string(num);
    return *this;
}