#ifndef MESSAGE_PROCESSOR_H
#define MESSAGE_PROCESSOR_H

#include "stdTcpServer.h"
#include "connManager.h"
#include <thread>
#include <atomic>

class MessageProcessor {
public:
    MessageProcessor(std::shared_ptr<StdTcpSocket> client) 
        : client(client), running(true) {
        processorThread = std::thread(&MessageProcessor::processMessages, this);
    }

    ~MessageProcessor() {
        running = false;
        if (processorThread.joinable()) {
            processorThread.join();
        }
    }

private:
    void processMessages() {
        while (running) {
            ChatMessage msg;
            if (client->getMessageQueue()->wait_and_pop(msg)) {
                // 构建JSON消息
                json_object* jsonObj = json_object_new_object();
                json_object_object_add(jsonObj, "type", json_object_new_int(msg.type));
                json_object_object_add(jsonObj, "from", json_object_new_string(msg.fromUser.c_str()));
                json_object_object_add(jsonObj, "content", json_object_new_string(msg.content.c_str()));
                
                const char* jsonStr = json_object_to_json_string(jsonObj);
                client->sendMessage(std::string(jsonStr));
                
                json_object_put(jsonObj);
            }
        }
    }

    std::shared_ptr<StdTcpSocket> client;
    std::thread processorThread;
    std::atomic<bool> running;
};

#endif // MESSAGE_PROCESSOR_H 