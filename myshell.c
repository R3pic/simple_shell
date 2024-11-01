#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>

#define BUF_SIZE 256

void readEnv(char *envpath, char *PATH[]);
void readUserInput(char *buf);
int wantExit(char *string);
int split(char *string, char *output[], char ch);
int findProgram(char *commandName, char *commandPath);
void execute(char *commandPath, char *commands[]);

char *PATH[2] = {NULL};

int main(int argc, char *argv[]) {
    char envpath[] = "./.myshell";
    // printf("envpath : %s\n", envpath);
    
    readEnv(envpath, PATH);

    char userinput[BUF_SIZE];
    memset(userinput, 0, sizeof(userinput));

    char *command[128];
    memset(command, 0, sizeof(command));

    char programPath[BUF_SIZE];
    while(1) {
        readUserInput(userinput); // stdin으로 부터 문자열 읽음
        if (wantExit(userinput))
            break;

        split(userinput, command, ' '); // 문자열을 공백 기준으로 잘라냄
        if(!strcmp(command[0], "cd")) {
            chdir(command[1]);
            continue;
        }

        if(findProgram(command[0], programPath) == -1) { // 파일 경로 추출
            printf("%s : not found.\n", command[0]);
            continue;
        }

        execute(programPath, command); // 실행
    }

    exit(0);
}

int wantExit(char *string) {
    return !strcmp(string, "quit") || !strcmp(string, "exit");
}

int split(char *string, char *output[], char ch) {
    int i = 0;
    char *start = string;
    for (char *p = string; *p != '\0'; p++) {
        if (*p == ch) {
            *p = '\0';
            output[i] = start;
            i++;
            start = p + 1;
        }
    }

    if (*start != '\0') {
        output[i] = start;
        i++;
    }

    output[i] = NULL;

    return i;
}

void readEnv(char *envpath, char *PATH[]) {
    int fd = open(envpath, O_RDONLY);
    if (fd == -1) {
        perror(envpath);
        exit(1);
    }
    char buf[BUF_SIZE];
    memset(buf, 0, sizeof(buf));
    int read_len;
    read_len = read(fd, buf, BUF_SIZE);
    for (int i = 0 ; i < read_len ; i++) {
        if (buf[i] == '\n'){ // 첫번째 줄의 끝을 만나면
            buf[i] = '\0';
            break;
        }
    }
    char *line = strndup(buf, read_len);

    // printf("env string : %s\n", line);

    split(strchr(line, '=') + 1, PATH, ':');

    // for (int i = 0 ; PATH[i] != NULL ; i++) {
    //     printf("PATH[%d] = %s\n", i, PATH[i]);
    // }
}

void readUserInput(char *buf) {
    memset(buf, 0, sizeof(buf));
    write(STDOUT_FILENO, "% ", 3);
    int _read_len = read(STDIN_FILENO, buf, 255);
    buf[_read_len - 1] = '\0'; // 개행문자를 널문자로 변경 (\n -> \0)
}

int findProgram(char *commandName, char *commandPath) {
    char buf[256];
    for (int i = 0 ; PATH[i] != NULL ; i++) {
        int len = strlen(commandName) + strlen(PATH[i]) + 2;
        snprintf(buf, len, "%s/%s", PATH[i], commandName);

        if (access(buf, F_OK) != 0) {
            continue;
        }

        strcpy(commandPath, buf);
        return 0;
    }

    return -1;
}

void execute(char *commandPath, char *commands[]) {
    pid_t pid;

    if ((pid = fork()) == 0) {
        execv(commandPath, commands);
        perror(commands[0]);
        exit(1);
    }

    int status;
    waitpid(pid, &status, 0);
    // printf("status : %d\n", status);
}