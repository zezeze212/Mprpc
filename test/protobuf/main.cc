#include"test.pb.h"
#include<iostream>
#include<string>
using namespace fixbug;

int main()
{
    // LoginResponse rsp;
    // //对于成员变量本身又是另一个message类的话，会提供一个mutable用于改变数值
    // ResultCode *rc = rsp.mutable_result();   //mutable可改变的
    // rc->set_errcode(1);
    // rc->set_errmsg("登录处理失败");

    GetFriendListsResponse rsp;
    ResultCode *rc = rsp.mutable_result();
    rc->set_errcode(0);//设置成0故障，不用设置报错信息了

    User *user1 = rsp.add_frinend_list();
    user1->set_name("zhangsanfeng");
    user1->set_age(99);
    user1->set_sex(User::MAN);
    //因为用的枚举选择性别，定义在User类里，所以赋值的时候声明User

    User *user2 = rsp.add_frinend_list();
    user2->set_name("zhangsan");
    user2->set_age(99);
    user2->set_sex(User::MAN);

    std::cout << rsp.frinend_list_size() << std::endl;
    User user3 = rsp.frinend_list(1);//从0开始第一个用户，1是第二个用户
    std::cout << user3.name() << std::endl;
    return 0;
}
    



int main1()
{
    //封装了login请求对象的数据
    LoginRequest req;
    req.set_name("zhangsanfeng");
    req.set_pwd("123456");

    //对象数据序列化=》char
    std::string send_str;
    if(req.SerializePartialToString(&send_str))
    {
        std::cout<<send_str<<std::endl;
    }

    //从send_str反序列化一个login请求对象
    LoginRequest reqB;
    if(reqB.ParseFromString(send_str))
    {
        std::cout << reqB.name() << std::endl;
        std::cout << reqB.pwd() << std::endl;
    }

    return 0;
}