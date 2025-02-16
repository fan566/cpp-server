#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/select.h>
#include <pthread.h>
#include <json-c/json.h>
#include "base64_c.h"

#define BUFFER_SIZE 1024
#define MAX_PATH 256
#define MIN_DELAY 5000    // 最小延时 5ms
#define MAX_DELAY 100000  // 最大延时 100ms
#define INITIAL_DELAY 10000  // 初始延时 10ms
#define MIN(a,b) ((a) < (b) ? (a) : (b))
#define MAX(a,b) ((a) > (b) ? (a) : (b))

// 定义消息类型
typedef enum {
    REQ_ChatTXT = 100,
    REQ_TRANSPHOTO = 101,
    REQ_FILE_START = 102,  // 文件传输开始
    REQ_FILE_DATA = 103,   // 文件数据块
    REQ_FILE_END = 104,    // 文件传输结束
    REQ_LOGIN = 105         // 登录请求
} MessageType;

// 函数声明
void show_menu(void);
void handle_text_chat(int socket);
void handle_image_transfer(int socket);
int send_data(int socket, const char* data, size_t len);
void receive_data(int socket);
// void cycle_read_file(char *filepath);
int send_json_data(int socket, const char* data, int type);
int send_file_start(int socket, const char* filename, size_t total_size);
int send_file_data(int socket, const char* data, size_t data_len);
int send_file_end(int socket);
int send_actual_data(int socket, const char* data, size_t data_len);

// 显示主菜单
void show_menu(void) {
    printf("\n=== 聊天客户端菜单 ===\n");
    printf("1. 文本聊天\n");
    printf("2. 图片传输\n");
    printf("输入 'quit' 退出程序\n");
    printf("请选择功能: ");
    fflush(stdout);
}

// 发送JSON格式的数据
int send_json_data(int socket, const char* data, int type) {
    json_object *json_msg = json_object_new_object();
    json_object_object_add(json_msg, "type", json_object_new_int(type));
    json_object_object_add(json_msg, "data", json_object_new_string(data));
    
    const char *json_str = json_object_to_json_string(json_msg);
    size_t len = strlen(json_str);

    int ret = 0;
    if (send(socket, &len, sizeof(len), 0) != sizeof(len)) {
        ret = -1;
    } else if (send(socket, json_str, len, 0) != len) {
        ret = -1;
    }

    json_object_put(json_msg);
    return ret;
}

// 发送文件开始信息
int send_file_start(int socket, const char* filename, size_t total_size) {
    json_object *json_msg = json_object_new_object();
    json_object_object_add(json_msg, "filename", json_object_new_string(filename));
    json_object_object_add(json_msg, "total_size", json_object_new_int64(total_size));
    
    const char *json_str = json_object_to_json_string(json_msg);
    int ret = send_json_data(socket, json_str, REQ_FILE_START);
    
    json_object_put(json_msg);
    return ret;
}

// 发送文件数据块
int send_file_data(int socket, const char* data, size_t data_len) {
    static int current_delay = INITIAL_DELAY;
    static int success_count = 0;
    
    // 发送数据
    int result = send_actual_data(socket, data, data_len);
    
    if (result < 0) {
        // 发送失败，增加延时
        current_delay = MIN(current_delay * 2, MAX_DELAY);
        success_count = 0;
    } else {
        // 发送成功，考虑减少延时
        success_count++;
        if (success_count > 5) {  // 连续成功5次
            current_delay = MAX(current_delay / 2, MIN_DELAY);
            success_count = 0;
        }
    }
    
    usleep(current_delay);
    return result;
}

// 发送文件结束信息
int send_file_end(int socket) {
    return send_json_data(socket, "", REQ_FILE_END);  // 发送空数据
}

// 修改后的 send_actual_data
int send_actual_data(int socket, const char* data, size_t data_len) {
    // 将二进制数据转换为base64
    char *base64_data = base64_encode_c((unsigned char*)data, data_len);
    if (!base64_data) return -1;
    
    // 使用 send_json_data 发送数据
    int ret = send_json_data(socket, base64_data, REQ_FILE_DATA);
    
    base64_free(base64_data);
    return ret;
}

// 处理文本聊天
void handle_text_chat(int socket) {
    char send_buffer[BUFFER_SIZE];
    printf("\n===   ===\n");
    printf("输入 'backward' 返回主菜单\n");

    while (1) {
        printf("请输入消息: ");
        fflush(stdout);

        if (fgets(send_buffer, BUFFER_SIZE, stdin) == NULL) {
            break;
        }

        // 移除换行符
        send_buffer[strcspn(send_buffer, "\n")] = 0;

        if (strcmp(send_buffer, "backward") == 0) {
            return;
        }

        // 发送JSON格式的文本消息
        if (send_json_data(socket, send_buffer, REQ_ChatTXT) < 0) {
            printf("发送消息失败\n");
            return;
        }
    }
}

