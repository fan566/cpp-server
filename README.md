# C++ 音乐播放器服务器

一个支持音乐播放和即时通讯功能的服务器框架。

## 主要功能

### 1. 音乐播放功能
- 音乐列表管理
- 在线音乐播放
- 音乐下载
- 收藏音乐管理
- 播放历史记录
- 音乐搜索

### 2. 即时通讯功能
- 文本消息传输
- 图片传输
  - 支持大文件分块传输
  - Base64 编码传输
  - 拥塞避免机制
- 广播消息
- 私聊功能（基于生产者消费者模型）

### 3. 用户管理
- 用户注册/登录
- 头像管理
- 个人信息管理

## 技术架构

### 1. 基础通信
- TCP Socket 通信
- 多客户端并发支持
- 线程池管理
- 消息队列处理

### 2. 数据存储
- MySQL 数据库
  - 用户信息表
  - 播放记录表
  - 音乐信息表
  - 用户收藏表
- BLOB 类型支持
- 预处理语句防注入

### 3. 性能优化
- 池化技术
  - 对象池
  - 内存池
  - 线程池
  - 数据库连接池

### 4. 安全特性
- 线程安全保护
- 异常处理机制
- 日志记录系统
- 错误恢复机制

## API 接口

### 1. 用户接口
#### 注册接口
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

#### 登录接口
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
    "reason": "错误原因",
    "user_id": "用户ID",
    "account": "用户名",
    "heading_image": "base64编码的头像"
}
```

### 2. 音乐接口
#### 音乐列表获取
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

#### 音乐收藏
```javascript
// 请求
{
    "type": REQ_STORLOVEMUSIC,
    "user_id": "用户ID",
    "music_id": "音乐ID"
}
```

### 3. 聊天接口
#### 文本消息
```javascript
// 请求
{
    "type": REQ_ChatTXT,
    "data": "消息内容",
    "to": "接收者用户名"
}
```

#### 图片传输
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

## 数据库设计

### 用户表 (userInfo)
```sql
CREATE TABLE userInfo (
    id INT PRIMARY KEY AUTO_INCREMENT,
    username VARCHAR(255) UNIQUE NOT NULL,
    passwd VARCHAR(255) NOT NULL,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);
```

### 播放记录表 (user_play_history)
```sql
CREATE TABLE user_play_history (
    id INT PRIMARY KEY AUTO_INCREMENT,
    user_id INT,
    song_name VARCHAR(255) NOT NULL,
    volume INT NOT NULL,
    play_position INT NOT NULL,
    play_duration INT NOT NULL,
    source ENUM('local', 'online', 'favorite') NOT NULL,
    playback_mode ENUM('ORDER_MODE', 'CYCLE_MODE', 'RANDOM_MODE') NOT NULL,
    is_favorite BOOLEAN NOT NULL DEFAULT FALSE,
    played_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    FOREIGN KEY (user_id) REFERENCES userInfo(id) ON DELETE CASCADE,
    INDEX idx_user_id (user_id)
);
```

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
- json-c：JSON 解析
- mysql-client：MySQL 客户端
- pthread：多线程支持
- C++11 或更高版本
