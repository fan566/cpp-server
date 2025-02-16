#include "function.h"
#include "base64.h"
#include <set>
#include <mysql/mysql.h>
#include <sys/stat.h>  // mkdir
#include <errno.h>   // errno
#define REQ_FILE_START  102   // 文件传输开始
#define REQ_FILE_DATA  103    // 文件数据块
#define REQ_FILE_END  104     // 文件传输结束
Function::Function(std::shared_ptr<StdTcpSocket> &clientInfo) : m_clientInfo(clientInfo), m_mysqlDatabase(std::make_shared<MysqlDataBase>())
{
    // m_sqliteDatabase.connectDB("musicPlayer");
    m_mysqlDatabase->connectDB("musicplayer");

    unsigned long max_allowed_packet = 16*1024*1024; // 例如设置为16MB
    mysql_options(dynamic_cast<MysqlDataBase*>(m_mysqlDatabase.get())->get(), MYSQL_OPT_MAX_ALLOWED_PACKET, &max_allowed_packet);
}
// 文本聊天
void Function::handleChatTXT(const std::string &msg){
    json_object* jsonObject = json_tokener_parse(msg.c_str());
    const char* data = json_object_get_string(json_object_object_get(jsonObject,"data"));
    printf("收到消息:[ip:%s:%d:%s]%s\n",m_clientInfo->getClientIP().c_str(),m_clientInfo->getClientPort(),m_clientInfo->getClientName().c_str(),data);
    json_object_put(jsonObject);
    
}
// 图片传输
void Function::handleTRANSPHOTO(const std::string &msg) {
    static FILE *current_fp = nullptr;
    static std::string current_filename;
    
    json_object *jsonObj = json_tokener_parse(msg.c_str());
    if (!jsonObj) {
        log("Invalid JSON message");
        return;
    }

    json_object *type_obj;
    if (!json_object_object_get_ex(jsonObj, "type", &type_obj)) {
        json_object_put(jsonObj);
        return;
    }

    int type = json_object_get_int(type_obj);
    log("Received message type: " + std::to_string(type));

    switch (type) {
        case REQ_FILE_START: {
            // 如果有未关闭的文件，先关闭
            if (current_fp) {
                fclose(current_fp);
                current_fp = nullptr;
            }

            // 创建接收目录（如果不存在）
            mkdir("../recv_photo", 0777);

            json_object *data_obj;
            if (!json_object_object_get_ex(jsonObj, "data", &data_obj)) {
                log("No data field in start message");
                break;
            }

            // 解析文件信息JSON
            json_object *start_info = json_tokener_parse(json_object_get_string(data_obj));
            if (!start_info) {
                log("Invalid start info JSON");
                break;
            }

            json_object *filename_obj, *size_obj;
            if (!json_object_object_get_ex(start_info, "filename", &filename_obj) ||
                !json_object_object_get_ex(start_info, "total_size", &size_obj)) {
                log("Missing filename or size in start info");
                json_object_put(start_info);
                break;
            }

            const char *filename = json_object_get_string(filename_obj);
            // 获取文件名（不包含路径）
            const char *base_name = strrchr(filename, '/');
            base_name = base_name ? base_name + 1 : filename;

            // 构造带时间戳的文件名
            time_t now = time(0);
            tm *ltm = localtime(&now);
            char new_filename[256];
            snprintf(new_filename, sizeof(new_filename), 
                    "../recv_photo/%d.%d.%d-%d:%d:%d-%s",
                    1900 + ltm->tm_year, 1 + ltm->tm_mon, ltm->tm_mday,
                    ltm->tm_hour, ltm->tm_min, ltm->tm_sec, base_name);
            
            current_fp = fopen(new_filename, "wb");
            if (!current_fp) {
                log("Failed to create file: " + std::string(new_filename));
                json_object_put(start_info);
                break;
            }
            current_filename = new_filename;
            log("Started receiving file: " + current_filename);
            json_object_put(start_info);
            break;
        }
        case REQ_FILE_DATA: {
            if (!current_fp) {
                log("No file is currently open");
                break;
            }

            json_object *data_obj;
            if (json_object_object_get_ex(jsonObj, "data", &data_obj)) {
                const char *base64_data = json_object_get_string(data_obj);
                try {
                    std::string decoded_data = base64_decode(std::string(base64_data));
                    size_t written = fwrite(decoded_data.data(), 1, decoded_data.size(), current_fp);
                    if (written != decoded_data.size()) {
                        log("Failed to write all data: " + std::to_string(written) + "/" + 
                            std::to_string(decoded_data.size()));
                    }
                    fflush(current_fp);  // 确保数据写入磁盘
                } catch (const std::exception& e) {
                    log("Error decoding data: " + std::string(e.what()));
                }
            }
            break;
        }
        case REQ_FILE_END: {
            if (current_fp) {
                fclose(current_fp);
                current_fp = nullptr;
                log("Completed receiving file: " + current_filename);
            }
            break;
        }
        default:
            log("Unknown message type: " + std::to_string(type));
            break;
    }

    json_object_put(jsonObj);
}
void Function::handleAddClientName(const std::string &msg)
{
    json_object* jsonObject = json_tokener_parse(msg.c_str());
    const char* data = json_object_get_string(json_object_object_get(jsonObject,"data"));
    printf("收到消息:[ip:%s:%d:%s]%s\n",m_clientInfo->getClientIP().c_str(),m_clientInfo->getClientPort(),m_clientInfo->getClientName().c_str(),data);
    m_clientInfo->setClientName(data);
    
    json_object_put(jsonObject);
}
void Function::handleERROR(const std::string &msg)
{
    m_clientInfo->sendMessage(msg);
}

