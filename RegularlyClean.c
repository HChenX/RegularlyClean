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
char **appArray = NULL;
char *modulePath = NULL;
bool isDebug = false;

void logVa(char *msg, ...);

void logStr(char *msg);

void logDebug(char *msg);

void logDebugVa(char *msg, ...);

void logWrite();

bool killPid(char **pidArray);

char **config(char *check);

char *onlyReadOne(char **valueArray);

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

char *joint(char *value) {
    char result[MAX_MEMORY];
    strcpy(result, FILE_PATH);
    return strcat(result, value);
}

char *removeLinefeed(char *value) { // 移除换行符
    size_t len = strlen(value);
    if (len > 0 && value[len - 1] == '\n') {
        value[len - 1] = '\0';
    }
    return value;
}

char *removeLastPath(char *value, char remove) { // 移除路径最后的字段
    size_t len = strlen(value);
    for (int i = (int) len - 1; i >= 0; i--) {
        if (value[i] == remove) {
            value[i] = '\0';
            break;
        }
    }
    return value;
}

bool findCharAtEnd(char *value, char need) {
    size_t len = strlen(value);
    for (int i = (int) len - 1; i >= 0; i--) {
        if (value[i] == need) {
            return true;
        }
    }
    return false;
}

void vaLog(char *msg, va_list vaList) {
    char *result = vaPrintf(msg, vaList);
    strcpy(mMsg, "");
    strcpy(mMsg, result);
    free(result);
    result = NULL;
    logWrite();
}

void logDebug(char *msg) {
    if (isDebug) {
        logStr(msg);
    }
}

void logDebugVa(char *msg, ...) {
    if (isDebug) {
        va_list vaList;
        va_start(vaList, msg);
        vaLog(msg, vaList);
        va_end(vaList);
    }
}

void logStr(char *msg) {
    strcpy(mMsg, "");
    strcpy(mMsg, msg);
    logWrite();
}

void logVa(char *msg, ...) {
    va_list vaList;
    va_start(vaList, msg);
    // vprintf(msg, vaList);
    vaLog(msg, vaList);
    va_end(vaList);
}

void logWrite() {
    FILE *fp;
    char *nowTime = getTime();
    fp = fopen(joint("log.txt"), "a");
    if (fp != NULL) {
        fseek(fp, 0, SEEK_END);
        fprintf(fp, "[%s] | %s\n", nowTime, mMsg);
        fclose(fp);
    } else {
        perror("Unable to write to logs");
        free(nowTime);
        nowTime = NULL;
        exit(EXIT_FAILURE);
    }
    strcpy(mMsg, "");
    free(nowTime);
    nowTime = NULL;
}

char *getModePath() { // 获取模块目录
    char *path = (char *) malloc(MAX_MEMORY);
    if (path == NULL) {
        reportErrorExit("[E] --开辟内存时失败>>>getModePath()", "malloc");
        return NULL;
    }

    ssize_t len = readlink("/proc/self/exe", path, MAX_MEMORY - 1);
    if (len == -1) {
        free(path);
        path = NULL;
        reportErrorExit("[E] --获取模块目录失败>>>/proc/self/exe", "readlink");
        return NULL;
    }

    // path[len] = '\0'; // 添加字符串结束符
    path = removeLastPath(path, '/');
    return path;
}

char *vaPrintf(char *msg, va_list vaList) { // 格式化字符串
    int size = vsnprintf(NULL, 0, msg, vaList);
    if (size < 0) {
        logDebug("[E] --格式化字符串时失败>>>vaPrintf()");
        reportErrorExit(NULL, "vaPrintf");
        return NULL;
    }
    char *result = (char *) malloc((size + 1) * sizeof(char));
    if (result == NULL) {
        logDebug("[E] --开辟内存时失败>>>vaPrintf()");
        reportErrorExit(NULL, "vaPrintf");
        return NULL;
    }
    vsnprintf(result, size + 1, msg, vaList);
    return result;
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
        mCommand = NULL;
        reportErrorExit("[E] --执行Shell命令时失败,程序退出>>>createShell()",
                        "createShell");
        return NULL;
    }
    free(mCommand);
    mCommand = NULL;
    return fp;
}

