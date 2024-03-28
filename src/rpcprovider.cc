#include "rpcprovider.h"
#include "mprpcapplication.h"
#include "rpcheader.pb.h"
#include "logger.h"
#include "zookeeperutil.h"

/*
service_name => service描述
                        =>service* 记录服务对象
                        method_name=> method方法对象
*/
//这里是框架提供给外部使用的，可以发布rpc方法的函数接口
void RpcProvider::NotifyService(google::protobuf::Service *service) 
{
    ServiceInfo service_info;

    //获取服务对象的描述信息
    const google::protobuf::ServiceDescriptor *pserviceDesc = service->GetDescriptor();
    //获取服务对象的名字
    std::string service_name = pserviceDesc->name();
    //获取服务对象的方法数量
    int methodCnt = pserviceDesc->method_count();

    //std::cout<< "service_name:" << service_name << std::endl;
    LOG_INFO("service_name:%s", service_name.c_str());

    for(int i=0; i<methodCnt; i++)
    {
        //获取了服务对象指定下标的服务方法的描述（抽象描述）
        const google::protobuf::MethodDescriptor* pmethodDesc = pserviceDesc->method(i);
        std::string method_name = pmethodDesc->name();
        service_info.m_methodMap.insert({method_name, pmethodDesc});
        // std::cout << "method_name:" << method_name << std::endl;
        LOG_INFO("method_name:%s", method_name.c_str());
    }
    service_info.m_service = service;
    m_serviceMap.insert({service_name, service_info});
}

//启动rpc服务节点，开始提供rpc远程网络调用服务
void RpcProvider::Run() 
{
    std::string ip = MprpcApplication::GetInstance().GetConfig().Load("rpcserverip");

    /*1、MprpcApplication::GetInstance() 调用返回了 MprpcApplication 类的单一实例。
    2、GetConfig() 是 MprpcApplication 类的一个成员函数，用于获取配置对象。
    3、Load("rpcserverip") 是 Config 对象的成员函数，用于从配置文件中加载指定键的值，此处加载的是 rpcserverip 的值。
    4、c_str() 函数将返回的 std::string 对象转换为 C 风格的字符串指针。
    5、atoi() 函数将 C 风格的字符串转换为整数。在这里，它将 rpcserverip 的值转换为整数形式。
    6、最后，将转换后的整数值赋给 prot 变量，该变量的类型是 uint16_t，即无符号 16 位整数类型，通常用于表示端口号。*/

    uint16_t port = atoi(MprpcApplication::GetInstance().GetConfig().Load("rpcserverport").c_str());

    // muduo::net::InetAddress 是 Muduo 网络库中用于表示 IP 地址和端口号的类
    muduo::net::InetAddress address(ip,port);

    // 创建TcpServer对象,TcpServer 是 Muduo 网络库中用于创建 TCP 服务器的类。
    //它提供了一种简单而高效的方式来编写异步、事件驱动的 TCP 服务器程序。
    muduo::net::TcpServer server(&m_eventLoop, address, "RpcProvider");

    //绑定连接回调和消息读写方法，分离了网络代码和业务代码
    server.setConnectionCallback(std::bind(&RpcProvider::OnConnection,this, std::placeholders::_1));
    server.setMessageCallback(std::bind(&RpcProvider::OnMessage, this, std::placeholders::_1,
                std::placeholders::_2, std::placeholders::_3    ));

    // 设置muduo库的线程数量 1个io线程3个工作线程
    server.setThreadNum(4);

    //把当前rpc节点要发布的服务全部注册到zk上面，让rpc licent可以从zk上找服务
    //session timeout 30s zklient 网络I/O线程 会以3/1 * timeout 时间发送ping消息，防止清除
    ZkClient zkCli;
    zkCli.Start();
    //service_name为永久节点  method_name为临时节点
    for(auto &sp : m_serviceMap)
    {
        // service_name    举例 Userservice
        std::string service_path = "/" +sp.first;
        zkCli.Create(service_path.c_str(), nullptr, 0);
        for(auto &mp : sp.second.m_methodMap)
        {
            // /service_name/method_name   举例 /Userservice/Login
            std::string method_path = service_path + "/" + mp.first;
            char method_path_data[10000] = {0};
            sprintf(method_path_data, "%s:%d", ip.c_str(), port);
            // ZOO_EPHEMERAL表示znode是一个临时节点
            zkCli.Create(method_path.c_str(), method_path_data, strlen(method_path_data), ZOO_EPHEMERAL);
        }
    }

    std::cout << "RpcProvider start service at ip:" << ip << "port:" << port<<std::endl;

    //启动网络服务
    server.start();

    //.loop() 方法的作用是启动事件循环，使程序能够不断地监听和处理各种事件，以实现异步、高效的事件驱动编程。
    m_eventLoop.loop();
}
//新的socket连接回调
void RpcProvider::OnConnection(const muduo::net::TcpConnectionPtr &conn)
{
    //rpc的请求是一个短链接的请求，服务端返回相应之后可以断开
    if(!conn->connected())
    {
        //和rpc client断开了
        conn->shutdown();
    }

}


