#ifndef MESSAGE_HANDLE_H
#define MESSAGE_HANDLE_H
#include <unordered_map>
#include <functional>
#include "SharedTypes.h"
#include "function.h"
#include <json-c/json.h>

/* 函数注册表 */
class MessageHandle
{
public:
    using HandleFunction = std::function<void(const std::string &)>;
    /* 构造函数 */
    MessageHandle(std::shared_ptr<StdTcpSocket> &clientInfo);
    /* 析构函数 */
    ~MessageHandle();

public:
    /* 处理信息 */
    void handleMessage(const std::string &msg);

private:
    Function m_function;
    /* 映射 */
    std::unordered_map<int, HandleFunction> m_handles;
};

#endif // MESSAGE_HANDLE_H