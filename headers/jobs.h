#ifndef JOBS_H
#define JOBS_H

typedef struct jobinfo JobInfo;
typedef JobInfo* Job; 

struct jobinfo {
    int num;
    int pid;
    char state;
    char progname[50];
    struct jobinfo* next;
    struct jobinfo* prev;
};

typedef struct Node {
    struct jobinfo* head;
} ListNode;

typedef ListNode* List;

int jobs(char **token_ptr, List L);

#endif