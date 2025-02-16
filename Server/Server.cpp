#include "stdTcpServer.h"
#include <unistd.h>
#include <sstream>
#include <vector>
#include <set>
#include "SharedTypes.h"
// #include "../path/to/StdTcpSocketPrivate.h"
#include "messageHandle.h"
#include "threadPool.h"
#include <thread>
#include "connManager.h"
#include <sys/resource.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
using namespace std;

void enableCoreDump() {
    struct rlimit limit;
    limit.rlim_cur = RLIM_INFINITY;
    limit.rlim_max = RLIM_INFINITY;
    if (setrlimit(RLIMIT_CORE, &limit) != 0) {
        perror("setrlimit failed");
    }
}

void *handleClientInfo(void *arg)
{
    (void)arg;
    /* 线程分离 */
    pthread_detach(pthread_self());

    auto *pClientInfo = static_cast<std::shared_ptr<StdTcpSocket> *>(arg);
    auto clientInfo = *pClientInfo;
    connManager::getInstance()->addConn(clientInfo);
    // 获取客户端IP和端口
    struct sockaddr_in addr;
    socklen_t addr_size = sizeof(addr);
    memset(&addr, 0, sizeof(addr));
    
    if (getpeername(clientInfo->getSockAttr()->connfd, (struct sockaddr *)&addr, &addr_size) < 0) {
        perror("getpeername failed");
        return NULL;
    }
    
    std::string clientIP = inet_ntoa(addr.sin_addr);
    int clientPort = ntohs(addr.sin_port);
    clientInfo->setClientIP(clientIP);
    clientInfo->setClientPort(clientPort);

    // // 等待客户端发送名字
    // std::string buffer;
    // int readBytes = clientInfo->recvMessage(buffer);
    // log("Client connected: " + clientIP + ":" + std::to_string(clientPort));
    // if (readBytes > 0) {
    //     json_object *jsonObj = json_tokener_parse(buffer.c_str());
    //     log("Received client name: " + std::string((json_object_get_string(json_object_object_get(jsonObj, "data")))));
    //     if (jsonObj != nullptr) {
    //         json_object *type_obj = json_object_object_get(jsonObj, "type");
    //         json_object *data_obj = json_object_object_get(jsonObj, "data");
            
    //         if (json_object_get_int(type_obj) == REQ_CHAT_LOGIN) {
    //             std::string clientName = json_object_get_string(data_obj);
    //             clientInfo->setClientInfo(clientName, clientIP, clientPort);
    //             log("New client connected: " + clientName + " (" + clientIP + ":" + std::to_string(clientPort) + ")");
                
    //             // 只有在成功接收到用户名后才添加到连接管理中
    //             connManager::getInstance()->addConn(clientInfo);
    //         }
    //         json_object_put(jsonObj);
    //     }
    // }
    int readBytes;
    std::string buffer;
    while (true) {
        // buffer.clear();
        readBytes = clientInfo->recvMessage(buffer);
        if (readBytes <= 0) {
            break;
        }
        
        // json_object *jsonObj = json_tokener_parse(buffer.c_str());
        // if (jsonObj != nullptr) {
            // json_object *type_obj = json_object_object_get(jsonObj, "type");
            // json_object *data_obj = json_object_object_get(jsonObj, "data");
            
            // if (json_object_get_int(type_obj) == REQ_ChatTXT) {
            //     std::string msg = json_object_get_string(data_obj);
            //     // 添加发送者信息到消息前面
            //     std::cout << clientInfo->getClientName() << "(" 
            //              << clientInfo->getClientIP() << "): " 
            //              << msg << std::endl;
            // }
            
            MessageHandle handles(clientInfo);
            handles.handleMessage(buffer);
            
            // json_object_put(jsonObj);
        // }
    }

    connManager::getInstance()->removeConn(clientInfo);
    log("Client disconnected: " + clientInfo->getClientName());
    return NULL;
}

void server_commands_help() {
    printf("\n=== 服务器命令 ===\n");
    printf("list - 显示在线用户列表\n");
    printf("msg <username> <message> - 向指定用户发送消息\n");
    printf("broadcast <message> - 向所有用户广播消息\n");
    printf("==================\n");
}

