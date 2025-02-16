池化技术：
    对象池、内存池、线程池、数据库连接池

# 注册：
## 请求端：
```javascript
request:{"type":1, "username":"xxx", "passwd":"xxxxx"}
```
## 响应端：
```javascript
response:{"type":1, "result":"success", "reason":"XXX"}
```

# 登录：
## 请求端：
```javascript
request:{"type":1, "username":"xxx", "passwd":"xxxxx"}
```
## 响应端：
```javascript
response:{"type":1, "result":"success", "reason":"XXX"}
```

## 搜索歌曲:
### 请求端：
```javascript
request:{"type":SEARCH_MUSIC, "musicName":"一直很安静"}
```
### 回复端:
```javascript
response:{"type":SEARCH_MUSIC, "musicName":"一直很安静", "musicContent":"xxxx"}
```

## 下载音乐：
### 请求端：
```javascript
request:{"type":Download, "musicName":"我怀念的"}
```
### 响应端：
```javascript
response:{"type":Download, "musicName":"我怀念的", "musicContent":"xxxx"}
```

> /* 项目要求：
>         1.不允许魔数 (一处扣 5分)
>         2.程序异常退出 (60分以下)
>         ——————————————————————————————————
>         3.本地播放功能OK (具备记忆功能 - 10)
>         4.在线播放功能OK (具备记忆功能)
>         5.收藏的音乐歌单功能OK (具备记忆功能)
>         6.下载音乐功能 OK (具备记忆功能)
>         7.AI人工智能 (具备搜索)
>         8.歌词具备隐藏功能, 滚动功能
>         9.批量下载OK    (加分项)
> */

```sql
-- 用户表
CREATE TABLE userInfo (
    id INT PRIMARY KEY AUTO_INCREMENT,  -- 用户 ID
    username VARCHAR(255) UNIQUE NOT NULL,  -- 用户名
    passwd VARCHAR(255) NOT NULL,  -- 密码
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP  -- 创建时间
);
```

```sql
-- 播放记录表
CREATE TABLE user_play_history (
    id INT PRIMARY KEY AUTO_INCREMENT,  -- 记录 ID
    user_id INT,  -- 关联的用户 ID
    song_name VARCHAR(255) NOT NULL,  -- 歌曲名称
    volume INT NOT NULL,  -- 音量（百分比存储，例如 50 表示 50% 音量）
    play_position INT NOT NULL,  -- 播放位置（毫秒为单位）
    play_duration INT NOT NULL,  -- 播放总时长（毫秒为单位）
    source ENUM('local', 'online', 'favorite') NOT NULL,  -- 播放来源
    playback_mode ENUM('ORDER_MODE', 'CYCLE_MODE', 'RANDOM_MODE') NOT NULL,  -- 播放模式
    is_favorite BOOLEAN NOT NULL DEFAULT FALSE,  -- 是否收藏
    played_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,  -- 播放时间
    FOREIGN KEY (user_id) REFERENCES userInfo(id) ON DELETE CASCADE,  -- 外键约束，用户被删除时删除对应记录
    INDEX idx_user_id (user_id)  -- 用户 ID 索引，加快查询速度
);
```

```sql
-- 音乐信息表
CREATE TABLE music_library (
    id INT PRIMARY KEY AUTO_INCREMENT,  -- 歌曲 ID
    song_name VARCHAR(255) NOT NULL,  -- 歌曲名称
    artist_name VARCHAR(255),  -- 艺术家名称
    album_name VARCHAR(255),  -- 专辑名称（如果有）
    duration INT NOT NULL,  -- 总时长（以毫秒为单位）
   duration_str varchar(8) NOT NULL,			  --  总时长（字符串存储）
    file_format ENUM('mp3', 'm4a', 'wav', 'flac', 'aac', 'ogg') NOT NULL,  -- 音乐文件类型
    file_path VARCHAR(512) NOT NULL,  -- 服务器上的文件路径(可选)
    lyrics_path varchar(512) DEFAULT NULL, -- 服务器上的歌词文件路径(可选)
    added_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,  -- 添加时间
   INDEX idx_song_name (song_name),  -- 歌曲名索引，加快查询速度
   INDEX idx_artist_name (artist_name)  -- 艺术家名索引，加快查询速度
);
```



## 更新播放历史记录
### 请求端：
```javascript
request:{"type":FUNC_UPDATE_PLAY_HISTORY, "userId":"1"}
```
### 回复端:
```javascript
response:{"type":FUNC_UPDATE_PLAY_HISTORY, "song_name":"一直很安静", "volume":33, "play_position":80403, "play_duration":196757, "source":"local", "playback_mode":"CYCLE_MODE", "is_favorite":true}
```
## 存储播放历史记录
### 请求端：
```javascript
request:{"type":FUNC_SAVE_PLAY_HISTORY, "userId":"1", "song_name":"一直很安静", "volume":33, "play_position":80403, "play_duration":196757, "source":"local", "playback_mode":"CYCLE_MODE", "is_favorite":true}
```
### 回复端:
```javascript
response:{"type":FUNC_SAVE_PLAY_HISTORY, "userId":"1", ...执行sql语句处理 }
```

