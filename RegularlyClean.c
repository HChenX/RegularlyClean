#include <stdio.h>
#include <time.h>
#include <string.h>
#include <malloc.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdbool.h>

#define MAX_MEMORY 1024

static char FILE_PATH[50] = "/data/media/0/Android/RegularlyClean/";
char mMsg[50];

void logInt(int result);

void logStr(char *msg);

bool killPid(char **pidArray);

char **config(char *check);

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
    char result[MAX_MEMORY];
    strcpy(result, FILE_PATH);
    return strcat(result, value);
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
        reportErrorExit("[E] --获取模块目录失败", "malloc");
        return NULL;
    }

    ssize_t len = readlink("/proc/self/exe", path, PATH_MAX - 1);
    if (len == -1) {
        free(path);
        reportErrorExit("[E] --获取模块目录失败", "readlink");
        return NULL;
    }

    path[len] = '\0'; // 添加字符串结束符
    path = removeLastPath(path);
    return path;
}

FILE *createShell(char *command, ...) {
    FILE *fp;
    char mCommand[MAX_MEMORY];
    va_list vaList;
    va_start(vaList, command);
    vsnprintf(mCommand, MAX_MEMORY, command, vaList);
    va_end(vaList);
    fp = popen(mCommand, "r");
    if (fp == NULL) {
        reportErrorExit("[W] --创建Shell窗口失败", "createShell");
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
    fp = createShell("pgrep -f '%s' | grep -v $$", find);
    while (fgets(read, sizeof(read), fp) != NULL) {
        char *result = removeLinefeed(read);
        pidArray[count] = result;
        if (pidArray[count] == NULL) {
            reportErrorExit(NULL, "findPid");
        }
        count = count + 1;
    }
    pidArray[count] = NULL;
    pclose(fp);
    return pidArray;
}

bool killPid(char **pidArray) {
    bool result = false;
    for (int i = 0; pidArray[i] != NULL; i++) {
        pid_t pid = (pid_t) strtol(pidArray[i], NULL, 10);
        if (kill(pid, 0) == 0) {
            if (kill(pid, SIGTERM) == 0) {
                logInt(snprintf(mMsg, MAX_MEMORY, "[Stop] --成功终止定时进程:%d", pid));
                result = true;
            } else {
                logInt(snprintf(mMsg, MAX_MEMORY, "[W] --不存在指定进程:%d", pid));
                result = false;
                break;
            }
        }
    }
    free(pidArray);
    return result;
}

void changeSed(char *path) {
    FILE *fp;
    fp = createShell(
            "sed -i \"/^description=/c description=CROND: [定时进程已经终止，等待恢复中···]\" \"%smodule.prop\"", path);
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
    fp = createShell("su -c \"dumpsys window | grep \"%s\" | grep \"mFocusedApp\"\"", pkg);
    int have = getc(fp);
    pclose(fp);
    if (have == EOF) {
        return false;
    } else {
        return true;
    }
}

char **config(char *check) {
    char *path = joint("config.ini");
    char read[MAX_MEMORY];
    char *name;
    char *value;
    int count = 0;
    bool keep = false;
    char ch = '#';
    FILE *fp;
    fp = fopen(path, "r");
    if (fp == NULL) {
        reportErrorExit("[W] --读取配置文件失败", "config");
    }
    char **valueArray = (char **) malloc(20 * sizeof(char *));
    if (valueArray == NULL) {
        fclose(fp);
        reportErrorExit(NULL, "config");
        return NULL;
    }
    while (fgets(read, MAX_MEMORY, fp) != NULL) {
        strcpy(read, removeLinefeed(read));
        char *result = strchr(read, ch);
        free(name);
        free(value);
        name = NULL;
        value = NULL;
        if (result == NULL) {
            char *token = strtok(read, "=");
            if (token != NULL || keep) {
                name = strdup(token);
                token = strtok(NULL, "=");
                if (token != NULL || keep) {
                    if (!keep) value = strdup(token);
                    else value = strdup(read);
                    if (strcmp(value, "\"") == 0 && keep) {
                        keep = false;
                        continue;
                    }
                    if (strcmp(value, "\"") == 0 && strcmp(name, check) == 0) {
                        keep = true;
                        continue;
                    } else {
                        if (keep) {
                            valueArray[count] = strdup(read);
                            count = count + 1;
                            continue;
                        } else {
                            if (strcmp(name, check) == 0) {
                                valueArray[0] = strdup(value);
                                count = 1;
                                break;
                            }
                        }
                    }
                }
            }
        } else {
            continue;
        }
    }
    valueArray[count] = NULL;
    fclose(fp);
    return valueArray;
}

char *onlyReadOne(char **valueArray) {
    char *read = valueArray[0];
    if (read == NULL) {
        read = "n";
    }
    free(valueArray);
    return read;
}

_Noreturn void whenWhile(bool foregroundClear, bool stateCheck) {
    bool lastState = true;
    while (true) {
        if (foregroundClear) {
            bool mt = foregroundApp("bin.mt.plus");
            if (mt) {
                runSh("", "");
            }
        }
        if (stateCheck) {
            char file[MAX_MEMORY];
            char *path = getModePath();
            snprintf(file, PATH_MAX, "%s/disable", path);
            if (access(file, 0) == 0 && lastState) {
                char **pids = findPid("RegularlyClean");
                if (!killPid(pids)) {
                    logStr("[E] --停止模块定时进程失败");
                } else {
                    changeSed(path);
                    lastState = false;
                }
            } else if (access(file, 0) != 0 && !lastState) {
                runSh("", "");
                lastState = true;
            }
            free(path);
        }
        sleep(1);
    }
}

int main() {
    bool foregroundClear = strcmp(onlyReadOne(config("auto_clear")), "y");
    bool stateCheck = strcmp(onlyReadOne(config("state_check")), "y");
    char **appArray = config("app_map");
    for (int i = 0; appArray[i] != NULL; i++) {
        printf("app: %s\n", appArray[i]);
    }
    free(appArray);
    printf("cl: %d,check: %d\n", foregroundClear, stateCheck);
//    whenWhile(foregroundClear, stateCheck);
    return 0;
}
