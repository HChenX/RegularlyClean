#include <stdio.h>
#include <time.h>
#include <string.h>
#include <malloc.h>
#include <unistd.h>
#include <stdlib.h>

static char *FILE_PATH = "/data/media/0/Android/RegularlyClean/";
char mMsg[50];

void putLog(char *put);

void error(char *log, char *msg) {
    putLog(log);
    perror(msg);
    strcpy(mMsg, "");
    exit(EXIT_FAILURE);
}

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

void putLog(char *put) { // 输出日志
    if (put == NULL) {
        return;
    }
    FILE *fp;
    char *nowTIme = getTime();
    fp = fopen(joint("log.txt"), "a");
    if (fp != NULL) {
        fseek(fp, 0, SEEK_END);
        fprintf(fp, "[%s] | %s\n", nowTIme, put);
        fclose(fp);
    } else {
        perror("Unable to write to logs");
        exit(EXIT_FAILURE);
    }
    free(nowTIme);
}

char *getExecutablePath() {
    char *path = (char *) malloc(PATH_MAX);
    if (path == NULL) {
        error(NULL, "malloc");
    }

    ssize_t len = readlink("/proc/self/exe", path, PATH_MAX - 1);
    if (len == -1) {
        free(path);
        error(NULL, "readlink");
    }

    path[len] = '\0'; // 添加字符串结束符
    path = removeLastPath(path);
    return path;
}

char **findPid(char *find) {
    FILE *fp;
    char pid[PATH_MAX];
    char read[PATH_MAX];
    int count = 0;
    char **pidArray = (char **) malloc(20 * sizeof(char *));
    if (pidArray == NULL) {
        error(NULL, "findPid");
        return NULL;
    }
    snprintf(pid, PATH_MAX, "pgrep -f '%s' | grep -v $$", find);
    fp = popen(pid, "r");
    if (fp == NULL) {
        error("[W] --获取进程pid失败", "findPid");
        return NULL;
    }
    while (fgets(read, PATH_MAX, fp) != NULL) {
        char *result = removeLinefeed(read);
        pidArray[count] = result;
        if (pidArray[count] == NULL) {
            error(NULL, "findPid");
        }
        count = count + 1;
    }
    pidArray[count + 1] = NULL;
    pclose(fp);
    return pidArray;
}

int main() {
    char **pidList = findPid("home");
    if (pidList == NULL) {
        error(NULL, "List is Null");
        return 1;
    }
    for (int i = 0; pidList[i] != NULL; i++) {
        printf("PID: %s", pidList[i]);
    }
    free(pidList);
    return 0;
}
