#include <json-c/json.h>
#include <netinet/in.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define MAX_CLIENTS 10
#define defaultServerPort 8089

char userDict[MAX_CLIENTS][50];
int client_sockets[MAX_CLIENTS] = {0}; // 存储活跃的客户端套接字

// 从文件加载用户信息
json_object *load_users()
{
    FILE *file = fopen("users.json", "r");
    if (!file)
        return NULL;

    fseek(file, 0, SEEK_END);
    long length = ftell(file);
    fseek(file, 0, SEEK_SET);

    char *data = (char *)malloc(length + 1);
    fread(data, 1, length, file);
    data[length] = '\0';

    json_object *users = json_tokener_parse(data);
    free(data);
    fclose(file);
    return users;
}

// 存储用户信息
void store_user(const char *username, const char *password)
{
    // 加载现有用户
    json_object *users = load_users();
    if (!users)
    {
        users = json_object_new_object(); // 如果文件不存在或为空，则创建新的JSON对象
    }

    // 添加或更新用户信息
    json_object *j_password = json_object_new_string(password);
    json_object_object_add(users, username, j_password);

    // 重新写入整个用户列表
    FILE *file = fopen("users.json", "w");
    if (file != NULL)
    {
        fprintf(file, "%s", json_object_to_json_string(users));
        fclose(file);
    }
    else
    {
        // 处理文件打开错误
        perror("Error opening file for writing");
    }

    json_object_put(users);
}

// 验证用户
int validate_user(const char *username, const char *password)
{
    json_object *users = load_users();
    if (!users)
        return 0;

    json_object *j_password = NULL;
    if (json_object_object_get_ex(users, username, &j_password))
    {
        int result = strcmp(password, json_object_get_string(j_password)) == 0;
        json_object_put(users);
        return result;
    }

    json_object_put(users);
    return 0;
}
void broadcast_message(const char *message, int sender_socket)
{
    for (int i = 0; i < MAX_CLIENTS; ++i)
    {
        if (client_sockets[i] != 0)
        {
            send(client_sockets[i], message, strlen(message), 0);
        }
    }
}

void *handle_client(void *arg)
{
    int client_socket = *(int *)arg;
    free(arg); // 释放分配的套接字指针内存

    // 将新客户端添加到数组中
    for (int i = 0; i < MAX_CLIENTS; ++i)
    {
        if (client_sockets[i] == 0)
        {
            client_sockets[i] = client_socket;
            break;
        }
    }

    char buffer[1024];
    char message_buffer[1024]; // 更大的缓冲区来存储完整的消息

    while (1)
    {
        memset(buffer, 0, sizeof(buffer));
        int bytes_read = read(client_socket, buffer, sizeof(buffer) - 1);
        if (bytes_read <= 0)
        {
            break; // 客户端断开连接或发生错误
        }

        char command[10], username[50], password[50], message[1024];
        sscanf(buffer, "%s", command);

        if (strcmp(command, "REGISTER") == 0)
        {
            sscanf(buffer, "%s %s %s", command, username, password);
            store_user(username, password);
            send(client_socket, "Registration successful\n", 25, 0);
        }
        else if (strcmp(command, "LOGIN") == 0)
        {
            sscanf(buffer, "%s %s %s", command, username, password);
            if (validate_user(username, password))
            {
                strcpy(userDict[client_socket % MAX_CLIENTS], username);
                send(client_socket, "Login successful\n", 18, 0);
            }
            else
            {
                send(client_socket, "Login failed\n", 13, 0);
            }
        }
        else if (strcmp(command, "CHAT") == 0)
        {
            sscanf(buffer, "%s %[^\n]", command, message);
            snprintf(message_buffer, sizeof(message_buffer), "[%s]: %s\n", userDict[client_socket % MAX_CLIENTS], message);
            printf("[%s]: %s\n", userDict[client_socket % MAX_CLIENTS], message);
            broadcast_message(message_buffer, client_socket); // 转发消息给其他客户端
        }
    }

    // 断开连接后从数组中移除客户端
    for (int i = 0; i < MAX_CLIENTS; ++i)
    {
        if (client_sockets[i] == client_socket)
        {
            client_sockets[i] = 0;
            break;
        }
    }

    close(client_socket);
    return NULL;
}

int main(int argc, char *argv[])
{
    int serverPort = defaultServerPort; // 默认端口号
    if (argc == 2)
    {
        serverPort = atoi(argv[1]); // 使用用户指定的端口号
    }
    int server_fd;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);

    memset(userDict, 0, sizeof(userDict));

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt));
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(serverPort);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0)
    {
        perror("can't bind local address");
        exit(1);
    };
    listen(server_fd, 3);
    printf("Server is listening on port %d...\n", serverPort);

    while (1)
    {
        int *new_socket = malloc(sizeof(int));
        *new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen);
        if (*new_socket == -1)
        {
            free(new_socket);
            continue; // 处理 accept 错误
        }

        pthread_t thread_id;
        if (pthread_create(&thread_id, NULL, handle_client, new_socket) != 0)
        {
            perror("Could not create thread");
            free(new_socket);
            continue; // 处理线程创建错误
        }

        pthread_detach(thread_id); // 让线程结束时自动释放资源
    }

    close(server_fd);
    return 0;
}
