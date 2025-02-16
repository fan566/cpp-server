#ifndef MYSQL_DATA_BASE_H
#define MYSQL_DATA_BASE_H

#include "stdDataBse.h"
#include <mysql/mysql.h>
#include <unistd.h>
#define MYSQL_PORT 3306

using namespace std;

class MysqlDataBase : public StdDataBase
{
public:
    /* 构造函数 */
    MysqlDataBase();

    /* 析构函数 */
    ~MysqlDataBase() override;

public:
    /* 连接数据库 */
    bool connectDB(const std::string &dbFileName) override;

    /* 执行SQL语句 */
    virtual bool execute(const std::string &sql) override;

    /* 查询SQl语句 */
    virtual VecResult query(const std::string &sql) override;

    /* 关闭数据库 */
    virtual void closeDB() override;

    MYSQL* get(){ return m_mysql;};

private:
    MYSQL *m_mysql;
};

#endif // MYSQL_DATA_BASE_H