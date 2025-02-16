#include <iostream>             // 用于输入输出流
#include <thread>               // 用于线程操作
#include <queue>                // 用于消息队列（FIFO）
#include <mutex>                // 用于互斥锁，保护共享数据
#include <condition_variable>   // 用于线程间通知等待
#include <functional>           // 用于 std::function，保存回调函数
#include <vector>               // 用于存储多个线程

// 定义消息类型枚举，用于标识发送到 Actor 的消息种类
enum class MessageType {
    Increment,  // 表示增加计数的消息
    GetCount,   // 表示获取当前计数值的消息
    Stop        // 表示停止 Actor 的消息
};

// 定义消息结构体，表示一条消息
struct Message {
    MessageType type;               // 消息类型
    // 对于需要回复结果的消息，可以传入一个回调函数，参数为当前计数值
    std::function<void(int)> reply;
};

//
// CounterActor 类：实现一个计数器 Actor，采用独立线程和消息队列处理消息
//
class CounterActor {
public:
    // 构造函数：初始化计数器并启动内部线程
    CounterActor() : count(0), running(true) {
        // 创建一个线程，并指定该线程执行 CounterActor::run 成员函数
        // "this" 作为参数传入，便于在成员函数中访问对象数据
        actorThread = std::thread(&CounterActor::run, this);
    }

    // 析构函数：在对象销毁前发送停止消息，并等待内部线程结束
    ~CounterActor() {
        // 发送一个 Stop 消息，告诉 Actor 退出消息处理循环
        send({ MessageType::Stop, nullptr });
        // 如果内部线程还在运行，则等待它结束
        if (actorThread.joinable()) {
            actorThread.join();
        }
    }

    // 向 Actor 发送消息的接口
    // 参数为 const 引用，保证传入的消息不被修改
    void send(const Message &msg) {
        {
            // 加锁，保护对消息队列的访问，防止多线程竞争
            std::lock_guard<std::mutex> lock(mtx);
            messages.push(msg);  // 将消息放入队列中
        }
        // 通知等待的线程，消息队列中有新消息到达
        cv.notify_one();
    }

private:
    int count;                         // 内部计数器状态
    bool running;                      // 是否继续运行的标志，当为 false 时退出消息处理循环
    std::thread actorThread;           // Actor 内部的工作线程，用于处理消息
    std::queue<Message> messages;      // 消息队列，存放等待处理的消息
    std::mutex mtx;                    // 互斥锁，用于保护消息队列的线程安全
    std::condition_variable cv;        // 条件变量，用于等待消息到达

    // Actor 的主循环函数，不断从消息队列中取消息并处理
    void run() {
        while (running) {  // 只要 running 为 true，就不断处理消息
            Message msg;
            {
                // 使用 unique_lock 对消息队列上锁
                std::unique_lock<std::mutex> lock(mtx);
                // 条件变量等待，直到消息队列非空
                // Lambda 表达式返回 true 时（队列不空）继续，否则阻塞等待
                cv.wait(lock, [&] { return !messages.empty(); });
                // 取出队列头部的消息
                msg = messages.front();
                messages.pop();  // 弹出已经取出的消息
            }
            // 根据消息的类型进行不同的处理
            switch (msg.type) {
                case MessageType::Increment:
                    // 增加计数消息：将计数器加 1
                    count++;
                    break;
                case MessageType::GetCount:
                    // 获取计数消息：如果有回调函数，则调用回调函数将当前计数返回
                    if (msg.reply) {
                        msg.reply(count);
                    }
                    break;
                case MessageType::Stop:
                    // 停止消息：将 running 置为 false，从而退出循环
                    running = false;
                    break;
            }
        }
    }
};

//
// 主函数：模拟大量并发消息发送，展示 Actor 模型的高并发处理能力
//
int main() {
    // 创建一个 CounterActor 对象，表示一个计数器 Actor
    CounterActor actor;

    // 定义并发测试参数：
    // numThreads 表示启动的线程数量
    // incrementsPerThread 表示每个线程将发送多少个增加计数的消息
    const int numThreads = 10;
    const int incrementsPerThread = 10000; // 每个线程发送 10000 次增加操作

    // 创建存储线程对象的 vector
    std::vector<std::thread> threads;
    // 启动多个线程来并发发送消息给 Actor
    for (int i = 0; i < numThreads; ++i) {
        // emplace_back 在 vector 尾部直接构造一个 std::thread 对象
        // Lambda 表达式捕获 actor（引用捕获）和 incrementsPerThread（值捕获）
        threads.emplace_back([&actor, incrementsPerThread]() {
            // 每个线程循环发送 incrementsPerThread 次消息
            for (int j = 0; j < incrementsPerThread; ++j) {
                // 发送 Increment 类型的消息，回调函数传入 nullptr 表示不需要回复
                actor.send({ MessageType::Increment, nullptr });
            }
        });
    }

    // 等待所有启动的线程完成发送消息工作
    for (auto &t : threads) {
        t.join();  // 等待每个线程结束
    }

    // 使用简单的同步机制来获取最终计数结果
    int result = 0;                      // 用于存储获取的计数值
    std::mutex resultMutex;              // 互斥锁，用于保护对 result 和 gotResult 的访问
    std::condition_variable resultCV;    // 条件变量，用于等待结果返回
    bool gotResult = false;              // 标记是否已经收到计数器结果

    // 发送 GetCount 消息，请求获取当前计数值
    // Lambda 回调函数会在 Actor 内部处理完 GetCount 消息后被调用
    actor.send({ MessageType::GetCount, [&](int countValue) {
        {
            // 加锁后设置结果，防止并发问题
            std::lock_guard<std::mutex> lock(resultMutex);
            result = countValue;
            gotResult = true;  // 标记已获取结果
        }
        // 通知等待获取结果的线程
        resultCV.notify_one();
    }});

    // 等待 GetCount 消息的回调返回结果
    {
        std::unique_lock<std::mutex> lock(resultMutex);
        // 条件变量等待，直到 gotResult 为 true
        resultCV.wait(lock, [&] { return gotResult; });
    }

    // 输出最终计数值，理论上结果应为 numThreads * incrementsPerThread = 10 * 10000 = 100000
    std::cout << "最终计数值: " << result << std::endl;

    return 0;
}
