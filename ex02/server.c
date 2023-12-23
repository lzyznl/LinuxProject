#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <time.h>

#define defaultServerPort 8088
int personalBankingNumber = 0; // 个人银行业务办理排号
int enterpriseBankingNumber = 0; // 企业银行业务办理排号
int investBankingNumber = 0; // 投资理财银行业务办理排号
int otherBankingNumber = 0; // 投资理财银行业务办理排号

void error(const char *msg)
{
    perror(msg);
}

int create_socket()
{
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
        error("ERROR opening socket");
    return sockfd;
}

int accept_connection(int sockfd)
{
    struct sockaddr_in cli_addr;
    socklen_t clilen = sizeof(cli_addr);

    int newsockfd = accept(sockfd, (struct sockaddr *)&cli_addr, &clilen);
    if (newsockfd < 0)
        error("ERROR on accept");
    return newsockfd;
}

void bind_socket(int sockfd, int portno)
{
    struct sockaddr_in serv_addr;
    memset(&serv_addr, 0, sizeof(serv_addr)); // 使用memset替代bzero

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(portno);

    if (bind(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
        error("ERROR on binding");
}

void generate_service_number(int sockfd, const char *serviceType, int *serviceNumber, const char *logFileName)
{
    (*serviceNumber)++;

    // 获取当前时间
    time_t now = time(NULL);
    char timeStr[64];
    strftime(timeStr, sizeof(timeStr), "%Y-%m-%d %H:%M:%S", localtime(&now));

    char buffer[256];
    char fileBuffer[256];
    snprintf(buffer, 256, "------------------------\n服务时间: %s\n服务项目: %s\n流水号: %d\n------------------------\n%s", timeStr, serviceType, *serviceNumber,
             "服务选项:\n1. 个人银行业务办理\n2. 企业银行业务办理\n3. 投资理财办理\n4. 其他业务\n5.退出");
    snprintf(fileBuffer, 256, "------------------------\n服务时间: %s\n服务项目: %s\n流水号: %d\n------------------------\n", timeStr, serviceType, *serviceNumber);
    int n = write(sockfd, buffer, strlen(buffer));
    if (n < 0)
        error("ERROR writing to socket");

    // 将请求信息写入文件
    FILE *file = fopen(logFileName, "a"); // 以追加模式打开文件
    if (file == NULL)
    {
        error("ERROR opening file");
    }
    fprintf(file, "%s\n", fileBuffer); // 写入文件时也保留这个格式
    fclose(file);
}

void send_options(int sockfd)
{
    char buffer[256];
    memset(buffer, 0, 256); // 使用memset替代bzero
    strcpy(buffer, "服务选项:\n1. 个人银行业务办理\n2. 企业银行业务办理\n3. 投资理财办理\n4. 其他业务\n5.退出");
    int n = write(sockfd, buffer, strlen(buffer));
    if (n < 0)
        error("ERROR writing to socket");
}

void *handle_client(void *newsockfd_ptr)
{
    int newsockfd = *(int *)newsockfd_ptr;
    free(newsockfd_ptr);
    int exitFlag = 0;
    int first = 0;

    while (1)
    {
        if (first == 0)
        {
            send_options(newsockfd);
            first = 1;
        }
        if (exitFlag == 1)
        {
            break;
        }
        char buffer[256];
        int n = read(newsockfd, buffer, 255);
        if (n < 0)
            error("ERROR reading from socket");

        int choice;
        sscanf(buffer, "%d", &choice);

        switch (choice)
        {
        case 1:
            generate_service_number(newsockfd, "个人银行业务", &personalBankingNumber, "personal_banking_requests.log");
            break;
        case 2:
            generate_service_number(newsockfd, "企业银行业务", &enterpriseBankingNumber, "enterprise_banking_requests.log");
            break;
        case 3:
            generate_service_number(newsockfd, "投资理财银行业务", &investBankingNumber, "invest_banking_requests.log");
            break;
        case 4:
            generate_service_number(newsockfd, "其它银行业务", &otherBankingNumber, "other_banking_requests.log");
            break;
        case 5:
            exitFlag = 1;
        // 可以添加更多业务类型的处理
        default:
            break;
        }
    }
    close(newsockfd);
    return NULL;
}

int main(int argc, char *argv[])
{
    int serverPort = defaultServerPort; // 默认端口号
    if (argc == 2)
    {
        serverPort = atoi(argv[1]); // 使用用户指定的端口号
    }
    int sockfd, newsockfd;

    sockfd = create_socket();
    bind_socket(sockfd, serverPort);
    listen(sockfd, 5);
    printf("Server is listening on port %d...\n", serverPort);

    while (1)
    {
        newsockfd = accept_connection(sockfd);

        int *newsockfd_ptr = malloc(sizeof(int));
        *newsockfd_ptr = newsockfd;
        pthread_t thread;
        if (pthread_create(&thread, NULL, handle_client, newsockfd_ptr) != 0)
        {
            error("ERROR creating thread");
        }
        pthread_detach(thread);
    }

    printf("aaaaaa");
    close(sockfd);
    return 0;
}
