#define _GNU_SOURCE
#define __USE_GNU 1
#define __USE_BSD 1

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <time.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <pwd.h>
#include <grp.h>
#include <unistd.h>
#include <limits.h>
#include <errno.h>

#include "ls.h"

void ls(char **token_ptr, char **cwd_ptr, char *home_path) {

    // List of target directories of ls command
    char targets[50][PATH_MAX];
    targets[0][0] = '.';
    targets[0][1] = '\0';

    // n - Number of directories to run ls on
    // a - '-a' flag check
    // l - '-l' flag check
    int n = 0, a = 0, l = 0;

    int flag = 0;
    
    while (*token_ptr != NULL && strcmp(*token_ptr, ";") != 0 && n < 50) {
        if (strcmp(*token_ptr, "~") == 0) {
            strcpy(targets[n], home_path);
            n++;
        } else if (strcmp(*token_ptr, "-a") == 0) {
            a = 1;
        } else if (strcmp(*token_ptr, "-l") == 0) {
            l = 1;
        } else if (strcmp(*token_ptr, "-al") == 0 || strcmp(*token_ptr, "-la") == 0) {
            a = 1;
            l = 1;
        } else if (strcmp(*token_ptr, ">") == 0) {
            flag = 1;
            break;
        } else if (strcmp(*token_ptr, ">>") == 0) {
            flag = 2;
            break;
        } else if (strcmp(*token_ptr, "<") == 0) {
            *token_ptr = strtok(NULL, " \t\n");
            if (*token_ptr == NULL) {
                printf("Error: Missing filename\n");
                return;
            }
            if (n == 0) {
                strcpy(targets[n], *token_ptr);
                n++;
            }
        } else {
            strcpy(targets[n], *token_ptr);
            n++;
        }
        *token_ptr = strtok(NULL, " \t\n");
    }

    // Check for redirection
    char file_name[PATH_MAX];
    int original = 1, fd;

    if (flag) {
        *token_ptr = strtok(NULL, " \t\n");
        if (*token_ptr == NULL) {
            printf("Error: Invalid syntax. Mising filename.\n");
            return;
        }
        strcpy(file_name, *token_ptr);

        if (flag == 1) {
            fd = open(file_name, O_WRONLY | O_CREAT | O_TRUNC, 0644);
        } else {
            fd = open(file_name, O_WRONLY | O_APPEND | O_CREAT, 0644);
        }
        if (fd < 0) {
            perror("Failed to open file");
            return;
        }
        if ((original = dup(STDOUT_FILENO)) < 0) {
            perror("Unable to duplicate file descriptor");
            close(fd);
            return;
        }
        if (dup2(fd, STDOUT_FILENO) < 0) {
            perror("Unable to duplicate file descriptor");
            close(fd);
            return;
        }
    }

    // Run ls at least once
    if (!n) {
        n++;
    }


    int i = 0;
    long int total = 0;
    char path[PATH_MAX];

    DIR *d;
    struct dirent *dir;
    struct stat statbuf, dirstatbuf, s;
    
    // Loop through list of target directories
    while (i < n) {
        // For '-l' option, get total block size
        if (l) {
            if (stat(targets[i], &dirstatbuf) == 0) {
                // Only for directories
                if (S_ISDIR(dirstatbuf.st_mode)) {
                    d = opendir(targets[i]);
                    if (d != NULL) {
                        while ((dir = readdir(d)) != NULL) {
                            if (a) {                                    // Consider hidden directories/files
                                strcpy(path, targets[i]);
                                strcat(path, "/");
                                strcat(path, dir->d_name);
                                if (stat(path, &s) == 0) {
                                    total += s.st_blocks;
                                } else {
                                    printf("Unable to get file properties.\n");
                                    printf("Please check whether '%s' file exists.\n", path);
                                }
                            } else if (dir->d_name[0] != '.') {         // Ignore hidden directories/files
                                strcpy(path, targets[i]);
                                strcat(path, "/");
                                strcat(path, dir->d_name);
                                if (stat(path, &s) == 0) {
                                    total += s.st_blocks;
                                } else {
                                    printf("Unable to get file properties.\n");
                                    printf("Please check whether '%s' file exists.\n", path);
                                }
                            }
                        }
                    } else {
                        perror("opendir()");
                        break;
                    }
                    closedir(d);
                    printf("total %ld\n", total / 2);
                }
            } else {
                printf("Unable to get file properties.\n");
                printf("Please check whether '%s' file exists.\n", path);
            }
        }

        // Parse the directory stream
        d = opendir(targets[i]);
        if (d != NULL) {
            // Title
            if (n > 1) {
                printf("%s:\n", targets[i]);
            }

            // Handle flags
            if (a && l) {
                while ((dir = readdir(d)) != NULL) {
                    strcpy(path, targets[i]);
                    strcat(path, "/");
                    strcat(path, dir->d_name);
                    if (stat(path, &statbuf) == 0) {
                        printFileProperties(&statbuf, dir);
                    } else {
                        printf("Unable to get file properties.\n");
                        printf("Please check whether '%s' file exists.\n", path);
                    }
                }
            } else if (a) {
                while ((dir = readdir(d)) != NULL) {
                    printf("%s\n", dir->d_name);
                }
            } else if (l) {
                while ((dir = readdir(d)) != NULL) {
                    if (dir->d_name[0] != '.') {
                        strcpy(path, targets[i]);
                        strcat(path, "/");
                        strcat(path, dir->d_name);
                        if (stat(path, &statbuf) == 0) {
                            printFileProperties(&statbuf, dir);
                        } else {
                            printf("Unable to get file properties.\n");
                            printf("Please check whether '%s' file exists.\n", path);
                        }
                    }
                }
            } else {
                while ((dir = readdir(d)) != NULL) {
                    if (dir->d_name[0] != '.') {
                        printf("%s\n", dir->d_name);
                    }
                }
            }
            closedir(d);
        } else {
            perror("opendir()");
            break;
        }

        // Whitespace
        if (n > 1 && i != n - 1) {
            printf("\n");
        }

        i++;
    }

    if (flag) {
        if (dup2(original, STDOUT_FILENO) < 0) {
            perror("Unable to duplicate file descriptor");
            close(fd);
            return;
        }
        close(fd);
        close(original);
    }
}

