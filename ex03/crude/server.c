
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/epoll.h>
#include <pthread.h>
#include <fcntl.h>    // For open
#include <sys/stat.h> // For stat

#define MAX_EVENTS 10
#define BUFFER_SIZE 1024

void error(const char *msg)
{
    perror(msg);
    exit(1);
}

// Function to send a simple HTTP response header
void send_http_header(int sockfd, const char *status, const char *content_type, long content_length)
{
    char header[BUFFER_SIZE];
    snprintf(header, BUFFER_SIZE, "HTTP/1.1 %s\\r\\nContent-Type: %s\\r\\nContent-Length: %ld\\r\\n\\r\\n", status, content_type, content_length);
    write(sockfd, header, strlen(header));
}

// Function to send the content of a file
void send_file_content(int sockfd, const char *filepath)
{
    FILE *file = fopen(filepath, "rb");
    if (file == NULL)
    {
        send_http_header(sockfd, "404 Not Found", "text/plain", 0);
        return;
    }

    // Seek to the end of the file to determine its size
    fseek(file, 0, SEEK_END);
    long filesize = ftell(file);
    fseek(file, 0, SEEK_SET);

    // Send HTTP header
    send_http_header(sockfd, "200 OK", "text/plain; charset=utf-8", filesize);

    // Send file content
    char buffer[BUFFER_SIZE];
    size_t bytes_read;
    while ((bytes_read = fread(buffer, 1, BUFFER_SIZE, file)) > 0)
    {
        write(sockfd, buffer, bytes_read);
    }

    fclose(file);
}

void parse_query_string(const char *query, char *path, char *type)
{
    // This function should parse the query string and extract 'path' and 'type' parameters
    // For simplicity, it assumes the query string is well-formed and parameters are URL-decoded
    // TODO: Implement robust parsing and URL decoding

    // Example implementation (very basic and not robust)
    sscanf(query, "path=%s&type=%s", path, type);
}

void *handleRequest(void *newsockfd_ptr)
{
    int newsockfd = *((int *)newsockfd_ptr);
    free(newsockfd_ptr);

    char request[BUFFER_SIZE], path[BUFFER_SIZE], type[BUFFER_SIZE];
    memset(request, 0, BUFFER_SIZE);
    memset(path, 0, BUFFER_SIZE);
    memset(type, 0, BUFFER_SIZE);
    int bytes_read = read(newsockfd, request, BUFFER_SIZE);

    FILE *file = fopen("received_request.txt", "w");
    if (file != NULL)
    {
        fwrite(request, 1, bytes_read, file);
        fclose(file);
    }


    close(newsockfd);
    return NULL;
}

int main(int argc, char *argv[])
{
    int sockfd, portno;
    struct sockaddr_in serv_addr;
    int epollfd, nfds;
    struct epoll_event ev, events[MAX_EVENTS];

    if (argc < 2)
    {
        fprintf(stderr, "ERROR, no port provided\n");
        exit(1);
    }

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
        error("ERROR opening socket");

    memset((char *)&serv_addr, 0, sizeof(serv_addr));
    portno = atoi(argv[1]);
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(portno);

    if (bind(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
        error("ERROR on binding");

    listen(sockfd, 5);
    printf("Server is listening on port %d...\n", portno);

    epollfd = epoll_create1(0);
    if (epollfd == -1)
    {
        perror("epoll_create1");
        exit(EXIT_FAILURE);
    }

    ev.events = EPOLLIN;
    ev.data.fd = sockfd;
    if (epoll_ctl(epollfd, EPOLL_CTL_ADD, sockfd, &ev) == -1)
    {
        perror("epoll_ctl: sockfd");
        exit(EXIT_FAILURE);
    }

    for (;;)
    {
        nfds = epoll_wait(epollfd, events, MAX_EVENTS, -1);
        if (nfds == -1)
        {
            perror("epoll_wait");
            exit(EXIT_FAILURE);
        }

        for (int n = 0; n < nfds; ++n)
        {
            if (events[n].data.fd == sockfd)
            {
                struct sockaddr_in cli_addr;
                socklen_t clilen = sizeof(cli_addr);
                int newsockfd = accept(sockfd, (struct sockaddr *)&cli_addr, &clilen);
                if (newsockfd == -1)
                {
                    perror("accept");
                    exit(EXIT_FAILURE);
                }

                int *newsockfd_ptr = malloc(sizeof(int));
                if (newsockfd_ptr == NULL)
                    error("ERROR allocating memory for new socket descriptor");

                *newsockfd_ptr = newsockfd;

                pthread_t thread;
                if (pthread_create(&thread, NULL, handleRequest, newsockfd_ptr) < 0)
                    error("ERROR creating thread");

                pthread_detach(thread); // Detach the thread to free resources upon completion
            }
        }
    }

    close(sockfd);
    return 0;
}