#include <iostream>
#include "user.pb.h"
#include "mprpcchannel.h"
#include "mprpcapplication.h"


int main(int argc, char **argv)
{
    //整个程序启动以后，想使用mprpc框架来享受rpc服务调用，一定需要先调用框架的初始化函数（只初始化一次）
    MprpcApplication::Init(argc,argv);
    
    
    //演示调用远程发布的rpc方法Login
    fixbug::UserServiceRpc_Stub stub(new MprpcChannel());
    //rpc方法的请求参数
    fixbug::LoginRequest request;
    request.set_name("aztqaq");
    request.set_pwd("0306");
    // rpc方法的响应
    fixbug::LoginResponse response;

    //发起rpc方法的调用，同步的rpc调用过程 MprpcChannel::callMethod
    stub.Login(nullptr, &request, &response, nullptr);
    //RpcChannel=》RpcChannel::callMethod 集中做所有rpc调用


    //一次rpc调用完成，读调用的结果
    if(response.result().errcode() == 0)
    {
        std::cout << "rpc login response success:" << response.success() << std::endl;
    }
    else
    {
        std::cout << "rpc login response error:" << response.result().errmsg() << std::endl;
    }

    // 演示调用远程发布的rpc方法Register
    fixbug::RegisterRequest req;
    req.set_id(2019);
    req.set_name("zhangsan");
    req.set_pwd("0802");

    fixbug::RegisterResponse rep;

    //以同步的方式发起rpc请求调用，等待返回结果
    stub.Register(nullptr, &req, &rep, nullptr);

    if (rep.result().errcode() == 0)
    {
        std::cout << "rpc register response success:" << rep.success() << std::endl;
    }
    else
    {
        std::cout << "rpc register response error:" << rep.result().errmsg() << std::endl;
    }

    return 0;
}