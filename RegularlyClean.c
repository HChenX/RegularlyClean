#include <stdio.h>
#include <time.h>
#include <string.h>
#include <malloc.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdbool.h>

#define MAX_MEMORY 1024

static char FILE_PATH[50] = "/data/media/0/Android/RegularlyClean/";
char mMsg[200];
char **appArray;

void logInt(int result);

void logStr(char *msg);

bool killPid(char **pidArray);

char **config(char *check);

char *vaPrintf(char *msg, va_list vaList);

void reportErrorExit(char *log, char *msg) { // 报告错误并退出
    perror(msg);
    if (log != NULL) {
        logStr(log);
    }
    exit(EXIT_FAILURE);
}

char *getTime() { // 获取当前时间
    int TIME_PATH = 30; // 目测最少需要的字符数
    time_t curr_time;
    struct tm *local_time;
    char mTime[TIME_PATH];
    curr_time = time(NULL);
    local_time = localtime(&curr_time);
    strftime(mTime, TIME_PATH, "%g/%m/%d %H:%M", local_time);
    return strdup(mTime);
}

long getVagueTime() {
    long vagueTime;
    time_t curr_time;
    vagueTime = time(&curr_time);
    return vagueTime;
}

char *removeLinefeed(char *value) { // 移除换行符
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
    for (int i = (int) len - 1; i >= 0; i--) {
        if (value[i] == '/') {
            value[i] = '\0';
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
        char *nowTime = getTime();
        fp = fopen(joint("log.txt"), "a");
        if (fp != NULL) {
            fseek(fp, 0, SEEK_END);
            fprintf(fp, "[%s] | %s\n", nowTime, mMsg);
            fclose(fp);
        } else {
            perror("Unable to write to logs");
            exit(EXIT_FAILURE);
        }
        strcpy(mMsg, "");
        free(nowTime);
    } else {
        printf("The copied string is incomplete!");
        strcpy(mMsg, "");
    }
}

char *getModePath() { // 获取模块目录
    char *path = (char *) malloc(MAX_MEMORY);
    if (path == NULL) {
        reportErrorExit("[E] --获取模块目录失败", "malloc");
        return NULL;
    }

    ssize_t len = readlink("/proc/self/exe", path, MAX_MEMORY - 1);
    if (len == -1) {
        free(path);
        reportErrorExit("[E] --获取模块目录失败", "readlink");
        return NULL;
    }

//    path[len] = '\0'; // 添加字符串结束符
    path = removeLastPath(path);
    return path;
}

char *vaPrintf(char *msg, va_list vaList) { // 格式化字符串
    int size = vsnprintf(NULL, 0, msg, vaList);
    if (size < 0) {
        reportErrorExit(NULL, "vaPrintf");
        return NULL;
    }
    char *mCommand = (char *) malloc((size + 1) * sizeof(char));
    if (mCommand == NULL) {
        reportErrorExit(NULL, "vaPrintf");
        return NULL;
    }
    vsnprintf(mCommand, size + 1, msg, vaList);
    return mCommand;
}

FILE *createShell(char *command, ...) { // 创建 Shell
    FILE *fp;
    va_list vaList;
    va_start(vaList, command);
    char *mCommand = vaPrintf(command, vaList);
    va_end(vaList);
    fp = popen(mCommand, "r");
    if (fp == NULL) {
        free(mCommand);
        reportErrorExit(NULL, "createShell");
        return NULL;
    }
    free(mCommand);
    return fp;
}

char **findPid(char *find) { // 查找 PID
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
        pidArray[count] = strdup(result);
        if (pidArray[count] == NULL) {
            reportErrorExit(NULL, "findPid");
        }
        count = count + 1;
    }
    pidArray[count] = NULL;
    pclose(fp);
    return pidArray;
}

bool killPid(char **pidArray) { // 杀死指定 PID
    bool result = false;
    for (int i = 0; pidArray[i] != NULL; i++) {
        pid_t pid = (pid_t) strtol(pidArray[i], NULL, 10);
        free(pidArray[i]);
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

char **config(char *check) { // 读取配置
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

_Noreturn void whenWhile(bool foregroundClear, bool clearOnce,
                         bool stateCheck, char **apps) {
    bool lastState = true;
    bool shouldSleep = false;
    bool isForeground = false;
    bool isCleared = false;
    long lastTime = 0;
    while (true) {
        if (foregroundClear) {
            if (shouldSleep) {
                long nowTime = getVagueTime();
                if ((nowTime - lastTime) > 120) {
                    shouldSleep = false;
                }
            } else {
                for (int i = 0; apps[i] != NULL; i++) {
                    char *app = apps[i];
                    isForeground = foregroundApp(app);
                    if (isForeground) break;
                }
                if (isForeground) {
                    if (clearOnce) {
                        if (!isCleared) {
                            runSh("", "");
                            isCleared = true;
                        }
                    } else {
                        runSh("", "");
                    }
                    lastTime = getVagueTime();
                    shouldSleep = true;
                } else {
                    if (clearOnce) isCleared = false;
                }
            }
        }
        if (stateCheck) {
            char file[MAX_MEMORY];
            char *path = getModePath();
            snprintf(file, MAX_MEMORY, "%s/disable", path);
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
        sleep(5);
    }
}

void cleanup() {
    for (int i = 0; appArray[i] != NULL; i++) {
        free(appArray[i]);
    }
    free(appArray);
}

int main() {
    bool foregroundClear = strcmp(onlyReadOne(config("auto_clear")), "y") == 0;
    bool clearOnce = strcmp(onlyReadOne(config("clear_only_once")), "y") == 0;
    bool stateCheck = strcmp(onlyReadOne(config("state_check")), "y") == 0;
    appArray = config("app_map");
    atexit(cleanup);
    whenWhile(foregroundClear, clearOnce, stateCheck, appArray);
    /*for (int i = 0; appArray[i] != NULL; i++) {
        printf("app: %s con: %d\n", appArray[i], i);
        free(appArray[i]);
    }
    free(appArray);
    char file[MAX_MEMORY];
    char *path = getModePath();
    snprintf(file, MAX_MEMORY, "%s/disable", path);
    printf("path:%s\nforeground:%d state:%d\n", file, foregroundClear, stateCheck);
    free(path);*/
//    whenWhile(foregroundClear, stateCheck);
//    return 0;
}
