#include <stdio.h>
#include <time.h>
#include <string.h>
#include <malloc.h>
#include <unistd.h>
#include <stdlib.h>

static char *FILE_PATH = "/data/media/0/Android/RegularlyClean/";

char *getTime() { // 获取当前时间
    int TIME_PATH = 14; // 目测最少需要的字符数
    time_t curr_time;
    struct tm *local_time;
    char mTime[TIME_PATH];
    curr_time = time(NULL);
    local_time = localtime(&curr_time);
    strftime(mTime, TIME_PATH, "%g/%m/%d %H:%M", local_time);
    return strdup(mTime);
}

char *removeLinefeed(char *value) {
    size_t len = strlen(value);
    if (len > 0 && value[len - 1] == '\n') {
        value[len - 1] = '\0';
    }
    return value;
}

char *joint(char *value) {
    return strcat(FILE_PATH, value);
}

char *removeLastPath(char *value) { // 移除路径最后的字段
    size_t len = strlen(value);
    if (value[len - 1] == '/') {
        return value;
    }
    for (int i = len - 1; i >= 0; i--) {
        if (value[i] == '/') {
            value[i + 1] = '\0';
            break;
        }
    }
    return value;
}

void outPut(char *put) { // 输出日志
    FILE *fp;
    char *nowTIme = getTime();
    fp = fopen(joint("log.txt"), "a");
    if (fp != NULL) {
        fseek(fp, 0, SEEK_END);
        fprintf(fp, "[%s] | %s\n", nowTIme, put);
        fclose(fp);
    } else {
        printf("[%s] | [W] --打开日志文件失败\n", nowTIme);
    }
    free(nowTIme);
}

char *getExecutablePath() {
    char *path = (char *) malloc(PATH_MAX);
    if (path == NULL) {
        outPut("分配内存过少！");
        perror("malloc");
        exit(EXIT_FAILURE);
    }

    ssize_t len = readlink("/proc/self/exe", path, PATH_MAX - 1);
    if (len == -1) {
        outPut("获取自身绝对路径失败！");
        perror("readlink");
        free(path);
        exit(EXIT_FAILURE);
    }

    path[len] = '\0'; // 添加字符串结束符
    return path;
}

int main() {
//    char *timeStr = getTime();
    char *path = getExecutablePath();
    printf("Hello, World:%s\n", removeLastPath(path));
    return 0;
}
