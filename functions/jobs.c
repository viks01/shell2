#include <stdio.h>
#include <string.h>
#include <limits.h>
#include <errno.h>

#include "jobs.h"

int jobs(char **token_ptr, List L) {
    int r = 0, s = 0;
    char status[15];
    char file_name[100];
    FILE *fp;

    // Handle flags
    while (*token_ptr != NULL) {
        if (strcmp(*token_ptr, "-r") == 0) {
            r = 1;
        }
        if (strcmp(*token_ptr, "-s") == 0) {
            s = 1;
        }
        *token_ptr = strtok(NULL, " \t\n");
    }

    Job curr = L->head;
    while (curr != NULL) {
        sprintf(file_name, "/proc/%d/stat", curr->pid);
        fp = fopen(file_name, "r");
        if (fp == NULL) {
            sprintf(status, "Stopped");
        } else {
            sprintf(status, "Running");
            fclose(fp);
        }

        if (s && !r) {
            if (strcmp(status, "Stopped") == 0) {
                printf("[%d] %s %s [%d]\n", curr->num, status, curr->progname, curr->pid);
            }
        } else if (r && !s) {
            if (strcmp(status, "Running") == 0) {
                printf("[%d] %s %s [%d]\n", curr->num, status, curr->progname, curr->pid);
            }
        } else {
            printf("[%d] %s %s [%d]\n", curr->num, status, curr->progname, curr->pid);
        }
        curr = curr->next;
    }

    return 0;
}