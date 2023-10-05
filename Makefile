CC = gcc
CFLAGS=-Wall -Wextra
TARGET=SimpleCache
TARGET2=L1Cache

all:
	$(CC) $(CFLAGS) SimpleProgram.c SimpleCache.c -o $(TARGET)
	$(CC) $(CFLAGS) L1Cache.c -o $(TARGET2)

clean:
	rm $(TARGET)
	rm $(TARGET2)