#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <limits.h>
#include <errno.h>

#include "echo.h"

void echo(char **token_ptr) {

    // Print all tokens as they appear and just new line if entire command is 'echo'
    int i = 0, j = 0;
    int fd, flag = 0, original = 1;
    char file_name[PATH_MAX];
    file_name[0] = '?';
    file_name[1] = '\0';
    char *tokens[100]; 

    *token_ptr = strtok(NULL, " \t\n");

    while (*token_ptr != NULL && i < 100) {
        if (strcmp(*token_ptr, "<") == 0) {
            *token_ptr = strtok(NULL, " \t\n");
            if (*token_ptr == NULL) {
                printf("Error: Missing filename\n");
                goto final;
            }
            *token_ptr = strtok(NULL, " \t\n");
            continue;
        } else if (strcmp(*token_ptr, ">") == 0) {
            flag = 1;
            *token_ptr = strtok(NULL, " \t\n");
            if (*token_ptr == NULL) {
                printf("Error: Missing filename\n");
                goto final;
            }
            strcpy(file_name, *token_ptr);
            *token_ptr = strtok(NULL, " \t\n");
            continue;
        } else if (strcmp(*token_ptr, ">>") == 0) {
            flag = 2;
            *token_ptr = strtok(NULL, " \t\n");
            if (*token_ptr == NULL) {
                printf("Error: Missing filename\n");
                goto final;
            }
            strcpy(file_name, *token_ptr);
            *token_ptr = strtok(NULL, " \t\n");
            continue;
        } 
        tokens[i] = strdup(*token_ptr);
        *token_ptr = strtok(NULL, " \t\n");
        i++;
    }

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

    for (j = 0; j < i; j++) {
        printf("%s ", tokens[j]);
    }
    printf("\n");

    if (flag) {
        if (dup2(original, STDOUT_FILENO) < 0) {
            perror("Unable to duplicate file descriptor");
            close(fd);
            return;
        }
        close(fd);
        close(original);
    }

    final:
    for (j = 0; j < i; j++) {
        free(tokens[j]);
    }
}