# pragma

#include <semaphore.h>
#include <zookeeper/zookeeper.h>
#include <string>

class ZkClient
{
public:
    ZkClient();
    ~ZkClient();//析构函数，解释见最下

    //zkclient启动链接zkserver
    void Start();

    //在zkserver上根据指定的path创建znode节点
    void Create(const char *path, const char *data, int datalen, int state=0);

    //根据参数指定的znode节点路径，获取znode节点的值
    std::string GetData(const char *path);

private:
    //zk的客户端句柄
    zhandle_t *m_zhandle;

};

/*
在C++中，波浪号（~）是析构函数的标识符。
析构函数是在对象销毁时自动调用的特殊成员函数，用于释放对象所占用的资源、清理对象的状态等操作。

在类中定义析构函数的目的是在对象生命周期结束时执行必要的清理工作，
例如释放动态分配的内存、关闭文件句柄、断开网络连接等。
析构函数的命名规则是在类名前加上波浪号，后跟无参数的函数名。

例如，在上面的代码中，ZkClient 类定义了析构函数 ~ZkClient()。
这意味着当 ZkClient 类的对象销毁时，将会自动调用析构函数，执行清理操作，确保对象被正确地销毁和释放。*/