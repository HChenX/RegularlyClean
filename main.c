#include <stdio.h>
#include <time.h>
#include <string.h>
#include <malloc.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdbool.h>

#define MAX_MEMORY 1024

static char *FILE_PATH = "/data/media/0/Android/RegularlyClean/";
char mMsg[50];

void putLog(int result);

void putLogStr(char *msg);

bool killPid(char **pidArray);

void error(char *log, char *msg) {
    if (log != NULL) {
        strcpy(mMsg, log);
        putLog(0);
    }
    perror(msg);
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
    for (int i = (int) len - 1; i >= 0; i--) {
        if (value[i] == '/') {
            value[i + 1] = '\0';
            break;
        }
    }
    return value;
}

void putLogStr(char *msg) {
    strcpy(mMsg, msg);
    putLog(0);
}

void putLog(int result) { // 输出日志
    if (result >= 0 && result < MAX_MEMORY) {
        FILE *fp;
        char *nowTIme = getTime();
        fp = fopen(joint("log.txt"), "a");
        if (fp != NULL) {
            fseek(fp, 0, SEEK_END);
            fprintf(fp, "[%s] | %s\n", nowTIme, mMsg);
            fclose(fp);
        } else {
            perror("Unable to write to logs");
            exit(EXIT_FAILURE);
        }
        strcpy(mMsg, "");
        free(nowTIme);
    } else {
        printf("The copied string is incomplete!");
        strcpy(mMsg, "");
    }
}

char *getExecutablePath() {
    char *path = (char *) malloc(PATH_MAX);
    if (path == NULL) {
        error(NULL, "malloc");
        return NULL;
    }

    ssize_t len = readlink("/proc/self/exe", path, PATH_MAX - 1);
    if (len == -1) {
        free(path);
        error(NULL, "readlink");
        return NULL;
    }

    path[len] = '\0'; // 添加字符串结束符
    path = removeLastPath(path);
    return path;
}

char **findPid(char *find) {
    FILE *fp;
    char pid[MAX_MEMORY];
    char read[10];
    int count = 0;
    char **pidArray = (char **) malloc(20 * sizeof(char *));
    if (pidArray == NULL) {
        error(NULL, "findPid");
        return NULL;
    }
    snprintf(pid, sizeof(pid), "pgrep -f '%s' | grep -v $$", find);
    fp = popen(pid, "r");
    if (fp == NULL) {
        error("[W] --获取进程pid失败", "findPid");
        return NULL;
    }
    while (fgets(read, sizeof(read), fp) != NULL) {
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

bool killPid(char **pidArray) {
    for (int i = 0; pidArray[i] != NULL; i++) {
        pid_t pid = (pid_t) strtol(pidArray[i], NULL, 10);
        if (kill(pid, 0) == 0) {
            if (kill(pid, SIGTERM) == 0) {
                putLog(snprintf(mMsg, MAX_MEMORY, "[Stop] --成功终止定时进程:%d", pid));
                free(pidArray);
                return true;
            } else {
                putLog(snprintf(mMsg, MAX_MEMORY, "[W] --不存在指定进程:%d", pid));
                free(pidArray);
                return false;
            }
        }
    }
    putLogStr("[W] --终止定时进程失败");
    free(pidArray);
    return false;
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
