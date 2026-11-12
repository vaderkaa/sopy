# Kod startowy

```C
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#define ERR(source) (perror(source), fprintf(stderr, "%s:%d\n", __FILE__, __LINE__), exit(EXIT_FAILURE))

void usage(char *pname) 
{
    fprintf(stderr, "USAGE: %s\n", pname);
    exit(EXIT_FAILURE);
}

int main(int argc, char **argv) 
{
    // Tutaj kod programu
    
    return EXIT_SUCCESS;
}
```

# Makefile

```Makefile
CC = gcc
CFLAGS = -Wall -fsanitize=address,undefined
TARGET = zadanie

all: $(TARGET)

$(TARGET): $(TARGET).c   
	$(CC) $(CFLAGS) -o $(TARGET) $(TARGET).c

.PHONY: clean all

clean:
	rm -f $(TARGET)
```
