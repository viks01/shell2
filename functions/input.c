#include <stdio.h>

#include "input.h"

int read_line(char *buffer, int len) {
    int j = 0;
    char ch;
    while (1) {
        ch = getchar();
        
        if (ch == EOF) {
            buffer[j] = 0;
            if (j == 0) {
                return RL_EOF;
            } else {
                return RL_NEWLINE;
            } 
        } else if (ch == '\n') {
            buffer[j] = 0;
            return RL_NEWLINE;
        }

        if (j == len) {
            return RL_LIMIT;
        } 

        buffer[j++] = ch;
    }
}
