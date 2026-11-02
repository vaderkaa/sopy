CC = gcc
CFLAGS = -Wall -fsanitize=address,undefined

all: t2_getcwd_chdir

prog1: t2_getcwd_chdir.c	
	$(CC) $(CFLAGS) -o t2_getcwd_chdir t2_getcwd_chdir.c

.PHONY: clean all

clean:
	rm -f t2_getcwd_chdir