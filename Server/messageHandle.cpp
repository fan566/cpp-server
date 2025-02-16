#include "messageHandle.h"

/* 构造函数 */
MessageHandle::MessageHandle(std::shared_ptr<StdTcpSocket> &clientInfo) : m_function(clientInfo)
{
    m_handles[REQ_SONGLIST] = [this](const std::string &msg)
    { m_function.handleReqSonglist(msg);};

    m_handles[REQ_SONGLISTEN]=[this](const std::string &msg)
    { m_function.handleReqSongListen(msg);};

    m_handles[REQ_STORLOVEMUSIC]=[this](const std::string &msg)
    { m_function.handleReqStoreLoveMusic(msg);};

    m_handles[REQ_LOGIN]=[this](const std::string &msg)
    { m_function.handleLogin(msg);};

    m_handles[REQ_REGIS]=[this](const std::string &msg)
    { m_function.handleRegister(msg);};

    m_handles[REQ_REQ_SONGLIST]=[this](const std::string &msg)
    { m_function.handleUpdateHeadingImage(msg);};

    m_handles[REQ_DOWNLOAD]=[this](const std::string &msg)
    { m_function.handleReqSongListen(msg);};

    m_handles[REQ_GETHEADINGIMAGE]=[this](const std::string &msg)
    { m_function.handleGetHeadingImage(msg);};

    m_handles[REQ_LOGIN]=[this](const std::string &msg)
    { m_function.handleLogin(msg);};

    m_handles[REQ_REGIS]=[this](const std::string &msg)
    { m_function.handleRegister(msg);};

    m_handles[REQ_ERROR]=[this](const std::string &msg)
    { m_function.handleERROR(msg);};

    m_handles[REQ_ChatTXT]=[this](const std::string &msg)
    { m_function.handleChatTXT(msg);};

    // m_handles[REQ_TRANSPHOTO]=[this](const std::string &msg)
    // { m_function.handleTRANSPHOTO(msg);};
    m_handles[REQ_FILE_START]=[this](const std::string &msg)
    { m_function.handleTRANSPHOTO(msg);};

    m_handles[REQ_FILE_DATA]=[this](const std::string &msg)
    { m_function.handleTRANSPHOTO(msg);};

    m_handles[REQ_FILE_END]=[this](const std::string &msg)
    { m_function.handleTRANSPHOTO(msg);};
    
    m_handles[REQ_CHAT_LOGIN]=[this](const std::string &msg)
    { m_function.handleAddClientName(msg);};
    
}

/* 析构函数 */
MessageHandle::~MessageHandle() = default;

void MessageHandle::handleMessage(const std::string &msg)
{
    /* msg是json字符串 */
    /* 1.将json字符串转成json对象 */

    json_object *jsonObj = json_tokener_parse(msg.c_str());
    int type = 0;
    if (jsonObj != nullptr)
    {
        /* 2.根据key得到的value */
        type = json_object_get_int(json_object_object_get(jsonObj, "type"));
        json_object_put(jsonObj);

        // log("正在处理 type: "+std::to_string(type));
    }
    else
    {
        cout << "parse error:"+msg << endl;
        m_handles[REQ_ERROR](msg);
        return;
    }
    auto iter = m_handles.find(type);
    if (iter != m_handles.end())
    {
        /* 执行回调函数 */
        iter->second(msg);
    }
    else
    {
        /* 处理未知消息的类型 */
        std::cout << "unknown message type." << std::endl;
        m_handles[REQ_ERROR](msg);
    }
}