#pragma once
#include <google/protobuf/service.h>
#include<string>

class MprpcController : public google::protobuf::RpcController
{
public:
    MprpcController();
    void Reset();
    // const 关键字用于修饰 Failed() 方法，表示该方法是一个常量成员函数。
    // 这意味着在 Failed() 方法中，不会修改 MprpcController 对象的任何成员变量。
    bool Failed() const;

    std::string ErrorText() const;
    void SetFailed(const std::string& reason);

    //目前未实现具体功能
    void StartCancel();
    bool IsCanceled() const;
    void NotifyOnCancel(google::protobuf::Closure* callback);

private:
    bool m_failed;//RPC方法执行过程中的状态
    std::string m_errText;//RPC方法执行过程中的错误信息


};