User Function::isUserExist(const char *username)
{
    std::string sql = "SELECT id ,account ,password,heading_image FROM user where account = '%s';";
    char requestSql[128] = {0};
    sprintf(requestSql, sql.c_str(), username);

    log("SQL: " + std::string(requestSql));

    if (!m_mysqlDatabase->execute(requestSql))
    {
        log("m_mysqlDatabase->execute查询失败");
        return User();
    }

    MYSQL_RES *result = mysql_store_result(dynamic_cast<MysqlDataBase*>(m_mysqlDatabase.get())->get());
    if (result == nullptr)
    {
        std::cout << "mysql_store_result error: " << mysql_error(dynamic_cast<MysqlDataBase*>(m_mysqlDatabase.get())->get()) << endl;
        return User() ;
    }
    /* 获取结果集合的列数 */
    // unsigned int columns = mysql_num_fields(result);
    /* 获取数据 */
    MYSQL_ROW row;
    if((row = mysql_fetch_row(result)) == nullptr)
    {
        log(" mysql_fetch_row(result)): 查询失败");
        return User() ;
    }

    unsigned long *lengths = mysql_fetch_lengths(result); // 获取每列数据的长度

    // 将heading_image（BLOB）转换为Base64编码
    std::string headingImageBase64 = base64_encode(reinterpret_cast<const unsigned char*>(row[3]), lengths[3]);

    // log("heading_image Base64 encoded: " + headingImageBase64);

    User user = {stoi(row[0]), row[1], row[2], headingImageBase64}; // 将Base64字符串存入User
    return user;

# if 0  
    std::string sql = "SELECT id ,account ,password,heading_image FROM user where account = '%s';";
    char requestSql[128] = {0};
    sprintf(requestSql, sql.c_str(), username);

    log("SQL: " + std::string(requestSql));

    VecResult res = m_mysqlDatabase->query(requestSql);
    if (res.empty() || res[0].empty())
    {
        return User();
    }
    log("res[0][3]"+res[0][3]);
    User user = {stoi(res[0][0]),res[0][1],res[0][2],res[0][3]};
    return user;
# endif
}

