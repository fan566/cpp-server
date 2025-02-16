using namespace std;
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <cstring>
#include <netinet/in.h>
#include <pthread.h>
#include "stdTcpServer.h"
/* 地址转成二进制头文件 */
#include <arpa/inet.h>


struct stdTcpServerPrivate
{
    /* 通信的文件描述符 */
    int sockfd;
    /* 是否正在监听 */
    bool m_isRunning;
};

StdTcpServer::StdTcpServer()
{
    m_tcpAttr = std::make_unique<stdTcpServerPrivate>();
    /* 监听套接字 */
    m_tcpAttr->sockfd = -1;
    /* 是否监听套接字 */
    m_tcpAttr->m_isRunning = false;
}

StdTcpServer::~StdTcpServer()
{
    /* 关闭套接字句柄 */
    if (m_tcpAttr->sockfd > 0)
    {
        close(m_tcpAttr->sockfd);
    }
}



bool StdTcpServer::setListen(int port)
{
    /* 维护内部端口 */
    this->m_port = port;

    /* 创建套接字 */
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1)
    {
        perror("socket error");
        throw std::runtime_error("socket create error.");
    }

    /* 设置套接字 */
    m_tcpAttr->sockfd = sockfd;

    /* 设置端口复用 */
    int optVal = 1;
    int ret = setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &optVal, sizeof(optVal));
    if (ret != 0)
    {
        perror("bind error");
        throw std::runtime_error("bind error.");
    }

    /* 将文件描述符和本地的IP与端口进行绑定 */
    struct sockaddr_in localAddress;
    memset(&localAddress, 0, sizeof(localAddress));
    /* 地址族 */
    localAddress.sin_family = AF_INET;
    /* 本地地址小端字节序，转换成网络大端字节序 */
    localAddress.sin_port = htons(SERVER_PORT);
    localAddress.sin_addr.s_addr = htonl(INADDR_ANY); /* 具体的ip地址 */
    /* 绑定 */
    ret = bind(sockfd, reinterpret_cast<const sockaddr *>(&localAddress), sizeof(localAddress));
    if (ret != 0)
    {
        perror("bind error");
        throw std::runtime_error("bind error.");
    }

    /* 给监听的套接字设置监听 */
    ret = listen(sockfd, 0);
    if (ret != 0)
    {
        perror("listen error");
        throw std::runtime_error("listen error.");
    }

    /* 改变监听状态 */
    m_tcpAttr->m_isRunning = true;

    return true;
}

std::shared_ptr<StdTcpSocket> StdTcpServer::getClientSock()
{
    int clientConnfd = accept(m_tcpAttr->sockfd, NULL, NULL);
    if (clientConnfd == -1)
    {
        perror("accept error");
        throw std::runtime_error("accept error.");
    }
    /* 程序到这个地方，就说明有客户端连接 */
    cout << "ClientConnfd: " << clientConnfd << endl;

    /* 通信类 */
    auto ptr = std::make_shared<StdTcpSocket>();

    /* 套接字 */
    ptr->getSockAttr()->connfd = clientConnfd;
    ptr->getSockAttr()->m_connected = true;

    return ptr;
}

/* 构造函数 */
StdTcpSocket::StdTcpSocket()
{
    m_sockAttr = std::make_unique<StdTcpSocketPrivate>();
    msgQueue = std::make_unique<MessageQueue>();
    /* 通信句柄 */
    m_sockAttr->connfd = -1;
    /* 通信是否建立 */
    m_sockAttr->m_connected = false;
}

StdTcpSocket::~StdTcpSocket()
{
    if (m_sockAttr->connfd > 0)
    {
        close(m_sockAttr->connfd);
    }
}

/* 连接服务器 */
int StdTcpSocket::connectToServer(const char *ip, int port)
{
    /* 创建套接字 */
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1)
    {
        perror("socket error");
        /* 抛异常 */
        throw std::runtime_error("socket create error.");
    }
    m_sockAttr->connfd = sockfd;

    /* 连接服务器 */
    struct sockaddr_in serverAddress;
    memset(&serverAddress, 0, sizeof(serverAddress));

    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(port);
    /* 地址转换成二进制 */
    inet_pton(AF_INET, ip, &serverAddress.sin_addr.s_addr);

    /* 开始连接 */
    int ret = connect(sockfd, reinterpret_cast<const sockaddr *>(&serverAddress), sizeof(serverAddress));
    if (ret != 0)
    {
        perror("connect error");
        return (-1);
    }

    m_sockAttr->m_connected = true;
    return 0;
}

/* 判断是否连接成功 */
bool StdTcpSocket::isConnected() const
{
    return m_sockAttr->m_connected;
}

int StdTcpSocket::sendMessage(const std::string &sendMsg)
{
    return sendMessage(sendMsg.c_str(), sendMsg.size());
}

/* 发送信息 */
int StdTcpSocket::sendMessage(const void *sendMsg, size_t n)
{
    // printf("sendMsg: %s\n", (char *)sendMsg);
    /* 1.先写包数据大小 */
    int writeBytes1 = write(m_sockAttr->connfd, (const void *)&n, sizeof(n));
    if (writeBytes1 < 0)
    {
        perror("write error");
        return -1;
    }

    /* 2.再写数据本身 */
    int writeBytes = write(m_sockAttr->connfd, sendMsg, n);
    if (writeBytes < 0)
    {
        perror("write error");
        return -1;
    }
    return writeBytes;
}

int StdTcpSocket::recvMessage(std::string &recvMsg)
{
    size_t size = 0;

    ssize_t readBytes = read(m_sockAttr->connfd, &size, sizeof(size));
    if (readBytes < 0)
    {
        perror("read error");
        cerr << "file: (" << __FILE__ << ")\tLine: (" << __LINE__ << ")" << endl;
        return -1;
    }
    else if (readBytes == 0)
    {
        return 0;
    }

    char *msg = new char[size + 1];
    /* 清除脏数据 */
    memset(msg, 0, sizeof(char) * (size + 1));

    /* 处理流数据 */
    size_t totalReceived = 0;
    while (totalReceived < size)
    {
        size_t recvBytes = read(m_sockAttr->connfd, msg + totalReceived, size - totalReceived);
        if (recvBytes <= 0)
        {
            perror("recvBytes error");
            delete[] msg;
            return -1;
        }
        totalReceived += recvBytes;
    }

    recvMsg = msg;
    delete[] msg;

    return totalReceived;
}

/* 接受信息 */
int StdTcpSocket::recvMessage(void *buf, size_t n)
{
    int readBytes = read(m_sockAttr->connfd, buf, n);
    return readBytes;
}

StdTcpSocketPrivate *StdTcpSocket::getSockAttr()
{
    return m_sockAttr.get();
}

void StdTcpSocket::setClientInfo(const std::string& name, const std::string& ip, int port) {
    m_sockAttr->clientName = name;
    m_sockAttr->clientIP = ip;
    m_sockAttr->clientPort = port;
}

std::string StdTcpSocket::getClientName() const {
    return m_sockAttr->clientName;
}

std::string StdTcpSocket::getClientIP() const {
    return m_sockAttr->clientIP;
}

int StdTcpSocket::getClientPort() const {
    return m_sockAttr->clientPort;
}

void StdTcpSocket::setClientName(const std::string &name)
{
    m_sockAttr->clientName = name;
}

void StdTcpSocket::setClientIP(const std::string &ip)
{
    m_sockAttr->clientIP = ip;
}

void StdTcpSocket::setClientPort(int port)
{
    m_sockAttr->clientPort = port;
}