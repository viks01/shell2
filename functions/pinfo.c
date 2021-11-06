#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <limits.h>

#include "pinfo.h"

void pinfo(char **token_ptr, char **cwd_ptr) {

    pid_t pid;
    char pid_str[10];
    char *status;
    char *mem;
    char path[PATH_MAX], temp[PATH_MAX];

    int flag = 0;
    char file_name[PATH_MAX];
    
    // Obtain effective pid and executable path
    if (*token_ptr == NULL) {
        pid = getpid();
        strcpy(path, "~/a.out");
    } else if ((*token_ptr)[0] == '>') {
        pid = getpid();
        strcpy(path, "~/a.out");
        if (strlen(*token_ptr) == 1) {
            flag = 1;
        } else {
            flag = 2;
        }
        *token_ptr = strtok(NULL, " \t\n");
        if (*token_ptr == NULL) {
            printf("Error: Missing filename\n");
            return;
        }
        strcpy(file_name, *token_ptr);
    } else {
        if ((*token_ptr)[0] == '<') {
            *token_ptr = strtok(NULL, " \t\n");
            if (*token_ptr == NULL) {
                printf("Error: Missing input to pinfo command\n");
                return;
            }
        }
        if ((*token_ptr)[0] < '0' || (*token_ptr)[0] > '9') {
            printf("Invalid pid\n");
            return;
        }
        pid = atoi(*token_ptr);
        sprintf(temp, "/proc/%d/exe", pid);
        readlink(temp, path, PATH_MAX);

        *token_ptr = strtok(NULL, " \t\n");
        if (*token_ptr != NULL) {
            if (strcmp(*token_ptr, ">") == 0) {
                flag = 1;
            } else if (strcmp(*token_ptr, ">>") == 0) {
                flag = 2;
            }
            *token_ptr = strtok(NULL, " \t\n");
            if (*token_ptr == NULL) {
                printf("Error: Missing filename\n");
                return;
            }
            strcpy(file_name, *token_ptr);
        }
    }
    sprintf(pid_str, "%d", pid);
    
    // Memory and process status
    char process_file[100];
    char line[201];

    sprintf(process_file, "/proc/%d/statm", pid);
    FILE *fp = fopen(process_file, "r");
    fgets(line, 200, fp);
    mem = strtok(line, " ");
    fclose(fp);

    char line2[PATH_MAX];
    sprintf(process_file, "/proc/%d/stat", pid);
    fp = fopen(process_file, "r");
    fgets(line2, 200, fp);
    status = strtok(line2, " ");
    for (int i = 0; i < 2; i++) {
        status = strtok(NULL, " ");
    } 
    fclose(fp);

    // Check if process is running in foreground or background
    int fd = open("/dev/tty", O_RDONLY);
    int check = 0;
    if (getpgrp() == tcgetpgrp(fd)) {
        check = 1;
    }
    close(fd);

    int original;
    if (flag) {
        if (flag == 1) {
            fd = open(file_name, O_WRONLY | O_CREAT | O_TRUNC, 0644);
        } else {
            fd = open(file_name, O_WRONLY | O_APPEND | O_CREAT, 0644);
        }
        if (fd < 0) {
            perror("Failed to open file");
            return;
        }
        if ((original = dup(STDOUT_FILENO)) < 0) {
            perror("Unable to duplicate file descriptor");
            close(fd);
            return;
        }
        if (dup2(fd, STDOUT_FILENO) < 0) {
            perror("Unable to duplicate file descriptor");
            close(fd);
            return;
        }
    }

    // Print process information
    printf("pid -- %d\n", pid);
    printf("Process Status -- %s", status);
    if (check) {
        putchar('+');
    } 
    putchar('\n');    
    printf("memory -- %s kB {Virtual Memory}\n", mem);
    printf("Executable Path -- %s\n", path);

    if (flag) {
        if (dup2(original, STDOUT_FILENO) < 0) {
            perror("Unable to duplicate file descriptor");
            close(fd);
            return;
        }
        close(fd);
        close(original);
    }
}
