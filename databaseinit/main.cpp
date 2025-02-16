#include <iostream>
#include <mysql/mysql.h>
#include <taglib/fileref.h>
#include <taglib/tag.h>
#include <taglib/tpropertymap.h>
#include <taglib/tstring.h>
#include <fstream>
#include <sstream>
#include <vector>
#include <filesystem>

namespace fs = std::filesystem;

bool insertIntoDatabase(MYSQL *conn, const std::string &title, const std::string &author, 
                        int duration, int size, const std::vector<unsigned char> &musicMeta, 
                        const std::vector<unsigned char> &image, const std::string &lyric) {
    std::stringstream ss;
    ss << "INSERT INTO music (title, autor, duration, size, musicmeta, image, lyric, create_time) "
       << "VALUES (?, ?, ?, ?, ?, ?, ?, NOW())";

    MYSQL_STMT *stmt = mysql_stmt_init(conn);
    if (mysql_stmt_prepare(stmt, ss.str().c_str(), ss.str().length())) {
        std::cerr << "MySQL prepare statement failed: " << mysql_stmt_error(stmt) << std::endl;
        return false;
    }

    MYSQL_BIND bind[7] = {};

    bind[0].buffer_type = MYSQL_TYPE_STRING;
    bind[0].buffer = const_cast<char *>(title.c_str());
    bind[0].buffer_length = title.length();

    bind[1].buffer_type = MYSQL_TYPE_STRING;
    bind[1].buffer = const_cast<char *>(author.c_str());
    bind[1].buffer_length = author.length();

    bind[2].buffer_type = MYSQL_TYPE_LONG;
    bind[2].buffer = &duration;

    bind[3].buffer_type = MYSQL_TYPE_LONG;
    bind[3].buffer = &size;

    bind[4].buffer_type = MYSQL_TYPE_BLOB;
    bind[4].buffer = const_cast<unsigned char *>(musicMeta.data());
    bind[4].buffer_length = musicMeta.size();

    bind[5].buffer_type = MYSQL_TYPE_BLOB;
    bind[5].buffer = const_cast<unsigned char *>(image.data());
    bind[5].buffer_length = image.size();

    bind[6].buffer_type = MYSQL_TYPE_STRING;
    bind[6].buffer = const_cast<char *>(lyric.c_str());
    bind[6].buffer_length = lyric.length();

    if (mysql_stmt_bind_param(stmt, bind)) {
        std::cerr << "MySQL bind parameters failed: " << mysql_stmt_error(stmt) << std::endl;
        mysql_stmt_close(stmt);
        return false;
    }

    if (mysql_stmt_execute(stmt)) {
        std::cerr << "MySQL execute statement failed: " << mysql_stmt_error(stmt) << std::endl;
        mysql_stmt_close(stmt);
        return false;
    }

    mysql_stmt_close(stmt);
    return true;
}

std::vector<unsigned char> readFileToBuffer(const std::string &filePath) {
    std::ifstream file(filePath, std::ios::binary);
    return std::vector<unsigned char>((std::istreambuf_iterator<char>(file)),
                                      std::istreambuf_iterator<char>());
}

int main() {
    MYSQL *conn = mysql_init(nullptr);
    if (conn == nullptr) {
        std::cerr << "MySQL initialization failed." << std::endl;
        return 1;
    }

    if (mysql_real_connect(conn, "localhost", "root", "1", "musicplayer", 3306, nullptr, 0) == nullptr) {
        std::cerr << "MySQL connection failed: " << mysql_error(conn) << std::endl;
        mysql_close(conn);
        return 1;
    }

    std::string musicDir = "../Music";
    for (const auto &entry : fs::directory_iterator(musicDir)) {
        if (entry.is_regular_file() && entry.path().extension() == ".mp3") {
            TagLib::FileRef file(entry.path().c_str());
            if (!file.isNull() && file.tag()) {
                std::string title = file.tag()->title().to8Bit(true);
                std::string author = file.tag()->artist().to8Bit(true);
                int duration = file.audioProperties()->length();
                int size = fs::file_size(entry);

                // 读取音乐元数据
                std::vector<unsigned char> musicMeta = readFileToBuffer(entry.path());

                // 假设图像和歌词文件与音乐文件同名
                std::string imageFilePath = entry.path().stem().string() + ".png";
                std::vector<unsigned char> image;
                if (fs::exists(musicDir + "/" + imageFilePath)) {
                    image = readFileToBuffer(musicDir + "/" + imageFilePath);
                }

                std::string lyricFilePath = entry.path().stem().string() + ".lrc";
                std::ifstream lyricFile(musicDir + "/" + lyricFilePath);
                std::string lyric((std::istreambuf_iterator<char>(lyricFile)), std::istreambuf_iterator<char>());

                if (!insertIntoDatabase(conn, title, author, duration, size, musicMeta, image, lyric)) {
                    std::cerr << "Failed to insert data for file: " << entry.path() << std::endl;
                }
            } else {
                std::cerr << "Failed to read tag information for file: " << entry.path() << std::endl;
            }
        }
    }

    mysql_close(conn);
    return 0;
}