void Function::handleLogin(const std::string &msg)
{
    log("开始处理用户登录信息...");

    const char *username = nullptr;
    const char *passwd = nullptr;
    json_object *jsonObj = json_tokener_parse(msg.c_str());
    if (jsonObj)
    {
        username = json_object_get_string(json_object_object_get(jsonObj, "acccount"));
        passwd = json_object_get_string(json_object_object_get(jsonObj, "password"));
    } 

    /* 构建json字符串 */
    /* 1. 创建json对象 */
    json_object *resJsonObj = json_object_new_object();
    /* 2. 设置 <key, value> */
    json_object_object_add(resJsonObj, "type", json_object_new_int(REQ_LOGIN));

    /* 判断用户是否已经注册 */
    User user = isUserExist(username);
    if (user.id == 0)
    {
        /* 程序进入，说明用户不存在 */
        json_object_object_add(resJsonObj, "result", json_object_new_string("failed"));
        json_object_object_add(resJsonObj, "reason", json_object_new_string("用户不存在"));
    }
    else
    {
        /* 如果用户存在，判断用户名和密码是否匹配 */
        if (passwd != user.password)
        {
            json_object_object_add(resJsonObj, "result", json_object_new_string("failed"));
            json_object_object_add(resJsonObj, "reason", json_object_new_string("密码错误"));
        }
        else
        {
            json_object_object_add(resJsonObj, "result", json_object_new_string("success"));
            json_object_object_add(resJsonObj, "user_id", json_object_new_int(user.id));
            json_object_object_add(resJsonObj, "account", json_object_new_string(user.account.c_str()));
            // std::string heading_image = base64_encode(reinterpret_cast<const unsigned char*>(user.heading_image.c_str()), user.heading_image.length());
            // log("heading_image :"+user.heading_image);
            // log("user.heading_image length: " + std::to_string(user.heading_image.length()));
            json_object_object_add(resJsonObj, "heading_image", json_object_new_string(user.heading_image.c_str()));
        }
    }
    // log("id"+std::to_string(user.id));
    // log("account"+user.account);
    // log("account"+user.password);
    // log("heading_image"+user.heading_image);

    /* 将json对象转成json字符串 */
    const char *resStr = json_object_to_json_string(resJsonObj);
    // log("发送登录处理结果至客户端... " + std::string(resStr));
    /* 将消息发送到客户端 */
    m_clientInfo->sendMessage(resStr);

    /* 释放json对象 */
    json_object_put(resJsonObj);

    if (jsonObj)
    {
        json_object_put(jsonObj);
    }
    log("登录操作已完成...");
}

bool Function::saveUserInfo(const char *username, const char *password)
{
    std::string sql = "INSERT INTO user (account, password) values ('%s', '%s');";
    char requestSql[128] = {0};
    sprintf(requestSql, sql.c_str(), username, password);

    if (m_mysqlDatabase->execute(requestSql) != true)
    {
        cout << "file " << __FILE__ << " :" << __LINE__ << endl;
        return false;
    }
    return true;
}


void Function::handleRegister(const std::string &msg)
{
    log("正在处理注册函数");
    const char *username = nullptr;
    const char *passwd = nullptr;
    json_object *jsonObj = json_tokener_parse(msg.c_str());
    if (jsonObj)
    {
        username = json_object_get_string(json_object_object_get(jsonObj, "account"));
        passwd = json_object_get_string(json_object_object_get(jsonObj, "password"));
    }

    /* 构建json字符串 */
    /* 1. 创建json对象 */
    json_object *resJsonObj = json_object_new_object();
    /* 2. 设置 <key, value> */
    json_object_object_add(resJsonObj, "type", json_object_new_int(REQ_REGIS));

    /* 判断用户是否存在 */
    if (isUserExist(username).id!=0)
    {
        json_object_object_add(resJsonObj, "result", json_object_new_string("failed"));
        json_object_object_add(resJsonObj, "reason", json_object_new_string("用户已存在"));
        log("Registration failed - User already exists: " + std::string(username), "WARNING");
    }
    else
    { 
        saveUserInfo(username, passwd);
        json_object_object_add(resJsonObj, "result", json_object_new_string("success"));
        log("Registration successful for user: " + std::string(username));
    }

    /* 将json对象转成json字符串 */
    const char *resStr = json_object_to_json_string(resJsonObj);
    m_clientInfo->sendMessage(resStr);
    /* 销毁对象 */
    json_object_put(resJsonObj);

    if (jsonObj)
    {
        json_object_put(jsonObj);
    }
    log("注册函数处理完成...");
}

