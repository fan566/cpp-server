#ifndef FUNCTION_H
#define FUNCTION_H_

#include <string>
#include <cstring>
#include <json-c/json.h>
#include <fstream>
#include "stdTcpServer.h"
#include "SharedTypes.h"
#include "sqliteDataBase.h"
#include "mysqlDataBase.h"
#include "base64.h"
#include <utility> /* std::pair */

using namespace std;

class Function
{
public:
    /* 构造函数 */
    Function(std::shared_ptr<StdTcpSocket> &clientInfo);
    /* 析构函数 */

    ~Function() = default;

private:
#if 0
    /* 判断用户名是否存在 */
    std::pair<bool, int> isUserExist(const char *username);

    /* 保存用户信息 */
    bool saveUserInfo(const char *username, const char *password);

    /* 判断用户名和密码是否匹配 */
    bool isUserPasswordMatch(const char *username, const char *password);

    /* 用户是否已经在线 */
    bool isUserOnline(const char *username);

    /* 读取本地服务器文件夹路径 */
    std::string readMusicFile(const std::string &filePath);

    /* 打包音乐文件成json对象 */
    void convertMusicToJson(json_object *resObj, const char *musicName);

    /* 读取用户收藏列表信息 */
    bool fetchUserFavorites(json_object *resObj, int userId);
#endif 




public:
#if 0
    /* 处理注册 */
    void handleRegister(const std::string &msg);

    /* 处理登录 */
    void handleLogin(const std::string &msg);

    /* 处理添加好友 */
    void handleAddFriend(const std::string &msg);

    /* 处理删除好友 */
    void handleDeleteFriend(const std::string &msg);

    /* 处理创建群组 */
    void handleCreateGroup(const std::string &msg);

    /* 处理退出群组 */
    void handleExitGroup(const std::string &msg);

    /* 处理播放在线音乐 */
    void handleOnlineMusicInfo(const std::string &msg);

    /* 处理更新播放历史请求 */
    void handleUpdatePlayHistoryRequest(const std::string &msg);

    /* 保存用户播放历史记录 */
    void storeUserPlayHistory(const std::string &msg);

    /* 处理更新在线音乐列表请求 */
    void handleRefreshMusicLibRequest();

    /* 处理单个歌曲收藏请求的函数*/
    void handleFavoriteSongRequest(const std::string &msg);

    /* 处理收藏列表更新的请求 */
    void handleFavoriteListUpdateRequest(const std::string &msg);

    /* 处理用户初始化收藏信息的请求 */
    void handleInitializeUserFavoritesRequest(const std::string &msg);
#endif
/* 请求个单列表*/
void handleReqSonglist(const string& msg);
/* 请求在线歌曲 */
void handleReqSongListen(const string& msg);
/* 请求存储我的音乐 */
void handleReqStoreLoveMusic(const std::string& msg);
/* 登录 */
void handleLogin(const std::string &msg);
/* 注册 */
void handleRegister(const std::string &msg);
/* 更新头像 */
void handleUpdateHeadingImage( const std::string &msg );

User isUserExist(const char *username);

bool saveUserInfo(const char *username, const char *password);

void handleGetHeadingImage(const std::string &msg);
/* 错误 回传 echo*/
void handleERROR(const std::string &msg);

//chatroom--------------------------------
/* 文本聊天 */
void handleChatTXT(const std::string &msg);

/* 图片传输 */
void handleTRANSPHOTO(const std::string &msg);

/* 添加连接属性*/
void handleAddClientName(const std::string &msg);

private:
    /* 通信类对象 */
    std::shared_ptr<StdTcpSocket> & m_clientInfo;
    /* Sqlite3数据库 */
    SqliteDataBase m_sqliteDatabase;
    /* Mysql数据库 */
    std::shared_ptr<StdDataBase> m_mysqlDatabase;
};

#endif // FUNCTION_H