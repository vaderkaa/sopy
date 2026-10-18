CC = gcc
CFLAGS = -Wall -fsanitize=address,undefined

all: l1-2

prog1: l1-2.c	
	$(CC) $(CFLAGS) -o l1-2 l1-2.c

.PHONY: clean all

clean:
	rm -f l1-2