void Function::handleUpdateHeadingImage(const std::string &msg)
{
    log("开始处理头像更新操作....");
    // 1. 解析 JSON 字符串
    struct json_object *parsed_json;
    parsed_json = json_tokener_parse(msg.c_str());
    if (!parsed_json) {
        std::cerr << "Failed to parse JSON" << std::endl;
        return;
    }
    // 2. 获取 type, user_id, heading_image
    struct json_object *jtype, *juser_id, *jheading_image;

    if (!json_object_object_get_ex(parsed_json, "type", &jtype) ||
        !json_object_object_get_ex(parsed_json, "user_id", &juser_id) ||
        !json_object_object_get_ex(parsed_json, "heading_image", &jheading_image)) {
        std::cerr << "Failed to get required fields from JSON" << std::endl;
        json_object_put(parsed_json);  // 释放 JSON 对象
        return;
    }
    int type = json_object_get_int(jtype);
    int user_id = json_object_get_int(juser_id);
    const char* heading_image_base64 = json_object_get_string(jheading_image);

    std::cout << "Type: " << type << ", User ID: " << user_id << std::endl;

    // 3. Base64 解码 heading_image
    std::string heading_image_str(heading_image_base64);

    // log("head_image_str"+heading_image_str);
    string heading_image_data = base64_decode(heading_image_str);

    if (heading_image_data.empty()) {
        std::cerr << "Failed to decode heading image" << std::endl;
        json_object_put(parsed_json);  // 释放 JSON 对象
        return;
    }
    // 准备 SQL 语句
    std::string query = "UPDATE user SET heading_image = ? WHERE account = ?";

    // 准备 MySQL 语句
    MYSQL_STMT *stmt = mysql_stmt_init(dynamic_cast<MysqlDataBase*>(m_mysqlDatabase.get())->get());
    if (!stmt) {
        std::cerr << "mysql_stmt_init() failed" << std::endl;
        m_mysqlDatabase->closeDB();
        json_object_put(parsed_json);  // 释放 JSON 对象
        return;
    }

    if (mysql_stmt_prepare(stmt, query.c_str(), query.length())) {
        std::cerr << "mysql_stmt_prepare() failed" << std::endl;
        mysql_stmt_close(stmt);
        m_mysqlDatabase->closeDB();
        json_object_put(parsed_json);  // 释放 JSON 对象
        return;
    }

    // 绑定参数
    MYSQL_BIND bind[2];
    memset(bind, 0, sizeof(bind));

    // 绑定 heading_image 数据
    bind[0].buffer_type = MYSQL_TYPE_BLOB;
    bind[0].buffer = (void*)heading_image_data.data();
    bind[0].buffer_length = heading_image_data.size();

    // 绑定 user_id
    bind[1].buffer_type = MYSQL_TYPE_LONG;
    bind[1].buffer = (void*)&user_id;
    bind[1].buffer_length = sizeof(user_id);

    if (mysql_stmt_bind_param(stmt, bind)) {
        log("mysql_stmt_bind_param() failed with error: " + std::string(mysql_stmt_error(stmt)));
        std::cerr << "mysql_stmt_bind_param() failed" << std::endl;
        mysql_stmt_close(stmt);
        m_mysqlDatabase->closeDB();
        json_object_put(parsed_json);  // 释放 JSON 对象
        return;
    }

    // 执行 SQL 语句
    if (mysql_stmt_execute(stmt)) {
        std::cerr << "mysql_stmt_execute() failed" << std::endl;
    } else {
        std::cout << "User heading_image updated successfully." << std::endl;
    }
    // 关闭 MySQL 资源
    mysql_stmt_close(stmt);
   
    json_object_put(parsed_json);  // 释放 JSON 对象
    log("头像更新操作完成....");
}

