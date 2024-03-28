#include "mprpcapplication.h"
#include<iostream>
#include<unistd.h>
#include<string>

//类内的静态变量在类外进行初始化不用加static
MprpcConfig MprpcApplication::m_config;

void ShowArgsHelp()
{
    std::cout<<"format: command -i <configfile>" << std::endl;
}

void MprpcApplication::Init(int argc, char **argv)
{
    if(argc<2)//没有传入参数
    {
        ShowArgsHelp();
        exit(EXIT_FAILURE);
    }

    // getopt 函数的用法是基于 POSIX 标准。它用于解析命令行选项和参数。参数说明如下：
    //argc：命令行参数的数量，包括程序名称在内。
    //argv：命令行参数的数组，其中 argv[0] 是程序名称，之后是实际的参数。
    //第三个参数是一个字符串，其中包含了所有支持的选项。
    //每个选项字符后可以跟一个冒号 :，表示该选项需要一个参数。
    //getopt 函数会返回下一个选项字符，如果选项需要参数，则该参数存储在全局变量 optarg 中。 
    int c = 0;
    std::string config_file;
    while((c = getopt(argc, argv, "i:")) != -1)
    {
        switch (c)
        {
        case 'i':
            config_file = optarg;
            break;
        case '?':
            std::cout<< "invalid args" << std::endl;
            ShowArgsHelp();
            exit(EXIT_FAILURE);
        case ':':
            ShowArgsHelp();
            exit(EXIT_FAILURE);
        default:
            break;
        }
    }
    // 开始加载配置文件 rpcserver_ip= rpcserver_port=  zookeeper_ip=  zookeeper_port=
    m_config.LoadConfigFile(config_file.c_str());

    // std::cout<< "rpcserverip:" << m_config.Load("rpcserverip") <<std::endl;
    // std::cout << "rpcserverport:" << m_config.Load("rpcserverport") << std::endl;
    // std::cout << "zookeeperip:" << m_config.Load("zookeeperip") << std::endl;
    // std::cout << "zookeeperport:" << m_config.Load("zookeeperport") << std::endl;
}

MprpcApplication& MprpcApplication::GetInstance()
{
    /*这是一个经典的单例模式的实现，用于获取 MprpcApplication 类的单一实例。
    在这个实现中，使用了静态局部变量，确保了只有一个 MprpcApplication 实例，
    并且在需要时进行了延迟初始化。这种方式保证了线程安全性和懒加载。*/
    static MprpcApplication app;
    return app;
}

MprpcConfig &MprpcApplication::GetConfig()
{
    return m_config;
}