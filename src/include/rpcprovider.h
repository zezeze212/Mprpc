#pragma once
// 框架提供的专门发布rpc服务的网络对象类
#include "google/protobuf/service.h"
#include<muduo/net/EventLoop.h>
#include<muduo/net/TcpServer.h>
#include<muduo/net/InetAddress.h>
#include<muduo/net/TcpConnection.h>
#include <functional>
#include <google/protobuf/descriptor.h>
#include <string>
#include<unordered_map>


class RpcProvider
{
//这都是方法的声明，实现写在对应的cc文件中
public:
    // 这里是框架提供给外部使用的，可以发布rpc方法的函数接口
    //这里使用Service 是为了让指针能接受所有类型的对象
    //因为你注册的服务类对象继承了从protobuf中生成的类，而protobuf中的类又继承自Service
    //所有用Service能接受所有对象
    void NotifyService(google::protobuf::Service *service);

    //启动rpc服务节点，开始提供rpc远程网络调用服务
    void Run();

private:
    // 组合了EventLoop,
    /*EventLoop 是一个循环，不断地监听事件并且处理它们。
    事件可以是用户的输入、IO 操作完成、定时器到期等。
    EventLoop 负责将这些事件分派给相应的处理程序，以便进行处理。*/
    muduo::net::EventLoop m_eventLoop;

    //服务类型信息
    struct ServiceInfo
    {
        google::protobuf::Service *m_service; //保存服务对象
        //保存服务方法
        std::unordered_map<std::string, const google::protobuf::MethodDescriptor*> m_methodMap;
    };
    //存储注册成功的服务对象和其服务方法的所有信息
    std::unordered_map<std::string, ServiceInfo> m_serviceMap;

    //新的socket连接回调
    void OnConnection(const muduo::net::TcpConnectionPtr&);
    //已建立连接用户的读写事件回调
    void OnMessage(const muduo::net::TcpConnectionPtr &, muduo::net::Buffer *, muduo::Timestamp);
    //Closure 的回调操作，用于序列化rpc的响应和网络发送
    void SendRpcResponse(const muduo::net::TcpConnectionPtr&, google::protobuf::Message*);
};