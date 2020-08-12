#pragma once

#include <string>
#include <memory>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <fstream>
#include <vector>
#include "Buffer.h"

class AsyncLog
{
private:
    std::string log_file;
    std::shared_ptr<Buffer> cur_buff;
    std::shared_ptr<Buffer> next_buff;
    std::vector<std::shared_ptr<Buffer>> buffs;
    std::vector<std::shared_ptr<Buffer>> buffs_to_write;
    std::mutex lock_;
    std::condition_variable cond_;
    std::thread thread_;
    static bool thread_running;
public:
    AsyncLog(std::string log_file);
    ~AsyncLog();
    void append(std::string str);
    void thread_fun();
    void thread_exit();
};