#ifndef CONN_MANAGER_H
#define CONN_MANAGER_H

#include <set>
#include <unistd.h>
#include <pthread.h>
#include "stdTcpServer.h"

class connManager
{
    std::set<std::shared_ptr<StdTcpSocket>> connfds;
    pthread_mutex_t mutex;
    
    // 使用指针声明静态实例
    static connManager* instance;

    // 私有构造函数和析构函数
    connManager();
    ~connManager();

public:
    // 禁止拷贝构造和赋值操作
    connManager(const connManager&) = delete;
    connManager& operator=(const connManager&) = delete;

    // 获取单例实例的静态方法
    static connManager* getInstance();

    void addConn(std::shared_ptr<StdTcpSocket> connfd);
    void removeConn(std::shared_ptr<StdTcpSocket> connfd);
    // void closeAllConn();
    std::set<std::shared_ptr<StdTcpSocket>> getAllConns();
};

#endif // CONN_MANAGER_H



