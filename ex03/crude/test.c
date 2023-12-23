#include <stdio.h>
#include <string.h>

void extract_parameters(const char *request, char *path, char *type) {
    const char *path_prefix = "path=";
    const char *type_prefix = "type=";
    char *path_start, *type_start, *param_end;

    // 初始化path和type为空字符串
    path[0] = '\0';
    type[0] = '\0';

    // 查找path参数
    path_start = strstr(request, path_prefix);
    if (path_start) {
        path_start += strlen(path_prefix);
        param_end = strpbrk(path_start, " &"); // 查找参数结束的位置
        if (!param_end) param_end = strchr(path_start, ' ');
        if (param_end) {
            strncpy(path, path_start, param_end - path_start);
            path[param_end - path_start] = '\0';
        } else {
            strcpy(path, path_start); // 复制直到行结束
        }
    }

    // 查找type参数
    type_start = strstr(request, type_prefix);
    if (type_start) {
        type_start += strlen(type_prefix);
        param_end = strpbrk(type_start, " &"); // 查找参数结束的位置
        if (!param_end) param_end = strchr(type_start, ' ');
        if (param_end) {
            strncpy(type, type_start, param_end - type_start);
            type[param_end - type_start] = '\0';
        } else {
            strcpy(type, type_start); // 复制直到行结束
        }
    }
}

int main() {
    char request[] = "GET /?path=/home/lzyyyy/pythonProject/LinuxFinalProject/ex03/makefile HTTP/1.1";
    char path[1024], type[1024];

    extract_parameters(request, path, type);

    printf("Path: %s\n", path);
    printf("Type: %s\n", type);

    return 0;
}
