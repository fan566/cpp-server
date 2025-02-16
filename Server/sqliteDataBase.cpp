#include "sqliteDataBase.h"

SqliteDataBase::SqliteDataBase()
{
    m_sqlite = nullptr;
}

SqliteDataBase::~SqliteDataBase()
{
    if (m_sqlite)
    {
        sqlite3_close(m_sqlite);
    }
}

bool SqliteDataBase::connectDB(const std::string &dbFileName)
{
    /* 连接数据库 */
    int ret = sqlite3_open(dbFileName.c_str(), &m_sqlite);
    if (ret != SQLITE_OK)
    {
        cout << " SQL open error: " << sqlite3_errmsg(m_sqlite) << endl;
        return false;
    }

    return true;
}

bool SqliteDataBase::execute(const std::string &sql)
{
    int rc = sqlite3_exec(m_sqlite, sql.c_str(), nullptr, nullptr, NULL);
    if (rc != SQLITE_OK)
    {
        std::cerr << "SQL error: " << sqlite3_errmsg(m_sqlite) << std::endl;
        return false;
    }
    return true;
}

VecResult SqliteDataBase::query(const std::string &sql)
{
    VecResult result;

    int pnRow = 0, pnColumn = 0;
    char **pazResult = nullptr;
    int ret = sqlite3_get_table(m_sqlite, sql.c_str(), &pazResult, &pnRow, &pnColumn, NULL);
    if (ret != SQLITE_OK)
    {
        cout << "sqlite3 open error: " << sqlite3_errmsg(m_sqlite) << endl;
        return result;
    }

    for (int idx = 1; idx <= pnRow; idx++)
    {
        vector<string> rowData;

        for (int jdx = 0; jdx < pnColumn; jdx++)
        {
            rowData.push_back(pazResult[idx * pnColumn + jdx]);
        }
        result.push_back(rowData);
    }

    /* 释放内存 */
    sqlite3_free_table(pazResult);

    return result;
}

void SqliteDataBase::closeDB()
{
    if (m_sqlite)
    {
        sqlite3_close(m_sqlite);
        m_sqlite = nullptr;
    }
}