void printFileProperties(struct stat *stats, struct dirent *dir) {

    // Build the permissions string
    char perm[15];
    if (S_ISDIR(stats->st_mode)) {
        perm[0] = 'd';
    } else {
        perm[0] = '-';
    }
    perm[1] = '\0';

    if (stats->st_mode & S_IRUSR)
        strcat(perm, "r");
    else
        strcat(perm, "-");
    if (stats->st_mode & S_IWUSR)
        strcat(perm, "w");
    else
        strcat(perm, "-");
    if (stats->st_mode & S_IXUSR)
        strcat(perm, "x");
    else
        strcat(perm, "-");

    if (stats->st_mode & S_IRGRP)
        strcat(perm, "r");
    else
        strcat(perm, "-");
    if (stats->st_mode & S_IWGRP)
        strcat(perm, "w");
    else
        strcat(perm, "-");
    if (stats->st_mode & S_IXGRP)
        strcat(perm, "x");
    else
        strcat(perm, "-");

    if (stats->st_mode & S_IROTH)
        strcat(perm, "r");
    else
        strcat(perm, "-");
    if (stats->st_mode & S_IWOTH)
        strcat(perm, "w");
    else
        strcat(perm, "-");
    if (stats->st_mode & S_IXOTH)
        strcat(perm, "x");
    else
        strcat(perm, "-");


    // Time parameters
    char t[30], tim[30], y[7];
    int year;

    strcpy(t, ctime(&stats->st_mtime));
    char *loc;
    loc = strrchr(t, '\n');
    *loc = '\0';
    loc = strrchr(t, ' ');
    strcpy(y, loc);
    year = atoi(y+1);

    time_t s = time(NULL);
    struct tm* current_time = localtime(&s);

    if (current_time->tm_year + 1900 > year) {
        loc = strchr(t, ' ');
        loc++;
        strcpy(tim, loc);
        loc = strrchr(tim, ':');
        *loc = '\0';
        loc = strrchr(tim, ' ');
        loc++;
        *loc = '\0';
        strcat(tim, y);
    } else {
        loc = strrchr(t, ':');
        *loc = '\0';
        loc = strchr(t, ' ');
        loc++;
        strcpy(tim, loc);
    }

    // File properties string
    printf("%s %2ld %s %s %10ld %s %s\n", perm, stats->st_nlink, getpwuid(stats->st_uid)->pw_name, getgrgid(stats->st_gid)->gr_name, stats->st_size, tim, dir->d_name);
}
