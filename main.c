#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <limits.h>
#include <errno.h>

#include "input.h"
#include "cd.h"
#include "echo.h"
#include "pwd.h"
#include "ls.h"
#include "pinfo.h"
#include "syscommands.h"
#include "repeat.h"
#include "jobs.h"
#include "sig.h"
#include "fg.h"
#include "bg.h"

int child_pid = -20;
int bg_count;

void sigtstp_handler(int signum);
void sigint_handler(int signum);

void DeleteList(List L);

// Moves any forground job to background => ctrl + Z 
void sigtstp_handler(int signum) {
    kill(child_pid, SIGTSTP);
}

// Interrupts any running foreground processes => ctrl + C
void sigint_handler(int signum) {
    if (child_pid != -20) {
        kill(child_pid, SIGINT);
        child_pid = -20;
    } 
}

int main(int argc, char *argv[]) {

    // Prompt display requirement
    char user[40], hostname[HOST_NAME_MAX + 1];
    char home_path[PATH_MAX];

    char *prompt = (char*) malloc(PATH_MAX + HOST_NAME_MAX + 45);
    prompt[0] = '<';
    prompt[1] = '\0';

    char *cwd = (char*) malloc(PATH_MAX);
    if (cwd == NULL) {
        printf("Memory full\n");
        return 1;
    }
    if (getcwd(cwd, PATH_MAX) == NULL) {
        perror("getcwd() error");
        return 1;
    } else {
        strcpy(home_path, cwd);
    }

    char *prwd = (char*) malloc(PATH_MAX);
    if (prwd == NULL) {
        printf("Memory full\n");
        return 1;
    }
    strcpy(prwd, home_path);

    getlogin_r(user, 40);
    strcat(prompt, user);
    strcat(prompt, "@");
    gethostname(hostname, HOST_NAME_MAX + 1);
    strcat(prompt, hostname);
    strcat(prompt, ":~>");

    // Initialize background process count
    bg_count = 0;

    // Allocate memory to the list of background processes
    List L = (List)malloc(sizeof(ListNode));
    if (L == NULL) {
        printf("Memory full\n");
        exit(1);
    }
    L->head = NULL;

    // User input
    char input[101];   
    char *token;
    char *command;
    char command_copy[1000], cmd_cpy[1000];
    char *copy;
    char output_file[PATH_MAX];
    int output_red1 = 0, output_red2 = 0;
    int i = 0, j = 0, k = 0;
    int input_status, original = 1, stdin_original = 0;
    int fildes[2];

    // Signal handling
    signal(SIGINT, sigint_handler);
    signal(SIGTSTP, sigtstp_handler);
    
    while (1) {
        printf("\033[1;33m%s \033[0m", prompt);

        // Check for ctrl + D in input
        input_status = read_line(input, 100);
        if (input_status == RL_EOF) {
            break;
        } else if (input_status == RL_LIMIT) {
            printf("Error: Input size too large\n");
            break;
        } 

        copy = input;

        // Tokenize for multiple commands
        command = strtok_r(copy, ";\n", &copy);

        while (command != NULL) {
            strcpy(command_copy, command);
            strcpy(cmd_cpy, command_copy);

            // Check for piping
            token = strtok(command_copy, " \t\n");
            if (strcmp(token, "|") == 0) {
                printf("Error: Invalid syntax. Missing command\n");
                break;
            }
            
            j = 0;
            while (token != NULL) {
                if (strcmp(token, "|") == 0) {
                    j++;
                }
                token = strtok(NULL, " \t\n");
            }

            // Piping
            if (j > 0) {
                char pipcommands[10][100];
                char *pipcmds[10][20];
                i = 0;
                
                token = strtok(cmd_cpy, "|");
                
                // Store list of commands separated by |
                while (token != NULL) {
                    strcpy(pipcommands[i], token);
                    i++;
                    token = strtok(NULL, "|");
                }

                // Store list of tokens of each command and check for I/O redirection
                for (j = 0; j < i; j++) {
                    token = strtok(pipcommands[j], " \t\n");
                    k = 0;
                    while (token != NULL) {
                        if (strcmp(token, "<") == 0) {
                            token = strtok(NULL, " \t\n");
                            if (token == NULL) {
                                printf("Invalid syntax: Missing filename\n");
                                break;
                            }
                        } else if (strcmp(token, ">") == 0) {
                            output_red1 = 1;
                            token = strtok(NULL, " \t\n");
                            if (token == NULL) {
                                printf("Invalid syntax: Missing filename\n");
                                break;
                            }
                            strcpy(output_file, token);
                            token = strtok(NULL, " \t\n");
                            continue;
                        } else if (strcmp(token, ">>") == 0) {
                            output_red2 = 1;
                            token = strtok(NULL, " \t\n");
                            if (token == NULL) {
                                printf("Invalid syntax: Missing filename\n");
                                break;
                            }
                            strcpy(output_file, token);
                            token = strtok(NULL, " \t\n");
                            continue;
                        }
                        pipcmds[j][k] = strdup(token);
                        k++;
                        token = strtok(NULL, " \t\n");
                    }
                    pipcmds[j][k] = NULL;
                }

                // Maintain copy of std file descriptors
                if ((original = dup(STDOUT_FILENO)) < 0) {
                    perror("Unable to duplicate STDOUT file descriptor");
                    exit(1);
                }

                if ((stdin_original = dup(STDIN_FILENO)) < 0) {
                    perror("Unable to duplicate STDIN file descriptor");
                    exit(1);
                }

                for (j = 0; j < i; j++) {
                    if (pipe(fildes) < 0) {
                        perror("Could not create pipe");
                        exit(1);
                    }
                    int pid = fork();
                    if (pid < 0) {
                        perror("Child could not be created");
                        exit(1);
                    } else if (pid == 0) {                                 // Child
                        // Write to fildes[1]  
                        close(fildes[0]);
                        if (j == i-1) {
                            if (output_red1 || output_red2) {
                                int fd;
                                if (output_red1) {
                                    fd = open(output_file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
                                } else if (output_red2) {
                                    fd = open(output_file, O_WRONLY | O_APPEND | O_CREAT, 0644);
                                } 
                                if (fd < 0) {
                                    perror("Failed to open file");
                                    exit(1);
                                }

                                if (dup2(fd, STDOUT_FILENO) < 0) {
                                    perror("Unable to duplicate file descriptor");
                                    close(fd);
                                    exit(1);
                                }
                                close(fd);
                            } else {
                                if (dup2(original, STDOUT_FILENO) < 0) {
                                    perror("Unable to duplicate STDOUT original file descriptor");
                                    exit(1);
                                }
                                close(original);
                            }
                        } else {
                            if (dup2(fildes[1], STDOUT_FILENO) < 0) {
                                perror("Unable to duplicate descriptor");
                                exit(1);
                            }
                        }
                        close(fildes[1]);

                        // Run command
                        if (execvp(pipcmds[j][0], pipcmds[j]) == -1) {
                            printf("ERROR: %s\n", strerror(errno));
                            exit(1);
                        }
                    } else {                                                // Parent
                        wait(NULL);
                        close(fildes[1]);

                        // Next command reads from fildes[0]
                        if (dup2(fildes[0], STDIN_FILENO) < 0) {
                            perror("Unable to duplicate file descriptor");
                            exit(1);
                        }
                        close(fildes[0]);
                    }
                } 

                // Set STDIN and STDOUT to their original state
                if (dup2(stdin_original, STDIN_FILENO) < 0) {
                    perror("Unable to duplicate STDIN original descriptor");
                    exit(1);
                }
                close(stdin_original);

                if (dup2(original, STDOUT_FILENO) < 0) {
                    perror("Unable to duplicate STDOUT original descriptor");
                    exit(1);
                }
                close(original);
                
                // Free duplicated string tokens
                for (j = 0; j < i; j++) {
                    k = 0;
                    while (pipcmds[j][k] != NULL) {
                        free(pipcmds[j][k]);
                        k++;
                    }
                }
            } else {                                            // No piping
                // Tokenize to remove whitespace
                token = strtok(command, " \t\n");

                // Compare first word of user input and call appropriate function
                if (strcmp(token, "pwd") == 0) {
                    token = strtok(NULL, " \t\n");
                    pwd(&token, &cwd);
                } else if (strcmp(token, "echo") == 0) {
                    echo(&token);
                } else if (strcmp(token, "cd") == 0) {
                    token = strtok(NULL, " \t\n");
                    cd(&token, &prompt, &cwd, &prwd, home_path);
                } else if (strcmp(token, "ls") == 0) {
                    token = strtok(NULL, " \t\n");
                    ls(&token, &cwd, home_path);
                } else if (strcmp(token, "pinfo") == 0) {
                    token = strtok(NULL, " \t\n");
                    pinfo(&token, &cwd);
                } else if (strcmp(token, "repeat") == 0) {
                    token = strtok(NULL, " \t\n");
                    repeat(&token, &prompt, &cwd, &prwd, home_path, input, &child_pid, &bg_count, L);
                } else if (strcmp(token, "jobs") == 0) {
                    token = strtok(NULL, " \t\n");
                    jobs(&token, L);
                } else if (strcmp(token, "sig") == 0) {
                    token = strtok(NULL, " \t\n");
                    sig(&token, L);
                } else if (strcmp(token, "fg") == 0) {
                    token = strtok(NULL, " \t\n");
                    fg(&token, L);
                } else if (strcmp(token, "bg") == 0) {
                    token = strtok(NULL, " \t\n");
                    bg(&token, L);
                } else if (strcmp(token, "exit") == 0) {
                    goto end;
                } else {
                    syscommands(&token, token, &child_pid, &bg_count, L);
                }
            }
            
            command = strtok_r(copy, ";\n", &copy);
        }
    }

    // Fix memory leaks
end:
    free(prwd);
    free(cwd);
    free(prompt);
    DeleteList(L);
    free(L);
    
    return EXIT_SUCCESS;
}

void DeleteList(List L) {
    Job cur = L->head;
    if (cur == NULL) {
        return;
    } 
    Job prev;
    while (cur->next != NULL) {
        cur = cur->next;
    }
    prev = cur->prev;
    while (prev != NULL) {
        free(cur);
        cur = prev;
        prev = prev->prev;
    }
    free(cur);
}
