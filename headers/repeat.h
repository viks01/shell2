#ifndef REPEAT_H
#define REPEAT_H

#include "jobs.h"

int repeat(char **token_ptr, char **prompt_ptr, char **cwd_ptr, char **prwd_ptr, char *home_path, char *command, int *child_pid_ptr, int *bg_count_ptr, List L);

#endif
