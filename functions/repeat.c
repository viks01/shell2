#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <limits.h>

#include "cd.h"
#include "echo.h"
#include "pwd.h"
#include "ls.h"
#include "pinfo.h"
#include "repeat.h"
#include "syscommands.h"
#include "jobs.h"

int repeat(char **token_ptr, char **prompt_ptr, char **cwd_ptr, char **prwd_ptr, char *home_path, char *command, int *child_pid_ptr, int *bg_count_ptr, List L) {
    
    // Check if user input follows correct format
    if (*token_ptr == NULL) {
        printf("repeat: No number or command given\n");
        printf("Format: repeat <num> <command>\n");
        return -1;
    }
    int n = atoi(*token_ptr);
    if (!n) {
        return -1;
    }

    *token_ptr = strtok(NULL, " \t\n");
    if (*token_ptr == NULL) {
        printf("repeat: No command given\n");
        printf("Format: repeat <num> <command>\n");
        return -1;
    }
    char prog_name[100];
    strcpy(prog_name, *token_ptr);

    // Build command to be repeated, to make parsing easier
    char *loc = strchr(command, 't');
    loc++;
    loc++;
    while (*loc == ' ' || *loc == '\t') {
        loc++;
    }
    loc++;
    loc++;
    while (*loc == ' ' || *loc == '\t') {
        loc++;
    }
    char new_command[200], new_cmd_copy[200];
    strcpy(new_command, loc);
    strcat(new_command, " ");
    loc = strchr(loc, '\0');
    loc++;
    strcat(new_command, loc);
    strcpy(new_cmd_copy, new_command);

    // Parse command multiple times and execute corresponding function
    char *new_token;
    while (n--) {
        new_token = strtok(new_command, " \t\n");

        if (strcmp(new_token, "pwd") == 0) {
            pwd(&new_token, cwd_ptr);
            new_token = strtok(NULL, " \t\n");
        } else if (strcmp(new_token, "echo") == 0) {
            echo(&new_token);
        } else if (strcmp(new_token, "cd") == 0) {
            new_token = strtok(NULL, " \t\n");
            cd(&new_token, prompt_ptr, cwd_ptr, prwd_ptr, home_path);
        } else if (strcmp(new_token, "ls") == 0) {
            new_token = strtok(NULL, " \t\n");
            ls(&new_token, cwd_ptr, home_path);
            if (n > 0) {
                printf("\n");
            }
        } else if (strcmp(new_token, "pinfo") == 0) {
            new_token = strtok(NULL, " \t\n");
            pinfo(&new_token, cwd_ptr);
        } else if (strcmp(new_token, "repeat") == 0) {
            new_token = strtok(NULL, " \t\n");
            repeat(&new_token, prompt_ptr, cwd_ptr, prwd_ptr, home_path, new_command, child_pid_ptr, bg_count_ptr, L);
        } else {
            syscommands(&new_token, prog_name, child_pid_ptr, bg_count_ptr, L);
        }
        
        strcpy(new_command, new_cmd_copy);
    }

    return 0;
}
