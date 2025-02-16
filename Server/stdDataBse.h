#ifndef STD_DATA_BASE_H
#define STD_DATA_BASE_H

#include <string>
#include <vector>
#include <iostream>
#include <ctime>
#include <iomanip>

using VecResult = std::vector<std::vector<std::string>>;

/* 服务器日志信息规范打印处理函数 */
inline std::string getCurrentTime()
{
    auto t = std::time(nullptr);
    auto tm = *std::localtime(&t);
    std::ostringstream oss;
    oss << std::put_time(&tm, "%Y-%m-%d %H:%M:%S");
    return oss.str();
}

inline void log(const std::string &message, const std::string &level = "INFO")
{
    std::cout << "[" << getCurrentTime() << "] [" << level << "] " << message << std::endl;
}

class StdDataBase
{

public:
    virtual ~StdDataBase() = default;

public:
    /* 连接数据库 */
    virtual bool connectDB(const std::string &dbFileName) = 0;

    /* 执行SQL语句 */
    virtual bool execute(const std::string &sql) = 0;

    /* 查询SQl语句 */
    virtual VecResult query(const std::string &sql) = 0;

    /* 关闭数据库 */
    virtual void closeDB() = 0;

    
};

#endif // STD_DATA_BASE_H