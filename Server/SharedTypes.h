#ifndef SHARED_TYPES_H
#define SHARED_TYPES_H
#include <string>
using namespace std;
struct User
{
    int id;
    string account;
    string password;
    string heading_image;
};

enum FunctionOption
{
    
    FUNC_EXIT,                            /* 退出 */
    FUNC_ADD_FRIEND,                      /* 添加好友 */
    FUNC_DEL_FRIEND,                      /* 删除好友 */
    FUNC_NEW_GROUP,                       /* 建群 */
    FUNC_EXIT_GROUP,                      /* 退群 */
    FUNC_SEARCH_MUSIC,                    /* 搜索服务器音乐 */
    FUNC_ONLINE_MUSIC,                    /* 处理在线音乐 */
    FUNC_UPDATE_PLAY_HISTORY,             /* 处理更新播放历史 */
    FUNC_LOAD_ONLINE_HISTORY,             /* 加载在线播放历史 */
    FUNC_SAVE_PLAY_HISTORY,               /* 存储播放历史 */
    FUNC_REFRESH_MUSIC_LIBRARY,           /* 更新在线音乐列表 */
    FUNC_UPDATE_FAVORITE_STATUS_FOR_SONG, /* 更新单个音乐的收藏信息 */
    FUNC_UPDATE_FAVORITE_LIST,            /*  更新收藏音乐列表 */
};
enum REQ
{
    REQ_SONGLIST,
    REQ_SONGLISTEN,
    REQ_STORLOVEMUSIC,
    REQ_LOGIN,
    REQ_REGIS,
    REQ_REQ_SONGLIST,
    REQ_DOWNLOAD,
    REQ_GETHEADINGIMAGE,
    REQ_ERROR,
    REQ_ChatTXT = 100,
    REQ_TRANSPHOTO,
    REQ_FILE_START,
    REQ_FILE_DATA,
    REQ_FILE_END,
    REQ_CHAT_LOGIN

};

enum NOTIFY
{
    NOTIFY_HEADING_IMAGE_UPDATE
};


enum ReplyCode
{
    REPLY_REGISTER_SUCCESS,    /* 注册成功 */
    REPLY_REGISTER_USER_EXIST, /* 用户已存在 */
    REPLY_LOGIN_SUCCESS,       /* 登录成功 */
    REPLY_LOGIN_PASSWD_ERROR,  /* 密码错误 */
    REPLY_LOGIN_NO_USER        /* 用户不存在 */
};

struct Message
{
    FunctionOption type;
    char name[20];
    char passwd[20];
    char targetName[20];
};

struct ReplyMsg
{
    FunctionOption type;  /* 操作类型 */
    ReplyCode statusCode; /* 回复的状态码 */
};




#endif // SHARED_TYPES_H