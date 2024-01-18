#include <microhttpd.h>
#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <string.h>
#include <pthread.h>
#include <time.h>

// 日志文件名
const char *log_filename = "server_log.txt";

//用于记录日志
static void log_request(const char *client_ip, const char *method, const char *url, int status_code) {
    FILE *log_file = fopen(log_filename, "a");
    if (log_file == NULL) {
        perror("Error opening log file");
        return;
    }

    // 获取当前时间并格式化
    time_t now = time(NULL);
    char time_str[64]; // 足够大，用于容纳格式化后的时间
    struct tm *tm_now = localtime(&now);
    strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", tm_now);

    // 写入日志，格式化输出
    fprintf(log_file, "[%s] - %s - %s - %s - %d\n", time_str, client_ip, method, url, status_code);
    fclose(log_file);
}


static int send_file(struct MHD_Connection *connection, const char *filepath, int download)
{
    FILE *file = fopen(filepath, "rb");
    if (file == NULL)
    {
        // 文件不存在，发送 404 Not Found
        return MHD_NO;
    }

    // 获取文件大小
    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    fseek(file, 0, SEEK_SET);

    // 读取文件内容
    char *buffer = malloc(size);
    if (buffer == NULL)
    {
        fclose(file);
        return MHD_NO;
    }
    fread(buffer, 1, size, file);
    fclose(file);

    struct MHD_Response *response = MHD_create_response_from_buffer(size, buffer, MHD_RESPMEM_MUST_FREE);
    // 设置 Content-Type 头部，假设文件是 UTF-8 编码的文本
    MHD_add_response_header(response, "Content-Type", "text/plain; charset=UTF-8");
    // 如果是下载请求，添加适当的头部
    if (download)
    {
        // 提取文件名
        const char *filename = strrchr(filepath, '/');
        if (filename == NULL)
        {
            filename = filepath; // 没有路径分隔符，整个路径就是文件名
        }
        else
        {
            filename++; // 跳过路径分隔符
        }

        // 设置 Content-Disposition 头部
        char cd_header[1024];
        snprintf(cd_header, sizeof(cd_header), "attachment; filename=\"%s\"", filename);
        MHD_add_response_header(response, "Content-Disposition", cd_header);
    }

    int ret = MHD_queue_response(connection, MHD_HTTP_OK, response);
    MHD_destroy_response(response);
    return ret;
}

// 获取客户端 IP 地址
static void get_client_ip(char *ip_str, const struct sockaddr *addr)
{
    if (addr->sa_family == AF_INET)
    {
        // IPv4 地址
        struct sockaddr_in *addr_in = (struct sockaddr_in *)addr;
        inet_ntop(AF_INET, &(addr_in->sin_addr), ip_str, INET_ADDRSTRLEN);
    }
    else if (addr->sa_family == AF_INET6)
    {
        // IPv6 地址
        struct sockaddr_in6 *addr_in6 = (struct sockaddr_in6 *)addr;
        inet_ntop(AF_INET6, &(addr_in6->sin6_addr), ip_str, INET6_ADDRSTRLEN);
    }
    else
    {
        // 未知或不支持的地址类型
        strcpy(ip_str, "Unknown Address Type");
    }
}

static int request_handler(void *cls, struct MHD_Connection *connection,
                           const char *url, const char *method,
                           const char *version, const char *upload_data,
                           size_t *upload_data_size, void **con_cls)
{

    const union MHD_ConnectionInfo *conn_info = MHD_get_connection_info(connection, MHD_CONNECTION_INFO_CLIENT_ADDRESS);
    char client_ip[INET6_ADDRSTRLEN]; // 大到足以容纳 IPv6 地址

    get_client_ip(client_ip, conn_info->client_addr);

    int ret;
    if (strcmp(method, "GET") == 0)
    {
        // 解析查询字符串以获取 path 和 type
        const char *path = MHD_lookup_connection_value(connection, MHD_GET_ARGUMENT_KIND, "path");
        const char *type = MHD_lookup_connection_value(connection, MHD_GET_ARGUMENT_KIND, "type");

        // 检查 path 参数
        if (path == NULL)
        {
            // 如果没有提供 path，则返回 400 Bad Request
            ret = MHD_queue_response(connection, MHD_HTTP_BAD_REQUEST, NULL);
            return ret;
        }

        int download = 0;
        if (type != NULL && strcmp(type, "download") == 0)
        {
            download = 1;
        }

        // 使用解析出的 path 作为文件路径
        ret = send_file(connection, path, download);

        // 记录日志（包括请求的响应状态码）
        log_request(client_ip, method, url, MHD_HTTP_OK); // 或使用实际的响应状态码
        return ret;
    }

    // 对于非 GET 请求，返回 400 Bad Request
    ret = MHD_queue_response(connection, MHD_HTTP_BAD_REQUEST, NULL);

    return ret;
}

int main()
{
    struct MHD_Daemon *daemon;

    daemon = MHD_start_daemon(MHD_USE_THREAD_PER_CONNECTION, 8088, NULL, NULL,
                              &request_handler, NULL, MHD_OPTION_END);
    if (daemon == NULL)
        return 1;

    printf("HTTP Server is running on port 8088\n");
    getchar(); // 等待用户输入，以便保持服务器运行

    MHD_stop_daemon(daemon);
    return 0;
}
