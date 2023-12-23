#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <pthread.h>

const int defaultServerPort = 8089;
int sock;
int is_logged_in = 0; // 共享变量，用于表示用户是否已登录

void *receive_messages(void *arg)
{
    char buffer[1024];
    while (1)
    {
        memset(buffer, 0, sizeof(buffer));
        int len = read(sock, buffer, sizeof(buffer) - 1);
        if (len > 0)
        {
            printf("%s\n", buffer);
            if (strstr(buffer, "Login successful"))
            {
                is_logged_in = 1; // 更新登录状态
            }
        }
    }
    return NULL;
}

int main(int argc, char *argv[])
{
    int serverPort = defaultServerPort; // 默认端口号
    if (argc == 2)
    {
        serverPort = atoi(argv[1]); // 使用用户指定的端口号
    }
    struct sockaddr_in serv_addr;
    sock = socket(AF_INET, SOCK_STREAM, 0);
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(serverPort);

    inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr);
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        perror("Error in connection");
        exit(1);
    };

    pthread_t thread_id;
    pthread_create(&thread_id, NULL, receive_messages, NULL);
    pthread_detach(thread_id);

    char buffer[1024] = {0};

    while (1)
    {
        if (is_logged_in == 1)
        {
            break;
        }
        printf("------------------\n(r):REDISTER\n(l)LOGIN\n(q)QUIT\n------------------\n");
        char action;
        scanf(" %c", &action); // 注意：在%c前加空格，以跳过之前留下的换行符

        if(action!='q'&&action!='r'&&action!='l'){
            printf("your inputInfo is wrong,please input again:\n");
            continue;
        }

        if (action == 'q')
        {
            break;
        }

        char username[50], password[50];
        printf("Enter username: ");
        scanf("%s", username);
        printf("Enter password: ");
        scanf("%s", password);

        if (action == 'r' || action == 'l')
        {
            const char *command = (action == 'r') ? "REGISTER" : "LOGIN";
            sprintf(buffer, "%s %s %s\n", command, username, password);
            send(sock, buffer, strlen(buffer), 0);
        }
        sleep(1);
    }

    while (1)
    {
        // 重新提示用户输入
        printf("------------------\n(c)CAHT\n(q)QUIT\n------------------\n");
        char action;
        scanf(" %c", &action);

        if (action == 'q')
        {
            break;
        }
        else if (action == 'c')
        {
            printf("Enter message: ");
            char message[1024];
            scanf(" %[^\n]s", message); // 允许输入包含空格的消息
            sprintf(buffer, "CHAT %s\n", message);
            send(sock, buffer, strlen(buffer), 0);
        }
        sleep(1);
    }

    close(sock);
    return 0;
}
