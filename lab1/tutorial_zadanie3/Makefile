CC = gcc
CFLAGS = -Wall -fsanitize=address,undefined
TARGET = l1-3

all: $(TARGET)

$(TARGET): $(TARGET).c   
	$(CC) $(CFLAGS) -o $(TARGET) $(TARGET).c

.PHONY: clean all

clean:
	rm -f $(TARGET)