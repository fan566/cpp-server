#ifndef MESSAGE_QUEUE_H
#define MESSAGE_QUEUE_H

#include <queue>
#include <mutex>
#include <condition_variable>
#include <string>

struct ChatMessage {
    std::string fromUser;  // 发送者
    std::string toUser;    // 接收者
    std::string content;   // 消息内容
    int type;             // 消息类型
};

class MessageQueue {
public:
    void push(const ChatMessage& msg) {
        std::unique_lock<std::mutex> lock(mtx);
        queue.push(msg);
        cv.notify_one();
    }

    bool pop(ChatMessage& msg) {
        std::unique_lock<std::mutex> lock(mtx);
        if (queue.empty()) {
            return false;
        }
        msg = queue.front();
        queue.pop();
        return true;
    }

    bool wait_and_pop(ChatMessage& msg) {
        std::unique_lock<std::mutex> lock(mtx);
        cv.wait(lock, [this] { return !queue.empty(); });
        msg = queue.front();
        queue.pop();
        return true;
    }

private:
    std::queue<ChatMessage> queue;
    mutable std::mutex mtx;
    std::condition_variable cv;
};

#endif // MESSAGE_QUEUE_H 