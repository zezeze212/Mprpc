#include "mprpcchannel.h"
#include "rpcheader.pb.h"
#include <sys/types.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include<errno.h>
#include "mprpcapplication.h"
#include<unistd.h>
#include "mprpccontroller.h"
#include "logger.h"
#include "zookeeperutil.h"

/*
header_size service_name method_name args_size +args
*/
void MprpcChannel::CallMethod(const google::protobuf::MethodDescriptor *method,
                google::protobuf::RpcController *controller,
                const google::protobuf::Message *request,
                google::protobuf::Message *response,
                google::protobuf::Closure *done)     
{
    // 在 Google Protobuf 中，服务描述符 (ServiceDescriptor) 用于描述一个服务，
    // 而方法描述符 (MethodDescriptor) 用于描述服务中的方法。
    // 通过方法描述符的 service() 函数可以获取到与之相关联的服务描述符。
    const google::protobuf::ServiceDescriptor *sd = method->service();
    std::string service_name = sd->name();//service_name
    std::string method_name = method->name();

    //获取参数的序列化字符串长度
    uint32_t args_size = 0;
    std::string args_str;
    if(request->SerializeToString(&args_str))
    {
        args_size = args_str.size();
    }
    else
    {
        // std::cout << "serialize request error!" <<std::endl;
        controller->SetFailed("serialize request error!");
        return;
    }

    //定义rpc的请求header
    mprpc::RpcHeader rpcHeader;
    rpcHeader.set_service_name(service_name);
    rpcHeader.set_method_name(method_name);
    rpcHeader.set_args_size(args_size);

    uint32_t header_size=0;
    std::string rpc_header_str;
    if(rpcHeader.SerializeToString(&rpc_header_str))
    {
        header_size=rpc_header_str.size();
    }
    else
    {
        //std::cout << "serialize rpc header error" << std::endl;
        controller->SetFailed("serialize rpc header error");
    }

    //组织待发送的rpc请求字符串
    std::string send_rpc_str;
    send_rpc_str.insert(0, std::string((char*)&header_size, 4)); //header_size
    send_rpc_str += rpc_header_str; //header
    send_rpc_str += args_str; //args

    // 打印调试信息
    std::cout << "=====================================" << std::endl;
    std::cout << "header_size: " << header_size << std::endl;
    std::cout << "service_name: " << service_name << std::endl;
    std::cout << "method_name: " << method_name << std::endl;
    std::cout << "args_size: " << args_size << std::endl;
    std::cout << "rpc_header_str: " << rpc_header_str << std::endl;
    //std::cout << "args_str: " << args_str << std::endl;
    LOG_INFO("args_str:%s", args_str.c_str());
    std::cout << "=====================================" << std::endl;

    // 使用tcp编程，完成rpc方法的远程调用
    //  socket(AF_INET, SOCK_STREAM, 0);：这个函数调用创建一个套接字。它接受三个参数：
    // AF_INET 表示使用 IPv4 协议族。
    // SOCK_STREAM 表示创建一个流式套接字，即 TCP 套接字。
    // 0 表示根据第二个参数自动选择协议。 int clientfd = socket(AF_INET, SOCK_STREAM, 0);
    
    int clientfd = socket(AF_INET, SOCK_STREAM, 0);

    if(clientfd == -1)
    {
        //std::cout << "create socket error errno:" << errno <<std::endl;
        char errtxt[100000] = {0};
        sprintf(errtxt, "create socket error errno: %d", errno);
        controller->SetFailed(errtxt);
        return;

        /*sprintf 函数是 C 语言标准库 <cstdio> 中的一个函数，用于将格式化的数据写入字符串中。
        其函数原型如下：
        int sprintf(char* str, const char* format, ...);
        str 是一个指向字符数组的指针，用于存储格式化后的字符串。
        format 是一个格式化字符串，可以包含转换说明符 % 以及其他字符。
        ... 是可变数量的参数，用于填充格式化字符串中的占位符。*/
    }

    //读取配置文件rpcserver的信息
    // std::string ip = MprpcApplication::GetInstance().GetConfig().Load("rpcserverip");
    // uint16_t port = atoi(MprpcApplication::GetInstance().GetConfig().Load("rpcserverport").c_str());

    ZkClient zkCli;
    zkCli.Start();
    std::string method_path = "/" + service_name + "/" + method_name;
    std::string host_data = zkCli.GetData(method_path.c_str());
    if(host_data == "")
    {
        controller->SetFailed(method_path + "is not exist!");
        return;
    }

    int idx = host_data.find(":");
    if(idx == -1)
    {
        controller->SetFailed(method_path + "address is invalid!");
        return;
    }

    std::string ip = host_data.substr(0, idx);
    uint16_t port = atoi(host_data.substr(idx+1, host_data.size()-idx).c_str());


    /*sockaddr_in 结构体用于表示 IPv4 地址信息，具体内容包括 IP 地址和端口号。
    通常情况下，struct sockaddr_in 结构体包含以下字段：
    sin_family：表示地址族，通常设置为 AF_INET 表示 IPv4 地址族。
    sin_port：表示端口号，以网络字节顺序存储（通常使用 htons() 函数转换）。
    sin_addr：表示 IP 地址，以 struct in_addr 类型存储。*/

    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port =  htons(port);
    server_addr.sin_addr.s_addr = inet_addr(ip.c_str());
    // inet_addr() 函数用于将点分十进制表示的 IP 地址转换为 in_addr_t 类型的二进制网络字节序表示。
    /*sin_addr 是 sockaddr_in 结构体中的一个字段，
    用于存储 IPv4 地址信息。在网络编程中，IPv4 地址通常以 32 位二进制形式表示，
    但在 sockaddr_in 结构体中，sin_addr 被定义为一个结构体 in_addr 的变量，
    该结构体包含一个 32 位的整数字段 s_addr，用于存储 IPv4 地址。*/

    //连接rpc服务节点
    if(-1 == connect(clientfd, (struct sockaddr*)&server_addr, sizeof(server_addr)))
    {
        //std::cout << "connect error! errno:" << errno << std::endl;
        close(clientfd);
        char errtxt[100000] = {0};
        sprintf(errtxt, "connect error errno: %d", errno);
        controller->SetFailed(errtxt);
        return;
    }

    //发送rpc请求
    if(-1 == send(clientfd, send_rpc_str.c_str(), send_rpc_str.size(), 0))
    {
        //std::cout << "send error! errno:" << errno << std::endl;
        close(clientfd);
        char errtxt[100000] = {0};
        sprintf(errtxt, "send error errno: %d", errno);
        controller->SetFailed(errtxt);
        return;
    }

    //接受rpc请求的响应值
    char recv_buf[1024] = {0};
    int recv_size = 0;
    if(-1 ==(recv_size = recv(clientfd, recv_buf, 1024, 0)))
    {
        //std::cout << "receive error! errno:" << errno << std::endl;
        close(clientfd);
        char errtxt[100000] = {0};
        sprintf(errtxt, "receive error errno: %d", errno);
        controller->SetFailed(errtxt);
        return;
    }

    //反序列化rpc调用的响应数据
    //std::string response_str(recv_buf, 0 ,recv_size); //bug出现问题，recv_buf中出现/0后边数据就不再读取，后边数据不能存储
    //if(!response->ParseFromString(response_str))

    if(!response->ParseFromArray(recv_buf, recv_size))
    {
        //std::cout << "parse error! resonse_str:" << recv_buf << std::endl;
        close(clientfd);
        char errtxt[100000] = {0};

        sprintf(errtxt, "parse error resonse_str: %s", recv_buf);

        // snprintf(errtxt, sizeof(errtxt), 
        // "parse error resonse_str: %.*s", 
        // static_cast<int>(sizeof(errtxt) - 1), 
        // recv_buf);
        /*
        %.*s，它表示输出 recv_buf 字符串的前几个字符。
        static_cast<int>(sizeof(errtxt) - 1)：
        将 errtxt 缓冲区的大小减去 1 转换为整数类型，
        用作 %.*s 占位符的宽度参数，确保 recv_buf 不会超出 errtxt 缓冲区的大小。*/

        controller->SetFailed(errtxt);
        return;
    }
    close(clientfd);
}