#include <iostream>
#include <string>
#include "friend.pb.h"
// #include "../../src/include/mprpcapplication.h"
#include "mprpcapplication.h"
#include "rpcprovider.h"
#include "logger.h"

class FriendService : public fixbug::FriendServiceRpc
{
public:
    std::vector<std::string> GetFriendsList(uint32_t userid)
    {
        std::cout << "do GetFriendsList service! userid: " << userid <<std::endl;
        std::vector<std::string> vec;
        vec.push_back("anzetao");
        vec.push_back("liuyang");
        vec.push_back("mqx");
        return vec;
    }

    //重写基类方法
    virtual void GetFriendsList(::google::protobuf::RpcController *controller,
                                const ::fixbug::GetFriendsListRequest *request,
                                ::fixbug::GetFriendsListResponse *response,
                                ::google::protobuf::Closure *done)
    {

        uint32_t user_id = request->userid();
        
        std::vector<std::string> friendList = GetFriendsList(user_id);

        response->mutable_result()->set_errcode(0);
        response->mutable_result()->set_errmsg("");

        /*调用 response->add_friends() 函数向响应中添加一个新的好友名称。
        将当前好友的名称 name 赋值给指向新添加好友名称的指针 p，即 *p = name。*/
        for(std::string &name : friendList)
        {
            std::string *p = response->add_friends();
            *p = name;
        }

        done->Run();
    }
};

int main(int argc, char **argv)
{

    LOG_INFO("first log message!");
    LOG_ERR("%s:%s:%d", __FILE__, __FUNCTION__, __LINE__);

    // UserService us;
    // us.Login("xxx","xxx");

    // 调用框架的初始化操作 希望他这样写参数 provider -i config.conf
    MprpcApplication::Init(argc, argv);

    // provider是一个rpc网络服务对象，把UserService对象发布到rpc节点上
    RpcProvider provider;
    provider.NotifyService(new FriendService());

    // 启动一个rpc服务发布节点,Run以后，进程进入阻塞状态，等待远程的rpc调用请求
    provider.Run();

    return 0;
}
