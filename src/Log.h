#pragma once

#include <string>

enum class Log_Level
{
    INFO,DEBUG,WARN,ERROR
};

class Log
{
private:
    std::string buff;
    Log_Level str_level;
public:
    static Log_Level log_level;
    Log(char* file_name,int line);
    ~Log();
    Log& info();
    Log& debug();
    Log& warn();
    Log& error();
    Log& operator<<(std::string str);
    Log& operator<<(char* str);
    Log& operator<<(int num);
    Log& operator<<(size_t num);
    Log& operator<<(double num);
};

#define Logi Log(__FILE__,__LINE__).info()
#define Logd Log(__FILE__,__LINE__).debug()
#define Logw Log(__FILE__,__LINE__).warn()
#define Loge Log(__FILE__,__LINE__).error()