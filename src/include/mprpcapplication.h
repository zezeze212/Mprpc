#pragma once
#include "mprpcconfig.h"
#include "mprpccontroller.h"
#include "mprpcchannel.h"

// mprpc框架的基础类，负责框架的初始化操作

class MprpcApplication
{
public:
    static void Init(int argc,char **argv);
    static MprpcApplication& GetInstance();
    static MprpcConfig& GetConfig();
private:

    static MprpcConfig m_config;

    //构造函数
    MprpcApplication(){}
    //把所有与拷贝相关的删掉
    MprpcApplication(const MprpcApplication&) = delete;
    MprpcApplication(MprpcApplication&&) = delete;
};