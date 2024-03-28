#include "logger.h"
#include <time.h>
#include<iostream>

// 获取日志单例,在类外实现类外成员不需要加static
Logger& Logger::GetInstance()
{
    static Logger logger;
    return logger;
}

Logger::Logger()
{
    //启动专门的写日志线程
    /*
    lambda 表达式 [&](){...} 中的 [&] 部分表示捕获外部作用域中的所有变量。
    这意味着 lambda 函数可以访问当前作用域中的所有变量，包括类成员变量和方法，以及其他局部变量。*/
    std::thread writeLogTask([&](){
        for(;;)
        {
            //获取当天日期，然后取日志信息，写入相应的日志文件中
            time_t now = time(nullptr);
            tm *nowtm = localtime(&now);
            
            char file_name[100000];
            sprintf(file_name, "%d-%d-%d-log.txt", nowtm->tm_year+1900, nowtm->tm_mon+1, nowtm->tm_mday);

            FILE *pf = fopen(file_name, "a+");//a+ 追加
            if(pf == nullptr)
            {
                std::cout << "logger file: " << file_name << "open error" <<std::endl;
                exit(EXIT_FAILURE);
            }

            std::string msg = m_lckQue.Pop();

            char time_buf[100000] = {0};
            sprintf(time_buf, "%d:%d:%d =>[%s]",
             nowtm->tm_hour, 
             nowtm->tm_min, 
             nowtm->tm_sec, 
             (m_loglevel == INFO ? "info" : "error"));
             
            msg.insert(0,time_buf);
            msg.append("\n");

            fputs(msg.c_str(), pf);
            fclose(pf);
        }
    });
    //设置分离线程
    writeLogTask.detach();

}


// 设置日志级别
void Logger::SetLogLevel(LogLevel level)
{
    m_loglevel = level;
}

// 写日志 ，把日志信息写入lcokqueue缓冲区中
void Logger::Log(std::string msg)
{
    m_lckQue.Push(msg);
}