/* 请求存储我的音乐 */
void Function::handleReqStoreLoveMusic(const std::string& msg)
{
    // 1. 解析 msg 字符串，首先检查输入是否为空
    log("进入喜爱音乐操作界面");
    if (msg.empty()) {
        std::cerr << "Received an empty message" << std::endl;
        return;
    }

    json_object *parsed_json = json_tokener_parse(msg.c_str());
    if (parsed_json == nullptr) {
        std::cerr << "JSON parsing error" << std::endl;
        return;
    }

    // 2. 从 JSON 中提取 'type' 字段
    json_object *type_obj;
    if (!json_object_object_get_ex(parsed_json, "type", &type_obj)) {
        std::cerr << "JSON 'type' field not found" << std::endl;
        json_object_put(parsed_json); // 释放 json 对象
        return;
    }


    // 3. 从 JSON 中提取 'user_id' 和 'music_id'，并进行有效性检查
    json_object *user_id_obj;
    json_object *music_id_obj;

    if (!json_object_object_get_ex(parsed_json, "user_id", &user_id_obj) || 
        !json_object_object_get_ex(parsed_json, "music_id", &music_id_obj)) {
        std::cerr << "JSON 'user_id' or 'music_id' field not found" << std::endl;
        json_object_put(parsed_json); // 释放 json 对象
        return;
    }

    int user_id = json_object_get_int(user_id_obj);
    int music_id = json_object_get_int(music_id_obj);

    // 4. 检查 user_id 和 music_id 是否有效
    if (user_id <= 0 || music_id <= 0) {
        std::cerr << "Invalid 'user_id' or 'music_id' values" << std::endl;
        json_object_put(parsed_json); // 释放 json 对象
        return;
    }

    // 5. 查询数据库中是否已经存在该记录
    char check_sql[256] = {0};
    snprintf(check_sql, sizeof(check_sql), 
             "SELECT COUNT(*) FROM user_music WHERE user_id = %d AND music_id = %d;", 
             user_id, music_id);

    int recordCount = 0;
    try {
        VecResult queryResult = m_mysqlDatabase->query(check_sql);
        recordCount = std::stoi (queryResult[0][0]);
    } catch (const std::exception& e) {
        std::cerr << "Database query error: " << e.what() << std::endl;
        json_object_put(parsed_json);
        return;
    }

    // 6. 如果记录存在，删除该记录，否则插入新记录
    if (recordCount > 0) {
        // 记录存在，执行删除操作
        char delete_sql[256] = {0};
        snprintf(delete_sql, sizeof(delete_sql), 
                 "DELETE FROM user_music WHERE user_id = %d AND music_id = %d;", 
                 user_id, music_id);

        try {
            m_mysqlDatabase->execute(delete_sql);
            log("音乐记录已删除");
        } catch (const std::exception& e) {
            std::cerr << "Database delete error: " << e.what() << std::endl;
        }
    } else {
        // 记录不存在，执行插入操作
        char insert_sql[256] = {0};
        snprintf(insert_sql, sizeof(insert_sql), 
                 "INSERT INTO user_music (user_id, music_id) VALUES (%d, %d);", 
                 user_id, music_id);

        try {
            m_mysqlDatabase->execute(insert_sql);
            log("音乐记录已插入");
        } catch (const std::exception& e) {
            std::cerr << "Database insert error: " << e.what() << std::endl;
        }
    }

    // 7. 释放 JSON 对象
    json_object_put(parsed_json);
    log("喜欢音乐操作成功");
}

