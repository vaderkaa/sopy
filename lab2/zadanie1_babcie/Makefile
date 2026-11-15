CC = gcc
CFLAGS = -Wall -fsanitize=address,undefined
TARGET = sop-cake

all: $(TARGET)

$(TARGET): $(TARGET).c   
	$(CC) $(CFLAGS) -o $(TARGET) $(TARGET).c

.PHONY: clean all

clean:
	rm -f $(TARGET)