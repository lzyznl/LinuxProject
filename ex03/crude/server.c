#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/epoll.h>
#include <stdbool.h>

#define PORT 8088
#define MAX_EVENTS 10
#define BUFFER_SIZE 1024

// 创建套接字并绑定
int setup_server() {
    int server_fd;
    struct sockaddr_in address;

    // 创建套接字
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    // 绑定套接字
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    // 监听套接字
    if (listen(server_fd, 3) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }
    return server_fd;
}

// 设置多路复用
int setup_multiplexing(int server_fd) {
    int epoll_fd;
    struct epoll_event ev, events[MAX_EVENTS];

    // 创建epoll实例
    if ((epoll_fd = epoll_create1(0)) < 0) {
        perror("epoll_create1");
        exit(EXIT_FAILURE);
    }

    // 添加监听套接字到epoll
    ev.events = EPOLLIN;
    ev.data.fd = server_fd;
    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, server_fd, &ev) < 0) {
        perror("epoll_ctl: listen_sock");
        exit(EXIT_FAILURE);
    }
    
    return epoll_fd;
}

// 接受连接
void accept_connections(int server_fd, int epoll_fd) {
    struct epoll_event events[MAX_EVENTS];
    int event_count;

    // 等待事件发生
    while (1) {
        event_count = epoll_wait(epoll_fd, events, MAX_EVENTS, -1);
        for (int i = 0; i < event_count; i++) {
            if (events[i].data.fd == server_fd) {
                // 处理新连接
                int new_socket;
                if ((new_socket = accept(server_fd, (struct sockaddr *)NULL, NULL)) < 0) {
                    perror("accept");
                    exit(EXIT_FAILURE);
                }
                // 分发事件到工作线程
                dispatch_event(new_socket);
            }
        }
    }
}

// 从URL中获取文件路径和download参数
void parse_url(const char *url, char *file_path, bool *download) {
    *download = false; // 初始化download为false
    char download_param[10] = {0};
    char path_param[255] = {0};

    // 解析url中的查询参数，这里只处理了简单情况
    sscanf(url, "/?path=%[^&]&download=%s", path_param, download_param);
    // 解码URL（简单处理，实际应用时需要更完整的URL解码）
    decode_url(path_param, file_path);
    if (strcmp(download_param, "true") == 0) {
        *download = true;
    }
}

// 简单的URL解码，只处理空格的情况
void decode_url(const char *src, char *dest) {
    for (; *src; ++src) {
        if (*src == '+') {
            *dest = ' ';
        } else {
            *dest = *src;
        }
        ++dest;
    }
    *dest = '\0';
}

// 从URL中获取查询参数的值
void get_query_param(const char *url, const char *param_name, char *param_value, int param_value_size) {
    // 这里只处理了简单情况，实际情况可能更复杂
    char *param_start = strstr(url, param_name);
    if (param_start) {
        sscanf(param_start, "%*[^=]=%s", param_value);
        char *param_end = strchr(param_value, '&');
        if (param_end) {
            *param_end = '\0';
        }
    } else {
        strncpy(param_value, "", param_value_size);
    }
}


// 分发事件到工作线程
void dispatch_event(int socket_fd) {
    // 可以创建线程或使用线程池处理请求
    // pthread_create(...) 或将socket_fd添加到线程池的任务队列
    handle_request(socket_fd);
}

// 解析HTTP请求
void parse_http_request(const char *request, char *method, char *url, char *protocol) {
    sscanf(request, "%s %s %s", method, url, protocol);
}

// 从URL中获取文件路径
void get_file_path_from_url(const char *url, char *file_path) {
    // 解析url中的查询参数，这里只处理了简单情况
    sscanf(url, "/?path=%s", file_path);
}

// 处理GET请求
void handle_get_request(int socket_fd, const char *file_path, bool download) {
    char buffer[BUFFER_SIZE] = {0};
    FILE *file = fopen(file_path, "rb"); // 以二进制方式打开文件，确保对所有文件类型都有效

    if (file != NULL) {
        if (download) {
            // 用户请求下载文件
            sprintf(buffer, "HTTP/1.1 200 OK\r\nContent-Type: application/octet-stream\r\nContent-Disposition: attachment; filename=\"%s\"\r\n\r\n", strrchr(file_path, '/') + 1);
        } else {
            // 用户请求预览文件
            sprintf(buffer, "HTTP/1.1 200 OK\r\nContent-Type: text/plain; charset=UTF-8\r\n\r\n");
        }
        send(socket_fd, buffer, strlen(buffer), 0);

        // 读取文件内容并发送
        int bytes_read;
        while ((bytes_read = fread(buffer, 1, BUFFER_SIZE, file)) > 0) {
            send(socket_fd, buffer, bytes_read, 0);
        }
        fclose(file);
    } else {
        // 文件不存在，发送404 Not Found
        char *response = "HTTP/1.1 404 Not Found\r\nContent-Type: text/html; charset=UTF-8\r\n\r\n<html><body><h1>404 Not Found</h1></body></html>";
        send(socket_fd, response, strlen(response), 0);
    }
}

// 处理请求
void handle_request(int socket_fd) {
    char buffer[BUFFER_SIZE] = {0};
    char method[10] = {0}, url[255] = {0}, protocol[10] = {0};

    // 读取请求数据
    int bytes_read = recv(socket_fd, buffer, BUFFER_SIZE - 1, 0);
    if (bytes_read <= 0) return;
    buffer[bytes_read] = '\0';

    // 解析HTTP请求
    parse_http_request(buffer, method, url, protocol);

    // 根据请求类型处理请求
    if (strcmp(method, "GET") == 0) {
        char file_path[255] = {0};
        bool download;
        parse_url(url, file_path, &download);
        handle_get_request(socket_fd, file_path, download);
    } else {
        // 可以处理其他HTTP方法
    }

    close(socket_fd);
}
// 发送响应
void send_response(int socket_fd, const char *message) {
    send(socket_fd, message, strlen(message), 0);
}

int main() {
    int server_fd = setup_server();
    int epoll_fd = setup_multiplexing(server_fd);
    accept_connections(server_fd, epoll_fd);
    return 0;
}