void Function::handleReqSonglist(const std::string &msg)
{
    log("开始请求歌单...");
    // 1. 解析msg字符串
    json_object *parsed_json = json_tokener_parse(msg.c_str());
    if (parsed_json == nullptr) {
        std::cerr << "JSON parsing error" << std::endl;
        return;
    }

    json_object *type_obj;
    if (!json_object_object_get_ex(parsed_json, "type", &type_obj)) {
        std::cerr << "JSON 'type' field not found" << std::endl;
        json_object_put(parsed_json); // 释放json对象
        return;
    }

    int type = json_object_get_int(type_obj);
    json_object *user_id_obj;
    if (!json_object_object_get_ex(parsed_json, "user_id", &user_id_obj)) {
        std::cerr << "JSON 'type' field not found" << std::endl;
        json_object_put(parsed_json); // 释放json对象
        return;
    }
    int user_id = json_object_get_int(user_id_obj);

    // 2. 查询数据库中的音乐文件信息
    std::string sql = "SELECT id, title, autor, duration, size FROM music;";
    VecResult queryResult = m_mysqlDatabase->query(sql);

    // 2. 查询数据库中的喜爱音乐信息
    std::string sql2 = "SELECT music_id FROM user_music where user_id = '%d';";
    char req_sql[256]={0};
    snprintf( req_sql,sizeof(req_sql),sql2.c_str(),user_id );
    VecResult love_result = m_mysqlDatabase->query(req_sql);
    std::set<int> love_set;
    for(auto ite = love_result.begin();ite != love_result.end(); ite++)
    {
        if (!ite->empty()) {  // 检查当前行是否为空
            love_set.insert(std::stoi(*(ite->begin()))); 
        }
    }
    // 3. 封装查询结果为JSON字符串
    json_object *response_json = json_object_new_object();
    json_object *songs_array = json_object_new_array();
    cout<<__LINE__<<endl;
    for (const auto &row : queryResult) {
        json_object *song_json = json_object_new_object();
        if(love_set.count(std::stoi(row[0].c_str()))==1)
        {
            json_object_object_add(song_json,"is_love",json_object_new_boolean(true));
            // log("is_love : true");
        }
        else
        {
            json_object_object_add(song_json,"is_love",json_object_new_boolean(false));
            // log("is_love : false");
        }

        json_object_object_add(song_json, "id", json_object_new_int(std::stoi(row[0].c_str())));
        json_object_object_add(song_json, "title", json_object_new_string(row[1].c_str()));
        json_object_object_add(song_json, "autor", json_object_new_string(row[2].c_str()));
        json_object_object_add(song_json, "duration", json_object_new_int(std::stoi(row[3])));
        json_object_object_add(song_json, "size", json_object_new_int(std::stoi(row[4])));
        json_object_array_add(songs_array, song_json);
        
        
    }
    json_object_object_add(response_json,"type",json_object_new_int(type));
    json_object_object_add(response_json, "songs", songs_array);

    // 将JSON对象转为字符串
    const char *response_str = json_object_to_json_string(response_json);

    // 4. 将JSON字符串发送给客户端
    std::string response(response_str);
    m_clientInfo->sendMessage(response);

    // 释放JSON对象
    json_object_put(parsed_json);
    json_object_put(response_json);
   
    log("请求歌单成功");
}

