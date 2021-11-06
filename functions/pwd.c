#define _GNU_SOURCE

#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <limits.h>
#include <errno.h>

#include "pwd.h"

void pwd(char **token_ptr, char **cwd_ptr) {

    // Set cwd to the present working directory
    if (getcwd(*cwd_ptr, PATH_MAX) == NULL) {
        perror("getcwd() error");
        return;
    }

    int flag = 0;
    while (*token_ptr != NULL) {
        if (strcmp(*token_ptr, ">") == 0) {
            flag = 1;
            break;
        }
        if (strcmp(*token_ptr, ">>") == 0) {
            flag = 2;
            break;
        }
        *token_ptr = strtok(NULL, " \t\n");
    }
    if (flag) {
        *token_ptr = strtok(NULL, " \t\n");
        if (*token_ptr == NULL) {
            printf("Syntax error: No filename provided.\n");
            return;
        }
    
        int fd;
        if (flag == 1) {
            fd = open(*token_ptr, O_WRONLY | O_CREAT | O_TRUNC, 0644);
        } else {
            fd = open(*token_ptr, O_WRONLY | O_APPEND | O_CREAT, 0644);
        }
        if (fd < 0) {
            perror("Failed to open file");
            return;
        }

        write(fd, *cwd_ptr, strlen(*cwd_ptr));
        write(fd, "\n", 1);
        close(fd);
    } else {
        printf("%s\n", *cwd_ptr);
    }
}
