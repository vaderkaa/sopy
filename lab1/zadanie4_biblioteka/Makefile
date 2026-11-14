CC = gcc
CFLAGS = -Wall -fsanitize=address,undefined
TARGET = sop-bib

all: $(TARGET)

$(TARGET): $(TARGET).c   
	$(CC) $(CFLAGS) -o $(TARGET) $(TARGET).c

.PHONY: clean all

clean:
	rm -f $(TARGET)