void Function::handleGetHeadingImage(const std::string &msg)
{
    log("开始处理获取用户头像信息...");
    
    // 1. 解析 JSON 字符串
    json_object *parsed_json = json_tokener_parse(msg.c_str());
    if (!parsed_json) {
        log("Failed to parse JSON");
        return;
    }
    
    // 2. 获取 user_id 和 type
    json_object *juser_id, *jtype;
    if (!json_object_object_get_ex(parsed_json, "user_id", &juser_id) ||
        !json_object_object_get_ex(parsed_json, "type", &jtype)) {
        log("Failed to get required fields from JSON");
        json_object_put(parsed_json);
        return;
    }
    
    int user_id = json_object_get_int(juser_id);
    int type = json_object_get_int(jtype);
    json_object_put(parsed_json);

    // 3. 构建 SQL 查询语句
    std::string query = "SELECT heading_image FROM user WHERE id = ?";

    MYSQL *conn = dynamic_cast<MysqlDataBase*>(m_mysqlDatabase.get())->get();
    MYSQL_STMT *stmt = mysql_stmt_init(conn);
    if (!stmt) {
        log("mysql_stmt_init() failed: " + std::string(mysql_error(conn)));
        return;
    }

    if (mysql_stmt_prepare(stmt, query.c_str(), query.length())) {
        log("mysql_stmt_prepare() failed: " + std::string(mysql_stmt_error(stmt)));
        mysql_stmt_close(stmt);
        return;
    }

    // 4. 绑定参数
    MYSQL_BIND param[1];
    memset(param, 0, sizeof(param));
    param[0].buffer_type = MYSQL_TYPE_LONG;
    param[0].buffer = &user_id;
    param[0].is_null = 0;
    param[0].length = 0;

    if (mysql_stmt_bind_param(stmt, param)) {
        log("mysql_stmt_bind_param() failed: " + std::string(mysql_stmt_error(stmt)));
        mysql_stmt_close(stmt);
        return;
    }

    // 5. 执行查询
    if (mysql_stmt_execute(stmt)) {
        log("mysql_stmt_execute() failed: " + std::string(mysql_stmt_error(stmt)));
        mysql_stmt_close(stmt);
        return;
    }

    // 6. 绑定结果
    MYSQL_BIND result[1];
    memset(result, 0, sizeof(result));
    unsigned long length = 0;
    bool is_null = 0;
    bool error = 0;

    result[0].buffer_type = MYSQL_TYPE_BLOB;
    result[0].buffer = nullptr; // We'll allocate this after we know the size
    result[0].buffer_length = 0;
    result[0].length = &length;
    result[0].is_null = &is_null;
    result[0].error = &error;

    if (mysql_stmt_bind_result(stmt, result)) {
        log("mysql_stmt_bind_result() failed: " + std::string(mysql_stmt_error(stmt)));
        mysql_stmt_close(stmt);
        return;
    }

    // 7. 获取数据
    int fetch_result = mysql_stmt_fetch(stmt);
    if (fetch_result == 1) {
        log("mysql_stmt_fetch() failed: " + std::string(mysql_stmt_error(stmt)));
        mysql_stmt_close(stmt);
        return;
    }
    else if (fetch_result == MYSQL_NO_DATA) {
        log("No data found for user_id: " + std::to_string(user_id));
        mysql_stmt_close(stmt);
        return;
    }

    std::string heading_image_base64;

    if (!is_null && length > 0) {
        char *heading_image_data = new char[length];
        result[0].buffer = heading_image_data;
        result[0].buffer_length = length;

        if (mysql_stmt_fetch_column(stmt, result, 0, 0)) {
            log("mysql_stmt_fetch_column() failed: " + std::string(mysql_stmt_error(stmt)));
            delete[] heading_image_data;
            mysql_stmt_close(stmt);
            return;
        }

        heading_image_base64 = base64_encode((unsigned char*)heading_image_data, length);
        delete[] heading_image_data;
    } else {
        log("User has no heading image or it's NULL");
        heading_image_base64 = ""; // 或者设置一个默认的Base64编码的图片
    }

    // 8. 构建 JSON 响应
    json_object *response_json = json_object_new_object();
    json_object_object_add(response_json, "type", json_object_new_int(type));
    json_object_object_add(response_json, "headImage", json_object_new_string(heading_image_base64.c_str()));

    // 9. 发送响应
    const char *json_str = json_object_to_json_string(response_json);
    m_clientInfo->sendMessage(json_str);

    // 10. 清理资源
    json_object_put(response_json);
    mysql_stmt_close(stmt);

    log("头像获取操作已完成...");
}






# if 1

