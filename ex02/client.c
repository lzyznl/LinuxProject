#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#define defaultServerPort 8088

void error(const char *msg)
{
    perror(msg);
    exit(0);
}

int create_socket()
{
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
        error("ERROR opening socket");
    return sockfd;
}

void connect_to_server(int sockfd, const char *hostname, int portno)
{
    struct hostent *server;
    struct sockaddr_in serv_addr;

    server = gethostbyname(hostname);
    if (server == NULL)
    {
        fprintf(stderr, "ERROR, no such host\n");
        exit(0);
    }

    memset(&serv_addr, 0, sizeof(serv_addr)); // 使用memset替代bzero
    serv_addr.sin_family = AF_INET;
    memcpy(&serv_addr.sin_addr.s_addr, server->h_addr_list[0], server->h_length); // 使用memcpy替代bcopy
    serv_addr.sin_port = htons(portno);

    if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
        error("ERROR connecting");
}

void receive_options(int sockfd)
{
    char buffer[256];
    memset(buffer, 0, 256); // 使用memset替代bzero
    int n = read(sockfd, buffer, 255);
    if (n < 0)
        error("ERROR reading from socket");
    printf("%s\n", buffer);
}

void interact_with_server(int sockfd)
{
    char buffer[256];
    int choice, n;
    int first = 1;

    while (1)
    {
        if (first == 1)
        {
            receive_options(sockfd);
            first = 0;
        }

        while (1)
        {
            printf("Please Input Your Choice: ");
            if (fgets(buffer, sizeof(buffer), stdin) == NULL)
            {
                printf("Invalid input. Please enter a number between 1 and 5.\n");
                continue; // 在读取失败时继续尝试
            }

            if (sscanf(buffer, "%d", &choice) == 1)
            {
                if (choice >= 1 && choice <= 5)
                {
                    break; // 如果输入有效，跳出内部循环
                }
            }
            printf("Invalid input. Please enter a number between 1 and 5.\n");
        }

        sprintf(buffer, "%d", choice); // 把选择转换为字符串
        n = write(sockfd, buffer, strlen(buffer));
        if (n < 0)
            error("ERROR writing to socket");

        if (choice == 5)
        {
            printf("Exit successfully.\n");
            break;
        }

        // 接收服务器的响应
        memset(buffer, 0, 256);
        n = read(sockfd, buffer, 255);
        if (n < 0)
        {
            error("ERROR reading from socket");
        }
        printf("Server response:\n %s\n", buffer);

        // 根据选择来处理不同的逻辑
    }
}

int main(int argc, char *argv[])
{
    int serverPort = defaultServerPort; // 默认端口号
    if (argc == 2)
    {
        serverPort = atoi(argv[1]); // 使用用户指定的端口号
    }
    int sockfd;
    const char *hostname = "localhost"; // 服务器地址

    sockfd = create_socket();
    connect_to_server(sockfd, hostname, serverPort);
    interact_with_server(sockfd);

    close(sockfd);
    return 0;
}
