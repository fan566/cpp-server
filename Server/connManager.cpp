#include <set>
#include <unistd.h>
#include <pthread.h>
#include "connManager.h"

// 在cpp文件中定义静态成员变量
connManager* connManager::instance = nullptr;

connManager::connManager()
{
    pthread_mutex_init(&mutex, nullptr);
}

connManager::~connManager()
{
    pthread_mutex_destroy(&mutex);
}

void connManager::addConn(std::shared_ptr<StdTcpSocket> connfd)
{
    pthread_mutex_lock(&mutex);
    this->connfds.insert(connfd);
    pthread_mutex_unlock(&mutex);
}

void connManager::removeConn(std::shared_ptr<StdTcpSocket> connfd)
{
    pthread_mutex_lock(&mutex);
    this->connfds.erase(connfd);
    pthread_mutex_unlock(&mutex);
}

// void connManager::closeAllConn()
// {
//     pthread_mutex_lock(&mutex);
//     for (auto connfd : connfd)
//     {
//         close(connfd->);
//     }
//     pthread_mutex_unlock(&mutex);
// }

// 获取单例实例的静态方法实现
connManager* connManager::getInstance() {
    if (instance == nullptr) {
        instance = new connManager();
    }
    return instance;
}

std::set<std::shared_ptr<StdTcpSocket>> connManager::getAllConns() {
    // Implementation to return all connection IDs
    
    // Populate conns with all connection IDs
    return connfds;
}