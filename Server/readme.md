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

