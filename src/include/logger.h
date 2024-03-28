#pragma once
#include "lockqueue.h"
#include <string>

//Mprpc框架提供的日志系统

enum LogLevel
{
    INFO, //普通信息
    ERROR,//错误信息
};

class Logger
{
public:

    //获取日志单例
    static Logger& GetInstance();
    //设置日志级别
    void SetLogLevel(LogLevel level);
    //写日志
    void Log(std::string msg);

private:
    int m_loglevel; //记录日志级别
    LockQueue<std::string> m_lckQue; //日志缓冲队列

    Logger();
    Logger(const Logger&) = delete;
    Logger(Logger&&) = delete;

    /*Logger(); 是默认构造函数声明，表示该类具有一个无参数的构造函数。
    默认构造函数通常用于创建对象实例时的初始化工作。
    Logger(const Logger&) = delete; 和 Logger(Logger&&) = delete; 
    分别声明了拷贝构造函数和移动构造函数，并将它们标记为 delete。
    这意味着禁用了类的拷贝和移动构造函数，防止对象被复制或移动构造。
    这样做是为了避免意外的对象拷贝或移动，因为如果没有显式定义这些函数，编译器会自动生成它们。*/
};

// 定义宏 LOG_INFO("xxx %d %s", 20, "xxxx")
/*  #define LOG_INFO(logmsgformat, ...)：这是一个预处理器宏定义的开始。
    LOG_INFO 是宏的名称，logmsgformat 是接受日志信息格式的参数，
    而 ... 表示可变参数列表，用于传递日志信息中的其他参数。
    这里用宏简化了日志记录的过程，用户只需要调用 LOG_INFO 宏并传入相应的日志信息格式和参数即可，
    无需手动创建日志记录器实例、设置日志级别和调用日志记录方法。*/
#define LOG_INFO(logmsgformat, ...)                      \
    do                                                  \
    {                                                   \
        Logger &logger = Logger::GetInstance();         \
        logger.SetLogLevel(INFO);                      \
        char c[1024] = {0};                             \
        snprintf(c, 1024, logmsgformat, ##__VA_ARGS__); \
        logger.Log(c);                                  \
    } while (0)

#define LOG_ERR(logmsgformat, ...)                      \
    do                                                  \
    {                                                   \
        Logger &logger = Logger::GetInstance();         \
        logger.SetLogLevel(ERROR);                      \
        char c[1024] = {0};                             \
        snprintf(c, 1024, logmsgformat, ##__VA_ARGS__); \
        logger.Log(c);                                  \
    } while (0)
