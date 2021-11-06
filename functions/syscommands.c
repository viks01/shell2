#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <limits.h>
#include <errno.h>

#include "syscommands.h"
#include "jobs.h"

int syscommands(char **token_ptr, char *prog_name, int *child_pid_ptr, int *bg_count_ptr, List L) {

    // Store tokens in an array of strings for execvp()
    char *tokens[20];
    char *temp = *token_ptr;
    int i = 0;
    tokens[i] = strdup(prog_name);                  // Uses heap memory for dynamic allocation
    i++;

    int flag = 0;
    char file_name[PATH_MAX];

    *token_ptr = strtok(NULL, " \t\n");
    // Check for I/O redirection
    while (*token_ptr != NULL) {
        if (strcmp(*token_ptr, ">") == 0) {
            flag = 1;
            *token_ptr = strtok(NULL, " \t\n");
            if (*token_ptr == NULL) {
                printf("Error: Missing filename\n");
                return -1;
            }
            strcpy(file_name, *token_ptr);
            *token_ptr = strtok(NULL, " \t\n");
            continue;
        } else if (strcmp(*token_ptr, ">>") == 0) {
            flag = 2;
            *token_ptr = strtok(NULL, " \t\n");
            if (*token_ptr == NULL) {
                printf("Error: Missing filename\n");
                return -1;
            }
            strcpy(file_name, *token_ptr);
            *token_ptr = strtok(NULL, " \t\n");
            continue;
        } else if (strcmp(*token_ptr, "<") == 0) {
            *token_ptr = strtok(NULL, " \t\n");
            continue;
        }
        tokens[i] = strdup(*token_ptr);
        *token_ptr = strtok(NULL, " \t\n");
        i++;
    }
    tokens[i] = NULL;                               // Last parameter needs to be NULL for execvp()
    *token_ptr = temp;

    // Differentiate between background and foreground process
    if (strcmp(tokens[i-1], "&") == 0) {            // Background process 
        free(tokens[i-1]);
        tokens[i-1] = NULL;

        int forkReturn = fork();
        if (forkReturn == 0) {                      // Child process
            pid_t container = fork();
            if (container == 0) {                   // Child process
                if (execvp(tokens[0], tokens) == -1) {
                    i = 0;
                    while (tokens[i] != NULL) {
                        free(tokens[i]);
                        i++;
                    }
                    printf("ERROR: %s\n", strerror(errno));
                    exit(1);
                }
            } else {                                // Parent process
                printf("%d\n", container);
                FILE *fp;
                fp = fopen("bpids.txt", "w");
                fprintf(fp, "%d\n", container);
                fclose(fp);
                
                int status;
                waitpid(-1, &status, 0);
                fprintf(stderr, "\n%s with pid %d exited ", temp, container);
                if (status == 0)
                    fputs("normally", stderr);
                else
                    fputs("abnormally", stderr);

                i = 0;
                while (tokens[i] != NULL) {
                    free(tokens[i]);
                    i++;
                }
                exit(0);
            }
        } else {                                    // Parent process
            sleep(1);

            // Get pid of background process from file 
            int cpid = -20;
            FILE *fp;
            fp = fopen("bpids.txt", "r");
            fscanf(fp, "%d", &cpid);
            fclose(fp);
            if (cpid == -20) {
                printf("Child pid error\n");
            }

            // Create job structure and insert into linked list 
            Job task = CreateJob(cpid, *bg_count_ptr);
            Job curr = L->head;
            int p, y;
            int x = strlen(task->progname);
            if (*bg_count_ptr == 0 || curr == NULL) {
                L->head = task;
            } else {
                curr = L->head;
                if (curr->next == NULL) {
                    p = 0;
                    y = strlen(curr->progname);
                    while (p < x && p < y && task->progname[p] == curr->progname[p]) {
                        p++;
                    }
                    if (task->progname[0] < curr->progname[0]) {
                        task->next = curr;
                        curr->prev = task;
                        L->head = task;
                    } else {
                        curr->next = task;
                        task->prev = curr;
                    }
                } else {
                    while (curr->next != NULL) {
                        p = 0;
                        y = strlen(curr->progname);
                        while (p < x && p < y && task->progname[p] == curr->progname[p]) {
                            p++;
                        }
                        if (task->progname[p] < curr->progname[p]) {
                            break;
                        }
                        curr = curr->next;
                    }
                    if (curr->next == NULL) {
                        curr->next = task;
                        task->prev = curr;
                    } else {
                        task->next = curr;
                        curr->prev->next = task;
                        task->prev = curr->prev;
                        curr->prev = task;
                    }
                }
            }
            (*bg_count_ptr)++;

            // Fix memory leaks
            i = 0;
            while (tokens[i] != NULL) {
                free(tokens[i]);
                i++;
            }
        }
    } else {                                        // Foreground process
        int fd = -1, original = 1;
        if (flag && (original = dup(STDOUT_FILENO)) < 0) {
            perror("Unable to duplicate STDOUT original file descriptor");
            return -1;
        }

        int forkReturn = fork();
        if (forkReturn == 0) {                      // Child process
            if (flag) {
                if (flag == 1) {
                    fd = open(file_name, O_WRONLY | O_CREAT | O_TRUNC, 0644);
                } else {
                    fd = open(file_name, O_WRONLY | O_APPEND | O_CREAT, 0644);
                }
                if (fd < 0) {
                    perror("Failed to open file");
                    return -1;
                }
            
                if (dup2(fd, STDOUT_FILENO) < 0) {
                    perror("Unable to duplicate file descriptor");
                    close(fd);
                    return -1;
                }
            }
            
            if (execvp(tokens[0], tokens) == -1) {
                printf("ERROR: %s\n", strerror(errno));
                i = 0;
                while (tokens[i] != NULL) {
                    free(tokens[i]);
                    i++;
                }
                exit(1);
            }
        } else {                                    // Parent process
            // Signal handling
            *child_pid_ptr = forkReturn;
            int child_status;
            waitpid(forkReturn, &child_status, WUNTRACED);
            if (flag) {
                if (dup2(original, STDOUT_FILENO) < 0) {
                    perror("Unable to duplicate STDOUT original file descriptor");
                    return -1;
                }
                close(original);
            }
            i = 0;
            while (tokens[i] != NULL) {
                free(tokens[i]);
                i++;
            }
        }
    }

    return 0;
}

Job CreateJob(int pid, int count) {
    Job task = (Job)malloc(sizeof(JobInfo));
    task->pid = pid;
    task->num = count + 1;
    task->next = NULL;
    task->prev = NULL;

    char filename[50];
    char progname[50];
    int proc_pid;
    char st;
    sprintf(filename, "/proc/%d/stat", pid);
    FILE *fp = fopen(filename, "r");
    fscanf(fp, "%d (%s %c", &proc_pid, progname, &st);
    fclose(fp);
    int l = strlen(progname);
    progname[l-1] = '\0';
    strcpy(task->progname, progname);

    return task;
}
