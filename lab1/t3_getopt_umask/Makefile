CC = gcc
CFLAGS = -Wall -fsanitize=address,undefined
TARGET = t3_getopt_umask

all: $(TARGET)

$(TARGET): $(TARGET).c   
	$(CC) $(CFLAGS) -o $(TARGET) $(TARGET).c

.PHONY: clean all

clean:
	rm -f $(TARGET)