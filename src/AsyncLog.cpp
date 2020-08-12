#include "AsyncLog.h"
#include "Log.h"

const int FLUSH_SECONDS=3; //每3秒刷新缓冲区写入文件

bool AsyncLog::thread_running=true;

AsyncLog::AsyncLog(std::string file):log_file(file),cur_buff(new Buffer),
                                next_buff(new Buffer),thread_(&AsyncLog::thread_fun,this)
{
    Logi<<"AsyncLog start\n";
}

AsyncLog::~AsyncLog()
{
    thread_.join();
    Logi<<"AsyncLog finish\n";
}

void AsyncLog::append(std::string str)
{
    std::unique_lock<std::mutex> l(lock_);
    if(cur_buff->available()>str.size())
    {
        cur_buff->append(str); //缓存未满追加
    }
    else
    {
        buffs.push_back(cur_buff); //缓存满保存并使用备用缓存
        if(next_buff)
        {
            cur_buff=std::move(next_buff);
        }
        else
        {
            cur_buff.reset(new Buffer);
        }
        cur_buff->append(str);
        cond_.notify_one(); //通知线程将缓存写入文件
    }
}

void AsyncLog::thread_fun()
{
    std::shared_ptr<Buffer> new_buff1(new Buffer); //交换缓存
    std::shared_ptr<Buffer> new_buff2(new Buffer);
    std::ofstream file_(log_file,std::ios::app);
    while(thread_running)
    {
        {
            std::unique_lock<std::mutex> l(lock_);
            cond_.wait_for(l,std::chrono::seconds(FLUSH_SECONDS),[&](){return !buffs.empty() || !thread_running;}); //wait_for等待
            buffs.push_back(cur_buff);
            cur_buff=std::move(new_buff1);
            if(!next_buff)
            {
                next_buff=std::move(new_buff2);
            }
            buffs_to_write.swap(buffs);
        }
        for(auto &buff:buffs_to_write)
        {
            file_<<buff->data();
        }
        file_.flush();
        if(buffs_to_write.size()>2)
        {
            buffs_to_write.resize(2);
        }
        if(!new_buff1)
        {
            new_buff1=buffs_to_write.back();
            buffs_to_write.pop_back();
            new_buff1->clear();
        }
        if(!new_buff2)
        {
            new_buff2=buffs_to_write.back();
            buffs_to_write.pop_back();
            new_buff2->clear();
        }
        buffs_to_write.clear();
    }
    file_.flush();
    file_.close();
    Logi<<"AsyncLog thread exit\n";
}

void AsyncLog::thread_exit()
{
    std::unique_lock<std::mutex> l(lock_);
    thread_running=false;
    cond_.notify_one();
}