#ifndef SYSCMD_H
#define SYSCMD_H

#include "jobs.h"

Job CreateJob(int pid, int count);
int syscommands(char **token_ptr, char *prog_name, int *child_pid_ptr, int *bg_count_ptr, List L);

#endif
