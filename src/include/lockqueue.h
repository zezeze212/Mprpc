#pragma
#include<queue>
#include<thread>
#include<mutex>  //pthread_mutex_t 互斥锁
#include<condition_variable> //phread_condition_t 条件变量


//异步日志队列
/*template: 声明这是一个模板。
<typename T>: 指定模板参数列表，其中 typename 是用于声明类型参数的关键字，T 是模板参数的名称。
T: 模板参数的名称，可以是任意合法的标识符(任意 int string 之类的)，用于表示模板中的类型参数。*/
template<typename T>
class LockQueue
{
public:
    //多个工作线程都会写日志queue
    //模板代码只能写在头文件中
    void Push(const T &data)
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_queue.push(data);
        m_condvariable.notify_one();
    }

    //一个线程读日志queue，写日志文件
    T Pop()
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        while (m_queue.empty())
        {
            //日志队列为空，线程进入wait状态
            m_condvariable.wait(lock);
        }

        T data = m_queue.front();
        m_queue.pop();
        return data;
    }

private:
    std::queue<T> m_queue;
    std::mutex m_mutex;
    std::condition_variable m_condvariable;
};