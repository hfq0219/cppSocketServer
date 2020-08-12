#pragma once

#include <string>

const int BUFF_SIZE=4096;

class Buffer
{
private:
    char buff[BUFF_SIZE];
    int cur;
public:
    Buffer();
    ~Buffer();
    void append(std::string str);
    void clear();
    int available();
    char* data();
};