#include "mysqlDataBase.h"

MysqlDataBase::MysqlDataBase()
{
    m_mysql = nullptr;
}

MysqlDataBase::~MysqlDataBase()
{
    if (m_mysql)
    {
        mysql_close(m_mysql);
        m_mysql = nullptr;
    }
}

bool MysqlDataBase::connectDB(const std::string &dbFileName)
{
    /* 初始化数据库 */
    m_mysql = mysql_init(nullptr);
    if (m_mysql == nullptr)
    {
        log("初始化 MySQL 数据库失败...");
        return false;
    }

    log("MySQL 初始化成功...");

    /* 连接数据库 */
    const char *host = "localhost";
    const char *user = "root";
    const char *passwd = "1";

    if (mysql_real_connect(m_mysql, host, user, passwd, dbFileName.c_str(), MYSQL_PORT, NULL, 0) == nullptr)
    {
        cout << "File: " << __FILE__ << " Line:" << __LINE__ << " connect mySQL server error! " << mysql_error(m_mysql) << endl;
        return false;
    }
    else
    {
        log("数据库 'chatRoom' 连接成功");
    }

    return true;
}

bool MysqlDataBase::execute(const std::string &sql)
{
    /* 查询语句 */
    int ret = mysql_query(m_mysql, sql.c_str());
    if (ret != 0)
    {
        cout << "数据库sql语句执行失败: " << mysql_error(m_mysql) << endl;
        return false;
    }

    return true;
}

VecResult MysqlDataBase::query(const std::string &sql)
{
    VecResult resultString;
    /* 查询语句 */
    if (execute(sql) == false)
    {
        return resultString;
    }
    else
    {
        /* 结果集合 */
        MYSQL_RES *result = mysql_store_result(m_mysql);
        if (result == nullptr)
        {
            cout << "mysql_store_result error: " << mysql_error(m_mysql) << endl;
            return resultString;
        }

        /* 获取结果集合的列数 */
        unsigned int columns = mysql_num_fields(result);

        /* 获取数据 */
        MYSQL_ROW row;
        while ((row = mysql_fetch_row(result)) != nullptr)
        {
            vector<string> rowData;
            /* 每个row变量表示一行 */
            for (unsigned int col = 0; col < columns; col++)
            {
                rowData.push_back(row[col] ? row[col] : ""); /* 防止空指针 */
            }
            resultString.push_back(rowData);
        }

        /* 释放结果集 */
        mysql_free_result(result);
    }
    return resultString;
}

void MysqlDataBase::closeDB()
{
    /* 关闭数据库 */
    if (m_mysql)
    {
        mysql_close(m_mysql);
        m_mysql = nullptr;
    }
}
