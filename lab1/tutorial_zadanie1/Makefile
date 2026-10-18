CC = gcc
CFLAGS = -Wall -fsanitize=address,undefined

all: l1-1

prog1: l1-1.c	
	$(CC) $(CFLAGS) -o l1-1 l1-1.c

.PHONY: clean all

clean:
	rm -f l1-1