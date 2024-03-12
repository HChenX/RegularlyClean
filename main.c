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

void logInt(int result);

void logStr(char *msg);

bool killPid(char **pidArray);

char *config(char *check);

void reportErrorExit(char *log, char *msg) {
    perror(msg);
    if (log != NULL) {
        logStr(log);
    }
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

void logStr(char *msg) {
    strcpy(mMsg, "");
    strcpy(mMsg, msg);
    logInt(0);
}

void logInt(int result) { // 输出日志
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

char *getModePath() {
    char *path = (char *) malloc(PATH_MAX);
    if (path == NULL) {
        reportErrorExit("[E] --获取模块目录失败\n", "malloc");
        return NULL;
    }

    ssize_t len = readlink("/proc/self/exe", path, PATH_MAX - 1);
    if (len == -1) {
        free(path);
        reportErrorExit("[E] --获取模块目录失败\n", "readlink");
        return NULL;
    }

    path[len] = '\0'; // 添加字符串结束符
    path = removeLastPath(path);
    return path;
}

FILE *createShell(char *command, char *va1, char *va2) {
    FILE *fp;
    char mCommand[MAX_MEMORY];
    if (va1 == NULL && va2 == NULL) {
        snprintf(mCommand, MAX_MEMORY, "%s", command);
    } else if (va1 != NULL && va2 != NULL) {
        snprintf(mCommand, MAX_MEMORY, command, va1, va2);
    } else if (va1 != NULL) {
        snprintf(mCommand, MAX_MEMORY, command, va1);
    } else {
        snprintf(mCommand, MAX_MEMORY, command, va2);
    }
    fp = popen(mCommand, "r");
    if (fp == NULL) {
        reportErrorExit("[W] --创建Shell窗口失败\n", "createShell");
    }
    return fp;
}

char **findPid(char *find) {
    FILE *fp;
    char read[10];
    int count = 0;
    char **pidArray = (char **) malloc(20 * sizeof(char *));
    if (pidArray == NULL) {
        reportErrorExit(NULL, "findPid");
        return NULL;
    }
    fp = createShell("pgrep -f '%s' | grep -v $$", find, NULL);
    while (fgets(read, sizeof(read), fp) != NULL) {
        char *result = removeLinefeed(read);
        pidArray[count] = result;
        if (pidArray[count] == NULL) {
            reportErrorExit(NULL, "findPid");
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
                logInt(snprintf(mMsg, MAX_MEMORY, "[Stop] --成功终止定时进程:%d\n", pid));
                free(pidArray);
                return true;
            } else {
                logInt(snprintf(mMsg, MAX_MEMORY, "[W] --不存在指定进程:%d\n", pid));
                free(pidArray);
                return false;
            }
        }
    }
    logStr("[W] --终止定时进程失败");
    free(pidArray);
    return false;
}

void changeSed(char *path) {
    FILE *fp;
    fp = createShell(
            "sed -i \"/^description=/c description=CROND: [定时进程已经终止，等待恢复中···]\" \"%smodule.prop\"", path,
            NULL);
    fflush(fp);
    pclose(fp);
}

void runSh(char *path, char *value) {
    FILE *fp;
    fp = createShell("su -c \"sh %s%s &>/dev/null\"", path, value);
    fflush(fp);
    pclose(fp);
}

bool foregroundApp(char *pkg) {
    FILE *fp;
    fp = createShell("su -c \"dumpsys window | grep \"%s\" | grep \"mFocusedApp\"\"", pkg, NULL);
    int have = getc(fp);
    pclose(fp);
    if (have == EOF) {
        return false;
    } else {
        return true;
    }
}

char *config(char *check) {
    char *path = joint("ModuleConfig.ini");
    char read[MAX_MEMORY];
    char *name;
    char *value;
    char *end = NULL;
    char ch = '#';
    FILE *fp;
    fp = fopen(path, "r");
    if (fp == NULL) {
        reportErrorExit("[W] --读取配置文件失败\n", "config");
    }
    while (fgets(read, MAX_MEMORY, fp) != NULL) {
        strcpy(read, removeLinefeed(read));
        char *result = strchr(read, ch);
        name = NULL;
        value = NULL;
        end = NULL;
        if (result != NULL) {
            char *token = strtok(read, "=");
            if (token != NULL) {
                name = token;
                token = strtok(NULL, "=");
                if (token != NULL) {
                    value = token;
                    if (strcmp(name, check) == 0) {
                        end = strdup(value);
                        break;
                    }
                }
            }
        } else {
            continue;
        }
    }
    fclose(fp);
    return strdup(end);
}

int main() {
    char **pidList = findPid("home");
    if (pidList == NULL) {
        reportErrorExit(NULL, "List is Null");
        return 1;
    }
    for (int i = 0; pidList[i] != NULL; i++) {
        printf("PID: %s", pidList[i]);
    }
    free(pidList);
    return 0;
}