void *handleServerCommands(void *arg) {
    (void)arg;  // 添加这行来消除未使用参数的警告
    pthread_detach(pthread_self());
    
    // 显示帮助信息
    server_commands_help();

    while (true) {
        std::string input;
        std::getline(std::cin, input);
        
        if (input.empty()) {
            continue;
        }

        if (input == "list") {
            auto connections = connManager::getInstance()->getAllConns();
            std::cout << "\n=== Online Users ===\n";
            if (connections.empty()) {
                std::cout << "No online users.\n";
            } else {
                int i = 1;
                for (const auto &conn : connections) {
                    if (!conn->getClientName().empty()) {  // 只显示已经设置了名字的客户端
                        std::cout << i++ << ". " << conn->getClientName() 
                                << " (" << conn->getClientIP() << ":"
                                << conn->getClientPort() << ")\n";
                    }
                }
            }
            std::cout << "==================\n";
        }
        else if (input.substr(0, 4) == "msg ") {
            // 向指定用户发送消息
            std::string cmd = input.substr(4);
            size_t pos = cmd.find(" ");
            if (pos != std::string::npos) {
                std::string targetUser = cmd.substr(0, pos);
                std::string message = cmd.substr(pos + 1);
                
                bool userFound = false;
                auto connections = connManager::getInstance()->getAllConns();
                for (const auto &conn : connections) {
                    if (conn->getClientName() == targetUser) {
                        json_object* jsonObj = json_object_new_object();
                        json_object_object_add(jsonObj, "type", json_object_new_int(REQ_ChatTXT));
                        json_object_object_add(jsonObj, "data", json_object_new_string(("Server: " + message).c_str()));
                        const char* res = json_object_to_json_string(jsonObj);
                        conn->sendMessage(std::string(res));
                        json_object_put(jsonObj);
                        userFound = true;
                        std::cout << "消息已发送给 " << targetUser << std::endl;
                        break;
                    }
                }
                if (!userFound) {
                    std::cout << "未找到用户: " << targetUser << std::endl;
                }
            } else {
                std::cout << "使用方法: msg <username> <message>" << std::endl;
            }
        }
        else if (input.substr(0, 10) == "broadcast ") {
            // 广播消息给所有用户
            std::string message = input.substr(10);
            json_object* jsonObj = json_object_new_object();
            json_object_object_add(jsonObj, "type", json_object_new_int(REQ_ChatTXT));
            json_object_object_add(jsonObj, "data", json_object_new_string(("Server Broadcast: " + message).c_str()));
            const char* res = json_object_to_json_string(jsonObj);
            
            auto connections = connManager::getInstance()->getAllConns();
            for (const auto &conn : connections) {
                conn->sendMessage(std::string(res));
            }
            json_object_put(jsonObj);
            std::cout << "广播消息已发送" << std::endl;
        }
        else if (input == "help") {
            server_commands_help();
        }
        else {
            std::cout << "未知命令。可用命令：list, msg <username> <message>, broadcast <message>" << std::endl;
        }
    }
    return NULL;
}

int main()
{
    // 启用core dump
    enableCoreDump();
    
    /* 创建线程池对象 */
    ThreadPool pool(3, 8, 20);

    /* 创建服务器对象 */
    StdTcpServer server;

    /* 设置监听 */
    bool res = server.setListen(SERVER_PORT);
    if (res == false)
    {
        log("Server Listen ERROR...");
        _exit(-1);
    }

    log("QT Music Player Server Listening...");

    int ret = 0;
    // 创建服务器命令处理线程
    pthread_t commandThread;
    ret = pthread_create(&commandThread, NULL, handleServerCommands, NULL);
    if (ret != 0) {
        perror("command thread create error");
        _exit(-1);
    }

    while (1)
    {
        auto clientInfo = server.getClientSock();
        pool.addTask(handleClientInfo, new std::shared_ptr<StdTcpSocket>(clientInfo));
        /* 休眠 */
        sleep(5);
    }
}