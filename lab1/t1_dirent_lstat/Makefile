CC = gcc
CFLAGS = -Wall -fsanitize=address,undefined

all: t1_dirent_lstat

prog1: t1_dirent_lstat.c	
	$(CC) $(CFLAGS) -o t1_dirent_lstat t1_dirent_lstat.c

.PHONY: clean all

clean:
	rm -f t1_dirent_lstat