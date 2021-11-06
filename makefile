# The default C compiler
CC = gcc

# The CFLAGS variable sets compile flags for gcc:
CFLAGS = -Wall 

a.out: echo.h echo.c pwd.h pwd.c cd.h cd.c ls.h ls.c pinfo.h pinfo.c repeat.h repeat.c syscommands.h syscommands.c input.h input.c jobs.h jobs.c sig.h sig.c fg.h fg.c bg.h bg.c main.c 
	$(CC) $(CFLAGS) echo.c pwd.c cd.c ls.c pinfo.c repeat.c syscommands.c input.c jobs.c sig.c fg.c bg.c main.c 

.PHONY: clean

clean: 
	rm -f a.out *.o