/*
在框架内部，RpcProvider和RpcConsumer协商好通信用的protobuf数据类型
service_name method_name args 定义proto的message类型，进行数据头（除去args）的序列化和反序列化
12UserServiceLogin yiersan123456
header_size(四个字节) + header_str + args_str
读四个字节直接转成字符串不行  “10”和“10000” 字节明显不同并且“10000”是五个字节
std::string 的 insert和copy方法解决
*/
// 已建立连接用户的读写回调，如果远程有一个rpc服务的调用请求，那么OnMessage方法就会相应
// muduo::net::TcpConnectionPtr: 这是一个指针，指向了 muduo 网络库中的 TcpConnection 类型的对象。
// 通常情况下，这种指针用于表示一个 TCP 连接。
// muduo::net::Buffer：这是一个在 muduo 网络库中定义的缓冲区类，通常用于处理数据的读写操作。
void RpcProvider::OnMessage(const muduo::net::TcpConnectionPtr &conn, 
                            muduo::net::Buffer *buffer, 
                            muduo::Timestamp)
{

    //网络上接受的远程rpc调用请求的字符流 Login args
    std::string recv_buf = buffer->retrieveAllAsString();
    uint32_t header_size = 0;
    //从零开始拷贝四个字节到header_size
    recv_buf.copy((char*)&header_size, 4, 0);

    //根据header_size 读取数据头的原始字符流，反序列化数据，得到rpc请求的详细信息
    std::string rpc_header_str = recv_buf.substr(4, header_size);
    mprpc::RpcHeader rpcHeader;
    std::string service_name;
    std::string method_name;
    uint32_t args_size;
    if(rpcHeader.ParseFromString(rpc_header_str))
    {
        //数据头反序列化成功
        service_name = rpcHeader.service_name();
        method_name = rpcHeader.method_name();
        args_size = rpcHeader.args_size();
    }
    else
    {
        //数据头反序列化失败
        std::cout<< "rpc_header_str:" << rpc_header_str <<" parse error!" <<std::endl;
        return;
    }

    //获取rpc方法参数的字符流数据
    std::string args_str = recv_buf.substr(4 + header_size, args_size);

    //打印调试信息
    std::cout<<"====================================="<< std::endl;
    std::cout << "header_size: " << header_size << std::endl;
    std::cout << "service_name: " << service_name << std::endl;
    std::cout << "method_name: " << method_name << std::endl;
    std::cout << "args_size: " << args_size << std::endl;
    std::cout << "rpc_header_str: " << rpc_header_str << std::endl;
    //std::cout << "args_str: " << args_str << std::endl;
    LOG_INFO("args_str:%s", args_str.c_str());

    // 打印 args_str 中每个字符的 ASCII 值
    // std::cout << "args_str (ASCII values): ";
    // for (char c : args_str)
    // {
    //     std::cout << static_cast<int>(c) << " ";
    // }
    // std::cout << std::endl;
    std::cout << "=====================================" << std::endl;

    //获取service对象和method对象
    auto it = m_serviceMap.find(service_name);
    if(it == m_serviceMap.end())
    {
        std::cout << service_name << "is not exist" <<std::endl;
        return;
    }

    auto mit = it->second.m_methodMap.find(method_name);
    if(mit == it->second.m_methodMap.end())
    {
        std::cout << service_name << ":" << method_name << "is not exist" << std::endl;
        return;
    }

    google::protobuf::Service *service = it->second.m_service; // 获取service对象 现在是对应new出来的UserService
    const google::protobuf::MethodDescriptor *method = mit->second; // 获取method对象

    // 生成rpc方法调用请求的request和响应response参数
    // GetRequestPrototype(method): 这个方法用于获取指定RPC方法的请求消息的原型
    google::protobuf::Message *request = service->GetRequestPrototype(method).New();

    if(!request->ParseFromString(args_str))
    {
        std::cout<<"request parse error, content:" << args_str <<std::endl;
        return;
    }

    google::protobuf::Message *response = service->GetResponsePrototype(method).New();

    //给下面的method方法的调用，绑定一个回调函数
    //
    google::protobuf::Closure *done = google::protobuf::NewCallback<RpcProvider, 
                                                                    const muduo::net::TcpConnectionPtr &, 
                                                                    google::protobuf::Message *>
                                                                    (this,
                                                                    &RpcProvider::SendRpcResponse,
                                                                    conn,
                                                                    response);

    //在框架上远端rpc请求，调用当前rpc节点上发布的方法
    //在这里是new UserService().Login(controller, request,response,done)

    //调用方法
    service->CallMethod(method, nullptr, request, response, done); 


}

// Closure 的回调操作，用于序列化rpc的响应和网络发送
// 因为是protobuf 所以是c++，c++的普通方法都需要绑定一个对象，所以上述callback用了this
void RpcProvider::SendRpcResponse(const muduo::net::TcpConnectionPtr &conn, google::protobuf::Message *response)
{
    std::string response_str;
    if(response->SerializeToString(&response_str))
    {
        //序列化成功后，通过网络把rpc方法执行的结果发送给rpc调用方
        conn->send(response_str);
    }
    else
    {
        std::cout << "Serialize response_str error" << std::endl;
    }
    conn->shutdown(); // 模拟http的短链接服务，由rpcprovider主动断开连接
    
}