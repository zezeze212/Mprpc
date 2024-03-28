#include<iostream>
#include<string>
#include "user.pb.h"
//#include "../../src/include/mprpcapplication.h"
#include "mprpcapplication.h"
#include "rpcprovider.h"

/*
    UserService 原来是一个本地服务，提供了两个进程内的本地方法，Login和GetFriendLists
    class UserService: 这是一个类的声明，类名为 UserService。
    public fixbug::UserServiceRpc: 这表示 UserService 类公开继承了 fixbug::UserServiceRpc类。
    这意味着 UserService 类将继承 fixbug::UserServiceRpc 类中定义的所有成员变量和成员函数，
    并且这些成员在 UserService 中也是公开的。  
*/

class UserService : public fixbug::UserServiceRpc //使用在rpc服务发布端（rpc服务提供者）
{
public:
    bool Login(std::string name,std::string pwd)
    {
        std::cout << "doing Login service: Login" << std::endl;
        std::cout << "name: " << name << " pwd: " << pwd << std::endl;
        return true;
    }

    bool Register(uint32_t id, std::string name, std::string pwd)
    {
        std::cout << "doing Register service: Login" << std::endl;
        std::cout <<"id: "<< id << " name: " << name << " pwd: " << pwd << std::endl;
        return true;
    }

    // 重写基类UserServiceRpc虚函数，下面这些方法都是框架直接调用的
    //callee 服务提供端，对方发过来的调用方法请求和参数都是先被rpc框架接收
    //rpc根据请求和参数匹配到方法，上报上来
    //接收到请求之后，从请求中拿出数据，做本地业务，执行回调，把执行完的数据再塞回框架
    //再由框架序列化发送回去
    /*
    1、caller ===>   Login(LoginRequest) ===> muduo ===> callee
    2、callee ===>  Login(LoginRequest) ===> 交到下面重写的Login方法
    */
    virtual void Login(::google::protobuf::RpcController *controller,
                       const ::fixbug::LoginRequest *request,
                       ::fixbug::LoginResponse *response,
                       ::google::protobuf::Closure *done)
    {
        //框架给业务上报了请求参数LoginRequest，业务获取相应数据做本地业务
        std::string name = request->name();
        std::string pwd = request->pwd();

        //做本地业务
        bool login_result = Login(name,pwd);
        
        //把响应写入,包括在错误码，错误消息，返回值
        fixbug::ResultCode* code = response->mutable_result();
        code->set_errcode(0);
        code->set_errmsg("");
        response->set_success(login_result);

        //执行回调操作，执行响应对象数据的序列化和网络发送（都是由框架来完成的）
        done->Run();

    }

    virtual void Register(::google::protobuf::RpcController *controller,
                          const ::fixbug::RegisterRequest *request,
                          ::fixbug::RegisterResponse *response,
                          ::google::protobuf::Closure *done)
    {
        uint32_t id = request->id();
        std::string name = request->name();
        std::string pwd = request->pwd();

        bool ret = Register(id, name, pwd);

        response->mutable_result()->set_errcode(0);
        response->mutable_result()->set_errmsg("");
        response->set_success(ret);

        done->Run();
    }
};

int main(int argc, char **argv)
{
    // UserService us;
    // us.Login("xxx","xxx");

    //调用框架的初始化操作 希望他这样写参数 provider -i config.conf
    MprpcApplication::Init(argc,argv);

    //provider是一个rpc网络服务对象，把UserService对象发布到rpc节点上
    RpcProvider provider;
    provider.NotifyService(new UserService());

    //启动一个rpc服务发布节点,Run以后，进程进入阻塞状态，等待远程的rpc调用请求
    provider.Run();

    return 0;
}
