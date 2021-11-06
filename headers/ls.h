#ifndef LS_H
#define LS_H

#include <dirent.h>
#include <sys/stat.h>

void ls(char **token_ptr, char **cwd_ptr, char *home_path);
void printFileProperties(struct stat *stats, struct dirent *dir);

#endif
