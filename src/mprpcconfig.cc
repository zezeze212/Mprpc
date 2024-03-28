#include "mprpcconfig.h"
#include<iostream>
#include<string>

// 负责解析加载配置文件
// MprpcConfig:: 类的作用域为MprpcConfig
void MprpcConfig::LoadConfigFile(const char *config_file)
{
    FILE *pf = fopen(config_file, "r");
    if(nullptr == pf)
    {
        std::cout<< config_file << "is not exist" <<std::endl;
        exit(EXIT_FAILURE);
    }

    /*feof(pf) 是 C 语言中的一个函数，用于检查文件流 pf 的文件结束标志。
    当文件结束时，feof(pf) 返回一个非零值（通常是1）。
    当文件未结束时，feof(pf) 返回0。 
    */
   //处理 1.注释 2.正确的配置项 = 3.去掉开头的多余的空格 
   while(!feof(pf))
   {
        char buf[512] = {0};
        fgets(buf, 512, pf);

        /*fgets 是 C 语言中用于从文件流中读取一行文本的函数。它的原型如下：
        char *fgets(char *str, int n, FILE *stream);
        它接受三个参数：
        str：指向一个字符数组的指针，用于存储读取到的文本内容。注意，该数组应该足够大，能够容纳你期望的最大行长度（包括终止符 '\0'）。
        n：表示要读取的最大字符数，包括终止符 '\0'。
        stream：文件流指针，用于指定从哪个文件流中读取内容。*/

        //去掉字符串多余的空格
        std::string read_buf(buf);
        

        //判断#号的注释
        if (read_buf[0] == '#' || read_buf.empty())
        {
            continue;
        }

        //解析配置项
        int idx = read_buf.find('=');
        if(idx == -1)
        {
            //配置不合法
            continue;
        }

        std::string key;
        std::string value;
        key = read_buf.substr(0, idx);

        // 若出现这样的例子rpcserverip = 127.0.0.0.1，再清一下空格防止等号空格被录入
        Trim(key);

        // 从idx处开始，在字符串read_buf中查找第一个出现\n的位置,并将其索引赋值给endidx变量。
        int endidx = read_buf.find('\n',idx);
        value = read_buf.substr(idx + 1, endidx - idx - 1);

        Trim(value); 

        m_configMap.insert({key,value});

   }

}



// 查询配置项信息
std::string MprpcConfig::Load(const std::string &key)
{
    auto it = m_configMap.find(key);
    if(it == m_configMap.end())
    {
        return "";
    }

    return it->second;

}

// 去掉字符串前后的空格
void MprpcConfig::Trim(std::string &src_buf)
{
    int idx = src_buf.find_first_not_of(' ');
    if (idx != -1)
    {
        // 前边有空格
        src_buf = src_buf.substr(idx, src_buf.size() - idx);
    }
    // 去掉字符串后边多余的空格，从后往前找
    idx = src_buf.find_last_not_of(' ');
    if (idx != -1)
    {
        src_buf = src_buf.substr(0, idx + 1);
    }
}