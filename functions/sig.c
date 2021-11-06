#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <signal.h>
#include <limits.h>
#include <errno.h>

#include "sig.h"
#include "jobs.h"

int sig(char **token_ptr, List L) {
    if (*token_ptr == NULL) {
        printf("Usage: sig <job_number> <signal_number>\n");
        return 1;
    }
    int jobno = atoi(*token_ptr);
    *token_ptr = strtok(NULL, " \t\n");
    if (*token_ptr == NULL) {
        printf("Usage: sig <job_number> <signal_number>\n");
        return 1;
    }
    int signo = atoi(*token_ptr);

    // Check if background job with given number exists
    int flag = 0;
    Job curr = L->head;
    while (curr != NULL) {
        if (curr->num == jobno) {
            flag = 1;
            break;
        }
        curr = curr->next;
    }
    if (!flag) {
        printf("Error: No job with the given number exists\n");
        return 1;
    }

    if (kill(curr->pid, signo) == -1) {
        perror("Kill failed");
        exit(1);
    }

    return 0;
}
