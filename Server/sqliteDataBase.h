#ifndef SQLITE_DATA_BASE_H
#define SQLITE_DATA_BASE_H

#include "stdDataBse.h"
#include <sqlite3.h>
#include <iostream>
#include <unistd.h>

using namespace std;

class SqliteDataBase : public StdDataBase
{
public:
    /* 构造函数 */
    SqliteDataBase();

    /* 析构函数 */
    ~SqliteDataBase() override;

public:
    /* 连接数据库 */
    bool connectDB(const std::string &dbFileName) override;

    /* 执行SQL语句 */
    bool execute(const std::string &sql) override;

    /* 查询SQl语句 */
    VecResult query(const std::string &sql) override;

    /* 关闭数据库 */
    void closeDB() override;

private:
    sqlite3 *m_sqlite;
};

#endif // SQLITE_DATA_BASE_H