char **findPid(char *find) { // 查找 PID
    FILE *fp;
    char read[10];
    int count = 0;
    char **pidArray = (char **) malloc(20 * sizeof(char *));
    if (pidArray == NULL) {
        logDebug("[E] --开辟内存时失败>>>findPid()");
        reportErrorExit(NULL, "findPid");
        return NULL;
    }
    fp = createShell("pgrep -f '%s' | grep -v $$", find);
    while (fgets(read, sizeof(read), fp) != NULL) {
        char *result = removeLinefeed(read);
        pidArray[count] = strdup(result);
        if (pidArray[count] == NULL) {
            logDebugVa("[E] --写入数据时失败,数据: %s>>>findPid()", result);
            break;
            // reportErrorExit(NULL, "findPid");
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
        pidArray[i] = NULL;
        if (kill(pid, 0) == 0) {
            if (kill(pid, SIGTERM) == 0) {
                logVa("[Stop] --成功终止定时进程:%d", pid);
                result = true;
            } else {
                logVa("[W] --不存在指定进程:%d", pid);
                result = false;
                break;
            }
        }
    }
    free(pidArray);
    pidArray = NULL;
    return result;
}

void changeSed(char *path) {
    FILE *fp;
    fp = createShell(
            "su -c sed -i \"/^description=/c description=CROND: [定时进程已经终止，等待恢复中···]\" \"%s/module.prop\"",
            path);
    fflush(fp);
    pclose(fp);
}

void runSh(char *path, char *value) {
    FILE *fp;
    fp = createShell("su -c \"sh %s%s &>/dev/null\"", path, value);
    fflush(fp);
    pclose(fp);
}

void runModuleSh(char *value) {
    if (modulePath == NULL) {
        modulePath = getModePath();
    }
    runSh(modulePath, value);
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
    char *name = NULL;
    char *value = NULL;
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
        logDebug("[E] --开辟内存时失败>>>config()");
        reportErrorExit(NULL, "config");
        return NULL;
    }
    while (fgets(read, MAX_MEMORY, fp) != NULL) {
        strcpy(read, removeLinefeed(read));
        if (strcmp(read, "") == 0) continue;
        logDebugVa("[D] --读取配置文件: %s", read);
        char *result = strchr(read, ch);
        free(name);
        free(value);
        name = NULL;
        value = NULL;
        if (result == NULL) {
            char *token = strtok(read, "=");
            if (token != NULL || keep) {
                if (token != NULL) name = strdup(token);
                token = strtok(NULL, "=");
                if (token != NULL || keep) {
                    // printf("read: %s token: %s\n", read, token);
                    if (token != NULL && keep) {
                        reportErrorExit("[E] --读取clear配置错误", "clear_error");
                        break;
                    }
                    if (keep) {
                        value = strdup(read);
                        if (findCharAtEnd(value, '"')) {
                            value = removeLastPath(value, '"');
                            valueArray[count] = strdup(value);
                            count = count + 1;
                            keep = false;
                            continue;
                        } else if (strcmp(value, "\"") == 0) {
                            keep = false;
                            continue;
                        }
                    } else value = strdup(token);
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

bool checkState(char *value) {
    return strcmp(onlyReadOne(config(value)), "y") == 0;
}

char *onlyReadOne(char **valueArray) {
    int len = 0;
    for (int i = 0; valueArray[i] != NULL; ++i) {
        len = len + 1;
    }
    if (len > 1) {
        for (int i = 0; i < len; ++i) {
            free(valueArray[i]);
            valueArray[i] = NULL;
        }
        free(valueArray);
        valueArray = NULL;
        return "n";
    }
    char *read = valueArray[0];
    if (read == NULL) {
        read = "n";
    }
    free(valueArray[0]);
    valueArray[0] = NULL;
    free(valueArray);
    valueArray = NULL;
    return read;
}

_Noreturn void whenWhile(bool foregroundClear, bool clearOnce,
                         bool stateCheck) {
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
                for (int i = 0; appArray[i] != NULL; i++) {
                    char *app = appArray[i];
                    isForeground = foregroundApp(app);
                    if (isForeground) break;
                }
                if (isForeground) {
                    if (clearOnce) {
                        if (!isCleared) {
                            runModuleSh("clear.sh");
                            isCleared = true;
                        }
                    } else {
                        runModuleSh("clear.sh");
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
            if (modulePath == NULL) {
                modulePath = getModePath();
            }
            snprintf(file, MAX_MEMORY, "%s/disable", modulePath);
            if (access(file, 0) == 0 && lastState) {
                char **pids = findPid("RegularlyClean");
                if (!killPid(pids)) {
                    logStr("[E] --停止模块定时进程失败");
                } else {
                    changeSed(modulePath);
                    lastState = false;
                }
            } else if (access(file, 0) != 0 && !lastState) {
                runModuleSh("state.sh");
                lastState = true;
            }
        }
        sleep(5);
    }
}

void cleanup() {
    if (appArray != NULL) {
        for (int i = 0; appArray[i] != NULL; i++) {
            free(appArray[i]);
            appArray[i] = NULL;
        }
        free(appArray);
        appArray = NULL;
    }
    if (modulePath != NULL) {
        free(modulePath);
        modulePath = NULL;
    }
    // printf("clear\n");
}

int main() {
    atexit(cleanup);
    modulePath = getModePath();
    isDebug = checkState("c_debug");
    bool autoClear = checkState("clear_mod");
    bool clearOnce = checkState("manager_control");
    bool stateCheck = checkState("clear_only_once");
    appArray = config("app_map");
    whenWhile(autoClear, clearOnce, stateCheck);
}
