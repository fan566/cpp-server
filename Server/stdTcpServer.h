#ifndef STD_TCP_SERVER_H
#define STD_TCP_SERVER_H

#include <memory>
#include <iostream>
#include <string>
#define SERVER_PORT 8081
#define BUFFER_SIZE 1024

struct StdTcpSocketPrivate;
struct stdTcpServerPrivate;

/* 通信类 */
class StdTcpSocket
{
public:
    /* 构造函数 */
    StdTcpSocket();
    /* 析构函数 */
    ~StdTcpSocket();

public:
    /* 连接服务器 */
    int connectToServer(const char *ip, int port);
    /* 是否连接成功 */
    bool isConnected() const;
    /* 发送信息 */
    int sendMessage(const std::string &sendMsg);
    /* 发送信息2 */
    int sendMessage(const void *sendMsg, size_t n);

    /* 接受消息 */
    int recvMessage(std::string &recvMsg);
    /* 接受消息2 */
    int recvMessage(void *buf, size_t n);

    /* 得到属性 */
    StdTcpSocketPrivate* getSockAttr();

    /* 设置客户端信息 */
    void setClientInfo(const std::string& name, const std::string& ip, int port);
    /* 获取客户端信息 */
    std::string getClientName() const;
    std::string getClientIP() const;
    int getClientPort() const;

    void setClientName(const std::string& name);
    void setClientIP(const std::string& ip);
    void setClientPort(int port);

    MessageQueue* getMessageQueue() { return msgQueue.get(); }

private:
    /* 连接的uuid  */
    int m_uuid;

    /* 套接字属性  */
    std::unique_ptr<StdTcpSocketPrivate> m_sockAttr;
    // StdTcpSocketPrivate *m_sockAttr;

    std::unique_ptr<MessageQueue> msgQueue;
};

class StdTcpServer
{
public:
    /* 构造函数 */
    StdTcpServer();

    /* 析构函数 */
    ~StdTcpServer();

public:
    /* 设置监听 */
    bool setListen(int port);

    /* 接受连接 */
    std::shared_ptr<StdTcpSocket> getClientSock();

private:
    /* 端口 */
    int m_port;
    /* 服务器的属性 */
    std::unique_ptr<stdTcpServerPrivate> m_tcpAttr;
};
struct StdTcpSocketPrivate
{
    /* 通信是否连接成功 */
    bool m_connected;
    /* 通信的文件描述符 */
    int connfd;
    /* 客户端信息 */
    std::string clientName;
    std::string clientIP;
    int clientPort;
};

#endif  // STD_TCP_SERVER_H