/* 请求在线歌曲 和 请求下载*/
void Function::handleReqSongListen(const string& msg)
{
    // 1. 解析客户端请求的 JSON 数据
    log("正在请求在线歌曲或者请求下载...");
    json_object *req_obj = json_tokener_parse(msg.c_str());
    if (req_obj == nullptr)
    {
        std::cerr << "Failed to parse JSON request." << std::endl;
        return;
    }

    // 2. 获取请求类型
    json_object *type_obj = nullptr;
    log("query3");
    if (json_object_object_get_ex(req_obj, "type", &type_obj))
    {
        int req_type = json_object_get_int(type_obj);
        log("query2");
        if (req_type == REQ::REQ_SONGLISTEN||req_type == REQ::REQ_DOWNLOAD)
        {
            // 3. 查询数据库中的歌曲信息
            json_object *songs_obj = nullptr;
            log("query1");
            if (json_object_object_get_ex(req_obj, "songs", &songs_obj))
            {
                std::string query = "SELECT id ,lyric, musicmeta, image ,title,autor,duration,size FROM music WHERE id IN (";
                int song_count = json_object_array_length(songs_obj);
                
                // 将每个请求的歌曲ID添加到SQL查询
                for (int i = 0; i < song_count; ++i)
                {
                    json_object *song_id_obj = json_object_array_get_idx(songs_obj, i);
                    if (i > 0) query += ",";
                    query += json_object_get_string(song_id_obj);
                }
                query += ");";
                log("query:"+query);
                // 4. 执行数据库查询
                if (!m_mysqlDatabase->execute(query))
                {
                    log("数据库查询失败");
                    return;
                }

                MYSQL_RES *result = mysql_store_result(dynamic_cast<MysqlDataBase*>(m_mysqlDatabase.get())->get());
                if (result == nullptr)
                {
                    std::cout << "获取查询结果失败: " << mysql_error(dynamic_cast<MysqlDataBase*>(m_mysqlDatabase.get())->get()) << endl;
                    return;
                }

                // 5. 构建 JSON 响应对象
                json_object *resp_obj = json_object_new_object();
                json_object *songs_array = json_object_new_array();
                MYSQL_ROW row;
                while ((row = mysql_fetch_row(result)) != nullptr)
                {
                    unsigned long *lengths = mysql_fetch_lengths(result);
                    
                    // 将图片和元数据（BLOB）转换为Base64编码
                    std::string image_base64 = base64_encode(reinterpret_cast<const unsigned char*>(row[3]), lengths[3]);
                    std::string musicmeta_base64 = base64_encode(reinterpret_cast<const unsigned char*>(row[2]), lengths[2]);
                    json_object *song_obj = json_object_new_object();
                    json_object_object_add(song_obj, "id", json_object_new_int(stoi(row[0])));
                    log("id:"+string(row[0]));
                    json_object_object_add(song_obj, "lyric", json_object_new_string(row[1]));
                    log("id:"+string(row[1]));
                    json_object_object_add(song_obj, "musicmeta", json_object_new_string(musicmeta_base64.c_str()));
                    json_object_object_add(song_obj, "image", json_object_new_string(image_base64.c_str()));
                    json_object_object_add(song_obj, "title", json_object_new_string(row[4]));
                    json_object_object_add(song_obj, "autor", json_object_new_string(row[5]));
                    log("autor:"+string(row[5]));
                    json_object_object_add(song_obj, "duration", json_object_new_int(atoi(row[6])));
                    log("duration:"+string(row[6]));
                    json_object_object_add(song_obj, "size", json_object_new_int(atoi(row[7])));
                    log("size:"+string(row[7]));
                    json_object_array_add(songs_array, song_obj);
                }
                // 6. 将歌曲列表添加到响应JSON对象
                json_object_object_add(resp_obj, "type", json_object_new_int(req_type));
                log("type:"+ req_type);
                json_object_object_add(resp_obj, "songs", songs_array);
                // 7. 将 JSON 对象转换为字符串并发送给客户端
                const char *resp_str = json_object_to_json_string(resp_obj);
                m_clientInfo->sendMessage(std::string(resp_str));
                // 8. 释放 JSON 对象
                json_object_put(resp_obj);

            }
        }
    }
    // 释放请求 JSON 对象
    json_object_put(req_obj);
    log("请求在线歌曲或者请求下载成功...");
}



# endif