// 处理图片传输
void handle_image_transfer(int socket) {
    char filepath[MAX_PATH];
    printf("\n=== 图片传输模式 ===\n");
    printf("输入 'backward' 返回主菜单\n");

    while (1) {
        printf("请输入图片路径: ");
        fflush(stdout);

        if (fgets(filepath, MAX_PATH, stdin) == NULL) {
            break;
        }

        filepath[strcspn(filepath, "\n")] = 0;
        if (strcmp(filepath, "backward") == 0) {
            return;
        }

        FILE *fp = fopen(filepath, "rb");
        if (fp == NULL) {
            printf("无法打开文件: %s\n", filepath);
            continue;
        }

        // 获取文件大小
        fseek(fp, 0, SEEK_END);
        size_t filesize = ftell(fp);
        fseek(fp, 0, SEEK_SET);

        // 发送文件开始信息
        if (send_file_start(socket, filepath, filesize) < 0) {
            printf("发送文件信息失败\n");
            fclose(fp);
            continue;
        }
        printf("开始发送文件: %s (大小: %zu 字节)\n", filepath, filesize);

        // 分块读取并发送文件内容
        char *buffer = malloc(BUFFER_SIZE);
        if (!buffer) {
            printf("内存分配失败\n");
            fclose(fp);
            continue;
        }

        size_t total_sent = 0;
        int error_occurred = 0;
        
        while (total_sent < filesize && !error_occurred) {
            size_t to_read = (filesize - total_sent < BUFFER_SIZE) ? 
                            (filesize - total_sent) : BUFFER_SIZE;
            
            size_t bytes_read = fread(buffer, 1, to_read, fp);
            if (bytes_read <= 0) {
                printf("读取文件失败\n");
                error_occurred = 1;
                break;
            }

            if (send_file_data(socket, buffer, bytes_read) < 0) {
                printf("发送文件数据失败\n");
                error_occurred = 1;
                break;
            }

            total_sent += bytes_read;
            
            printf("\r发送进度: %.2f%% (%zu/%zu bytes)", 
                   (float)total_sent * 100 / filesize, total_sent, filesize);
            fflush(stdout);
        }

        if (!error_occurred) {
            send_file_end(socket);
            printf("\n文件发送完成\n");
        }

        free(buffer);
        fclose(fp);
    }
}

// 接收数据的线程函数
void receive_data(int socket) {
    pthread_detach(pthread_self());
    char recv_buffer[BUFFER_SIZE];
    size_t len;

    while (1) {
        // 接收数据长度
        if (recv(socket, &len, sizeof(len), 0) <= 0) {
            printf("服务器断开连接\n");
            exit(1);
        }

        // 接收JSON数据
        size_t total_received = 0;
        memset(recv_buffer, 0, BUFFER_SIZE);

        while (total_received < len) {
            ssize_t received = recv(socket, recv_buffer + total_received, 
                                  len - total_received, 0);
            if (received <= 0) {
                printf("服务器断开连接\n");
                exit(1);
            }
            total_received += received;
        }

        // 解析JSON数据
        json_object *json_msg = json_tokener_parse(recv_buffer);
        if (json_msg == NULL) {
            printf("接收到无效的JSON数据\n");
            continue;
        } 
        // printf("json_msg:%s\n",json_object_to_json_string(json_msg));
        // 获取消息类型
        json_object *type_obj;
        json_object *data_obj;
        if (json_object_object_get_ex(json_msg, "type", &type_obj) &&
            json_object_object_get_ex(json_msg, "data", &data_obj)) {
            
            int type = json_object_get_int(type_obj);
            const char *data = json_object_get_string(data_obj);

            printf("\n收到消息: %s\n", data);
            printf("请选择功能: ");
            fflush(stdout);
        }

        json_object_put(json_msg);
    }
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf("使用方法: %s <IP地址> <端口号>\n", argv[0]);
        return -1;
    }

    // 创建socket
    int client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket < 0) {
        perror("socket创建失败");
        return -1;
    }

    // 设置服务器地址
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(atoi(argv[2]));
    
    if (inet_pton(AF_INET, argv[1], &server_addr.sin_addr) <= 0) {
        perror("无效的IP地址");
        return -1;
    }

    // 连接服务器
    if (connect(client_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("连接失败");
        return -1;
    }

    printf("成功连接到服务器！\n");
    
    // 要求输入用户名
    char username[32];
    printf("请输入你的用户名: ");
    fgets(username, sizeof(username), stdin);
    username[strcspn(username, "\n")] = 0;
    
    // 发送用户名到服务器
    if (send_json_data(client_socket, username, REQ_LOGIN) < 0) {
        printf("发送用户名失败\n");
        close(client_socket);
        return -1;
    }

    // 创建接收线程
    pthread_t recv_thread;
    if (pthread_create(&recv_thread, NULL, (void *(*)(void *))receive_data, (void *)(long)client_socket) != 0) {
        perror("创建接收线程失败");
        close(client_socket);
        return -1;
    }

    char choice[32];
    while (1) {
        show_menu();
        
        if (fgets(choice, sizeof(choice), stdin) == NULL) {
            break;
        }
        choice[strcspn(choice, "\n")] = 0;

        if (strcmp(choice, "quit") == 0) {
            break;
        }

        switch (choice[0]) {
            case '1':
                handle_text_chat(client_socket);
                break;
            case '2':
                handle_image_transfer(client_socket);
                break;
            default:
                printf("无效的选择，请重试\n");
                break;
        }
    }

    // 清理并退出
    close(client_socket);
    return 0;
} 