```sql
-- 用户收藏表
CREATE TABLE userFavorites (
    id INT PRIMARY KEY AUTO_INCREMENT,  -- 收藏 ID
    user_id INT NOT NULL,  -- 关联的用户 ID
    song_name VARCHAR(255) NOT NULL,  -- 歌曲名称
    artist_name VARCHAR(255), -- 艺术家名称（可选）
    album_name VARCHAR(255),  -- 专辑名称（可选）
    duration INT,  -- 歌曲时长（以秒为单位）
    duration_str varchar(8) NOT NULL,			  --  总时长（字符串存储）
    source ENUM('local', 'online') NOT NULL,  -- 音乐来源
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,  -- 收藏时间
    FOREIGN KEY (user_id) REFERENCES userInfo(id) ON DELETE CASCADE,    -- 外键联系
    INDEX idx_user_id (user_id),    -- 提高查询性能
    UNIQUE KEY unique_favorite (user_id, song_name, source) -- 防止重复收藏
);
```

# 音乐播放器服务器

## 功能概述

### 1. 基础通信功能
- TCP Socket通信
- 支持多客户端并发连接
- 线程池管理客户端连接
- 消息队列处理客户端请求

### 2. 用户管理
- 用户注册
- 用户登录
- 头像管理（上传/获取）
- 用户信息存储

### 3. 音乐管理
- 音乐列表获取
- 在线音乐播放
- 音乐下载
- 收藏音乐管理

### 4. 聊天功能
- 文本消息
- 图片传输
- 广播消息
- 私聊功能

## API 接口说明

### 1. 用户相关接口

#### 1.1 用户注册
```javascript
// 请求
{
    "type": REQ_REGIS,
    "account": "username",
    "password": "password"
}

// 响应
{
    "type": REQ_REGIS,
    "result": "success/failed",
    "reason": "错误原因（如果失败）"
}
```

#### 1.2 用户登录
```javascript
// 请求
{
    "type": REQ_LOGIN,
    "acccount": "username",
    "password": "password"
}

// 响应
{
    "type": REQ_LOGIN,
    "result": "success/failed",
    "reason": "错误原因（如果失败）",
    "user_id": "用户ID",
    "account": "用户名",
    "heading_image": "base64编码的头像"
}
```

### 2. 音乐相关接口

#### 2.1 获取音乐列表
```javascript
// 请求
{
    "type": REQ_SONGLIST,
    "user_id": "用户ID"
}

// 响应
{
    "type": REQ_SONGLIST,
    "songs": [
        {
            "id": "音乐ID",
            "title": "标题",
            "autor": "作者",
            "duration": "时长",
            "size": "文件大小",
            "is_love": true/false
        }
    ]
}
```

#### 2.2 收藏/取消收藏音乐
```javascript
// 请求
{
    "type": REQ_STORLOVEMUSIC,
    "user_id": "用户ID",
    "music_id": "音乐ID"
}
```

### 3. 聊天相关接口

#### 3.1 文本消息
```javascript
// 请求
{
    "type": REQ_ChatTXT,
    "data": "消息内容",
    "to": "接收者用户名"
}
```

#### 3.2 图片传输
```javascript
// 开始传输
{
    "type": REQ_FILE_START,
    "data": {
        "filename": "文件名",
        "total_size": "文件大小"
    }
}

// 传输数据
{
    "type": REQ_FILE_DATA,
    "data": "base64编码的图片数据块"
}

// 结束传输
{
    "type": REQ_FILE_END
}
```

## 技术特性

1. 线程安全
   - 使用互斥锁保护共享资源
   - 条件变量实现线程同步
   - 线程池管理客户端连接

2. 数据库支持
   - MySQL数据库存储用户数据
   - 支持 BLOB 类型存储二进制数据
   - 预处理语句防止 SQL 注入

3. 消息处理
   - 基于消息队列的异步处理
   - JSON 格式的消息协议
   - 支持二进制数据的 Base64 编码传输

4. 内存管理
   - 智能指针管理动态内存
   - RAII 原则确保资源正确释放

5. 错误处理
   - 异常处理机制
   - 日志记录系统
   - 优雅的错误恢复机制

## 编译和运行

```bash
# 编译
make

# 运行服务器
make run

# 清理构建文件
make clean
```

## 依赖项
- json-c
- mysql-client
- pthread
- C++11 或更高版本
