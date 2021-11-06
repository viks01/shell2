#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>
#include <errno.h>

#include "cd.h"

int cd(char **token_ptr, char **prompt_ptr, char **cwd_ptr, char **prwd_ptr, char *home_path) {

    char new_path[PATH_MAX];
    
    // cd or cd ~
    if (*token_ptr == NULL || strcmp(*token_ptr, "~") == 0) {
        if (strcmp(*cwd_ptr, home_path) != 0) {
            if (chdir(home_path) != 0) {
                perror("chdir() to home directory failed");
                return -1;
            }
        }
        // Update prev. working dir before cwd
        strcpy(*prwd_ptr, *cwd_ptr);
        change_prompt(prompt_ptr, "~");
        if (getcwd(*cwd_ptr, PATH_MAX) == NULL) {
            perror("getcwd() error");
            return -1;
        }
    } else if (strcmp(*token_ptr, ".") == 0) {              // cd .
        strcpy(*prwd_ptr, *cwd_ptr);  
        return 0;
    } else {
        // Build new path string
        if (strcmp(*token_ptr, "-") == 0) {                 // cd -
            printf("%s\n", *prwd_ptr);
            strcpy(new_path, *prwd_ptr);
        } else {                                            // cd <directory>
            strcpy(new_path, *token_ptr);
        }

        // Update prev. working dir before cwd
        strcpy(*prwd_ptr, *cwd_ptr);
        if (chdir(new_path) != 0) {
            perror("chdir() failed");
            return -1;
        }
        if (getcwd(*cwd_ptr, PATH_MAX) == NULL) {
            perror("getcwd() error");
            return -1;
        }

        // Check if prompt needs to be modified to include tilde
        char *loc;
        if ((loc = strstr(*cwd_ptr, home_path))) {
            char temp[PATH_MAX];
            temp[0] = '~';
            temp[1] = '\0';
            int l = strlen(home_path);
            for (int i = 0; i < l; i++) {
                loc++;
            }
            strcat(temp, loc);
            change_prompt(prompt_ptr, temp);
        } else {
            change_prompt(prompt_ptr, *cwd_ptr);
        }
    }

    // Only 1 argument to cd command
    *token_ptr = strtok(NULL, " \t\n");
    if (*token_ptr != NULL && strcmp(*token_ptr, ";") != 0) {
        printf("cd: Too many arguments passed\n");
        return 1;
    }
    return 0;
}

void change_prompt(char **prompt_ptr, char *new_cwd) {

    // Modify the prompt string
    char *loc = strrchr(*prompt_ptr, ':');
    loc++;
    *loc = '\0';
    strcat(*prompt_ptr, new_cwd);
    strcat(*prompt_ptr, ">");
}
