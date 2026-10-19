CC = gcc
CFLAGS = -Wall -fsanitize=address,undefined
TARGET = main

all: $(TARGET)

$(TARGET): $(TARGET).c   
	$(CC) $(CFLAGS) -o $(TARGET) $(TARGET).c

.PHONY: clean all

clean:
	rm -f $(TARGET)